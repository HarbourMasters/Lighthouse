#include "EventDebugger.h"

#include <cinttypes>
#include <string>
#include <version>

#include "port/Engine.h"
#include "port/ui/UIWidgets.hpp"
#include "port/ui/cvar_prefixes.h"
#include <ship/utils/StringHelper.h>
#include <ship/events/EventSystem.h>
#include <ship/Context.h>

#define THEME_COLOR UIWidgets::Colors::Orange

static bool hookOptCollapseAll;
static bool hookOptExpandAll;

const ImVec4 grey = ImVec4(0.75, 0.75, 0.75, 1);
const ImVec4 yellow = ImVec4(1, 1, 0, 1);
const ImVec4 red = ImVec4(1, 0, 0, 1);

void DrawEventCallerInfo(std::string& name, Ship::EventRegistration& registry) {
    ImGui::Text("Total Callers Registered: %zu", registry.Callers.size());

    if (ImGui::BeginTable(("Table##" + std::string(name)).c_str(), 4,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                              ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Registration Info", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("# Calls", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        int i = 0;
        for (auto& [_, caller] : registry.Callers) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", i++);

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s:%d ", caller.Path, caller.Line);

            ImGui::TableNextColumn();
            ImGui::Text("%" PRIu64, caller.Count);
        }
        ImGui::EndTable();
    }
}

void DrawEventListenerInfo(std::string& name, Ship::EventRegistration& registry) {
    ImGui::Text("Total Listeners Registered: %zu", registry.Listeners.size());

    if (ImGui::BeginTable(("Table##" + std::string(name)).c_str(), 4,
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                              ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Listener Info", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableHeadersRow();

        int i = 0;
        for (auto& listener : registry.Listeners) {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::Text("%d", i++);

            ImGui::TableNextColumn();
            ImGui::TextWrapped("%s:%d ", listener.Metadata.Path, listener.Metadata.Line);

            ImGui::TableNextColumn();
            switch (listener.Priority) {
                case EVENT_PRIORITY_LOW:
                    ImGui::TextColored(grey, "Low");
                    break;
                case EVENT_PRIORITY_NORMAL:
                    ImGui::TextColored(yellow, "Normal");
                    break;
                case EVENT_PRIORITY_HIGH:
                    ImGui::TextColored(red, "High");
                    break;
            }
        }
        ImGui::EndTable();
    }
}

void EventDebuggerWindow::DrawElement() {
    bool collapseLogic = false;
    auto events = Ship::Context::GetRawInstance()->GetEventSystem()->GetEventRegistrations();
    bool doingCollapseOrExpand = hookOptExpandAll || hookOptCollapseAll;

    ImGui::BeginDisabled(CVarGetInteger(CVAR_SETTING("DisableChanges"), 0));

    if (UIWidgets::Button("Expand All", UIWidgets::ButtonOptions().Color(THEME_COLOR).Size(UIWidgets::Sizes::Inline))) {
        hookOptCollapseAll = false;
        hookOptExpandAll = true;
    }
    ImGui::SameLine();
    if (UIWidgets::Button("Collapse All",
                          UIWidgets::ButtonOptions().Color(THEME_COLOR).Size(UIWidgets::Sizes::Inline))) {
        hookOptExpandAll = false;
        hookOptCollapseAll = true;
    }

    ImGui::PushFont(GameEngine::Instance->fontMonoLarger);

    for (auto& [id, registry] : events) {
        auto name = StringHelper::Sprintf("%s (ID: %d) [%d]", registry.Name, id, registry.Listeners.size());

        if (doingCollapseOrExpand) {
            if (hookOptExpandAll) {
                collapseLogic = true;
            } else if (hookOptCollapseAll) {
                collapseLogic = false;
            }
            ImGui::SetNextItemOpen(collapseLogic, ImGuiCond_Always);
        }

        if (ImGui::TreeNode(name.c_str())) {
            DrawEventCallerInfo(name, registry);
            DrawEventListenerInfo(name, registry);
            ImGui::TreePop();
        }
    }

    ImGui::PopFont();
    ImGui::EndDisabled();

    if (doingCollapseOrExpand) {
        hookOptExpandAll = false;
        hookOptCollapseAll = false;
    }
}

void EventDebuggerWindow::InitElement() {
    hookOptExpandAll = false;
    hookOptCollapseAll = false;
}
