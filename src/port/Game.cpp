#include <libultraship.h>
#include <ship/Context.h>
#include <cstring>
#include <cstdlib>
#include <filesystem>

#include <fast/interpreter.h>
#include "Engine.h"
#include "ShipUtils.h"
#include "ui/LighthouseModMenuWindow.h"
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

    // While an inline mod extraction runs on its worker thread, freeze the game
    // and render only the GUI so the progress modal stays live and the extractor
    // gets the machine instead of fighting a full-speed game loop. The delay
    // keeps the otherwise-idle main thread from busy-spinning a core.
    if (IsInlineModExtractionBusy()) {
        GameEngine::Instance->RenderGuiFrame();
        SDL_Delay(16);
        return;
    }

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
    timeBeginPeriod(1);
#endif

    // Anchor relative paths to the executable instead of cwd
    // when SHIP_HOME is not in use
    std::error_code ec;
    const char* shipHome = std::getenv("SHIP_HOME");
    if (shipHome != nullptr && shipHome[0] != '\0') {
        std::filesystem::current_path(shipHome, ec);
    } else {
        std::string base = Ship::Context::GetAppBundlePath();
        if (!base.empty() && base != ".") {
            std::filesystem::current_path(base, ec);
        }
    }

    GameEngine::Create(argc, argv);
    core1_init();

    while (WindowIsRunning()) {
        push_frame();
    }
#ifdef _WIN32
    timeEndPeriod(1);
#endif
    GameEngine::Instance->Destroy();
    GameEngine::RelaunchIfRequested(argc, argv);
    return 0;
}
