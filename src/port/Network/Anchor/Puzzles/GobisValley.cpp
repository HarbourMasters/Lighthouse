#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h"
extern "C" {
#include "GV/ch/buriedpyramid.h"
#include "GV/pyramid_raise.h"

extern ActorMarker* D_80390C28[5];
void __matchingGame_setState(s32 next_state);
void chBuriedPyramid_setRaisedAmount(ActorMarker* this_marker, s32 arg1);
void chGobi2_setState(Actor* self, s32 next_state);
void chJinxyHeadSandybutt_setState(Actor* self, s32 next_state);
}

// Set while the pyramid rises for a teammate.
static bool sPyramidReplay = false;

static bool IsTeammateStep(Actor* actor) {
    if (actor == NULL) {
        return false;
    }
    switch (actor->modelCacheIndex) {
        case ACTOR_31D_SANDYBUTT_PYRAMID:
            return sPyramidReplay;
        case ACTOR_1F7_JINXY:
            return Anchor_WasMapFlagSetRemotely(0) || Anchor_WasMapFlagSetRemotely(1);
        case ACTOR_6D_GV_BANJO_DOOR:
            return Anchor_WasMapFlagSetRemotely(0) && Anchor_WasMapFlagSetRemotely(1);
        case ACTOR_142_GV_STAR_HATCH:
            return Anchor_WasMapFlagSetRemotely(5);
        case ACTOR_143_GV_KAZOOIE_DOOR:
            return Anchor_WasMapFlagSetRemotely(6);
        default:
            return false;
    }
}

// Each client shuffles which Ancient One rises for which ring.
static s32 AncientOnesFound() {
    s32 found = 0;
    for (s32 flag = 7; flag < 0xC && mapSpecificFlags_get(flag); flag++) {
        found++;
    }
    return found;
}

static bool GobiFreedByTeammate() {
    return jiggyscore_isSpawned(JIGGY_44_GV_GOBI_1) && !volatileFlag_get(VOLATILE_FLAG_1);
}

// Gobi or his rope, once a teammate freed him.
static void GobiUpdate(IEvent* event) {
    auto* ev = (ShouldActorUpdate*)event;
    Actor* actor = ev->actor;
    if (actor->volatile_initialized && (actor->state == 1 || actor->state == 2) && GobiFreedByTeammate()) {
        marker_despawn(actor->marker);
        *ev->should = false;
    }
}

// A teammate raised the pyramid past this head.
static void JinxyHeadUpdate(IEvent* event) {
    auto* ev = (ShouldActorUpdate*)event;
    Actor* head = ev->actor;
    auto* local = (ActorLocal_GV_6370*)&head->local;
    if (head->volatile_initialized && head->state != CHJINXYHEADSANDYBUTT_STATE_8_COMPLETED &&
        fileProgressFlag_getN(FILEPROG_F8_KING_SANDYBUTT_PYRAMID_STATE, 2) > *local->unk8) {
        chJinxyHeadSandybutt_setState(head, CHJINXYHEADSANDYBUTT_STATE_8_COMPLETED);
        *ev->should = false;
    }
}

void AnchorPuzzles_RegisterGobisValley(bool connected) {
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_147_ANCIENT_ONE, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* one = ev->actor;
        const s32 ring = one->actorTypeSpecificField;
        if (!Anchor::GetInstance()->IsWorldSyncActive() || D_80390C28[ring - 1] == NULL) {
            return;
        }
        if (jiggyscore_isSpawned(JIGGY_46_GV_ANCIENT_ONES)) {
            marker_despawn(one->marker);
            *ev->should = false;
            return;
        }
        // The team found this one's ring: it rises and sinks, and the next shows. Our own last ring waits on its
        // dialog.
        if (one->state == 1 && ring <= AncientOnesFound() && !gcdialog_hasCurrentTextId()) {
            subaddie_set_state_with_direction(one, 2, 0.0f, 1);
            actor_playAnimationOnce(one);
            if (ring < 5 && D_80390C28[ring]) {
                D_80390C28[ring]->propPtr->isNotFeatherEggOrNote = true;
            }
            // Only a ring found live sounds: flags 7 to 0xB, one per ring, marked when a teammate's lands.
            if (Anchor_WasMapFlagSetRemotely(6 + ring) && Anchor_InEarshot(one->position)) {
                if (ring < 5) {
                    coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 28000);
                } else {
                    coMusicPlayer_playMusic(COMUSIC_2D_PUZZLE_SOLVED_FANFARE, 0x7fff);
                }
            }
        }
    });
    // One we haven't seen yet still runs, to catch up to the rings a teammate found.
    COND_VB_SHOULD(VB_ANCIENT_ONE_UNSEEN, EVENT_PRIORITY_NORMAL, connected, {
        Actor* one = va_arg(args, Actor*);
        if (*should && D_80390C28[one->actorTypeSpecificField - 1] != NULL &&
            Anchor::GetInstance()->IsWorldSyncActive() && AncientOnesFound() > 0) {
            *should = false;
        }
    });

    // A teammate raised King Sandybutt's pyramid further.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_31D_SANDYBUTT_PYRAMID, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* pyramid = ((ShouldActorUpdate*)event)->actor;
        auto* local = (ActorLocal_chBuriedPyramid*)&pyramid->local;
        if (pyramid->volatile_initialized && pyramid->state == 1) {
            const s32 raised = fileProgressFlag_getN(FILEPROG_F8_KING_SANDYBUTT_PYRAMID_STATE, 2);
            if (raised > local->raised_state) {
                sPyramidReplay = true;
                chBuriedPyramid_setRaisedAmount(pyramid->marker, raised);
                sPyramidReplay = false;
                if (raised == 3 && Anchor_InEarshot(pyramid->position)) {
                    coMusicPlayer_playMusic(COMUSIC_2D_PUZZLE_SOLVED_FANFARE, 0x7fff);
                }
            }
        }
    });
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_285_JINXY_HEAD_SANDYBUTT_1, EVENT_PRIORITY_NORMAL, connected,
                 JinxyHeadUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_286_JINXY_HEAD_SANDYBUTT_2, EVENT_PRIORITY_NORMAL, connected,
                 JinxyHeadUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_287_JINXY_HEAD_SANDYBUTT_3, EVENT_PRIORITY_NORMAL, connected,
                 JinxyHeadUpdate);

    // Jinxy's door. Its open flag (0x10) doesn't last past a reload, so the team keeps it as a puzzle bit.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_6D_GV_BANJO_DOOR, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        if (mapSpecificFlags_get(0x10)) {
            PuzzleStep_OrBits(ANCHOR_PUZZLE_GV_JINXY_DOOR, 1);
        } else if (PuzzleStep_GetBits(ANCHOR_PUZZLE_GV_JINXY_DOOR) & 1) {
            mapSpecificFlags_set(0x10, true);
            mapSpecificFlags_set(2, true); // the helped dialog is the opener's
        }
    });

    // A teammate won the matching game and its jiggy is out: ours stops, or the hourglass times us out.
    COND_VB_SHOULD(VB_MATCHING_GAME_IDLE, EVENT_PRIORITY_NORMAL, connected, {
        const s32 state = va_arg(args, s32);
        if ((state == 1 || state == 2) && !volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME) &&
            jiggyscore_isSpawned(JIGGY_40_GV_MATCHING_GAME)) {
            __matchingGame_setState(0);
            *should = true;
        }
    });

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_12E_GOBI_1, EVENT_PRIORITY_NORMAL, connected, GobiUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_12F_GOBI_ROPE, EVENT_PRIORITY_NORMAL, connected, GobiUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_130_GOBI_ROCK, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* rock = ev->actor;
        if (rock->volatile_initialized && rock->state == 1 && GobiFreedByTeammate()) {
            marker_despawn(rock->marker);
            *ev->should = false;
        }
    });

    // A teammate's Gobi already watered Trunker and the jiggy is out.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_131_GOBI_2, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* gobi = ev->actor;
        if (gobi->volatile_initialized && (gobi->state == 1 || gobi->state == 2 || gobi->state == 8) &&
            jiggyscore_isSpawned(JIGGY_45_GV_GOBI_2)) {
            chGobi2_setState(gobi, 7);
            *ev->should = false;
        }
    });

    // A teammate busted Gobi for his honeycomb, which is out or collected.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_135_GOBI_3, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* gobi = ev->actor;
        if (gobi->volatile_initialized && (gobi->state == 1 || gobi->state == 2 || gobi->state == 7) &&
            (honeycombscore_get(HONEYCOMB_C_GV_GOBI_3) || Anchor_FindHoneycombMarker(HONEYCOMB_C_GV_GOBI_3) != NULL)) {
            marker_despawn(gobi->marker);
            *ev->should = false;
        }
    });

    // A teammate's step shows its dialogs and plays its sounds only within earshot. Always on: a door that opens while
    // connected plays out if the connection drops.
    COND_VB_SHOULD(VB_PLAY_PUZZLE_DIALOG, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsTeammateStep(actor) && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsTeammateStep(actor) && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });

    // A teammate's press opens our door too, but its countdown takes the hourglass only while it's free.
    COND_VB_SHOULD(VB_GV_DOOR_HOURGLASS_START, EVENT_PRIORITY_NORMAL, connected, {
        static const s32 kDoorFlag[] = { 3, 5, 6 }; // GV_DOOR_CAM_SUN, _STAR and _KAZOOIE
        const s32 door = va_arg(args, s32);
        if (door >= GV_DOOR_CAM_SUN && door <= GV_DOOR_CAM_KAZOOIE && Anchor_WasMapFlagSetRemotely(kDoorFlag[door]) &&
            !item_empty(ITEM_6_HOURGLASS)) {
            *should = false;
        }
    });
}
