#include <vector>
#include <string>

#include "ShipInit.hpp"
#include "port/ui/Menu.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/ui/cvar_prefixes.h"
#include <ship/window/gui/GuiWindow.h>
#include "nametag.h"

extern "C" {
    bool viewport_func_8024E030(float pos[3], float* screenOut);
    extern int gFramebufferWidth;
    extern int gFramebufferHeight;
}

struct NametagTableEntry {
    int32_t assetId;
    const char* label;
    float yOffset;
};

// Asset-id  nametag lookup. Add entries here to tag new actors, markered
// props, or static sprite props  all three dispatch paths converge on this
// single table by looking up the model/sprite asset.
constexpr NametagTableEntry sNametagTable[] = {
    { ASSET_350_MODEL_TERMITE,      "Termite",       150.0f },
    { ASSET_3C5_MODEL_GRUBLIN,      "Grublin",       150.0f },
    { ASSET_353_MODEL_BIGBUTT,      "BigButt",       150.0f },
    { ASSET_3C0_MODEL_JINJO_BLUE,   "Blue Jinjo",    150.0f },
    { ASSET_3C2_MODEL_JINJO_GREEN,  "Green Jinjo",   150.0f },
    { ASSET_3BC_MODEL_JINJO_ORANGE, "Orange Jinjo",  150.0f },
    { ASSET_3C1_MODEL_JINJO_PINK,   "Pink Jinjo",    150.0f },
    { ASSET_3BB_MODEL_JINJO_YELLOW, "Yellow Jinjo",  150.0f },
    { ASSET_6D6_SPRITE_MUSIC_NOTE,  "Note",           80.0f },
    { ASSET_41A_SPRITE_MUMBO_TOKEN, "Mumbo Token",    80.0f },
};

const NametagTableEntry* LookupNametag(int32_t assetId) {
    for (const auto& entry : sNametagTable) {
        if (entry.assetId == assetId) {
            return &entry;
        }
    }
    return nullptr;
}

struct NametagEntry {
    float pos[3];
    std::string text;
};
std::vector<NametagEntry> sNametagQueue;

namespace LighthouseGui {
    class NametagOverlay : public Ship::GuiWindow {
    public:
        using GuiWindow::GuiWindow;
        void InitElement() override {}
        void UpdateElement() override {}
        void DrawElement() override {}
        void Draw() override {
            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            ImVec2 vpSize = ImGui::GetMainViewport()->Size;
            ImVec2 vpPos = ImGui::GetMainViewport()->Pos;
            float scaleX = vpSize.x / static_cast<float>(gFramebufferWidth);
            float scaleY = vpSize.y / static_cast<float>(gFramebufferHeight);

            const ImU32 bgColor = IM_COL32(0, 0, 0, 200);
            const ImU32 borderColor = IM_COL32(64, 128, 255, 255);
            const ImU32 textColor = IM_COL32(255, 255, 255, 255);
            const ImVec2 padding(6.0f, 3.0f);

            for (const auto& entry : sNametagQueue) {
                float pos[3] = { entry.pos[0], entry.pos[1], entry.pos[2] };
                float screen[2];
                if (viewport_func_8024E030(pos, screen)) {
                    ImVec2 textSize = ImGui::CalcTextSize(entry.text.c_str());
                    ImVec2 anchor(vpPos.x + screen[0] * scaleX, vpPos.y + screen[1] * scaleY);
                    ImVec2 textTL(anchor.x - textSize.x * 0.5f, anchor.y - textSize.y * 0.5f);
                    ImVec2 boxTL(textTL.x - padding.x, textTL.y - padding.y);
                    ImVec2 boxBR(textTL.x + textSize.x + padding.x, textTL.y + textSize.y + padding.y);
                    drawList->AddRectFilled(boxTL, boxBR, bgColor, 3.0f);
                    drawList->AddRect(boxTL, boxBR, borderColor, 3.0f, 0, 1.5f);
                    drawList->AddText(textTL, textColor, entry.text.c_str());
                }
            }
        }
    };
} // namespace LighthouseGui

void RegisterNametagListener_Init() {
    // Clear the queue at the start of each game tick.
    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_HIGH, [](IEvent* event) {
        sNametagQueue.clear();
        });

    REGISTER_LISTENER(OnActorTick, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnActorTick*>(event);
        if (ev->actor == nullptr || ev->actor->actor_info == nullptr) {
            return;
        }
        const NametagTableEntry* entry = LookupNametag(ev->actor->actor_info->modelId);
        if (entry == nullptr) {
            return;
        }
        CALL_EVENT(OnNametagDraw, ev->actor, entry->label, entry->yOffset);
        });

    REGISTER_LISTENER(OnPropTick, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnPropTick*>(event);
        if (ev->marker == nullptr || ev->position == nullptr) {
            return;
        }
        const NametagTableEntry* entry = LookupNametag(ev->marker->modelId);
        if (entry == nullptr) {
            return;
        }
        NametagEntry queued;
        queued.pos[0] = ev->position[0];
        queued.pos[1] = ev->position[1] + entry->yOffset;
        queued.pos[2] = ev->position[2];
        queued.text = entry->label;
        sNametagQueue.push_back(std::move(queued));
        });

    REGISTER_LISTENER(OnSpritePropTick, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnSpritePropTick*>(event);
        if (ev->position == nullptr) {
            return;
        }
        const NametagTableEntry* entry = LookupNametag(ev->assetId);
        if (entry == nullptr) {
            return;
        }
        NametagEntry queued;
        queued.pos[0] = ev->position[0];
        queued.pos[1] = ev->position[1] + entry->yOffset;
        queued.pos[2] = ev->position[2];
        queued.text = entry->label;
        sNametagQueue.push_back(std::move(queued));
        });

    REGISTER_LISTENER(OnNametagDraw, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnNametagDraw*>(event);
        if (ev->actor == nullptr || ev->label == nullptr) {
            return;
        }
        NametagEntry entry;
        entry.pos[0] = ev->actor->position[0];
        entry.pos[1] = ev->actor->position[1] + ev->yOffset;
        entry.pos[2] = ev->actor->position[2];
        entry.text = ev->label;
        sNametagQueue.push_back(std::move(entry));
        });
}

static RegisterMenuInitFunc menuInitFunc([]() {
    auto gui = Ship::Context::GetInstance()->GetWindow()->GetGui();
    auto overlay = std::make_shared<LighthouseGui::NametagOverlay>(CVAR_PREFIX_WINDOW ".NametagOverlay", "Nametag Overlay");
    gui->AddGuiWindow(overlay);
    overlay->Show();
    });
static RegisterShipInitFunc initFunc(RegisterNametagListener_Init, {});