#include "Logic.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/UI/cvar_prefixes.h"

namespace Rando {

namespace Logic {

void RefreshReachableRegions() {
    std::unordered_set<RandoRegionId> seen;

    Rando::StaticData::RandoLogicData regionSnapshot[RR_MAX] = {};
    Rando::StaticData::RandoLogicData eventSnapshot[RA_MAX] = {};
    Rando::StaticData::RandoLogicData checkSnapshot[RC_MAX] = {};

    for (auto const& [regionId, regionData] : Rando::Logic::Regions) {
        if (regionData.mapId == gsworld_getMap()) {
            seen.insert(regionId);
        }
    }

    if (CVarGetInteger(CVAR_RANDOMIZER_SETTING("GeneratingSeed"), 0)) {
        seen.insert(RR_SPIRAL_MOUNTAIN_ENTRANCE);
    }

    bool changed = true;
    while (changed) {
        changed = false;
        std::vector<RandoRegionId> currentSnapshot(seen.begin(), seen.end());

        for (auto const& regionId : currentSnapshot) {
            if (!regionSnapshot[regionId].canAccess) {
                regionSnapshot[regionId].canAccess = true;
                changed = true;
            }

            const auto& regionData = Rando::Logic::Regions.at(regionId);

            for (const auto& [eventId, eventInfo] : regionData.events) {
                if (!eventSnapshot[eventId].canAccess && eventInfo()) {
                    eventSnapshot[eventId].canAccess = true;
                    changed = true;
                }
            }

            for (const auto& [targetId, connectionInfo] : regionData.connections) {
                if (seen.find(targetId) == seen.end() && connectionInfo.first()) {
                    seen.insert(targetId);
                    changed = true;
                }
            }
        }
    }

    for (int i = RR_UNKNOWN; i < RR_MAX; i++) {
        if (!regionSnapshot[i].canAccess) {
            continue;
        }

        const auto& regionData = Rando::Logic::Regions.at((RandoRegionId)i);

        for (const auto& [checkId, checkInfo] : regionData.checks) {
            if (checkInfo.first()) {
                checkSnapshot[checkId].canAccess = true;
            }
        }
    }

    for (int i = RR_UNKNOWN; i < RR_MAX; i++) {
        reachableRegions[i].canAccess = regionSnapshot[i].canAccess;
    }
    for (int i = RA_UNKNOWN; i < RA_MAX; i++) {
        reachableEvents[i].canAccess = eventSnapshot[i].canAccess;
    }
    for (int i = RC_UNKNOWN; i < RC_MAX; i++) {
        reachableChecks[i].canAccess = checkSnapshot[i].canAccess;
    }
}

} // namespace Logic

} // namespace Rando