// Per-romhack quirk handlers. Each block here is a port-side listener that
// adjusts engine behavior for a specific romhack identified via the
// RomhackTable.h SHA1 -> name mapping. Listeners run only when the loaded
// romhack matches the relevant identifier; vanilla and unrelated romhacks
// see no behavior change.
//
// To add a new quirk:
//   1. Make sure the romhack has an entry in RomhackTable.h.
//   2. Find the engine event the quirk should hook (or define a new one in
//      src/port/enhancements/events/hooks/list/).
//   3. Add a REGISTER_LISTENER block below, gated on
//      port_getRomhackIdentifier() returning the matching name.
//   4. Wire the registration into RegisterRomhackQuirks_Init().

#include <cstring>

#include <libultraship/bridge.h>
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Romhack/RomhackConfig.h"

namespace {

bool IsRomhack(const char* identifier) {
    const char* id = port_getRomhackIdentifier();
    return id != nullptr && std::strcmp(id, identifier) == 0;
}

// --- Banjo-Dreamie
//
// Pre-BB romhack whose BB-patched warp constants live in the relocated overlay
// copy. Dreamie's boot loader doesn't actually load that copy on N64.
//
// On every OnWarpResolveDest fire, restore the dispatcher's vanilla default,
// ignoring whatever the BKCF claims.

void RegisterDreamiePatches() {
    REGISTER_LISTENER(OnWarpResolveDest, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        if (!IsRomhack("Dreamie")) {
            return;
        }
        auto* ev = reinterpret_cast<OnWarpResolveDest*>(event);
        if (ev->warpId == WARP_ID_SM_EXIT_BANJOS_HOUSE || ev->warpId == WARP_ID_LAIR_ENTER_MM_LOBBY_FROM_SM_LEVEL) {
            *ev->dest = ev->defaultDest;
        }
    });
}

void RegisterRomhackQuirks_Init() {
    RegisterDreamiePatches();
}

RegisterShipInitFunc initFunc(RegisterRomhackQuirks_Init);

} // namespace
