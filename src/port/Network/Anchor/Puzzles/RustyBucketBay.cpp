#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h"
extern "C" {
#include "RBB/ch/bossboombox.h"
#include "RBB/ch/enginefanswitch.h"
#include "RBB/ch/propellorswitch.h"
#include "RBB/ch/whistlectrl.h"
#include "RBB/crane_boombox.h"

void __chWhistleCtrl_setState(Actor* actor, s32 arg1);
void chTNTDownSwitch_setState(Actor* self, s32 next_state);
void func_80387AC0(void);
void func_80387B24(void);
void RBB_func_80387960(f32 arg0);
}

#include "port/Patches/Patches.h"

void AnchorPuzzles_RegisterRustyBucketBay(bool connected) {
    // The engine fans slow on map flag 0, which syncs live but doesn't last past a reload, so the team keeps it as a
    // puzzle bit. A teammate's press, live or kept, lowers our switch.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1BE_RBB_GREY_PROPELLOR_SWITCH, EVENT_PRIORITY_NORMAL, connected,
                 [](IEvent* event) {
                     Actor* button = ((ShouldActorUpdate*)event)->actor;
                     if (!button->volatile_initialized) {
                         return;
                     }
                     if (mapSpecificFlags_get(RBB_MAIN_SPECIFIC_FLAG_0_PROPELLER)) {
                         port_puzzleStep_orBits(ANCHOR_PUZZLE_RBB_ENGINE_FANS, 1);
                     }
                     if (button->state == CH_RBB_GREY_PROPELLER_SWITCH_STATE_1_NOT_PRESSED &&
                         (mapSpecificFlags_get(RBB_MAIN_SPECIFIC_FLAG_0_PROPELLER) ||
                          (port_puzzleStep_get(ANCHOR_PUZZLE_RBB_ENGINE_FANS) & 1))) {
                         button->position_y -= 35.0f;
                         button->state = CH_RBB_GREY_PROPELLER_SWITCH_STATE_2_PRESSED;
                         mapSpecificFlags_set(RBB_MAIN_SPECIFIC_FLAG_0_PROPELLER, true);
                     }
                 });

    // A teammate opened the entrance (level flag 0x2D): our TNT switch snaps to done.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_174_RBB_TNT_DOWN_SWITCH, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* button = ((ShouldActorUpdate*)event)->actor;
        if (button->volatile_initialized && button->state == CH_TNT_DOWN_SWITCH_STATE_1_NOT_PRESSED &&
            levelSpecificFlags_get(LEVEL_FLAG_2D_RBB_UNKNOWN)) {
            func_80387AC0();
            RBB_func_80387960(0.0f);
            func_80387B24();
            chTNTDownSwitch_setState(button, 3);
        }
    });

    // A teammate's press arrives as the level flag: ours shows pressed, without the press's warp.
    COND_ID_HOOK(
        ShouldActorUpdate, ACTOR_176_RBB_GREEN_PROPELLOR_SWITCH, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
            Actor* button = ((ShouldActorUpdate*)event)->actor;
            auto* local = (ActorLocal_RBB_3CB0*)&button->local;
            if (button->volatile_initialized && button->state == CH_RBB_GREEN_PROPELLER_SWITCH_STATE_1_NOT_PRESSED &&
                levelSpecificFlags_get(local->unk0->unkA)) {
                button->position_y -= 40.0f;
                button->state = CH_RBB_GREEN_PROPELLER_SWITCH_STATE_2_PRESSED;
            }
        });

    // A teammate's whistle code spawned the jiggy: ours stops listening.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1C5_WHISTLE_CTRL, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* ctrl = ((ShouldActorUpdate*)event)->actor;
        if (ctrl->volatile_initialized && ctrl->state == CH_RBB_WHISTLE_CTRL_STATE_1_AWAITING_PLAYER &&
            jiggyscore_isSpawned(JIGGY_54_RBB_WHISTLE)) {
            __chWhistleCtrl_setState(ctrl, CH_RBB_WHISTLE_CTRL_STATE_2_COMPLETE);
        }
    });

    // A teammate beat the boss and his jiggy is out: ours leaves before our fight starts.
    COND_ID_HOOK(
        ShouldActorUpdate, ACTOR_281_BOSS_BOOM_BOX_LARGEST, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
            auto* ev = (ShouldActorUpdate*)event;
            Actor* boss = ev->actor;
            if (boss->volatile_initialized && boss->state == CH_BOSS_BOOM_BOX_STATE_1_WAITING_FOR_PLAYER &&
                jiggyscore_isSpawned(JIGGY_56_RBB_BOSS_BOOM_BOX) && !volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME) &&
                !volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE)) {
                marker_despawn(boss->marker);
                *ev->should = false;
            }
        });
}
