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

    // [port] OpenGL returns rows bottom-to-top from the default framebuffer (fb 0),
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

// [port] Auxiliary color-image readback for render-to-texture (Bottles Bonus 160x128 puzzle).
// On N64, gDPSetColorImage redirected RDP output to a CPU buffer. On PC, everything renders
// to the main FB. We use the interpreter's SetColorImage callback to detect transitions,
// then CopyFramebuffer + ReadFramebufferToCPU to populate the CPU buffer.
struct AuxColorImage {
    int fbId;
    uint32_t width, height;  // native dimensions
    uint32_t curFbW, curFbH; // current FBO dimensions (avoids per-frame resize)
};
static std::unordered_map<uintptr_t, AuxColorImage> sAuxColorImages;
static int sActiveAuxFb = -1;

static void auxColorImageCallback(void* oldAddr, void* newAddr) {
    bool oldIsAux = sAuxColorImages.count((uintptr_t)oldAddr) > 0;
    bool newIsAux = sAuxColorImages.count((uintptr_t)newAddr) > 0;

    if (newIsAux && sActiveAuxFb < 0) {
        sActiveAuxFb = sAuxColorImages[(uintptr_t)newAddr].fbId;
    }

    if (!oldIsAux || sActiveAuxFb < 0) {
        return;
    }

    auto interpreter = GameEngine_GetInterpreter();
    if (!interpreter || !interpreter->mRapi) {
        return;
    }
    auto& aux = sAuxColorImages[(uintptr_t)oldAddr];
    auto* rapi = interpreter->mRapi;

    int mainFb = interpreter->mRendersToFb ? interpreter->mGameFb : 0;
    uint32_t gpuW = 0, gpuH = 0;
    interpreter->GetCurDimensions(&gpuW, &gpuH);

    float ratioX = (float)gpuW / interpreter->mNativeDimensions.width;
    float ratioY = (float)gpuH / interpreter->mNativeDimensions.height;
    uint32_t srcW = (uint32_t)(aux.width * ratioX);
    uint32_t srcH = (uint32_t)(aux.height * ratioY);

    // OpenGL's glBlitFramebuffer scales during copy; DX11's CopySubresourceRegion is 1:1.
    const char* apiName = rapi->GetName();
    bool isOpenGL = (apiName && strstr(apiName, "OpenGL") != nullptr);

    if (isOpenGL) {
        if (aux.curFbW != aux.width || aux.curFbH != aux.height) {
            rapi->UpdateFramebufferParameters(aux.fbId, aux.width, aux.height, 1, true, true, true, false);
            aux.curFbW = aux.width;
            aux.curFbH = aux.height;
        }
        rapi->CopyFramebuffer(aux.fbId, mainFb, 0, 0, srcW, srcH, 0, 0, aux.width, aux.height);
        rapi->ReadFramebufferToCPU(aux.fbId, aux.width, aux.height, (uint16_t*)oldAddr);
    } else {
        if (aux.curFbW != srcW || aux.curFbH != srcH) {
            rapi->UpdateFramebufferParameters(aux.fbId, srcW, srcH, 1, true, true, true, false);
            aux.curFbW = srcW;
            aux.curFbH = srcH;
        }
        rapi->CopyFramebuffer(aux.fbId, mainFb, 0, 0, srcW, srcH, 0, 0, srcW, srcH);
        rapi->ReadFramebufferToCPU(aux.fbId, aux.width, aux.height, (uint16_t*)oldAddr);
    }

    // Byte-swap to big-endian (N64 pixel convention)
    uint16_t* pixels = (uint16_t*)oldAddr;
    uint32_t pixelCount = aux.width * aux.height;
    for (uint32_t i = 0; i < pixelCount; i++) {
        pixels[i] = (pixels[i] >> 8) | (pixels[i] << 8);
    }

    // Invalidate texture cache entries within the buffer range
    interpreter->TextureCacheDeleteRange((const uint8_t*)oldAddr, pixelCount * sizeof(uint16_t));
    sActiveAuxFb = -1;
}

extern "C" void port_registerAuxColorImage(void* cpuAddr, uint32_t width, uint32_t height) {
    uintptr_t key = (uintptr_t)cpuAddr;
    if (sAuxColorImages.count(key)) {
        return;
    }

    auto interpreter = GameEngine_GetInterpreter();
    if (!interpreter || !interpreter->mRapi) {
        return;
    }

    int fb = interpreter->mRapi->CreateFramebuffer();
    interpreter->mRapi->UpdateFramebufferParameters(fb, width, height, 1, true, true, true, false);
    interpreter->mFrameBuffers[fb] = { width, height, width, height, width, height, false };
    sAuxColorImages[key] = { fb, width, height, width, height };

    // Install callback if not already set
    interpreter->SetColorImageChangeCallback(auxColorImageCallback);
}

extern "C" void port_unregisterAuxColorImage(void* cpuAddr) {
    uintptr_t key = (uintptr_t)cpuAddr;
    auto it = sAuxColorImages.find(key);
    if (it == sAuxColorImages.end()) {
        return;
    }

    auto interpreter = GameEngine_GetInterpreter();
    if (interpreter) {
        interpreter->mFrameBuffers.erase(it->second.fbId);
    }
    if (sActiveAuxFb == it->second.fbId) {
        sActiveAuxFb = -1;
    }
    sAuxColorImages.erase(it);
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
    // [port] Audio processes after game logic to avoid data race on param lists
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

        // [port] Sleep remainder of 30fps budget if frame finished early.
        if (frameDuration < GAME_LOGIC_FRAME_TIME) {
            preciseSleep(GAME_LOGIC_FRAME_TIME - frameDuration);
        }
    }
#ifdef _WIN32
    timeEndPeriod(1);
#endif
    GameEngine::Instance->Destroy();
    return 0;
}
