#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "functions.h"
extern "C" {
#include "CCW/ch/adulteyrie.h"
#include "CCW/ch/beanstalk.h"
#include "CCW/ch/ccwgobi.h"
#include "CCW/ch/cutsceneadulteyrie.h"
#include "CCW/ch/eyrieegg.h"
#include "CCW/ch/gnawtyboulder.h"
#include "CCW/ch/outsideautumnnabnut.h"
#include "CCW/ch/seasonswitchdoors.h"
#include "CCW/eyrie_baby.h"

void chAutumnOutsideNabnut_setState(Actor* self, s32 next_state);
void chCCWBeanstalk_setNextState(Actor* self, s32 next_state);
void chEyrieBaby_setState(Actor* self, s32 next_state);
void chEyrieEgg_setNextState(Actor* self, s32 next_state);
void chGnawtyBoulder_setNextState(Actor* self, s32 next_state);
}

#include "port/Patches/Patches.h"

// outsideautumnnabnut.c's AUTUMN_NABNUT_ACORN_COUNT.
constexpr s32 kNabnutAcornCount = 6;

// Set while the flower grows for a teammate's watering.
static bool sFlowerReplay = false;
// Set while Gnawty's boulder breaks for a teammate.
static bool sBoulderReplay = false;
// Set while baby Eyrie starts eating a teammate's caterpillar.
static bool sEyrieBabyReplay = false;
// A teammate hatched the egg or gave the last feed, which plays out here.
static bool sEyrieTeammateDone = false;
// Nabnut is thanking a teammate for the last acorn.
static bool sNabnutTeammateDone = false;
// Baby Eyrie just set up, and takes the team's feed count without the eating.
static bool sEyrieBabySetUp = false;
// Eyrie's flight is a teammate's. Decided as he takes off: their flag 5 can land again during our flight.
static bool sEyrieFlightReplay = false;

static bool IsTeammateStep(Actor* actor) {
    if (actor == NULL) {
        return false;
    }
    switch (actor->modelCacheIndex) {
        case ACTOR_29D_CCW_BEANSTALK:
            return sFlowerReplay;
        case ACTOR_2A0_EAGLE_EGG:
            return sEyrieTeammateDone;
        case ACTOR_2A1_EEYRIE_BABY:
            return sEyrieBabyReplay || sEyrieTeammateDone;
        case ACTOR_2A8_NABNUT:
            return sNabnutTeammateDone;
        case ACTOR_2AC_GNAWTY_BOULDER:
            return sBoulderReplay;
        case ACTOR_30E_CUTSCENE_ADULT_EYRIE:
            return sEyrieFlightReplay;
        case ACTOR_2AB_GNAWTY_SUMMER:
            return Anchor_WasLevelFlagSetRemotely(LEVEL_FLAG_25_CCW_GNAWTY_JIGGY_COLLECTED);
        case ACTOR_1E3_CCW_SPRING_SEASON_DOOR:
        case ACTOR_16D_CCW_SUMMER_SEASON_DOOR:
        case ACTOR_16B_CCW_AUTUMN_SEASON_DOOR:
        case ACTOR_169_CCW_WINTER_SEASON_DOOR:
            return Anchor_WasLevelFlagSetRemotely(actor->unk10_12 + 7);
        default:
            return false;
    }
}

// A teammate pressed this season's switch (the flag syncs): ours drops to match.
static void SeasonSwitchUpdate(IEvent* event) {
    Actor* button = ((ShouldActorUpdate*)event)->actor;
    if (button->volatile_initialized && button->state != CH_CCW_SEASON_SWITCH_DOOR_STATE_4_NO_COLLISION &&
        fileProgressFlag_get((enum file_progress_e)(button->unk10_12 + FILEPROG_8B_CCW_SPRING_OPEN))) {
        actor_playAnimationOnce(button);
        subaddie_set_state_with_direction(button, CH_CCW_SEASON_SWITCH_DOOR_STATE_4_NO_COLLISION, 0.999f, 1);
        actor_collisionOff(button);
    }
}

// A teammate broke the Zubbas' door this visit: the other season's goes too.
static void ZubbaDoorUpdate(IEvent* event) {
    auto* ev = (ShouldActorUpdate*)event;
    Actor* door = ev->actor;
    if (door->volatile_initialized && levelSpecificFlags_get(LEVEL_FLAG_10_CCW_BREAK_ZUBBA_DOOR)) {
        marker_despawn(door->marker);
        *ev->should = false;
    }
}

// A teammate's take-off flies our Eyrie, but not once the jiggy is out or while we're meeting him: then our take-off
// flies him, and that flight ends the camera and look-at it started.
static void EyrieFlyerUpdate(IEvent* event) {
    Actor* flyer = ((ShouldActorUpdate*)event)->actor;
    if (!flyer->volatile_initialized || flyer->state != CH_CUTSCENE_ADULT_EYRIE_STATE_1_UNK ||
        !mapSpecificFlags_get(CCW_WINTER_SPECIFIC_FLAG_5_EEYRIE_ADULT)) {
        return;
    }
    sEyrieFlightReplay = Anchor_WasMapFlagSetRemotely(CCW_WINTER_SPECIFIC_FLAG_5_EEYRIE_ADULT);
    if (!sEyrieFlightReplay) {
        return;
    }
    Actor* adult = actorArray_findActorFromActorId(ACTOR_2A3_EEYRIE_ADULT);
    if (jiggyscore_isSpawned(JIGGY_49_CCW_EYRIE) ||
        (adult != NULL && (adult->state == CH_EERYIE_ADULT_STATE_2_MEET_DIALOG ||
                           adult->state == CH_EERYIE_ADULT_STATE_3_FLIGHT_TAKE_OFF))) {
        mapSpecificFlags_set(CCW_WINTER_SPECIFIC_FLAG_5_EEYRIE_ADULT, false);
        sEyrieFlightReplay = false;
    }
}

void AnchorPuzzles_RegisterClickClockWood(bool connected) {
    // A teammate watered this season's flower.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_29D_CCW_BEANSTALK, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* flower = ((ShouldActorUpdate*)event)->actor;
        auto* local = (ActorLocal_CCW_1B20*)&flower->local;
        // The last entry, the flower fully grown, has no flag; flag 0 is Tiptup's.
        if (flower->volatile_initialized && flower->state == CH_CCW_BEANSTALK_STATE_1_WAITING &&
            Anchor::GetInstance()->IsWorldSyncActive() && local->unk0->completeFileProgress != 0 &&
            fileProgressFlag_get((enum file_progress_e)local->unk0->completeFileProgress)) {
            sFlowerReplay = true;
            chCCWBeanstalk_setNextState(flower, CH_CCW_BEANSTALK_STATE_2_GROWING);
            sFlowerReplay = false;
        }
    });

    // A teammate's caterpillars count toward ours.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_2A1_EEYRIE_BABY, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* baby = ((ShouldActorUpdate*)event)->actor;
        auto* local = (ActorLocal_CCW_3310*)&baby->local;
        if (!baby->volatile_initialized) {
            sEyrieBabySetUp = baby->state == CH_EYRIE_BABY_STATE_0_NOT_HATCHED;
            return;
        }
        const s32 fed = port_puzzleCount_get(ANCHOR_COUNT_CCW_EYRIE_FED);
        if (sEyrieBabySetUp) {
            sEyrieBabySetUp = false;
            local->eatenCount = (u8)fed;
        } else if (baby->state == CH_EYRIE_BABY_STATE_1_IDLE && fed > local->eatenCount) {
            // The eating plays here. The last one sets the fed flag.
            local->eatenCount = (u8)fed;
            if (local->eatenCount < local->progression->caterpillarRequirement) {
                sEyrieBabyReplay = true;
                chEyrieBaby_setState(baby, CH_EYRIE_BABY_STATE_5_EATING_CATERPILLARS);
                sEyrieBabyReplay = false;
            }
        }
        // The flag is set while the baby still sits idle. Our last feed has it yawning by then, and our hatch has the
        // egg breaking, so this is a teammate's. The egg breaks from here so the baby's reaction follows it.
        if (baby->state == CH_EYRIE_BABY_STATE_1_IDLE &&
            fileProgressFlag_get((enum file_progress_e)local->progression->unlocksFileProgress)) {
            Actor* egg = actorArray_findActorFromActorId(ACTOR_2A0_EAGLE_EGG);
            if (egg == NULL) {
                sEyrieTeammateDone = true;
            } else if (egg->state == CH_EYRIE_EGG_STATE_1_IDLE) {
                sEyrieTeammateDone = true;
                chEyrieEgg_setNextState(egg, CH_EYRIE_EGG_STATE_2_BREAKING);
            }
        }
    });

    // A teammate's Eyrie took off, or already flew and his jiggy is out. While we're meeting him only a teammate's
    // take-off can have set 5, so it's dropped, and his take-off then sets it again as ours.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_2A3_EEYRIE_ADULT, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* eyrie = ev->actor;
        if (eyrie->volatile_initialized && eyrie->state == CH_EERYIE_ADULT_STATE_1_WAITING &&
            (jiggyscore_isSpawned(JIGGY_49_CCW_EYRIE) ||
             Anchor_WasMapFlagSetRemotely(CCW_WINTER_SPECIFIC_FLAG_5_EEYRIE_ADULT)) &&
            !volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE)) {
            marker_despawn(eyrie->marker);
            *ev->should = false;
        } else if ((eyrie->state == CH_EERYIE_ADULT_STATE_2_MEET_DIALOG ||
                    eyrie->state == CH_EERYIE_ADULT_STATE_3_FLIGHT_TAKE_OFF) &&
                   mapSpecificFlags_get(CCW_WINTER_SPECIFIC_FLAG_5_EEYRIE_ADULT)) {
            mapSpecificFlags_set(CCW_WINTER_SPECIFIC_FLAG_5_EEYRIE_ADULT, false);
        }
    });
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_30E_CUTSCENE_ADULT_EYRIE, EVENT_PRIORITY_NORMAL, connected, EyrieFlyerUpdate);

    // A teammate's flower grew this season, and Gobi has gone. Not mid-bust, whose timer holds his marker.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_29E_CCW_GOBI, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* gobi = ev->actor;
        if (gobi->volatile_initialized && gobi->state == CH_CCW_GOBI_STATE_1_WAITING && gobi->marker->collidable &&
            ((gsworld_getMap() == MAP_44_CCW_SUMMER && fileProgressFlag_get(FILEPROG_E4_CCW_FLOWER_SUMMER)) ||
             (gsworld_getMap() == MAP_45_CCW_AUTUMN && fileProgressFlag_get(FILEPROG_E5_CCW_FLOWER_AUTUMN)))) {
            marker_despawn(gobi->marker);
            *ev->should = false;
        }
    });

    // A teammate broke Gnawty's boulder.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_2AC_GNAWTY_BOULDER, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* boulder = ev->actor;
        if (boulder->volatile_initialized && boulder->state == GNAWTY_BOULDER_STATE_1_IDLE &&
            levelSpecificFlags_get(LEVEL_FLAG_25_CCW_GNAWTY_JIGGY_COLLECTED) && gsworld_getMap() != MAP_43_CCW_SPRING) {
            sBoulderReplay = true;
            chGnawtyBoulder_setNextState(boulder, GNAWTY_BOULDER_STATE_2_BREAK);
            sBoulderReplay = false;
            *ev->should = false;
        }
    });

    // A teammate's acorns count toward ours.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_2A8_NABNUT, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        Actor* nabnut = ((ShouldActorUpdate*)event)->actor;
        auto* local = (ActorLocal_ChAutumnNabnutPosition*)&nabnut->local;
        const s32 returned = port_puzzleCount_get(ANCHOR_COUNT_CCW_NABNUT_ACORNS);
        if (nabnut->volatile_initialized && nabnut->state == CH_AUTUMN_OUTDOOR_NABNUT_STATE_1_SAD &&
            returned > local->returned_acorn_count) {
            local->returned_acorn_count = returned;
            if (returned >= kNabnutAcornCount) {
                sNabnutTeammateDone = true;
                chAutumnOutsideNabnut_setState(nabnut, CH_AUTUMN_OUTDOOR_NABNUT_STATE_2_WAIT);
            }
        }
    });

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_1E2_CCW_SPRING_SWITCH, EVENT_PRIORITY_NORMAL, connected, SeasonSwitchUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_16C_CCW_SUMMER_SWITCH, EVENT_PRIORITY_NORMAL, connected, SeasonSwitchUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_16A_CCW_AUTUMN_SWITCH, EVENT_PRIORITY_NORMAL, connected, SeasonSwitchUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_168_CCW_WINTER_SWITCH, EVENT_PRIORITY_NORMAL, connected, SeasonSwitchUpdate);

    COND_ID_HOOK(ShouldActorUpdate, ACTOR_298_ZUBBA_DOOR_SUMMER, EVENT_PRIORITY_NORMAL, connected, ZubbaDoorUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_29A_ZUBBA_DOOR_AUTUMN, EVENT_PRIORITY_NORMAL, connected, ZubbaDoorUpdate);

    // A teammate opened the whipcrack room: ours opens too, not just when it next spawns.
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_2E7_WHIPCRACK_ROOM_DOOR, EVENT_PRIORITY_NORMAL, connected, [](IEvent* event) {
        auto* ev = (ShouldActorUpdate*)event;
        Actor* door = ev->actor;
        if (door->initialized && levelSpecificFlags_get(LEVEL_FLAG_38_CCW_UNKNOWN)) {
            marker_despawn(door->marker);
            *ev->should = false;
        }
    });

    // A teammate's step replayed here goes without our camera, and shows its dialogs and plays its sounds only within
    // earshot. Always on: a replay that starts while connected plays out if the connection drops.
    COND_VB_SHOULD(VB_PLAY_PUZZLE_CUTSCENE, EVENT_PRIORITY_NORMAL, true, {
        if (IsTeammateStep(va_arg(args, Actor*))) {
            *should = false;
        }
    });
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
    COND_VB_SHOULD(VB_SPLINE_PATH_SFX, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsTeammateStep(actor) && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });
    // A teammate's jiggy is theirs, as is Nabnut's once it's out.
    COND_VB_SHOULD(VB_SPAWN_PUZZLE_JIGGY, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsTeammateStep(actor) || (actor != NULL && actor->modelCacheIndex == ACTOR_2A8_NABNUT &&
                                      jiggyscore_isSpawned(JIGGY_4A_CCW_NABNUT))) {
            *should = false;
        }
    });
    // Always on, with the hooks that read them.
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        sEyrieTeammateDone = false;
        sNabnutTeammateDone = false;
        sEyrieFlightReplay = false;
    });
}
