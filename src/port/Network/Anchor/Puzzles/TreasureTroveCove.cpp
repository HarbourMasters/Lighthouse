#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"

#include "functions.h"
extern "C" {
#include "TTC/ch/blubber.h"
#include "TTC/ch/nipper.h"

extern u32 chtreasureHunt_puzzleCurrentStep;
extern f32 sChTreasurehunt_stepPositions[6][3];
void __chTreasurehunt_spawnActorForNextStep(void);
void __chTreasurehunt_spawnRedXForNextStep(void);
void __func_80388F88(void);
}

#include "port/Patches/Patches.h"

// A teammate's last delivery is playing out on Blubber within earshot.
static bool sBlubberTeammateFinish = false;

// The Nipper hits the team landed, one bit each.
static s32 NipperHits() {
    const s32 bits = port_puzzleStep_get(ANCHOR_PUZZLE_TTC_NIPPER) & 0x7;
    return (bits & 1) + ((bits >> 1) & 1) + ((bits >> 2) & 1);
}

// The hits this Nipper shows: its lifetime drops from 120 to 80 to 40 as he's hit.
static s32 NipperLocalHits(Actor* nipper) {
    if (nipper->state == CH_NIPPER_STATE_6_DEAD || nipper->state == CH_NIPPER_STATE_7_UNKNOWN) {
        return 3;
    }
    if (nipper->lifetime_value == 80.0f) {
        return 1;
    }
    if (nipper->lifetime_value == 40.0f) {
        return 2;
    }
    return 0;
}

// Replays a teammate's treasure hunt steps, one a frame. The hunt has no actor to poll until the first X is busted.
static void TreasureHuntTick() {
    static f32 sTreasurePosition[3];
    s32 bits = port_puzzleStep_get(ANCHOR_PUZZLE_TTC_XHUNT) & 0x3F;
    s32 sharedStep = 0;

    while (bits & 1) {
        sharedStep++;
        bits >>= 1;
    }
    if ((s32)chtreasureHunt_puzzleCurrentStep >= sharedStep) {
        return;
    }

    if (chtreasureHunt_puzzleCurrentStep < 5) {
        chtreasureHunt_puzzleCurrentStep++;
        __spawnQueue_add_0(__chTreasurehunt_spawnActorForNextStep);
        __spawnQueue_add_0(__chTreasurehunt_spawnRedXForNextStep);
    } else {
        chtreasureHunt_puzzleCurrentStep++;
        if (!jiggyscore_isSpawned(JIGGY_11_TTC_RED_X) && !jiggyscore_isCollected(JIGGY_11_TTC_RED_X)) {
            sTreasurePosition[0] = sChTreasurehunt_stepPositions[5][0];
            sTreasurePosition[1] = sChTreasurehunt_stepPositions[5][1];
            sTreasurePosition[2] = sChTreasurehunt_stepPositions[5][2];
            sTreasurePosition[1] = mapModel_getFloorY(sTreasurePosition);
            __spawnQueue_add_4((GenFunction_4)spawnQueue_actor_f32, ACTOR_F4_BURIED_TREASURE,
                               *(s32*)&sTreasurePosition[0], *(s32*)&sTreasurePosition[1],
                               *(s32*)&sTreasurePosition[2]);
        }
    }
}

void AnchorPuzzles_RegisterTreasureTroveCove(bool connected) {
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_115_BLUBBER, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* blubber = ev->actor;
        const s32 bits = port_puzzleStep_get(ANCHOR_PUZZLE_TTC_BLUBBER);

        // Our deliveries: half the gold, then all of it.
        if (mapSpecificFlags_get(TTC_SPECIFIC_FLAG_0_BLUBBER_UNKNOWN)) {
            port_puzzleStep_orBits(ANCHOR_PUZZLE_TTC_BLUBBER, 0x1);
        }
        if (mapSpecificFlags_get(TTC_SPECIFIC_FLAG_1_UNKNOWN)) {
            port_puzzleStep_orBits(ANCHOR_PUZZLE_TTC_BLUBBER, 0x3);
        }
        if (!blubber->volatile_initialized && (bits & 0x2) && !mapSpecificFlags_get(TTC_SPECIFIC_FLAG_1_UNKNOWN)) {
            marker_despawn(blubber->marker);
            *ev->should = false;
            return;
        }
        if ((bits & 0x1) && !mapSpecificFlags_get(TTC_SPECIFIC_FLAG_0_BLUBBER_UNKNOWN)) {
            mapSpecificFlags_set(TTC_SPECIFIC_FLAG_0_BLUBBER_UNKNOWN, true);
            // A teammate's delivery shows the half-gold dialog only live and within earshot.
            blubber->unk138_23 = !blubber->volatile_initialized || !Anchor_InEarshot(blubber->position);
            blubber->has_met_before = true;
        }
        if (blubber->initialized && (bits & 0x2) && !mapSpecificFlags_get(TTC_SPECIFIC_FLAG_1_UNKNOWN)) {
            auto* local = (ActorLocal_Blubber*)&blubber->local;
            mapSpecificFlags_set(TTC_SPECIFIC_FLAG_1_UNKNOWN, true);
            if (Anchor_InEarshot(blubber->position)) {
                // His thanks play out here, without our camera or a second jiggy.
                sBlubberTeammateFinish = true;
            } else {
                mapSpecificFlags_set(TTC_SPECIFIC_FLAG_2_BLUBBER_JIGGY_SPAWNED_TEXT_SHOWN, true);
                mapSpecificFlags_set(TTC_SPECIFIC_FLAG_3_BLUBBER_SHOW_JIGGY_SPAWNED_TEXT_FLAG, true);
                subaddie_set_state(blubber, CH_BLUBBER_STATE_4_UNKNOWN);
                actor_loopAnimation(blubber);
                blubber->actor_specific_1_f = 0.0f;
                local->unk24 = 0;
            }
        }

        // Before Blubber first sets up: our gold is the team's collected bars less the delivered ones, since carried
        // gold isn't synced.
        if (!blubber->volatile_initialized) {
            const s32 delivered = ((bits & 0x1) ? 1 : 0) + ((bits & 0x2) ? 1 : 0);
            const s32 pool = port_carriedSync_collectedCount(ANCHOR_COLLECTIBLE_GOLD) - delivered;
            if (pool > item_getCount(ITEM_18_GOLD_BULLIONS)) {
                item_adjustByDiffWithoutHud(ITEM_18_GOLD_BULLIONS, pool - item_getCount(ITEM_18_GOLD_BULLIONS));
            }
        }
    });

    // A teammate's hits on Nipper.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_117_NIPPER, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* nipper = ((ShouldActorUpdate*)event)->actor;
        if (!nipper->initialized || nipper->state == CH_NIPPER_STATE_4_DIEING ||
            nipper->state == CH_NIPPER_STATE_6_DEAD || nipper->state == CH_NIPPER_STATE_7_UNKNOWN) {
            return;
        }
        const s32 hits = NipperHits();
        if (NipperLocalHits(nipper) >= hits) {
            return;
        }
        if (hits >= 3) {
            nipper->lifetime_value = 40.0f;
            subaddie_set_state_with_direction(nipper, CH_NIPPER_STATE_6_DEAD, 0.01f, 1);
            actor_playAnimationOnce(nipper);
            sfx_playFadeShorthandDefault(SFX_78_EAGLECRY, 0.7f, 20000, nipper->position, 1500, 3000);
            comusic_8025AB44(COMUSIC_12_TTC_NIPPER, 0, 300);
            func_8025AABC(COMUSIC_12_TTC_NIPPER);
            func_8032BB88(nipper, -1, 300);
        } else {
            nipper->lifetime_value = (hits == 1) ? 80.0f : 40.0f;
        }
    });

    // A teammate finished feeding Leaky (level flag 2): the water drops here too.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1E_LEAKY, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* leaky = ((ShouldActorUpdate*)event)->actor;
        if (!leaky->volatile_initialized || leaky->state != 1 || leaky->unk38_31 >= 2 ||
            !levelSpecificFlags_get(LEVEL_FLAG_2_TTC_UNKNOWN)) {
            return;
        }
        Struct70s* water = func_8034C5AC(300);
        if (water != NULL) {
            func_8034E7B8(&water->type_73, -600, 4.0f, 2, 10.0f);
        }
        timedFunc_set_0(1.0f, __func_80388F88);
        subaddie_set_state(leaky, 2);
    });

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        if (gsworld_getMap() == MAP_7_TTC_TREASURE_TROVE_COVE && Anchor::GetInstance()->IsWorldSyncActive()) {
            TreasureHuntTick();
        }
    });

    // Blubber's thanks for a teammate's last delivery go without our camera, and the jiggy is theirs. Always on: a
    // scene that starts while connected plays out if the connection drops.
    COND_VB_SHOULD(VB_PLAY_PUZZLE_CUTSCENE, EVENT_PRIORITY_NORMAL, true, {
        if (va_arg(args, Actor*)->modelCacheIndex == ACTOR_115_BLUBBER && sBlubberTeammateFinish) {
            *should = false;
        }
    });
    COND_VB_SHOULD(VB_SPAWN_PUZZLE_JIGGY, EVENT_PRIORITY_NORMAL, true, {
        if (va_arg(args, Actor*)->modelCacheIndex == ACTOR_115_BLUBBER && sBlubberTeammateFinish) {
            *should = false;
        }
    });
    // Always on, with the hooks that read it.
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) { sBlubberTeammateFinish = false; });
}
