#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomCollectible/CustomCollectible.h"
#include "port/Enhancements/Retention/Retention.h"

extern "C" {
s32 item_adjustByDiffWithHud(enum item_e item, s32 diff);
}

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_JINJOS].optionValue

bool OverrideJinjoSpawn(OnActorSpawn* ev) {
    if (ev->actorId != ACTOR_5E_JINJO_YELLOW && ev->actorId != ACTOR_5F_JINJO_ORANGE &&
        ev->actorId != ACTOR_60_JINJO_BLUE && ev->actorId != ACTOR_61_JINJO_PINK &&
        ev->actorId != ACTOR_62_JINJO_GREEN) {
        return false;
    }

    int32_t spawnPosition[3] = { ev->posX, ev->posY, ev->posZ };
    RandoCheckId randoCheckId = Rando::StaticData::GetCheckByPosition(ev->posX, ev->posY, ev->posZ);

    if (randoCheckId == RC_UNKNOWN || !Rando::Logic::IsCheckShuffled(randoCheckId)) {
        return false;
    }

    if (!Rando::Logic::IsCheckObtained(randoCheckId)) {
        CustomCollectible::Spawn(spawnPosition, randoCheckId);
    }

    return true;
}

void RegisterRandoJinjos() {
    COND_VB_SHOULD(VB_UPDATE_JINJO_HUD, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, { *should = true; })

    COND_VB_SHOULD(VB_SET_JINJO_COUNT, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, { *should = true; })

    COND_HOOK(OnSetJiggyList, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnSetJiggyList* ev = (OnSetJiggyList*)event;
        u8 bits = collectedBits(ev->levelId);
        if (bits != 0) {
            item_adjustByDiffWithoutHud(ITEM_12_JINJOS, bits - item_getCount(ITEM_12_JINJOS));
        }
    });

    COND_HOOK(OnActorSpawn, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnActorSpawn* ev = (OnActorSpawn*)event;

        bool replaceJinjo = OverrideJinjoSpawn(ev);
        if (replaceJinjo) {
            event->Cancelled = true;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterRandoJinjos, { "IS_RANDO" });
