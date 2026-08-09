#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomCollectible/CustomCollectible.h"

extern "C" {
void item_inc(enum item_e item);
extern u8 D_80385FF0[0xE];
}

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_MUSIC_NOTES].optionValue

bool OverrideMusicNoteSpawn(s16 spawnPosition[3], int32_t propAsset) {
    if (propAsset != ASSET_6D6_SPRITE_MUSIC_NOTE) {
        return false;
    }

    s32 position[3] = { spawnPosition[0], spawnPosition[1], spawnPosition[2] };
    RandoCheckId randoCheckId = Rando::StaticData::GetCheckByPosition(position[0], position[1], position[2]);

    if (randoCheckId == RC_UNKNOWN || !Rando::Logic::IsCheckShuffled(randoCheckId)) {
        return false;
    }

    if (!Rando::Logic::IsCheckObtained(randoCheckId)) {
        CustomCollectible::QueueProp(position, randoCheckId);
    }

    return true;
}

void RegisterRandoMusicNotes() {
    COND_HOOK(OnSetJiggyList, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;

        item_set(ITEM_C_NOTE, D_80385FF0[ev->levelId]);
    });

    COND_VB_SHOULD(VB_OVERRIDE_PROP_SPAWN, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, {
        s16* spawnPosition = va_arg(args, s16*);
        int32_t propAsset = va_arg(args, int32_t);
        bool replaceNote = OverrideMusicNoteSpawn(spawnPosition, propAsset);
        if (replaceNote) {
            *should = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterRandoMusicNotes, { "IS_RANDO" });
