#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"

#include "functions.h"
extern "C" {
void __chSnowman_deathCallback(ActorMarker* marker, ActorMarker* other_marker);
void chXmasTreeIce_shatterIce(ActorMarker* marker);
void func_803877A8(ActorMarker* caller, enum asset_e text_id, s32 arg2);
}

#include "port/Patches/Patches.h"

struct CubPresent {
    s32 levelFlag;
    s32 bit;
    s32 kind;
    enum item_e item;
};

// Blue, green and red, in actor id order (bearcub.c's D_80391E80).
static const CubPresent kCubPresents[] = {
    { LEVEL_FLAG_11_FP_UNKNOWN, 0x1, ANCHOR_COLLECTIBLE_PRESENT_BLUE, ITEM_20_BLUE_PRESENT },
    { LEVEL_FLAG_12_FP_UNKNOWN, 0x2, ANCHOR_COLLECTIBLE_PRESENT_GREEN, ITEM_1F_GREEN_PRESENT },
    { LEVEL_FLAG_13_FP_UNKNOWN, 0x4, ANCHOR_COLLECTIBLE_PRESENT_RED, ITEM_21_RED_PRESENT },
};

// The tree's ice is shattering in vanilla's warp-in cutscene.
static bool sTreeIceShattering = false;
// Set while the tree's ice shatters for a teammate.
static bool sTreeIceReplay = false;
// Whether a teammate's Boggy rescue shows its dialog here, decided once as it starts: -1 until then.
static s32 sBoggyDialogHere = -1;
// Whether a snow puzzle's last step to count down was a teammate's, and where.
struct SnowStep {
    bool teammates;
    f32 position[3];
};
// The snowman buttons' and Sir Slushes' last steps, and the one whose manager is winning. The managers' wins pass no
// actor to the puzzle VBs, so they follow sSnowWin.
static SnowStep sSnowButtonStep = {};
static SnowStep sSlushStep = {};
static SnowStep sSnowWin = {};
// Set while a Sir Slush plays out a teammate's death, whose mark is then theirs.
static bool sSlushReplay = false;

static SnowStep& SnowStepOf(s32 puzzleId) {
    return puzzleId == ANCHOR_PUZZLE_FP_SNOWBUTTONS ? sSnowButtonStep : sSlushStep;
}

static void NoteSnowStep(s32 puzzleId, bool teammates, f32 x, f32 y, f32 z) {
    SnowStep& step = SnowStepOf(puzzleId);
    step.teammates = teammates;
    step.position[0] = x;
    step.position[1] = y;
    step.position[2] = z;
}

// The remote bit is 0 unless a teammate's flag landed this visit.
static bool IsBoggyTeammateRescue(Actor* actor) {
    return actor != NULL && actor->modelCacheIndex == ACTOR_160_BOGGY_1 &&
           Anchor_WasMapFlagSetRemotely(FP_SPECIFIC_FLAG_1_UNKNOWN);
}

static bool BoggyDialogHere(Actor* boggy) {
    if (sBoggyDialogHere < 0) {
        sBoggyDialogHere = Anchor_InEarshot(boggy->position);
    }
    return sBoggyDialogHere;
}

static void BearCubUpdate(IEvent* event) {
    Actor* cub = ((ShouldActorUpdate*)event)->actor;
    const CubPresent& present = kCubPresents[cub->modelCacheIndex - ACTOR_1EA_POLAR_BEAR_CUB_BLUE];
    const s32 bits = port_puzzleStep_get(ANCHOR_PUZZLE_FP_PRESENTS);

    // Before the cub sets up: our presents are the team's collected ones less the delivered, since carried presents
    // aren't synced.
    if (!cub->volatile_initialized) {
        const s32 pool = port_carriedSync_collectedCount(present.kind) - ((bits & present.bit) ? 1 : 0);
        if (pool > item_getCount(present.item)) {
            item_adjustByDiffWithoutHud(present.item, pool - item_getCount(present.item));
        }
    }
    if (levelSpecificFlags_get(present.levelFlag)) {
        port_puzzleStep_orBits(ANCHOR_PUZZLE_FP_PRESENTS, present.bit);
    } else if (bits & present.bit) {
        // A teammate gave this cub its present.
        levelSpecificFlags_set(present.levelFlag, true);
        subaddie_set_state_with_direction(cub, 2, 0.001f, 1);
    }
}

void AnchorPuzzles_RegisterFreezeezyPeak(bool connected) {
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1EA_POLAR_BEAR_CUB_BLUE, EVENT_PRIORITY_NORMAL, connected, BearCubUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1EB_POLAR_BEAR_CUB_GREEN, EVENT_PRIORITY_NORMAL, connected, BearCubUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1EC_POLAR_BEAR_CUB_RED, EVENT_PRIORITY_NORMAL, connected, BearCubUpdate);

    // A button a teammate pressed goes down here too, and one pressed here is recorded for them.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_116_FP_SNOWMAN_BUTTON, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* button = ((ShouldActorUpdate*)event)->actor;
        const s32 x = (s32)button->position[0];
        const s32 y = (s32)button->position[1];
        const s32 z = (s32)button->position[2];
        if (button->state == 1 && port_puzzlePos_isMarked(ANCHOR_PUZZLE_FP_SNOWBUTTONS, x, y, z)) {
            subaddie_set_state_with_direction(button, 2, 0.01f, 1);
            NoteSnowStep(ANCHOR_PUZZLE_FP_SNOWBUTTONS, true, button->position[0], button->position[1],
                         button->position[2]);
            maSnowButton_decRemaining();
        }
        if (button->state != 1) {
            port_puzzlePos_mark(ANCHOR_PUZZLE_FP_SNOWBUTTONS, x, y, z);
        }
    });

    // A Sir Slush a teammate beat goes here too: quietly if he hasn't set up, with his death if he has.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_124_SIR_SLUSH, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* slush = ev->actor;
        if (gsworld_getMap() != MAP_27_FP_FREEZEEZY_PEAK ||
            !port_puzzlePos_isMarked(ANCHOR_PUZZLE_FP_SLUSHES, (s32)slush->position[0], (s32)slush->position[1],
                                     (s32)slush->position[2])) {
            return;
        }
        if (slush->volatile_initialized) {
            sSlushReplay = true;
            __chSnowman_deathCallback(slush->marker, NULL);
            sSlushReplay = false;
        } else {
            NoteSnowStep(ANCHOR_PUZZLE_FP_SLUSHES, true, slush->position[0], slush->position[1], slush->position[2]);
            maSnowy_incTotal(); // his skipped setup's count
            maSnowy_decRemaining();
            marker_despawn(slush->marker);
        }
        *ev->should = false;
    });

    // A button press or Sir Slush death fires a mark as it counts down: ours, or a teammate's replayed here. Always on,
    // with the hooks below that read it.
    COND_HOOK(OnPuzzlePosMark, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnPuzzlePosMark*)event;
        if (ev->puzzleId == ANCHOR_PUZZLE_FP_SNOWBUTTONS || ev->puzzleId == ANCHOR_PUZZLE_FP_SLUSHES) {
            NoteSnowStep(ev->puzzleId, sSlushReplay, (f32)ev->x, (f32)ev->y, (f32)ev->z);
        }
    });
    // The snow managers' wins after a teammate's last step: no camera or jiggy here, the fanfare within earshot. Always
    // on: a win a teammate's step set off plays out if the connection drops.
    COND_HOOK(OnPuzzleWin, EVENT_PRIORITY_NORMAL, true,
              [](IEvent* event) { sSnowWin = SnowStepOf(((OnPuzzleWin*)event)->puzzleId); });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_CUTSCENE, EVENT_PRIORITY_NORMAL, true, {
        if (va_arg(args, Actor*) == NULL && sSnowWin.teammates) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_SPAWN_PUZZLE_JIGGY, EVENT_PRIORITY_NORMAL, true, {
        if (va_arg(args, Actor*) == NULL && sSnowWin.teammates) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        if (va_arg(args, Actor*) == NULL && sSnowWin.teammates && !Anchor_InEarshot(sSnowWin.position)) {
            *should = false;
        }
    });

    // A teammate shattered the tree's ice (recorded in the tree's map): the tree lights up, and a run of ours stops.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_15F_XMAS_TREE, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* tree = ((ShouldActorUpdate*)event)->actor;
        if (!levelSpecificFlags_get(LEVEL_FLAG_29_FP_XMAS_TREE_COMPLETE) &&
            (port_puzzleStep_getForMap(MAP_53_FP_CHRISTMAS_TREE, ANCHOR_PUZZLE_FP_TREE_ICE) & 0x1)) {
            levelSpecificFlags_set(LEVEL_FLAG_29_FP_XMAS_TREE_COMPLETE, true);
            if (tree->state == 4) {
                item_set(ITEM_6_HOURGLASS, false);
                mapSpecificFlags_set(FP_SPECIFIC_FLAG_2_XMAS_TREE_SWITCH, false);
            }
        }
    });

    // The tree's ice shatters here too.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_340_XMAS_TREE_ICE, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* ice = ev->actor;
        const bool shattered = port_puzzleStep_get(ANCHOR_PUZZLE_FP_TREE_ICE) & 0x1;
        if (!ice->volatile_initialized) {
            sTreeIceShattering = false;
            if (jiggyscore_isCollected(JIGGY_2F_FP_XMAS_TREE)) {
                return;
            }
            if (shattered) {
                levelSpecificFlags_set(LEVEL_FLAG_29_FP_XMAS_TREE_COMPLETE, true);
                marker_despawn(ice->marker);
                *ev->should = false;
            } else {
                sTreeIceShattering = levelSpecificFlags_get(LEVEL_FLAG_29_FP_XMAS_TREE_COMPLETE);
            }
            return;
        }
        if (shattered && !sTreeIceShattering && ice->unk38_31 == 0) {
            levelSpecificFlags_set(LEVEL_FLAG_29_FP_XMAS_TREE_COMPLETE, true);
            sTreeIceReplay = true;
            chXmasTreeIce_shatterIce(ice->marker);
            sTreeIceReplay = false;
        }
    });
    // Its glass and fanfare play only within earshot.
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, connected, {
        Actor* actor = va_arg(args, Actor*);
        if (sTreeIceReplay && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });

    // A teammate's Wozza jiggy comes out here without the level flag on a later visit.
    COND_VB_SHOULD(VB_WOZZA_JIGGY_DROPPED, EVENT_PRIORITY_NORMAL, connected, {
        if (jiggyscore_isSpawned(JIGGY_32_FP_WOZZA)) {
            *should = true;
        }
    });

    // A teammate's sled crash rescues Boggy here: his animations play without our camera or look-at, his dialog shows
    // only within earshot, and the jiggy comes through JIGGY_SPAWN. Always on, so a rescue that started as a
    // teammate's ends that way if we disconnect mid-scene.
    COND_VB_SHOULD(VB_PLAY_PUZZLE_CUTSCENE, EVENT_PRIORITY_NORMAL, true, {
        if (IsBoggyTeammateRescue(va_arg(args, Actor*))) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_DIALOG, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsBoggyTeammateRescue(actor) && !BoggyDialogHere(actor)) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_SPAWN_PUZZLE_JIGGY, EVENT_PRIORITY_NORMAL, true, {
        if (IsBoggyTeammateRescue(va_arg(args, Actor*))) {
            *should = false;
        }
    });
    // Away from a teammate's rescue no dialog ends it, so Boggy runs off once he's done coughing.
    COND_VB_SHOULD(VB_FP_BOGGY_WAIT_FOR_DIALOG, EVENT_PRIORITY_NORMAL, true, {
        Actor* boggy = va_arg(args, Actor*);
        if (IsBoggyTeammateRescue(boggy) && !BoggyDialogHere(boggy)) {
            func_803877A8(boggy->marker, (enum asset_e)0, 2);
            *should = false;
        }
    });
    // Always on, with the hooks that read them.
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        sBoggyDialogHere = -1;
        sSnowButtonStep = {};
        sSlushStep = {};
    });
}
