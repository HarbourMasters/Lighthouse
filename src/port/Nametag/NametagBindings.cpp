// BK-specific bindings for the portable Nametag overlay: projection, FB
// dimensions, tick-event listeners, and the asset-id -> label table.

#include <libultraship/libultraship.h>

#include "Nametag.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/UI/cvar_prefixes.h"
#include "port/ObjectExtension/ObjectExtension.h"

extern "C" {
#include "enums.h"
#include "core1/viewport.h"
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

static bool inRange(float x, float y, float z) {
    float pos[3] = { x, y, z };
    return viewport_getDistance(pos) < CVarGetFloat(CVAR_DEVELOPER_TOOLS("NametagDist"), 3000.0f);
}

static bool inRange(int16_t pos[3]) {
    return inRange((float)pos[0], (float)pos[1], (float)pos[2]);
}

void InitNametagBindings() {
    Nametag::SetProjectFn(&viewport_func_8024E030);
    Nametag::SetNativeFramebufferSize(&gFramebufferWidth, &gFramebufferHeight);
    Nametag::RegisterOverlay();
}

} // namespace

static RegisterShipInitFunc initNametagBindings(InitNametagBindings, { "BOOT" });
static RegisterShipInitFunc init([]() {
    REGISTER_LISTENER(GameFrameUpdate, EVENT_PRIORITY_HIGH, [](IEvent* event) { Nametag::Clear(); });
});