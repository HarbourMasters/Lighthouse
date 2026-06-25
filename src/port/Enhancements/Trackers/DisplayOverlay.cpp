#include "DisplayOverlay.h"
#include "port/UI/enhancementTypes.h"
#include "port/ShipUtils.h"
#include <spdlog/fmt/fmt.h>
#include "fast/Fast3dGui.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include <libultraship/include/libultraship/libultra/gbi.h>
#include <ship/Context.h>
#include <ship/window/Window.h>

#include "enums.h"

extern "C" {
enum map_e gsworld_getMap(void);
enum level_e map_getLevel(enum map_e map);
u16 itemscore_timeScores_get(enum level_e level_id);

uint64_t GetUnixTimestamp();
}

float windowScale = 1.0f;
ImVec4 windowBG = ImVec4(0, 0, 0, 0.5f);
static constexpr ImVec4 tintColor = {};

// void DrawInGameTimer(uint32_t timer, ImVec4 color = ImVec4(1, 1, 1, 1)) {
//     float windowScale = MAX(CVarGetFloat("gDisplayOverlay.Scale", 1.0f), 1.0f);
// 
//     std::string timerStr = port_FormatTimeDisplay(timer);
//     uint16_t textureIndex = 0;
//     for (const auto c : timerStr) {
//         if (c == ':' || c == '.') {
//             textureIndex = 10;
//         } else {
//             textureIndex = c - '0';
//         }
//         if (c == '.') {
//             ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (8.0f * windowScale));
//             ImGui::Image(Ship::Context::GetRawInstance()->GetWindow()->GetGui()->GetTextureByName(digitList[textureIndex]),
//                          ImVec2(8.0f * windowScale, 8.0f * windowScale), ImVec2(0, 0.5f), ImVec2(1, 1), color,
//                          tintColor);
//         } else {
//             ImGui::Image(Ship::Context::GetRawInstance()->GetWindow()->GetGui()->GetTextureByName(digitList[textureIndex]),
//                          ImVec2(8.0f * windowScale, 16.0f * windowScale), ImVec2(0, 0), ImVec2(1, 1), color, tintColor);
//         }
//         ImGui::SameLine(0, 0);
//     }
// }

int64_t DisplayOverlay_GetTotalInGameTime() {
    int64_t totalTime = 0;
    for (int i = LEVEL_1_MUMBOS_MOUNTAIN; i < LEVEL_D_CUTSCENE; i++) {
        totalTime += itemscore_timeScores_get((level_e)i);
    }
    return totalTime;
}

void DisplayOverlayWindow::Draw() {
    if (!IsVisible() || gsworld_getMap() == MAP_91_FILE_SELECT) {
        return;
    }

    // int displayOverlay = CVarGetInteger(CVAR_DISPLAY_OVERLAY_MODE, 0);
    // if (displayOverlay == TIMER_DISPLAY_NONE) {
    //     return;
    // }
	
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

    // auto gui = std::dynamic_pointer_cast<Fast::Fast3dGui>(Ship::Context::GetRawInstance()->GetWindow()->GetGui());
    // ImTextureID textureId = gui->GetTextureByName("Music Note");
    // ImGui::Image(textureId,
    //              ImVec2(16.0f * windowScale, 16.0f * windowScale));
    // ImGui::SameLine(0, 10.0f);

    uint64_t timeToDisplay = DisplayOverlay_GetTotalInGameTime();
    std::string timerStr = port_FormatTimeDisplay(timeToDisplay * 10);
    ImGui::Text(timerStr.c_str());
    // DrawInGameTimer(timeToDisplay / 100);
	
	ImGui::End();

    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(2);
}

void DisplayOverlayWindow::InitElement() {
}
