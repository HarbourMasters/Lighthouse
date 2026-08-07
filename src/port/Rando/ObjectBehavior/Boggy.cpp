#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"

#include "prop.h"

extern "C" {
void marker_despawn(ActorMarker* marker);
void func_80387760(ActorMarker* marker);
}

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_JIGGIES].optionValue

void Rando::ObjectBehavior::ModifyBoggyBehavior(void* boggyActor) {
    if (!IS_RANDO && !OPTION_ENABLED) {
        return;
    }

    Actor* actor = (Actor*)boggyActor;

    switch (actor->actor_info->actorId) {
        case ACTOR_160_BOGGY_1:
        case ACTOR_181_SCARF_SLED:
            if (RANDO_SAVE_CHECKS[RC_FP_JIGGY_SLED_TO_BOGGY].eligible) {
                marker_despawn(actor->marker);
                if (actor->actor_info->actorId == ACTOR_160_BOGGY_1) {
                    func_80387760(actor->marker);
                }
            }
            break;
        case ACTOR_C8_BOGGY_2:
            if (!RANDO_SAVE_CHECKS[RC_FP_JIGGY_SLED_TO_BOGGY].eligible) {
                marker_despawn(actor->marker);
            }
            break;
        case 0x33D:
            if (!RANDO_SAVE_CHECKS[RC_FP_JIGGY_BOGGY_RACE_2].eligible) {
                marker_despawn(actor->marker);
            }
            break;
        default:
            break;
    }
}
