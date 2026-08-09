#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "spdlog/spdlog.h"

extern "C" {
void destroyJiggy(Actor* thisx, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, enum volatile_flags_e arg6);
}

Actor* FindActorByRandoCheckId(RandoCheckId randoCheckId); // ObjectBehavior.cpp

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_JIGGIES].optionValue

void Rando::ObjectBehavior::ModifySwitchBehavior(int32_t switchActorId) {
    if (!IS_RANDO || !OPTION_ENABLED) {
        return;
    }

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

    Actor* findActor = FindActorByRandoCheckId(randoCheckId);

    if (findActor != NULL) {
        actor_collisionOff(findActor);
        if (randoCheckId == RC_BGS_JIGGY_ELEVATED_WALKWAY) {
            destroyJiggy(findActor, 4, 3, 0xD, 5, 2, VOLATILE_FLAG_AE_BGS_WALKWAY_JIGGY_MISSED);
        } else {
            destroyJiggy(findActor, 0xd, 0xc, 0x1e, 9, 0xb, VOLATILE_FLAG_AF_BGS_MAZE_JIGGY_MISSED);
        }
        CustomObject::RemoveSpawnedIdFromList(randoCheckId);
    }
}
