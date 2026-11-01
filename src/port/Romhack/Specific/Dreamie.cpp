/*
 * Banjo-Dreamie
 *
 * Dreamie ships no custom code at all - it rewires the world purely through BB
 * globalized-overlay data, so the BKCF carries everything. Spiral Mountain becomes
 * the hub, and two of the ~86 warp edits retarget the Spiral Mountain transitions:
 *
 *   warp_smExitBanjosHouse         0x0112 -> 0x0105
 *   warp_lairEnterLairFromSMLevel  0x6912 -> 0x0112
 *
 * Both resolve to map 0x01, which is also Dreamie's new-game map (START_MAP = 1,
 * START_LEVEL_2 = 1, down from vanilla 0x69). Honoring them verbatim drops every hub
 * transition back into the starting room instead of moving between areas.
 *
 * Both warps keep their vanilla destinations, whatever the BKCF claims. The other BKCF
 * warp remaps are genuine and pass through untouched.
 *
 */

#include "port/Romhack/RomhackConfig.h"

static constexpr int kWarpIdxSmExitBanjosHouse = 281;

void RegisterDreamiePatches() {
    port_clearRomhackWarpDest(kWarpIdxSmExitBanjosHouse);
    port_clearRomhackNamedWarps();
}
