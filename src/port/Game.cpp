#include <libultraship.h>
#include <cstring>

#include <fast/interpreter.h>
#include "Engine.h"
#include "ShipUtils.h"

// [port] timeBeginPeriod(1) improves SDL_Delay precision for no-vsync path.
// With vsync, sleep is unused since vsync paces the frame budget.
#ifdef _WIN32
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#endif

extern "C" {
#include "enums.h"
#include "core1/core1.h"
#include "core1/main.h"
}

extern "C" void Graphics_PushFrame(Gfx* data) {
    GameEngine::ProcessGfxCommands(data);
}

// [port] Emulate N64's osViBlack on PC. Two separate concerns:
//
// 1) Display blanking (s_viBlack): On N64, osViBlack(1) blanked the TV output
//    but the RDP still rendered to framebuffers. On PC, we let the GPU render
//    normally (so readback captures the world), then clear the backbuffer to
//    black before presenting. Controlled by port_setViBlack().
//
// 2) Readback freeze (s_freezeReadback): During FADE_IN, the game disables
//    scene drawing one frame before capturing gFramebuffers. Without freezing,
//    the readback would overwrite gFramebuffers with the blank frame. Freezing
//    preserves the world data for the transition capture. Controlled by
//    port_freezeReadback().
static bool s_viBlack = false;
static bool s_freezeReadback = false;
static int s_readbackRequestFrames = 0; // on-demand readback: counts down each frame

extern "C" void port_setViBlack(int active) {
    s_viBlack = (active != 0);
}

extern "C" int port_isViBlack(void) {
    return s_viBlack ? 1 : 0;
}

extern "C" void port_freezeReadback(int freeze) {
    s_freezeReadback = (freeze != 0);
}

// [port] On-demand readback — GPU→CPU costs ~25ms on DX11, so only run when requested.
// Counter persists for 2 frames to handle pipeline delay.
extern "C" void port_requestReadback(void) {
    s_readbackRequestFrames = 2;
}

// [port] GPU readback state — shared between regular readback and high-res tile sampling.
// The regular readback downsamples to 292x216 for gFramebuffers (used by most decomp code).
// The full-res buffer is kept so transition tile capture can sample at internal resolution.
static uint16_t* s_gpuReadbackBuffer = nullptr;
static uint32_t s_gpuReadbackSize = 0;
static uint32_t s_gpuReadbackW = 0;
static uint32_t s_gpuReadbackH = 0;
static bool s_gpuReadbackFlipY = false;

// [port] GPU→CPU framebuffer readback. On N64, CPU/RDP shared physical memory so
// gFramebuffers always had valid pixel data. On PC, we read the backbuffer after
// Run() but before EndFrame() (buffer swap) — see the expanded pipeline in Engine.cpp.
//
// This function is called from Engine.cpp's RunCommands every frame with the
// backbuffer still intact. It reads at internal resolution into s_gpuReadbackBuffer,
// then downsamples to N64 native resolution (292x216) into both gFramebuffers.
// OpenGL returns bottom-up rows; DX11 returns top-down.
void Framebuffer_ReadbackGPU_FromBackbuffer(Fast::Interpreter* interpreter) {
    if (s_freezeReadback) {
        return; // [port] preserve gFramebuffers for transition capture
    }
    if (s_readbackRequestFrames <= 0) {
        return; // [port] on-demand: skip unless a consumer needs data
    }
    s_readbackRequestFrames--;
    if (!interpreter || !interpreter->mRapi) {
        return;
    }

    uint32_t dstW = (uint32_t)gFramebufferWidth;
    uint32_t dstH = (uint32_t)gFramebufferHeight;
    if (dstW == 0 || dstH == 0) {
        return;
    }

    // When mRendersToFb is true (GUI has menus/letterboxing), game renders to mGameFb
    // and FB 0 is cleared at the end of Run(). Read from whichever has the frame.
    int fbId = interpreter->mRendersToFb ? interpreter->mGameFb : 0;

    uint32_t gpuW = 0, gpuH = 0;
    interpreter->GetCurDimensions(&gpuW, &gpuH);
    if (gpuW == 0 || gpuH == 0) {
        return;
    }

    uint32_t neededSize = gpuW * gpuH;
    if (s_gpuReadbackSize < neededSize) {
        free(s_gpuReadbackBuffer);
        s_gpuReadbackBuffer = (uint16_t*)malloc(neededSize * sizeof(uint16_t));
        s_gpuReadbackSize = neededSize;
    }

    interpreter->mRapi->ReadFramebufferToCPU(fbId, gpuW, gpuH, s_gpuReadbackBuffer);
    s_gpuReadbackW = gpuW;
    s_gpuReadbackH = gpuH;

    // but when rendering to an FBO with invertY=true (mGameFb), the interpreter flips
    // rendering so game-top is at low Y — glReadPixels row 0 is already game-top.
    const char* apiName = interpreter->mRapi->GetName();
    bool isOpenGL = (apiName && strstr(apiName, "OpenGL") != nullptr);
    s_gpuReadbackFlipY = isOpenGL && !interpreter->mRendersToFb;

    // Downsample to N64 resolution for gFramebuffers (used by most decomp code).
    // gpuW x gpuH IS the game viewport (from GetCurDimensions) — just downsample directly.
    // The tile draw on the display side stretches to fill the viewport.
    for (int buf = 0; buf < 2; buf++) {
        uint16_t* dst = gFramebuffers[buf];
        for (uint32_t y = 0; y < dstH; y++) {
            uint32_t srcY = s_gpuReadbackFlipY ? (dstH - 1 - y) * gpuH / dstH : y * gpuH / dstH;
            for (uint32_t x = 0; x < dstW; x++) {
                uint32_t srcX = x * gpuW / dstW;
                uint16_t px = s_gpuReadbackBuffer[srcY * gpuW + srcX];
                // Byte-swap to big-endian (N64 convention).
                dst[y * dstW + x] = (px >> 8) | (px << 8);
            }
        }
    }
}

// [port] Sample from full-resolution GPU readback for transition tile capture.
// Maps N64 framebuffer coordinates (292x216 space) to GPU internal resolution,
// so transition tiles capture detail at the actual render resolution instead of
// being limited to the downsampled 292x216 gFramebuffers.
extern "C" uint16_t port_sampleHiresReadback(int fbX, int fbY) {
    if (!s_gpuReadbackBuffer || s_gpuReadbackW == 0 || s_gpuReadbackH == 0) {
        return 0;
    }

    int gpuX = fbX * (int)s_gpuReadbackW / gFramebufferWidth;
    int gpuY = fbY * (int)s_gpuReadbackH / gFramebufferHeight;

    if (gpuX < 0) {
        gpuX = 0;
    }
    if (gpuY < 0) {
        gpuY = 0;
    }
    if (gpuX >= (int)s_gpuReadbackW) {
        gpuX = (int)s_gpuReadbackW - 1;
    }
    if (gpuY >= (int)s_gpuReadbackH) {
        gpuY = (int)s_gpuReadbackH - 1;
    }

    if (s_gpuReadbackFlipY) {
        gpuY = (int)s_gpuReadbackH - 1 - gpuY;
    }

    uint16_t px = s_gpuReadbackBuffer[gpuY * s_gpuReadbackW + gpuX];
    return (px >> 8) | (px << 8); // byte-swap to BE
}

// [port] no-op — readback is done in Framebuffer_ReadbackGPU_FromBackbuffer
extern "C" void Framebuffer_ReadbackGPU(int bufferIndex) {
    (void)bufferIndex;
}

// [port] BK game logic runs at 30fps (N64: 2 VI per game frame at 60Hz).
// The main loop must tick at exactly 30fps regardless of render rate.
// Without this, gGlobalTimer increments too fast, time_getDelta() returns
// tiny values, and all tick-based timing (animations, cutscenes, AI) breaks.
static constexpr double GAME_LOGIC_FPS = 30.0;
static constexpr double GAME_LOGIC_FRAME_TIME = 1.0 / GAME_LOGIC_FPS;

void push_frame() {
    GameEngine::Instance->StartFrame();
    mainLoop();
    GameEngine::StartAudioFrame();
    GameEngine::EndAudioFrame();
}

/* Rename SDL_main to main for SDL compatibility */
#ifdef __GNUC__
#define SDL_main main
#endif

// [port] Precise sleep: SDL_Delay for the bulk, then spin-wait the remainder.
static void preciseSleep(double seconds) {
    if (seconds <= 0) {
        return;
    }
    double freq = (double)SDL_GetPerformanceFrequency();
    uint64_t target = SDL_GetPerformanceCounter() + (uint64_t)(seconds * freq);

    // Sleep the bulk (leave 1.5ms margin for spin)
    double sleepMs = (seconds * 1000.0) - 1.5;
    if (sleepMs > 0.5) {
        SDL_Delay((uint32_t)sleepMs);
    }

    // Spin-wait the remainder for precise timing
    while (SDL_GetPerformanceCounter() < target) {
        // busy-wait
    }
}

int SDL_main(int argc, char* argv[]) {
#ifdef _WIN32
    timeBeginPeriod(1); // [port] Improve Sleep precision from ~15.6ms to ~1ms
#endif
    GameEngine::Create(argc, argv);
    core1_init();

    double freq = (double)SDL_GetPerformanceFrequency();

    while (WindowIsRunning()) {
        uint64_t frameStart = SDL_GetPerformanceCounter();
        push_frame();
        uint64_t frameEnd = SDL_GetPerformanceCounter();
        double frameDuration = (double)(frameEnd - frameStart) / freq;

        // [port] During demo playback, match the N64's original frame display time.
        // When the N64 dropped frames (viCount > 2), it displayed that frame longer.
        // Without this, animations appear to speed up because the larger delta is
        // applied to a constant-length PC frame.
        int viCount = port_getDemoViCount();
        double targetFrameTime = (viCount > 0) ? (viCount / 60.0) : GAME_LOGIC_FRAME_TIME;

        if (frameDuration < targetFrameTime) {
            preciseSleep(targetFrameTime - frameDuration);
        }
    }
#ifdef _WIN32
    timeEndPeriod(1);
#endif
    GameEngine::Instance->Destroy();
    return 0;
}
