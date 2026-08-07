#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"

#include "prop.h"

extern "C" {
void marker_despawn(ActorMarker* marker);
void chGobi2_setState(Actor* thisx, s32 next_state);
void chGobi3_setState(Actor* thisx, s32 next_state);
}

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_JIGGIES].optionValue

void Rando::ObjectBehavior::ModifyGobiBehavior(void* gobiActor) {
    if (!IS_RANDO && !OPTION_ENABLED) {
        return;
    }

    Actor* actor = (Actor*)gobiActor;

    switch (actor->actor_info->actorId) {
        case ACTOR_12E_GOBI_1:
        case ACTOR_12F_GOBI_ROPE:
        case ACTOR_130_GOBI_ROCK:
            if (RANDO_SAVE_CHECKS[RC_GV_JIGGY_GOBI_1].eligible) {
                marker_despawn(actor->marker);
            }
            break;
        case ACTOR_131_GOBI_2:
            if (RANDO_SAVE_CHECKS[RC_GV_JIGGY_GOBI_1].eligible && !RANDO_SAVE_CHECKS[RC_GV_JIGGY_GOBI_2].eligible) {
                if (actor->state == 1) {
                    chGobi2_setState(actor, 2);
                    actor->marker->collidable = true;
                }
            } else {
                if (actor->state != 1) {
                    chGobi2_setState(actor, 1);
                    actor->marker->propPtr->unk8_3 = false;
                    actor->marker->collidable = false;
                }
            }
            break;
        case ACTOR_135_GOBI_3:
            if (RANDO_SAVE_CHECKS[RC_GV_JIGGY_GOBI_2].eligible) {
                if (actor->state == 0) {
                    chGobi3_setState(actor, 2);
                    actor->marker->collidable = true;
                }
            } else {
                if (actor->state != 0) {
                    chGobi3_setState(actor, 0);
                    actor->marker->propPtr->unk8_3 = false;
                    actor->marker->collidable = false;
                }
            }
            break;
        default:
            break;
    }
}
