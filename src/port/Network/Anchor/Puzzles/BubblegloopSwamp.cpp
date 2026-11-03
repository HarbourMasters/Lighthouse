#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <algorithm>

#include "functions.h"
extern "C" {
#include "BGS/ch/croctus.h"
#include "BGS/ch/frogminigame.h"
#include "BGS/ch/pinkegg.h"
#include "BGS/ch/tanktup.h"
#include "BGS/ch/yellowflibbit.h"

extern ActorMarker* bgs_D_803907B8[5];

void chCroctus_activateNextCroctus(ActorMarker* caller, enum asset_e text_id, s32 arg2);
void chFrogMinigame_setState(Actor* arg0, u32 next_state);
void chFrogMinigame_stopFlibbitFightMusic(Actor* arg0);
void chPinkEgg_collision(ActorMarker* self, ActorMarker* other_marker);
void chTanktupLeg_despawn(ActorMarker* self, ActorMarker* other_marker);
void chYellowFlibbit_setState(Actor* self, s32 next_state);
}

#include "port/Patches/Patches.h"

// The heads a teammate fed, a bit per actorTypeSpecificField, until the map reloads. A feed's scene outlasts the
// update that replays it: the sounds, camera and dialogs come from timers and dialog callbacks.
static s32 sCroctusTeammateFeeds = 0;
// The teammate's feeds whose ding has played, a bit per head: what each head plays after it is the glitter.
static s32 sCroctusTeammateDings = 0;
// The feeds already in the record when we arrived, which catch up without sounds or dialogs; -1 until a head updates.
static s32 sCroctusArrivalFeeds = -1;
// A teammate's leg is playing out on Tanktup's body.
static bool sTanktupReplay = false;
// The legs a teammate knocked in, a bit per leg, for the sounds they make as they retract.
static s32 sTanktupTeammateLegs = 0;
// Tanktup raised his head for a teammate's solve: the pose, without the presentation.
static bool sTanktupCatchUp = false;

static bool IsCroctusTeammateFeed(Actor* croctus) {
    return sCroctusTeammateFeeds & (1 << (croctus->actorTypeSpecificField - 1));
}

static bool IsCroctusArrivalFeed(Actor* croctus) {
    return sCroctusArrivalFeeds & (1 << (croctus->actorTypeSpecificField - 1));
}

// The head a feed of this one wakes, or this one for the last.
static Actor* CroctusNextHead(Actor* croctus) {
    const s32 field = croctus->actorTypeSpecificField;
    ActorMarker* next = field < 5 ? bgs_D_803907B8[field] : NULL;
    return next != NULL ? marker_getActor(next) : croctus;
}

// Where a teammate's feed sounds: its ding first and its crunch once fed at this head, its glitter at the next.
static Actor* CroctusSoundSource(Actor* croctus) {
    const s32 bit = 1 << (croctus->actorTypeSpecificField - 1);
    if (croctus->state == CHCROCTUS_STATE_5_FED) {
        return croctus;
    }
    if (!(sCroctusTeammateDings & bit)) {
        sCroctusTeammateDings |= bit;
        return croctus;
    }
    return CroctusNextHead(croctus);
}

static bool IsTanktupTeammateLeg(Actor* actor) {
    return actor->modelCacheIndex >= ACTOR_E9_TANKTUP_LEG_FL && actor->modelCacheIndex <= ACTOR_EC_TANKTUP_LEG_BR &&
           (sTanktupTeammateLegs & (1 << actor->unk10_12));
}

// A teammate broke this layer, or the whole chain.
static void PinkEggUpdate(IEvent* event) {
    auto* ev = (ShouldActorUpdate*)event;
    Actor* egg = ev->actor;
    if (!egg->initialized || egg->state == CH_PINK_EGG_STATES_3_BREAKING) {
        return;
    }
    if (jiggyscore_isSpawned(JIGGY_21_BGS_PINKEGG)) {
        marker_despawn(egg->marker);
        *ev->should = false;
    } else if (port_puzzleStep_get(ANCHOR_PUZZLE_BGS_PINKEGG) & (1 << ((ActorLocal_PinkEgg*)&egg->local)->actorId)) {
        chPinkEgg_collision(egg->marker, NULL);
        *ev->should = false;
    }
}

static void TanktupLegUpdate(IEvent* event) {
    Actor* leg = ((ShouldActorUpdate*)event)->actor;
    if (leg->initialized && leg->state == 1 && leg->marker->collidable &&
        (port_puzzleStep_get(ANCHOR_PUZZLE_BGS_TANKTUP) & (1 << leg->unk10_12))) {
        sTanktupReplay = true;
        sTanktupTeammateLegs |= 1 << leg->unk10_12;
        chTanktupLeg_despawn(leg->marker, NULL);
    }
}

void AnchorPuzzles_RegisterBubblegloopSwamp(bool connected) {
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1FA_CROCTUS, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* croctus = ev->actor;
        if (sCroctusArrivalFeeds < 0) {
            sCroctusArrivalFeeds = port_puzzleStep_get(ANCHOR_PUZZLE_BGS_CROCTUS);
        }
        if (!croctus->volatile_initialized || croctus->state == CHCROCTUS_STATE_5_FED ||
            croctus->state == CHCROCTUS_STATE_6_DESPAWN) {
            return;
        }
        // The team finished the chain.
        if (jiggyscore_isSpawned(JIGGY_22_CROCTUS)) {
            marker_despawn(croctus->marker);
            *ev->should = false;
            return;
        }
        // A teammate fed the open head. Feeding the fifth spawns the jiggy, which the check above covers.
        const s32 field = croctus->actorTypeSpecificField;
        if (field < 5 && !croctus->unk38_31 && croctus->marker->propPtr->isNotFeatherEggOrNote &&
            (port_puzzleStep_get(ANCHOR_PUZZLE_BGS_CROCTUS) & (1 << (field - 1)))) {
            sCroctusTeammateFeeds |= 1 << (field - 1);
            croctus->unk38_31 = true;
        }
    });

    // The first head's dialog leads on to the second; a teammate's feed goes straight there.
    COND_VB_SHOULD(VB_CROCTUS_FIRST_FEED_DIALOG, EVENT_PRIORITY_NORMAL, connected, {
        Actor* croctus = va_arg(args, Actor*);
        if (IsCroctusTeammateFeed(croctus)) {
            chCroctus_activateNextCroctus(croctus->marker, ASSET_C86_DIALOG_CROCTUS_FIRST_SUCCESS, 0);
            *should = false;
        }
    });

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_E8_TANKTUP, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* body = ((ShouldActorUpdate*)event)->actor;
        if (!body->volatile_initialized) {
            sTanktupCatchUp = false;
            sTanktupReplay = false;
            // Legs a teammate already knocked in don't spawn.
            const s32 bits = port_puzzleStep_get(ANCHOR_PUZZLE_BGS_TANKTUP);
            for (s32 i = 0; i < 4; i++) {
                if (bits & (1 << i)) {
                    body->tanktup.unk0[i] = 1;
                }
            }
            return;
        }
        // The team's record has all four legs in while ours stands idle: his head comes up too. A collected jiggy alone
        // leaves his legs out, since a save keeps none and he can be solved again.
        if (Anchor::GetInstance()->IsWorldSyncActive() && body->state == CH_TANKTUP_STATE_1_UNK &&
            !body->tanktup.unk10 && body->tanktup.unk0[0] && body->tanktup.unk0[1] && body->tanktup.unk0[2] &&
            body->tanktup.unk0[3]) {
            sTanktupCatchUp = true;
            subaddie_set_state_with_direction(body, CH_TANKTUP_STATE_3_UNK, 0.0f, -1);
            actor_playAnimationOnce(body);
        }
    });
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_E9_TANKTUP_LEG_FL, EVENT_PRIORITY_NORMAL, connected, TanktupLegUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_EA_TANKTUP_LEG_BL, EVENT_PRIORITY_NORMAL, connected, TanktupLegUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_EB_TANKTUP_LEG_FR, EVENT_PRIORITY_NORMAL, connected, TanktupLegUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_EC_TANKTUP_LEG_BR, EVENT_PRIORITY_NORMAL, connected, TanktupLegUpdate);

    // Tanktup's jiggy comes out once.
    COND_VB_SHOULD(VB_SPAWN_PUZZLE_JIGGY, EVENT_PRIORITY_NORMAL, connected, {
        Actor* actor = va_arg(args, Actor*);
        if (actor->modelCacheIndex == ACTOR_E8_TANKTUP && jiggyscore_isSpawned(JIGGY_26_BGS_TANKTUP)) {
            *should = false;
        }
    });

    // A teammate's step replayed here goes without our camera and dialogs, plays its jingles only within earshot, and
    // leaves its jiggy to them; the catch-up's pose has no jingles either. Always on: a replay that starts while
    // connected plays out if the connection drops.
    COND_VB_SHOULD(VB_PLAY_PUZZLE_CUTSCENE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if ((actor->modelCacheIndex == ACTOR_1FA_CROCTUS && IsCroctusTeammateFeed(actor)) ||
            (actor->modelCacheIndex == ACTOR_E8_TANKTUP && (sTanktupCatchUp || sTanktupReplay))) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_SPAWN_PUZZLE_JIGGY, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (actor->modelCacheIndex == ACTOR_E8_TANKTUP && sTanktupReplay) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if ((actor->modelCacheIndex == ACTOR_1FA_CROCTUS && IsCroctusTeammateFeed(actor) &&
             (IsCroctusArrivalFeed(actor) || !Anchor_InEarshot(CroctusSoundSource(actor)->position))) ||
            (IsTanktupTeammateLeg(actor) && !Anchor_InEarshot(actor->position)) ||
            (actor->modelCacheIndex == ACTOR_E8_TANKTUP &&
             (sTanktupCatchUp || (sTanktupReplay && !Anchor_InEarshot(actor->position))))) {
            *should = false;
        }
    });
    // Always on, with the hooks that read them. One of our legs going in ends a teammate's replay: a replayed leg's bit
    // is already in sTanktupTeammateLegs when it fires.
    COND_HOOK(OnPuzzleStep, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnPuzzleStep*)event;
        if (ev->puzzleId == ANCHOR_PUZZLE_BGS_TANKTUP && (ev->bits & ~sTanktupTeammateLegs)) {
            sTanktupReplay = false;
        }
    });
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        sCroctusTeammateFeeds = 0;
        sCroctusTeammateDings = 0;
        sCroctusArrivalFeeds = -1;
        sTanktupReplay = false;
        sTanktupTeammateLegs = 0;
        sTanktupCatchUp = false;
    });

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_5B_PINK_EGG_LARGEST, EVENT_PRIORITY_NORMAL, connected, PinkEggUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_ED_PINK_EGG_LARGE, EVENT_PRIORITY_NORMAL, connected, PinkEggUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_EE_PINK_EGG_MEDIUM, EVENT_PRIORITY_NORMAL, connected, PinkEggUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_EF_PINK_EGG_SMALL, EVENT_PRIORITY_NORMAL, connected, PinkEggUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_F0_PINK_EGG_SMALLEST, EVENT_PRIORITY_NORMAL, connected, PinkEggUpdate);

    // A teammate won the Flibbits' fight and their jiggy spawned here. Ours goes to the state vanilla gives it when the
    // jiggy is already out on arrival. Our win is past idle and battle before its jiggy spawns.
    COND_ID_HOOK(
        ShouldActorUpdate, ACTOR_136_YELLOW_FLIBBIT_CONTROLLER, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
            Actor* fight = ((ShouldActorUpdate*)event)->actor;
            if (fight->volatile_initialized &&
                (fight->state == CH_FROG_MINIGAME_STATE_1_IDLE || fight->state == CH_FROG_MINIGAME_STATE_3_BATTLE) &&
                jiggyscore_isSpawned(JIGGY_24_BGS_FLIBBITS)) {
                chFrogMinigame_stopFlibbitFightMusic(fight);
                chFrogMinigame_setState(fight, CH_FROG_MINIGAME_STATE_6_DESPAWN);
            }
        });
    // The Flibbits still out die, and the ones that haven't come up go.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_137_FLIBBIT_YELLOW, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* flibbit = ev->actor;
        if (!flibbit->volatile_initialized || !jiggyscore_isSpawned(JIGGY_24_BGS_FLIBBITS)) {
            return;
        }
        if (flibbit->state < CH_YELLOW_FLIBBIT_STATE_3_IDLE_EXIST) {
            marker_despawn(flibbit->marker);
            *ev->should = false;
        } else if (flibbit->state < CH_YELLOW_FLIBBIT_STATE_9_DIE) {
            chYellowFlibbit_setState(flibbit, CH_YELLOW_FLIBBIT_STATE_9_DIE);
        }
    });

    // Tiptup read our round when he set up; a teammate's later progress stays.
    COND_VB_SHOULD(VB_TIPTUP_SAVE_ROUND, EVENT_PRIORITY_NORMAL, connected, {
        u8* round = va_arg(args, u8*);
        *round = std::max<s32>(*round, fileProgressFlag_getN(FILEPROG_0_TIPTUP_MINIGAME_PROGRESS, 2));
    });
}
