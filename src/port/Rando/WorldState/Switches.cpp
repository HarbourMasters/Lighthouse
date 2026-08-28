#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomCollectible/CustomCollectible.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_JIGGIES].optionValue

extern "C" {
void destroyJiggy(Actor* thisx, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, enum volatile_flags_e arg6);
}

void ModifySwitchBehavior(int32_t switchActorId) {
    RandoCheckId randoCheckId = RC_UNKNOWN;

    if (item_getCount(ITEM_0_HOURGLASS_TIMER) == 0) {
        switch (switchActorId) {
            case ACTOR_14E_BGS_ELEVATED_WALKWAY_SWITCH:
                if (mapSpecificFlags_get(3)) {
                    randoCheckId = RC_BGS_JIGGY_ELEVATED_WALKWAY;
                }
                break;
            case ACTOR_1FB_BGS_MAZE_SWITCH:
                if (mapSpecificFlags_get(0xC)) {
                    randoCheckId = RC_BGS_JIGGY_MAZE;
                }
                break;
            default:
                return;
        }
    }

    if (randoCheckId == RC_UNKNOWN) {
        return;
    }

    // Don't play missed switch scene if the check's been obtained
    if (RANDO_SAVE_CHECKS[randoCheckId].eligible) {
        return;
    }

    Actor* findActor = CustomCollectible::GetActorByRC(randoCheckId);

    if (findActor != NULL) {
        actor_collisionOff(findActor);
        if (randoCheckId == RC_BGS_JIGGY_ELEVATED_WALKWAY) {
            destroyJiggy(findActor, 4, 3, 0xD, 5, 2, VOLATILE_FLAG_AE_BGS_WALKWAY_JIGGY_MISSED);
        } else {
            destroyJiggy(findActor, 0xd, 0xc, 0x1e, 9, 0xb, VOLATILE_FLAG_AF_BGS_MAZE_JIGGY_MISSED);
        }
    }
}

void RegisterWorldStateSwitches() {
    COND_HOOK(OnActorTick, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnActorTick* ev = (OnActorTick*)event;

        switch (ev->actor->actor_info->actorId) {
            case ACTOR_14E_BGS_ELEVATED_WALKWAY_SWITCH:
            case ACTOR_1FB_BGS_MAZE_SWITCH:
                ModifySwitchBehavior(ev->actor->actor_info->actorId);
                break;
            default:
                break;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterWorldStateSwitches, { "IS_RANDO" });
