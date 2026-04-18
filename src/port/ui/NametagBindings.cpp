// BK-specific bindings for the portable Nametag overlay: projection, FB
// dimensions, tick-event listeners, and the asset-id -> label table.

#include <libultraship/libultraship.h>

#include "port/ui/Nametag.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
bool viewport_func_8024E030(float pos[3], float* screenOut);
extern int gFramebufferWidth;
extern int gFramebufferHeight;
}

namespace {

struct NametagTableEntry {
    int32_t assetId;
    const char* label;
    float yOffset;
};

// Asset-id -> nametag lookup. Shared across the actor / prop / sprite-prop paths.
constexpr NametagTableEntry sNametagTable[] = {
    { ASSET_350_MODEL_TERMITE, "Termite", 150.0f },         { ASSET_3C5_MODEL_GRUBLIN, "Grublin", 150.0f },
    { ASSET_353_MODEL_BIGBUTT, "BigButt", 150.0f },         { ASSET_3C0_MODEL_JINJO_BLUE, "Blue Jinjo", 150.0f },
    { ASSET_3C2_MODEL_JINJO_GREEN, "Green Jinjo", 150.0f }, { ASSET_3BC_MODEL_JINJO_ORANGE, "Orange Jinjo", 150.0f },
    { ASSET_3C1_MODEL_JINJO_PINK, "Pink Jinjo", 150.0f },   { ASSET_3BB_MODEL_JINJO_YELLOW, "Yellow Jinjo", 150.0f },
    { ASSET_6D6_SPRITE_MUSIC_NOTE, "Note", 80.0f },         { ASSET_41A_SPRITE_MUMBO_TOKEN, "Mumbo Token", 80.0f },
};

const NametagTableEntry* LookupNametag(int32_t assetId) {
    for (const auto& entry : sNametagTable) {
        if (entry.assetId == assetId) {
            return &entry;
        }
    }
    return nullptr;
}

void InitNametagBindings() {
    Nametag::SetProjectFn(&viewport_func_8024E030);
    Nametag::SetNativeFramebufferSize(&gFramebufferWidth, &gFramebufferHeight);
    Nametag::RegisterOverlay();

    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_HIGH, [](IEvent* event) { Nametag::Clear(); });

    // Forwards through OnNametagDraw so non-dispatched actors can register too.
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
        Nametag::Push(ev->position[0], ev->position[1] + entry->yOffset, ev->position[2], entry->label);
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
        Nametag::Push(ev->position[0], ev->position[1] + entry->yOffset, ev->position[2], entry->label);
    });

    REGISTER_LISTENER(OnNametagDraw, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnNametagDraw*>(event);
        if (ev->actor == nullptr || ev->label == nullptr) {
            return;
        }
        Nametag::Push(ev->actor->position[0], ev->actor->position[1] + ev->yOffset, ev->actor->position[2], ev->label);
    });
}

} // namespace

static RegisterShipInitFunc initNametagBindings(InitNametagBindings, {});
