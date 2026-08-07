#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomCollectible/CustomCollectible.h"

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_BLUE_EGGS].optionValue

bool OverrideBlueEggSpawn(s16 spawnPosition[3], int32_t propAsset) {
    if (propAsset != ASSET_6D7_SPRITE_BLUE_EGGS) {
        return false;
    }

    s32 position[3] = { spawnPosition[0], spawnPosition[1], spawnPosition[2] };
    RandoCheckId randoCheckId = Rando::StaticData::GetCheckByPosition(position[0], position[1], position[2]);

    if (randoCheckId == RC_UNKNOWN || !Rando::Logic::IsCheckShuffled(randoCheckId)) {
        return false;
    }

    // Let vanilla blue eggs spawn after check has been obtained.
    if (Rando::Logic::IsCheckObtained(randoCheckId)) {
        return false;
    }

    CustomCollectible::QueueProp(position, randoCheckId);
    return true;
}

void RegisterRandoBlueEggs() {
    COND_VB_SHOULD(VB_OVERRIDE_PROP_SPAWN, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, {
        s16* spawnPosition = va_arg(args, s16*);
        int32_t propAsset = va_arg(args, int32_t);
        bool replaceEgg = OverrideBlueEggSpawn(spawnPosition, propAsset);
        if (replaceEgg) {
            *should = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterRandoBlueEggs, { "IS_RANDO" });
