#include <libultraship.h>
#include <cstring>

#include <fast/interpreter.h>
#include "Engine.h"
#include "ShipUtils.h"
#include "patches/Patches.h"
#include "src/port/enhancements/events/hooks/Events.h"

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

// [port] BK game logic runs at 30fps (N64: 2 VI per game frame at 60Hz).
// The main loop must tick at exactly 30fps regardless of render rate.
// Without this, gGlobalTimer increments too fast, time_getDelta() returns
// tiny values, and all tick-based timing (animations, cutscenes, AI) breaks.
static constexpr double GAME_LOGIC_FPS = 30.0;
static constexpr double GAME_LOGIC_FRAME_TIME = 1.0 / GAME_LOGIC_FPS;

extern "C" int gsworld_getMap(void);
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
        port_setWindowTitle(gsworld_getMap());
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

        double targetFrameTime = port_getTargetFrameTime();

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
