#include <libultraship.h>
#include <cstring>

#include <fast/interpreter.h>
#include "Engine.h"
#include "ShipUtils.h"
#include "patches/Patches.h"
#include "src/port/enhancements/events/hooks/Events.h"
#include "interpolation/FrameInterpolation.h"

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

// Tracks whether mainLoop actually fed the renderer this iteration.
// BK's gameloop conditionally skips game_draw during scene transitions.
static bool sFrameRendered = false;

extern "C" void Graphics_PushFrame(Gfx* data) {
    sFrameRendered = true;
    GameEngine::ProcessGfxCommands(data);
}

extern "C" int gsworld_getMap(void);
extern "C" void port_setWindowTitle(int map_id);

void push_frame() {
    static int sTitleCounter = 0;
    sFrameRendered = false;
    GameEngine::Instance->StartFrame();
    FrameInterpolation_StartRecord();
    mainLoop();
    FrameInterpolation_StopRecord();
    GameEngine::StartAudioFrame();
    GameEngine::EndAudioFrame();

    // Refresh window title stats once per second (every 30 game ticks)
    if (++sTitleCounter >= 30) {
        sTitleCounter = 0;
        port_setWindowTitle(gsworld_getMap());
    }

    if (!sFrameRendered) {
        SDL_Delay(33);
    }
}

/* Rename SDL_main to main for SDL compatibility */
#ifdef __GNUC__
#define SDL_main main
#endif

int SDL_main(int argc, char* argv[]) {
#ifdef _WIN32
    timeBeginPeriod(1); // [port] Improve Sleep precision from ~15.6ms to ~1ms
#endif
    GameEngine::Create(argc, argv);
    core1_init();

    while (WindowIsRunning()) {
        push_frame();
    }
#ifdef _WIN32
    timeEndPeriod(1);
#endif
    GameEngine::Instance->Destroy();
    return 0;
}
