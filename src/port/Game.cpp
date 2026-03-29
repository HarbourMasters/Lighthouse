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

// [port] GPU FB for pause menu capture (created once, reused).
static int s_pauseFbId = -1;

extern "C" int port_getPauseFramebufferId(void) {
    if (s_pauseFbId < 0) {
        s_pauseFbId =
            gfx_create_framebuffer(gFramebufferWidth, gFramebufferHeight, gFramebufferWidth, gFramebufferHeight, 1);
    }
    return s_pauseFbId;
}

// [port] Consumed by the display list builder (bufferreadback.c) to emit
// gDPReadFB into the DL, populating gFramebuffers at native resolution.
extern "C" int port_consumeReadbackRequest(void) {
    if (s_freezeReadback || s_readbackRequestFrames <= 0)
        return 0;
    s_readbackRequestFrames--;
    return 1;
}

// [port] no-op — readback is done via gDPReadFB in the display list
extern "C" void Framebuffer_ReadbackGPU(int bufferIndex) {
    (void)bufferIndex;
}

// [port] BK game logic runs at 30fps (N64: 2 VI per game frame at 60Hz).
// The main loop must tick at exactly 30fps regardless of render rate.
// Without this, gGlobalTimer increments too fast, time_getDelta() returns
// tiny values, and all tick-based timing (animations, cutscenes, AI) breaks.
static constexpr double GAME_LOGIC_FPS = 30.0;
static constexpr double GAME_LOGIC_FRAME_TIME = 1.0 / GAME_LOGIC_FPS;

extern "C" int map_get(void);
extern "C" void port_setWindowTitle(int map_id);

void push_frame() {
    static int sTitleCounter = 0;
    GameEngine::Instance->StartFrame();
    mainLoop();
    GameEngine::StartAudioFrame();
    GameEngine::EndAudioFrame();

    // Refresh window title stats once per second (every 30 game ticks)
    if (++sTitleCounter >= 30) {
        sTitleCounter = 0;
        port_setWindowTitle(map_get());
    }
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
