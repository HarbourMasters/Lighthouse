#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"

#include "prop.h"

extern "C" {
void marker_despawn(ActorMarker* marker);
s32 levelSpecificFlags_get(s32 i);
}

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_JIGGIES].optionValue

std::map<actor_e, level_flags_e> presentLevelFlagMap = {
    { ACTOR_1ED_BLUE_PRESENT_COLLECTIBLE, LEVEL_FLAG_11_FP_UNKNOWN },
    { ACTOR_1EF_GREEN_PRESENT_COLLECTIBLE, LEVEL_FLAG_12_FP_UNKNOWN },
    { ACTOR_1F1_RED_PRESENT_COLLECTIBLE, LEVEL_FLAG_13_FP_UNKNOWN },
};

void GetPresentLevelFlag(Actor* actor, level_flags_e levelFlag) {
    if (levelSpecificFlags_get(levelFlag)) {
        marker_despawn(actor->marker);
    }
}

void Rando::ObjectBehavior::ModifyPresentBehavior(void* presentActor) {
    if (!IS_RANDO && !OPTION_ENABLED) {
        return;
    }

    Actor* actor = (Actor*)presentActor;
    int32_t actorAdjustment = 0;
    int32_t levelFlag = 0;

    switch (actor->actor_info->actorId) {
        case ACTOR_33A_BLUE_PRESENT:
        case ACTOR_33B_GREEN_PRESENT:
        case ACTOR_33C_RED_PRESENT:
            levelFlag = (LEVEL_FLAG_11_FP_UNKNOWN + (actor->actor_info->actorId - ACTOR_33A_BLUE_PRESENT));
            if (actor->unk38_31) {
                actor->unk38_31 = !levelSpecificFlags_get(levelFlag);
            }
            break;
        case ACTOR_1ED_BLUE_PRESENT_COLLECTIBLE:
        case ACTOR_1EF_GREEN_PRESENT_COLLECTIBLE:
        case ACTOR_1F1_RED_PRESENT_COLLECTIBLE:
            GetPresentLevelFlag(actor, presentLevelFlagMap.at((actor_e)actor->actor_info->actorId));
            break;
        default:
            break;
    }
}
