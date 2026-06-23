#include "Engine.h"
#include <cstring>
#include <cstdlib>
#include <filesystem>
#include <chrono>

#include <fast/interpreter.h>
#include <libultraship.h>
#ifdef _WIN32
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#endif
#include <SDL2/SDL.h>

#include "GameStatus.h"
#include "Interpolation/AdaptiveFps.h"
#include "Interpolation/FrameInterpolation.h"
#include "Network/Anchor/Anchor.h"
#include "Patches/Patches.h"
#include "ShipUtils.h"
#include "src/port/Enhancements/Events/Hooks/Events.h"
#include "UI/LighthouseModMenuWindow.h"

extern "C" {
#include "enums.h"
#include "core1/core1.h"
#include "core1/main.h"
// Non-interactive demo/playback modes (attract demo, file playback, etc.) -- decomp gameloop.c
bool func_802E4A08(void);
}

// Tracks whether mainLoop actually fed the renderer this iteration.
// BK's gameloop conditionally skips game_draw during scene transitions.
static bool sFrameRendered = false;

// Start of this tick. The gap until drawing begins is how long the game spent
// updating; Adaptive FPS uses it to budget how many interpolated frames fit.
static std::chrono::steady_clock::time_point sTickStart;

extern "C" void Graphics_PushFrame(Gfx* data) {
    // Only measure the first draw of the tick.
    if (!sFrameRendered) {
        auto logicNs =
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - sTickStart).count();
        AdaptiveFps_SampleTick((long long)logicNs);
    }
    sFrameRendered = true;
    GameEngine::ProcessGfxCommands(data);
}

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

    sTickStart = std::chrono::steady_clock::now();
    GameEngine::Instance->StartFrame();
    // Demo/playback modes render at native rate with no interpolation, so skip recording it.
    const bool recordInterpolation = GameEngine::IsInterpolationEnabled() && !func_802E4A08();
    if (recordInterpolation) {
        FrameInterpolation_StartRecord();
    }
    mainLoop();
    if (recordInterpolation) {
        FrameInterpolation_StopRecord();
    }
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
#ifdef USE_NETWORKING
    Anchor::GetInstance()->Disable();
    SDLNet_Quit();
#endif
#ifdef _WIN32
    timeEndPeriod(1);
#endif
    GameEngine::Instance->Destroy();
    GameEngine::RelaunchIfRequested(argc, argv);
    return 0;
}
