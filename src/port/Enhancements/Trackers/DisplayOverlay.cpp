#include "DisplayOverlay.h"
#include "port/UI/enhancementTypes.h"
#include "port/ShipUtils.h"
#include <spdlog/fmt/fmt.h>
#include "fast/Fast3dGui.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include <libultraship/include/libultraship/libultra/gbi.h>
#include <ship/Context.h>
#include <ship/window/Window.h>

#include "save.h"
#include "enums.h"

extern "C" {
enum map_e gsworld_getMap(void);
enum level_e map_getLevel(enum map_e map);
u16 itemscore_timeScores_get(enum level_e level_id);

uint64_t GetUnixTimestamp();
extern SaveData gameFile_saveData[4];
}

float windowScale = 1.0f;
ImVec4 windowBG = ImVec4(0, 0, 0, 0.5f);
static constexpr ImVec4 tintColor = {};

const std::vector<const char*> timerDisplayOptions = {
    "Display Off",  // TIMER_DISPLAY_NONE
    "Real-Time",    // TIMER_DISPLAY_RTA
    "In-Game Time", // TIMER_DISPLAY_IGT
};

int64_t DisplayOverlay_GetTotalInGameTime() {
    int64_t totalTime = 0;
    for (int i = LEVEL_1_MUMBOS_MOUNTAIN; i < LEVEL_D_CUTSCENE; i++) {
        totalTime += itemscore_timeScores_get((level_e)i);
    }
    return totalTime * 10;
}

void DisplayOverlayWindow::Draw() {
    if (!IsVisible() || gsworld_getMap() == MAP_91_FILE_SELECT) {
        return;
    }

    int displayOverlay = CVarGetInteger(CVAR_DISPLAY_OVERLAY_MODE, 0);
    if (displayOverlay == TIMER_DISPLAY_NONE) {
        return;
    }

    float windowScale = MAX(CVarGetFloat("gDisplayOverlay.Scale", 1.0f), 1.0f);
    ImVec4 windowBG = !CVarGetInteger("gDisplayOverlay.Background", 0) ? ImVec4(0, 0, 0, 0.5f) : ImVec4(0, 0, 0, 0);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, windowBG);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);

    ImGui::Begin("Overlay", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
    ImGui::SetWindowFontScale(windowScale);

    uint64_t timeToDisplay = 0;
    switch (displayOverlay) {
        case TIMER_DISPLAY_RTA:
            timeToDisplay =
                ((GetUnixTimestamp() - gameFile_saveData[gSelectedFileNum].shipSaveData.fileCreatedAt) / 100);
            break;
        case TIMER_DISPLAY_IGT:
            timeToDisplay = DisplayOverlay_GetTotalInGameTime();
            break;
        default:
            break;
    }
    std::string timerStr = port_FormatTimeDisplay(timeToDisplay);
    ImGui::Text(timerStr.c_str());

    ImGui::End();

    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(2);
}

void DisplayOverlayWindow::InitElement() {
}
