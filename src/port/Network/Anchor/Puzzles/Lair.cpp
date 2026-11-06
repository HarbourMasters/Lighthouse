#include "Puzzles.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/JigsawPedestal.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <algorithm>

#include "functions.h"
extern "C" {
#include "lair/jigsawpicture.h"

extern ActorArray* suBaddieActorArray;
void addOrRemovePieceFromDisplay(Actor* self, s32 arg1, bool arg2);
enum file_progress_e chWarpCauldron_getPairedFileProgressFlagIndex(Actor* self);
void func_80324CFC(f32, enum comusic_e, s32);
void func_8038A96C(Actor* self, s32 arg1);
s32 getPicturePiecePosition(Actor* self);
void gruntyLaughCallback(ActorMarker* marker, enum asset_e text_id, s32 arg2);
void jigsawPicture_setState(Actor* self, s32 next_state);
}

// The jigsaw podiums' two actors (JIGSAW_PICTURE_ACTOR and _2).
constexpr s32 kJigsawPodium = 0x3B7;
constexpr s32 kJigsawPodium2 = 0x3BC;
constexpr s32 kCcwPodiumField = 9;

static u32 sNoteDoorReplay = 0;
static s32 sLevelDoorReplay = 0;
static u32 sCauldronReplay = 0;
static bool sStatueHatReplay = false;
static bool sCcwPodiumHidden = false;
static bool sCcwPodiumTeammateReveal = false;
static s32 sBuzzedPodium = 0;

static bool IsTeammateStep(Actor* actor) {
    if (actor == NULL) {
        return false;
    }
    switch (actor->modelCacheIndex) {
        case ACTOR_213_GL_BGS_PUZZLE_GRATE:
            return Anchor_WasMapFlagSetRemotely(0);
        case ACTOR_215_GL_CC_ENTRANCE_LONG_PIPE_GREEN:
        case ACTOR_216_GL_CC_ENTRANCE_LONG_PIPE_BLUE:
            return Anchor_WasMapFlagSetRemotely(1);
        case ACTOR_218_GL_CC_ENTRANCE_SHORT_PIPE:
            return Anchor_WasMapFlagSetRemotely(2);
        case ACTOR_203_NOTE_DOOR:
            return sNoteDoorReplay & (1 << actor->actorTypeSpecificField);
        case ACTOR_23B_WARP_CAULDRON:
            return sCauldronReplay & (1 << actor->actorTypeSpecificField);
        case ACTOR_21B_GRUNTY_STATUE_BREAKABLE_HAT_TOP:
            return sStatueHatReplay;
        default:
            return actor->modelCacheIndex == sLevelDoorReplay;
    }
}

// A negative typeField matches any.
static Actor* FindActor(s32 actorId, s32 typeField) {
    if (suBaddieActorArray == NULL) {
        return NULL;
    }
    for (s32 i = 0; i < suBaddieActorArray->cnt; i++) {
        Actor* actor = &suBaddieActorArray->data[i];
        if (actor->marker != NULL && actor->modelCacheIndex == actorId &&
            (typeField < 0 || actor->actorTypeSpecificField == typeField)) {
            return actor;
        }
    }
    return NULL;
}

static enum file_progress_e BreakableFlag(Actor* actor) {
    switch (actor->modelCacheIndex) {
        case ACTOR_230_BREAKABLE_FLOOR_COBWEB:
            return actor->actorTypeSpecificField == 1 ? FILEPROG_CB_LAIR_COBWEB_OVER_FLIGHTPAD_BROKEN
                                                      : FILEPROG_CC_LAIR_COBWEB_OVER_GREEN_CAULDRON_BROKEN;
        case ACTOR_231_BREAKABLE_WALL_COBWEB:
            return FILEPROG_CA_COBWEB_BLOCKING_PURPLE_CAULDRON_BROKEN;
        case ACTOR_20D_BREAKABLE_BRICK_WALL:
            return actor->actorTypeSpecificField == 1 ? FILEPROG_C8_LAIR_BRICKWALL_TO_WADINGBOOTS_BROKEN
                                                      : FILEPROG_C9_LAIR_BRICKWALL_TO_SHOCKJUMP_PAD_BROKEN;
        case ACTOR_220_GRATE_TO_RBB_PUZZLE:
            return FILEPROG_C2_GRATE_TO_RBB_PUZZLE_OPEN;
        case ACTOR_224_ICE_BALL_TO_CHEATO:
            return FILEPROG_C3_ICE_BALL_TO_CHEATO_BROKEN;
        case ACTOR_227_GLASS_EYE:
            return FILEPROG_C4_STATUE_EYE_BROKEN;
        case ACTOR_225_RAREWARE_BOX:
            return FILEPROG_C5_RAREWARE_BOX_BROKEN;
        case ACTOR_21E_GRATE_TO_LEVEL_3_WATER_SWITCH:
            return FILEPROG_CD_GRATE_TO_WATER_SWITCH_3_OPEN;
        default: // ACTOR_21F_GRATE_BETWEEN_MMM_AND_RBB_PUZZLES
            return FILEPROG_CE_GRATE_TO_MMM_PUZZLE_OPEN;
    }
}

// A breakable a teammate broke while the map's actors were saved away comes back from the restore already set up,
// so its setup check doesn't run again.
static void BreakableUpdate(IEvent* event) {
    auto* ev = (ShouldActorUpdate*)event;
    Actor* actor = ev->actor;
    if (!actor->initialized || actor->volatile_initialized) {
        return;
    }
    actor->volatile_initialized = true;
    if (fileProgressFlag_get(BreakableFlag(actor))) {
        marker_despawn(actor->marker);
        *ev->should = false;
    }
}

static void JigsawPodiumUpdate(IEvent* event) {
    auto* ev = (ShouldActorUpdate*)event;
    Actor* podium = ev->actor;
    const s32 field = podium->actorTypeSpecificField;
    if (!podium->volatile_initialized) {
        if (field == kCcwPodiumField) {
            sCcwPodiumHidden = false;
        }
        return;
    }

    if (field == kCcwPodiumField && sCcwPodiumHidden) {
        if (!fileProgressFlag_get(FILEPROG_53_CCW_PUZZLE_PODIUM_SWITCH_PRESSED)) {
            *ev->should = false;
            return;
        }
        sCcwPodiumHidden = false;
        podium->marker->propPtr->unk8_3 = true;
        if (!fileProgressFlag_get(FILEPROG_54_CCW_PUZZLE_PODIUM_ACTIVE)) {
            __bundle_spawnFromFirstActor(BUNDLE_20__UNKNOWN, podium);
            if (!sCcwPodiumTeammateReveal || Anchor_InEarshot(podium->position)) {
                func_80324CFC(0.0f, COMUSIC_43_ENTER_LEVEL_GLITTER, 0x7FFF);
                func_80324D2C(2.1f, COMUSIC_43_ENTER_LEVEL_GLITTER);
                func_8030E6D4(SFX_113_PAD_APPEARS);
            }
        }
        sCcwPodiumTeammateReveal = false;
    }

    // A teammate's pieces land as the picture's flags: ours fade in or out (jigsawPicture_setState's states 5 and 8).
    // Within earshot we hear them, and a finished picture plays state 7's jingle and Grunty's line a second on, when
    // state 7 would start.
    auto* local = (ActorLocal_lair_86F0*)&podium->local;
    const s32 placed = fileProgressFlag_getN((enum file_progress_e)PICTURE_INFO[field - 1].progress_flag,
                                             PICTURE_INFO[field - 1].size_bits);
    if (Anchor::GetInstance()->IsWorldSyncActive() && local->unk4 != placed) {
        const bool heard = Anchor_InEarshot(podium->position);
        if (heard) {
            comusic_playTrack(placed > local->unk4 ? COMUSIC_67_INSERTING_JIGGY : SFX_REMOVE_JIGGY);
        }
        while (local->unk4 < placed) {
            local->unk4++;
            const s32 piece = getPicturePiecePosition(podium);
            addOrRemovePieceFromDisplay(podium, piece, true);
            local->unk0 |= 1 << piece;
        }
        while (local->unk4 > placed) {
            const s32 piece = getPicturePiecePosition(podium);
            addOrRemovePieceFromDisplay(podium, piece, false);
            local->unk4--;
            local->unk0 &= ~(1 << piece);
        }
        if (heard && jigsawPicture_isJigsawPictureComplete(field)) {
            timedFunc_set_1(1.0f, (GenFunction_1)comusic_playTrack, COMUSIC_65_WORLD_OPENING_B);
            if (field == 1 || field == 0xA) {
                func_80324DBC(2.0f, (enum asset_e)(field == 1 ? 0xF7E : 0xFAC), 4, NULL, podium->marker,
                              gruntyLaughCallback, NULL);
            }
        }
    }

    if (podium->state != JIGSAW_PICTURE_LEAVE_PODIUM && !JigsawPedestal_IsSelf(field)) {
        jigsawPicture_setState(podium, JIGSAW_PICTURE_LEAVE_PODIUM);
        *ev->should = false;
    } else if (podium->state == JIGSAW_PICTURE_LEAVE_PODIUM) {
        JigsawPedestal_Release(field);
    }
}

static void JigsawPodiumDraw(IEvent* event) {
    auto* ev = (ShouldActorDraw*)event;
    if (ev->actor->actorTypeSpecificField == kCcwPodiumField && sCcwPodiumHidden) {
        *ev->should = false;
    }
}

// In the order of their seen flags (0x28-0x30) and open flags (FILEPROG_31_MM_OPEN on).
constexpr s32 kLevelDoors[] = {
    ACTOR_20E_MM_ENTRANCE_DOOR,  ACTOR_211_TTC_ENTRANCE_CHEST_LID, ACTOR_212_CC_ENTRANCE_BARS,
    ACTOR_210_BGS_ENTRANCE_DOOR, ACTOR_235_FP_ENTRANCE_DOOR_LEFT,  ACTOR_226_GV_ENTRANCE,
    ACTOR_228_MMM_ENTRANCE_DOOR, ACTOR_20F_RBB_ENTRANCE_DOOR,      ACTOR_234_CCW_ENTRANCE_DOOR,
};
static enum file_progress_e LevelDoorOpenFlag(s32 actorId) {
    const auto index = std::find(std::begin(kLevelDoors), std::end(kLevelDoors), actorId) - std::begin(kLevelDoors);
    return (enum file_progress_e)(FILEPROG_31_MM_OPEN + index);
}

// The dance's end (bsjig_notedoor_end) brings the music back.
static void NoteDoorFanfareEnd() {
    func_8025A2FC(-1, 0xfa0);
    midichannel_incOrDecCounter(true);
}

void AnchorPuzzles_ReplayNoteDoorDance(float position[3]) {
    if (!Anchor::GetInstance()->IsWorldSyncActive() || !Anchor_InEarshot(position)) {
        return;
    }
    // The opener's dance fanfare, with the music ducked (bsjig_notedoor_init).
    midichannel_incOrDecCounter(false);
    func_8025A2FC(0, 0xfa0);
    coMusicPlayer_playMusic(COMUSIC_42_NOTEDOOR_OPENING_FANFARE, -1);
    timedFunc_set_0(3.6f, NoteDoorFanfareEnd);
}

void AnchorPuzzles_ReplayLairFlag(int32_t progressFlag) {
    Anchor_BreakLairBreakable(progressFlag);

    if (progressFlag >= FILEPROG_3A_NOTE_DOOR_50_OPEN && progressFlag <= FILEPROG_45_NOTE_DOOR_882_OPEN) {
        const s32 field = progressFlag - FILEPROG_39_CCW_OPEN;
        Actor* door = FindActor(ACTOR_203_NOTE_DOOR, field);
        if (door != NULL && door->alpha_124_19 == 0xFF) {
            sNoteDoorReplay |= 1 << field;
            if (Anchor_InEarshot(door->position)) {
                func_80324CFC(0.0f, COMUSIC_43_ENTER_LEVEL_GLITTER, 32700);
                func_80324D2C(2.4f, COMUSIC_43_ENTER_LEVEL_GLITTER);
            }
            door->alpha_124_19 -= 7;
        }
        return;
    }

    // A level opened (its unnamed seen flags).
    if (progressFlag >= 0x28 && progressFlag <= 0x30) {
        sLevelDoorReplay = kLevelDoors[progressFlag - 0x28];
        return;
    }

    switch (progressFlag) {
        case FILEPROG_E2_DOOR_OF_GRUNTY_OPEN:
            sLevelDoorReplay = ACTOR_2E5_LARGE_DOOR_TO_FINAL_BATTLE;
            break;

        // A warp cauldron lights up: chWarpCauldron_update's state 1, without the camera or the trip to its pair.
        case FILEPROG_49_PINK_CAULDRON_1_ACTIVE:
        case FILEPROG_4A_PINK_CAULDRON_2_ACTIVE:
        case FILEPROG_4B_GREEN_CAULDRON_1_ACTIVE:
        case FILEPROG_4C_GREEN_CAULDRON_2_ACTIVE:
        case FILEPROG_4D_RED_CAULDRON_1_ACTIVE:
        case FILEPROG_4E_RED_CAULDRON_2_ACTIVE:
        case FILEPROG_4F_UNUSED_CAULDRON_1_ACTIVE:
        case FILEPROG_50_UNUSED_CAULDRON_2_ACTIVE:
        case FILEPROG_51_YELLOW_CAULDRON_1_ACTIVE:
        case FILEPROG_52_YELLOW_CAULDRON_2_ACTIVE: {
            const s32 field = progressFlag - FILEPROG_49_PINK_CAULDRON_1_ACTIVE + 1;
            Actor* cauldron = FindActor(ACTOR_23B_WARP_CAULDRON, field);
            if (cauldron != NULL && cauldron->state == 1) {
                sCauldronReplay |= 1 << field;
                subaddie_set_state_forward(cauldron, 2);
                cauldron->unk38_0 = false;
                func_8038A96C(cauldron, 1);
                func_802D09B8(cauldron, 2);
                if (Anchor_InEarshot(cauldron->position)) {
                    const bool paired = fileProgressFlag_get(chWarpCauldron_getPairedFileProgressFlagIndex(cauldron));
                    sfxsource_play(paired ? SFX_107_CAULDRON_ACTIVATION_1 : SFX_108_CAULDRON_ACTIVATION_2, 32000);
                    if (!paired && !fileProgressFlag_get(FILEPROG_F5_COMPLETED_A_WARP_CAULDRON_SET)) {
                        gcdialog_showDialog(ASSET_F79_DIALOG_UNKNOWN, 4, NULL, NULL, NULL, NULL);
                    }
                }
            }
            break;
        }

        // Double health: the max-health recompute and refill that activateDoubleHealth runs after its flag.
        case FILEPROG_B9_DOUBLE_HEALTH:
            func_80347958();
            item_adjustByDiffWithHud(ITEM_14_HEALTH, 0);
            break;

        // Grunty's statue hat breaks on its countdown's last tick.
        case FILEPROG_A1_STATUE_HAT_OPEN: {
            Actor* hat = FindActor(ACTOR_21B_GRUNTY_STATUE_BREAKABLE_HAT_TOP, -1);
            if (hat != NULL && hat->volatile_initialized && hat->unk1C[0] == 0.0f) {
                hat->unk1C[0] = 1.0f;
                sStatueHatReplay = true;
            }
            break;
        }

        case FILEPROG_53_CCW_PUZZLE_PODIUM_SWITCH_PRESSED:
            sCcwPodiumTeammateReveal = true;
            break;

        default:
            break;
    }
}

void AnchorPuzzles_RegisterLair(bool connected) {
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_230_BREAKABLE_FLOOR_COBWEB, EVENT_PRIORITY_NORMAL, connected,
                 BreakableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_231_BREAKABLE_WALL_COBWEB, EVENT_PRIORITY_NORMAL, connected, BreakableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_20D_BREAKABLE_BRICK_WALL, EVENT_PRIORITY_NORMAL, connected, BreakableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_220_GRATE_TO_RBB_PUZZLE, EVENT_PRIORITY_NORMAL, connected, BreakableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_224_ICE_BALL_TO_CHEATO, EVENT_PRIORITY_NORMAL, connected, BreakableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_227_GLASS_EYE, EVENT_PRIORITY_NORMAL, connected, BreakableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_225_RAREWARE_BOX, EVENT_PRIORITY_NORMAL, connected, BreakableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_21E_GRATE_TO_LEVEL_3_WATER_SWITCH, EVENT_PRIORITY_NORMAL, connected,
                 BreakableUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_21F_GRATE_BETWEEN_MMM_AND_RBB_PUZZLES, EVENT_PRIORITY_NORMAL, connected,
                 BreakableUpdate);

    // Always on: a podium hidden while connected still shows when its switch lands after a disconnect. The rest
    // checks world sync.
    COND_ID_HOOK(ShouldActorUpdate, kJigsawPodium, EVENT_PRIORITY_NORMAL, true, JigsawPodiumUpdate);
    COND_ID_HOOK(ShouldActorUpdate, kJigsawPodium2, EVENT_PRIORITY_NORMAL, true, JigsawPodiumUpdate);
    COND_ID_HOOK(ShouldActorDraw, kJigsawPodium, EVENT_PRIORITY_NORMAL, true, JigsawPodiumDraw);
    COND_ID_HOOK(ShouldActorDraw, kJigsawPodium2, EVENT_PRIORITY_NORMAL, true, JigsawPodiumDraw);

    // Anchor suppresses the despawn: a despawned podium only comes back when the cube re-streams, which would strand a
    // teammate standing here when the flag arrives.
    COND_VB_SHOULD(VB_CCW_PODIUM_DESPAWN, EVENT_PRIORITY_NORMAL, connected, {
        Actor* podium = va_arg(args, Actor*);
        if (Anchor::GetInstance()->IsWorldSyncActive()) {
            *should = false;
            sCcwPodiumHidden = true;
            podium->marker->propPtr->unk8_3 = false;
        }
    });

    // One podium, one player: stepping on one a teammate holds buzzes once instead.
    COND_VB_SHOULD(VB_JIGSAW_PODIUM_ENTER, EVENT_PRIORITY_NORMAL, connected, {
        const s32 field = va_arg(args, Actor*)->actorTypeSpecificField;
        if (!*should) {
            if (sBuzzedPodium == field) {
                sBuzzedPodium = 0;
            }
        } else if (!JigsawPedestal_TryClaim(field)) {
            *should = false;
            if (sBuzzedPodium != field) {
                comusic_playTrack(COMUSIC_2C_BUZZER);
                sBuzzedPodium = field;
            }
        }
    });

    // A teammate's press delivers its map flag and file progress flag in the same packet drain; while we're here to
    // watch, the rise wins over the setup.
    COND_VB_SHOULD(VB_LAIR_GRATE_OR_PIPE_ALREADY_UP, EVENT_PRIORITY_NORMAL, connected, {
        if (IsTeammateStep(va_arg(args, Actor*))) {
            *should = false;
        }
    });

    // Always on, as are the door listeners below: a replay that starts while connected plays out if the connection
    // drops.
    COND_VB_SHOULD(VB_PLAY_PUZZLE_CUTSCENE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsTeammateStep(actor)) {
            *should = false;
            if (actor->modelCacheIndex == ACTOR_203_NOTE_DOOR && actor->alpha_124_19 == 0) {
                sNoteDoorReplay &= ~(1 << actor->actorTypeSpecificField);
            }
        }
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (IsTeammateStep(actor) && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });

    // A fading note door finishes after its flag lands, or our own fade would strand us in its player lock. A
    // teammate's fades whatever our notes, without the lock. LUS dispatches the lowest priority first, so
    // EVENT_PRIORITY_HIGH puts the note check after a romhack's door costs.
    COND_VB_SHOULD(VB_NOTEDOOR_CLOSED, EVENT_PRIORITY_NORMAL, true, {
        if (va_arg(args, Actor*)->alpha_124_19 != 0xFF) {
            *should = true;
        }
    });
    COND_VB_SHOULD(VB_NOTEDOOR_ENOUGH_NOTES, EVENT_PRIORITY_HIGH, true, {
        Actor* door = va_arg(args, Actor*);
        if (sNoteDoorReplay & (1 << door->actorTypeSpecificField)) {
            *should = true;
        }
    });
    COND_HOOK(OnMapLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        sNoteDoorReplay = 0;
        sLevelDoorReplay = 0;
        sCauldronReplay = 0;
        sStatueHatReplay = false;
        sCcwPodiumTeammateReveal = false;
    });

    // Lair door remote-open: Door of Grunty's open flag (0xE2) is already set on arrival; key off
    // visual state (fully open == 0x1B) instead. Other lair doors stay flag-based.
    COND_VB_SHOULD(VB_LEVELDOOR_OPENING, EVENT_PRIORITY_NORMAL, true, {
        Actor* door = va_arg(args, Actor*);
        if (door->modelCacheIndex == sLevelDoorReplay) {
            const bool open = door->modelCacheIndex == ACTOR_2E5_LARGE_DOOR_TO_FINAL_BATTLE
                                  ? door->state == 0x1B
                                  : fileProgressFlag_get(LevelDoorOpenFlag(door->modelCacheIndex));
            if (open) {
                sLevelDoorReplay = 0;
            } else {
                *should = true;
            }
        }
    });

    // FNV-1a, so every platform's build agrees on the team's pattern.
    COND_VB_SHOULD(VB_FF_PATTERN_ROLL, EVENT_PRIORITY_NORMAL, connected, {
        if (Anchor::GetInstance()->IsWorldSyncActive()) {
            const std::string key = std::string(CVarGetString(CVAR_REMOTE_ANCHOR("RoomId"), "")) + "/" +
                                    CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
            uint32_t hash = 2166136261u;
            for (unsigned char c : key) {
                hash = (hash ^ c) * 16777619u;
            }
            fileProgressFlag_setN(FILEPROG_D3_FF_PATTERN, hash & 0xFF, 8);
            *should = false;
        }
    });

    // The FP witch switch's press is a volatile flag, which a teammate's earlier session or a reload loses; its jiggy
    // flag lasts.
    COND_VB_SHOULD(VB_FP_WITCH_SWITCH_PRESSED, EVENT_PRIORITY_NORMAL, connected, {
        if (Anchor::GetInstance()->IsWorldSyncActive() &&
            fileProgressFlag_get(FILEPROG_47_FP_WITCH_SWITCH_JIGGY_PRESSED)) {
            *should = true;
        }
    });
}
