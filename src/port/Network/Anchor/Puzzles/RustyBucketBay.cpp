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
#include "RBB/crane_jiggycage.h"

extern ActorArray* suBaddieActorArray;
void __chWhistleCtrl_setState(Actor* actor, s32 arg1);
void chCageUpSwitch_setState(Actor* self, s32 next_state);
void chTNTDownSwitch_setState(Actor* self, s32 next_state);
void func_8038711C(s32 arg0, s32 arg1);
void func_80387AC0(void);
void func_80387B24(void);
void RBB_func_80387960(f32 arg0);
}

#include "port/Patches/Patches.h"

static bool sCraneReplay = false;
// What's left of a teammate's raised cage, in hourglass ticks.
static s32 sCraneTicks = 0;

static Actor* FindCageSwitch() {
    if (suBaddieActorArray == NULL) {
        return NULL;
    }
    return actorArray_findActorFromActorId(ACTOR_173_RBB_CAGE_UP_SWITCH);
}

// State 2's press and rise, done at once.
static void RaiseCageNow() {
    f32 rest[3] = { 0.0f, 0.0f, 0.0f };
    f32 pressed[3] = { 0.0f, 0.0f, -40.0f };
    f32 raised[3] = { 0.0f, 450.0f, 0.0f };
    for (s32 id : { 0x19C, 0x19D }) {
        if (Struct70s* part = func_8034C528(id)) {
            func_8034DDF0(&part->type_6D, rest, pressed, 0.0f, 1);
        }
    }
    if (Struct70s* cage = func_8034C528(0x19A)) {
        func_8034DDF0(&cage->type_6D, rest, raised, 0.0f, 1);
    }
    func_8038711C(0x19D, 0x1F4);
}

// A teammate raised (2) or lowered (4) RBB's jiggy cage, or had it up with ticks left as we arrived (3).
void AnchorPuzzles_ReplayJiggyCrane(int32_t stage, int32_t ticks) {
    Actor* button = FindCageSwitch();
    if (button == NULL) {
        return;
    }
    if (stage == CH_CAGE_UP_SWITCH_STATE_3_TIMED_RAISED_CAGE &&
        button->state == CH_CAGE_UP_SWITCH_STATE_1_NOT_PRESSED) {
        sCraneReplay = true;
        RaiseCageNow();
        chCageUpSwitch_setState(button, stage);
        sCraneTicks = ticks;
    } else if ((stage == CH_CAGE_UP_SWITCH_STATE_2_RAISING_CAGE &&
                button->state == CH_CAGE_UP_SWITCH_STATE_1_NOT_PRESSED) ||
               (stage == CH_CAGE_UP_SWITCH_STATE_4_LOWER_CAGE &&
                button->state == CH_CAGE_UP_SWITCH_STATE_3_TIMED_RAISED_CAGE)) {
        sCraneReplay = true;
        chCageUpSwitch_setState(button, stage);
    }
}

void AnchorPuzzles_ResendJiggyCrane() {
    Actor* button = FindCageSwitch();
    if (button == NULL) {
        return;
    }
    if (button->state == CH_CAGE_UP_SWITCH_STATE_2_RAISING_CAGE) {
        Anchor::GetInstance()->SendPacket_JiggyCrane(button->state);
    } else if (button->state == CH_CAGE_UP_SWITCH_STATE_3_TIMED_RAISED_CAGE) {
        Anchor::GetInstance()->SendPacket_JiggyCrane(
            button->state, sCraneReplay ? sCraneTicks : item_getCount(ITEM_0_HOURGLASS_TIMER));
    }
}

static bool IsCraneTeammateStep(Actor* actor) {
    return actor != NULL && actor->modelCacheIndex == ACTOR_173_RBB_CAGE_UP_SWITCH && sCraneReplay;
}

// The cage hangs about 3000 from its switch; its sounds go by the middle of its bounds.
static bool CageInEarshot() {
    Struct70s* cage = func_8034C528(0x19A);
    if (cage == NULL) {
        return false;
    }
    const Struct6Ds* bounds = &cage->type_6D;
    f32 center[3];
    for (s32 i = 0; i < 3; i++) {
        center[i] = (bounds->unk14[i] + bounds->unk1A[i]) / 2.0f;
    }
    return Anchor_InEarshot(center);
}

void AnchorPuzzles_RegisterRustyBucketBay(bool connected) {
    // Always on, with the listeners below: a replay that starts while connected plays out, and ends as the cage comes
    // back down.
    COND_HOOK(OnCageSwitchState, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnCageSwitchState*)event;
        if (ev->state == CH_CAGE_UP_SWITCH_STATE_1_NOT_PRESSED) {
            sCraneReplay = false;
        } else if (!sCraneReplay && Anchor::GetInstance()->isConnected &&
                   (ev->state == CH_CAGE_UP_SWITCH_STATE_2_RAISING_CAGE ||
                    ev->state == CH_CAGE_UP_SWITCH_STATE_4_LOWER_CAGE)) {
            Anchor::GetInstance()->SendPacket_JiggyCrane(ev->state);
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_CUTSCENE, EVENT_PRIORITY_NORMAL, true, {
        if (IsCraneTeammateStep(va_arg(args, Actor*))) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsCraneTeammateStep(actor) && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_RBB_CRANE_CAGE_SOUND, EVENT_PRIORITY_NORMAL, true, {
        if (IsCraneTeammateStep(va_arg(args, Actor*)) && !CageInEarshot()) {
            *should = false;
        }
    });
    // The hourglass is the presser's: a teammate's cage counts down here at its rate without showing it, and comes down
    // on their word or ours, whichever is first.
    COND_VB_SHOULD(VB_RBB_CRANE_HOURGLASS_START, EVENT_PRIORITY_NORMAL, true, {
        if (sCraneReplay) {
            *should = false;
            sCraneTicks = va_arg(args, s32);
        }
    });
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_173_RBB_CAGE_UP_SWITCH, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* button = ev->actor;
        if (sCraneReplay && button->volatile_initialized &&
            button->state == CH_CAGE_UP_SWITCH_STATE_3_TIMED_RAISED_CAGE) {
            sCraneTicks -= (s32)(time_getDelta() * FRAMERATE * 1.1);
            if (sCraneTicks <= 0) {
                chCageUpSwitch_setState(button, CH_CAGE_UP_SWITCH_STATE_4_LOWER_CAGE);
            }
            *ev->should = false;
        }
    });

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
