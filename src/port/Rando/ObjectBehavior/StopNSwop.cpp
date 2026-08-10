#include "ObjectBehavior.h"
#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"

extern "C" {
void marker_despawn(ActorMarker* marker);
}

#define OPTION_ENABLED RANDO_SAVE_OPTIONS[RO_SHUFFLE_STOP_N_SWOP].optionValue

void ModifyStopNSwopWorldBehavior(void* snsActor) {
    Actor* actor = (Actor*)snsActor;
    switch (actor->actor_info->actorId) {
        case 0x253: // FP Wozza's Cave Ice Wall
        case 0x191: // MMM Cellar SNS Entrance
        case ACTOR_243_GV_SNS_CHAMBER_DOOR:
            marker_despawn(actor->marker);
            break;
        case ACTOR_25C_SHARKFOOD_ISLAND:
            if (actor->position_y != 700.0f) {
                actor->position_y = 700.0f;
            }
            break;
        default:
            break;
    }
}

void RegisterRandoStopNSwap() {
    COND_HOOK(OnActorTick, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnActorTick* ev = (OnActorTick*)event;
        ModifyStopNSwopWorldBehavior(ev->actor);
    });

    COND_VB_SHOULD(VB_OVERRIDE_SNS_MAP_CHECK, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, { *should = true; });

    COND_VB_SHOULD(VB_OVERRIDE_TIMED_DIALOGUE, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, {
        asset_e textId = (asset_e)va_arg(args, int);

        if (textId == ASSET_DB3_DIALOG_SNS_EGG_1_TEXT || textId == ASSET_DB5_DIALOG_ICE_KEY_TEXT ||
            textId == ASSET_DB2_DIALOG_MUMBO_MISTAKE_2) {
            *should = true;
        }
    });

    COND_HOOK(OnSnSItemState, EVENT_PRIORITY_NORMAL, IS_RANDO && OPTION_ENABLED, [](IEvent* event) {
        OnSnSItemState* ev = (OnSnSItemState*)event;

        event->Cancelled = true;
        ev->result = false;
    });
}

static RegisterShipInitFunc initFunc(RegisterRandoStopNSwap, { "IS_RANDO" });
