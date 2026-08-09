/*
 * New Horizons Fun Facts
 *
 * Beyond the BKCF-covered data tables, New Horizons patches two ActorInfo
 * marker bindings inside its copy of the globalized-overlay blob.
 *
 */

#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"

extern "C" {
#include "enums.h"
#include "functions.h"

// { MARKER_235_CRACKED_SKYLIGHT, ACTOR_23F_CRACKED_SKYLIGHT, ... }
extern ActorInfo chCrackedSkylight;
// { 0x220, 0x22C, ... }
extern ActorInfo D_80392994;
}

void RegisterNewHorizonsPatches() {
    chCrackedSkylight.markerId = MARKER_60_BLUE_EGG_COLLECTIBLE;
    D_80392994.markerId = MARKER_60_BLUE_EGG_COLLECTIBLE;
}
