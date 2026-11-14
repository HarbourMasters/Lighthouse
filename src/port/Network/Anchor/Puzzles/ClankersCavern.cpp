#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h"
extern "C" {
#include "CC/ch/ccgrates.h"
#include "CC/ch/clankerkey.h"
#include "CC/ch/clankertoothext.h"
#include "CC/ma/clanker.h"

void CC_func_80386920(Actor* self, s32 next_state);
void chCCGrate_setNextState(Actor* self, s32 next_state);
void maClanker_raiseClanker(void);
void maClankerRings_endMinigame(void);
void maClankerTooth_setNextState(Actor* self, s32 next_state);
}

#include "port/Patches/Patches.h"

// clankerrings.c's RING_COUNT.
constexpr u8 kClankerRingCount = 9;

// Clanker is lifting for a teammate's key, not ours.
static bool sClankerTeammateRaise = false;
// The teeth a teammate knocked out, a bit per tooth (unk0), for the fanfare and dialog that follow.
static s32 sClankerTeammateTeeth = 0;
static bool sGrateReplay = false;

static bool IsClankerTeammateTooth(Actor* actor) {
    if (actor == NULL || (actor->modelCacheIndex != ACTOR_44_CLANKER_TOKEN_TOOTH_EXTERIOR &&
                          actor->modelCacheIndex != ACTOR_45_CLANKER_JIGGY_TOOTH_EXTERIOR)) {
        return false;
    }
    return sClankerTeammateTeeth & (1 << ((ActorLocal_CC_BF0*)&actor->local)->unk0);
}

static void CCGrateUpdate(IEvent* event) {
    auto* ev = (ShouldActorUpdate*)event;
    Actor* grate = ev->actor;
    if (!BreakObject_IsBroken((s32)gsworld_getMap(), (s32)grate->marker->id, (s32)grate->position[0],
                              (s32)grate->position[1], (s32)grate->position[2])) {
        return;
    }
    if (!grate->volatile_initialized) {
        marker_despawn(grate->marker);
        *ev->should = false;
    } else if (grate->state == CC_GRATES_STATE_1_IDLE) {
        sGrateReplay = true;
        chCCGrate_setNextState(grate, *((ActorLocal_CC_3400*)&grate->local)->unk0);
        sGrateReplay = false;
    }
}

// A teammate's eggs in this tooth. Each tooth has three bits.
static void ClankerToothUpdate(IEvent* event) {
    Actor* tooth = ((ShouldActorUpdate*)event)->actor;
    auto* local = (ActorLocal_CC_BF0*)&tooth->local;
    if (!tooth->volatile_initialized) {
        return;
    }
    const s32 bits = (PuzzleStep_GetBits(ANCHOR_PUZZLE_CC_CLANKER_TEETH) >> ((local->unk0 - 1) * 3)) & 0x7;
    const s32 eggs = (bits & 1) + ((bits >> 1) & 1) + ((bits >> 2) & 1);
    while (tooth->state == CLANKER_TOOTH_STATE_1_UPRIGHT && local->egg_count < eggs) {
        local->egg_count++;
        if (local->egg_count == 3) {
            sClankerTeammateTeeth |= 1 << local->unk0;
            maClankerTooth_setNextState(tooth, CLANKER_TOOTH_STATE_2_FALLING_OVER);
        } else if (Anchor_InEarshot(tooth->position)) {
            coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 28000);
        }
    }
}

void AnchorPuzzles_RegisterClankersCavern(bool connected) {
    // A teammate raised Clanker: he lifts here too, without the key-turner's cutscene.
    COND_HOOK(OnClankerUpdate, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        const s32 state = ((OnClankerUpdate*)event)->state;
        if (state != MACLANKER_STATE_2_LIFTING) {
            sClankerTeammateRaise = false;
        }
        if (state == MACLANKER_STATE_1_IDLE_UNDERWATER && jiggyscore_isSpawned(JIGGY_17_CC_CLANKER_RAISED)) {
            sClankerTeammateRaise = true;
            maClanker_raiseClanker();
        }
    });
    // The lift closes dialog for that cutscene; a teammate's raise leaves ours open. Always on: a raise that starts
    // while connected plays out if the connection drops.
    COND_VB_SHOULD(VB_CLANKER_LIFT_CLOSE_DIALOG, EVENT_PRIORITY_NORMAL, true, {
        if (sClankerTeammateRaise) {
            *should = false;
        }
    });
    // Always on, with the hooks that read them.
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        sClankerTeammateRaise = false;
        sClankerTeammateTeeth = 0;
    });
    // After a teammate's raise, the key snaps to turned.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_3C_CC_KEY, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* key = ((ShouldActorUpdate*)event)->actor;
        if (key->volatile_initialized && key->state == CLANKER_KEY_STATE_1_IDLE &&
            jiggyscore_isSpawned(JIGGY_17_CC_CLANKER_RAISED)) {
            CC_func_80386920(key, CLANKER_KEY_STATE_5_COMPLETED);
        }
    });

    // The three grates (ccgrates.c), whose actor ids have no names.
    COND_ID_HOOK(ShouldActorUpdate, 0x28C, EVENT_PRIORITY_NORMAL, connected, CCGrateUpdate);
    COND_ID_HOOK(ShouldActorUpdate, 0x28D, EVENT_PRIORITY_NORMAL, connected, CCGrateUpdate);
    COND_ID_HOOK(ShouldActorUpdate, 0x28E, EVENT_PRIORITY_NORMAL, connected, CCGrateUpdate);

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_44_CLANKER_TOKEN_TOOTH_EXTERIOR, EVENT_PRIORITY_NORMAL, connected,
                 ClankerToothUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_45_CLANKER_JIGGY_TOOTH_EXTERIOR, EVENT_PRIORITY_NORMAL, connected,
                 ClankerToothUpdate);
    // Always on: a tooth that falls while connected plays out if the connection drops.
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if ((sGrateReplay || IsClankerTeammateTooth(actor)) && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_DIALOG, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsClankerTeammateTooth(actor) && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });

    // A teammate finished the rings: our run ends. Their water rise plays here, so ours doesn't snap.
    COND_VB_SHOULD(VB_CC_RINGS_ACTIVE, EVENT_PRIORITY_NORMAL, connected, {
        u8* ring = va_arg(args, u8*);
        if (*should && jiggyscore_isSpawned(JIGGY_1C_CC_RINGS) && *ring < kClankerRingCount) {
            maClankerRings_endMinigame();
            *ring = 0;
            *should = false;
        }
    });

    // A teammate's Snippets jiggy is already out; our muties' defeat doesn't bring another.
    COND_VB_SHOULD(VB_SNIPPETS_DROP_JIGGY, EVENT_PRIORITY_NORMAL, connected, {
        if (Anchor_JiggyListHasObject(JIGGY_16_CC_SNIPPETS)) {
            *should = false;
        }
    });
}
