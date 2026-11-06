#include "VileSync.h"
#include "port/Network/Anchor/SeqGate.h"
#include "port/Network/Anchor/Authority.h"

#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "prop.h"
#include "BGS/ch/mrvile.h"
#include "BGS/ch/yumblie.h"
#include "BGS/mrvileminigame.h"

void chYumblie_setState(Actor* self, enum chyumblie_state_e next_state);
void chMrVile_setAction(Actor* self, s32 action);
bool chMrVileMinigame_didPlayerConsumePiece(f32 arg0[3]);
void func_8038A044(void);

SeqGate gVileSeq;

static s32 sForcedPieceType = -1;

// --- Actor lookup helpers ------------------------------------------------------------

static Actor* VileSync_FindActor(enum actor_e actor_id) {
    f32 origin[3] = { 0.0f, 0.0f, 0.0f };
    f32 dist;

    if (gsworld_getMap() != MAP_10_BGS_MR_VILE) {
        return NULL;
    }
    return actorArray_findClosestActorFromActorId(origin, actor_id, -1, &dist);
}

static Actor* VileSync_FindHoleActor(int32_t holeId) {
    const float* holePos = VileHoles_GetPosition(holeId);
    f32 pos[3];
    f32 dist;
    Actor* actor;

    if (holePos == NULL || gsworld_getMap() != MAP_10_BGS_MR_VILE) {
        return NULL;
    }
    pos[0] = holePos[0];
    pos[1] = -100.0f; // yumblie actors sit at y = -100
    pos[2] = holePos[1];
    actor = actorArray_findClosestActorFromActorId(pos, ACTOR_139_YUMBLIE, -1, &dist);
    if (actor == NULL || dist > 100.0f) {
        return NULL;
    }
    return actor;
}

static bool VileSync_IsLiveAuthority(void) {
    return NetAuthority_IsClaimed(NET_ACTIVITY_VILE_MINIGAME) && NetAuthority_IsSelf(NET_ACTIVITY_VILE_MINIGAME);
}

// --- The actors' state -----------------------------------------------------------------

// Drives this yumblie from an authoritative packet, forcing the piece type instead of
// rolling it locally.
static void VileSync_ApplyYumblieState(Actor* yumblie, s32 state, s32 pieceType) {
    ActorLocal_Yumblie* local = (ActorLocal_Yumblie*)&yumblie->local;
    f32 dist;

    // A packet can arrive before this actor's first update tick has resolved the
    // controller marker that chYumblie_setState relies on.
    if (local->game_marker == NULL) {
        local->game_marker =
            actorArray_findClosestActorFromActorId(yumblie->position, ACTOR_138_VILE_GAME_CTRL, -1, &dist)->marker;
    }
    sForcedPieceType = pieceType;
    chYumblie_setState(yumblie, state);
    sForcedPieceType = -1;
}

// Reads state + piece type for snapshot gathering.
static s32 VileSync_GetYumblieState(Actor* yumblie, s32* pieceType) {
    ActorLocal_Yumblie* local = (ActorLocal_Yumblie*)&yumblie->local;

    *pieceType = local->isGrumblie;
    return yumblie->state;
}

// Fills a snapshot from the live controller. Only the stable states (idle, playing, and
// the round-end results) are broadcast; dialog states cannot be reconstructed remotely.
static bool VileSync_GatherGame(Actor* controller, VileGameSnapshot* dst) {
    chMrVileMinigameActor* local = (chMrVileMinigameActor*)&controller->local;

    switch (controller->state) {
        case 1:   // idle
        case 5:   // playing
        case 6:   // round lost
        case 8:   // round won
        case 9:   // final round won (jiggy)
        case 0xA: // extra challenge won
            break;
        default:
            return false;
    }
    dst->gameState = controller->state;
    dst->round = local->dialogIndex;
    dst->maxRound = local->mode;
    dst->currentType = local->current_type;
    dst->typeChangeTimer = local->type_change_timer;
    dst->playerScore = local->player_score;
    dst->vileScore = local->vile_score;
    dst->hourglassRemaining = item_getCount(ITEM_0_HOURGLASS_TIMER);
    return true;
}

// Forces the controller to match an authoritative snapshot (followers + late joiners).
// Mirrors the state-5 side effects of chMrVileMinigame_setState without the dialog/round bookkeeping.
static void VileSync_ApplyGame(Actor* controller, const VileGameSnapshot* src) {
    chMrVileMinigameActor* local = (chMrVileMinigameActor*)&controller->local;
    s32 drift;

    local->dialogIndex = src->round;
    local->mode = src->maxRound;
    local->current_type = src->currentType;
    local->type_change_timer = src->typeChangeTimer;
    local->player_score = src->playerScore;
    local->vile_score = src->vileScore;
    item_set(ITEM_1A_PLAYER_VILE_SCORE, local->player_score);
    item_set(ITEM_1B_VILE_VILE_SCORE, local->vile_score);

    if (controller->state != src->gameState) {
        if (src->gameState == 5) {
            item_set(ITEM_0_HOURGLASS_TIMER, src->hourglassRemaining);
            item_set(ITEM_6_HOURGLASS, true);
            mapSpecificFlags_set(6, true);
            if (local->vile_marker != NULL) {
                chMrVile_setStatePlayMinigame(local->vile_marker);
            }
            func_8025A58C(0, 4000);
            timedFunc_set_2(1.0f, (GenFunction_2)coMusicPlayer_playMusic, COMUSIC_55_BGS_MR_VILE, 28000);
        } else {
            item_set(ITEM_6_HOURGLASS, false);
            mapSpecificFlags_set(6, false);
            // Round-end result presentation, mirroring chMrVileMinigame_setState's timed callbacks
            // (dialogs and the static camera stay on the authority).
            if (src->gameState == 6) {
                timedFunc_set_2(1.0f, (GenFunction_2)coMusicPlayer_playMusic, COMUSIC_3C_MINIGAME_LOSS, 28000);
                timedFunc_set_0(4.0f, (GenFunction_0)func_8038A044);
            } else if ((src->gameState == 8) || (src->gameState == 9) || (src->gameState == 0xA)) {
                timedFunc_set_2(1.0f, (GenFunction_2)coMusicPlayer_playMusic, COMUSIC_3B_MINIGAME_VICTORY, 28000);
                timedFunc_set_0(3.0f, (GenFunction_0)func_8038A044);
            } else {
                func_8025A58C(-1, 400);
            }
        }
        controller->state = src->gameState;
    } else if (controller->state == 5) {
        // tolerance band: only snap the hourglass when drift would be noticeable
        drift = item_getCount(ITEM_0_HOURGLASS_TIMER) - src->hourglassRemaining;
        if ((drift > 15) || (drift < -15)) {
            item_set(ITEM_0_HOURGLASS_TIMER, src->hourglassRemaining);
        }
    }
}

// Authority-side: consume the piece at position on behalf of a remote player. Mirrors
// chMrVileMinigame_playerConsumePiece without reading the local player's mouth position.
// position[1] must be 0 to match piece positions (see chMrVileMinigame_newPiece).
// On success, reports the consumed piece's type and whether it matched the required
// type so the requesting client can replay the croc's eat feedback (see
// VileSync_PlayLocalEatFeedback).
static bool VileSync_ConsumeRemote(Actor* controller, f32 position[3], s32* outPieceType, s32* outCorrectType) {
    chMrVileMinigameActor* local = (chMrVileMinigameActor*)&controller->local;
    bool isCorrectType;
    struct vilegame_piece* begin;
    struct vilegame_piece* end;
    struct vilegame_piece* piece;

    if (controller->state != 5) {
        return false;
    }
    begin = (struct vilegame_piece*)bk_vector_getBegin(local->game_pieces);
    end = (struct vilegame_piece*)bk_vector_getEnd(local->game_pieces);
    for (piece = begin; piece < end; piece++) {
        if ((ml_vec3f_distance(piece->position, position) < 65.25) && chyumblie_is_edible(piece->marker)) {
            isCorrectType = ((local->current_type != YUMBLIE) && (piece->type != YUMBLIE)) ||
                            (((local->current_type == YUMBLIE) && piece->type == YUMBLIE));
            if (isCorrectType) {
                local->player_score++;
            }
            if (outPieceType != NULL) {
                *outPieceType = piece->type;
            }
            if (outCorrectType != NULL) {
                *outCorrectType = isCorrectType;
            }
            func_8038B684(piece->marker);
            return true;
        }
    }
    return false;
}

// The hole of the first edible piece our chomp reached this frame (chMrVileMinigame_playerConsumePiece's test), or
// VILE_HOLE_NONE.
static int32_t VileSync_ChompedHole(chMrVileMinigameActor* local) {
    f32 mouth[3];
    struct vilegame_piece* begin = (struct vilegame_piece*)bk_vector_getBegin(local->game_pieces);
    struct vilegame_piece* end = (struct vilegame_piece*)bk_vector_getEnd(local->game_pieces);
    struct vilegame_piece* piece;

    if ((end == begin) || !chMrVileMinigame_didPlayerConsumePiece(mouth)) {
        return VILE_HOLE_NONE;
    }
    mouth[1] = 0.0f;
    for (piece = begin; piece < end; piece++) {
        if ((ml_vec3f_distance(piece->position, mouth) < 65.25) && chyumblie_is_edible(piece->marker)) {
            return VileHoles_IdFromPosition(piece->position[0], piece->position[2]);
        }
    }
    return VILE_HOLE_NONE;
}

// While a packet drives this yumblie, sets the piece type it carries; false when the roll is ours.
bool VileSync_ForcePieceType(Actor* yumblie) {
    if (sForcedPieceType < 0) {
        return false;
    }
    ((ActorLocal_Yumblie*)&yumblie->local)->isGrumblie = (u8)sForcedPieceType;
    return true;
}

// Followers: the authority's snapshots drive the round, so only our own chomps and the HUD run
// here. Returns the hole our chomp reached this frame, for an eat request, or VILE_HOLE_NONE.
int32_t VileSync_FollowerGameUpdate(Actor* controller) {
    chMrVileMinigameActor* local = (chMrVileMinigameActor*)&controller->local;
    int32_t hole = VILE_HOLE_NONE;
    f32 dist;

    if (local->vile_marker == NULL) {
        local->vile_marker =
            actorArray_findClosestActorFromActorId(controller->position, ACTOR_13A_MR_VILE, -1, &dist)->marker;
    }
    if ((controller->state == 5) && (local->dialogIndex != 7)) {
        hole = VileSync_ChompedHole(local);
        if (local->type_change_timer > 3.5) {
            if (local->current_type != 0) {
                item_adjustByDiffWithHud(ITEM_1D_GRUMBLIE, false);
            } else {
                item_adjustByDiffWithHud(ITEM_1E_YUMBLIE, false);
            }
        }
        ml_timer_update(&local->type_change_timer, time_getDelta());
        item_set(ITEM_1A_PLAYER_VILE_SCORE, local->player_score);
        item_set(ITEM_1B_VILE_VILE_SCORE, local->vile_score);
    }
    return hole;
}

// Followers receive Mr. Vile's transform/anim from the network; keep only his body collision
// running locally.
void VileSync_FollowerVileUpdate(Actor* vile) {
    if (vile->state != CH_MR_VILE_STATE_4_ATTACK_PLAYER) {
        func_8028E668(vile->position, 100.0f, -50.0f, 120.0f);
    }
}

// Mr. Vile's anim mode (chMrVile_setAction modes 101-104), for streaming.
uint8_t VileSync_VileAnimMode(Actor* vile) {
    return ((ActorLocal_MrVile*)&vile->local)->action;
}

// --- Application layer ---------------------------------------------------------------

void VileSync_ApplyHoleState(int32_t holeId, int32_t holeState, int32_t pieceType, uint32_t eaterClientId) {
    Actor* yumblie = VileSync_FindHoleActor(holeId);

    (void)eaterClientId; // attribution is cosmetic for now; scores arrive via snapshot
    if (yumblie == NULL || yumblie->state == holeState) {
        return;
    }
    VileSync_ApplyYumblieState(yumblie, holeState, pieceType);
}

bool VileSync_HandleEatRequest(int32_t holeId, uint32_t eaterClientId, int32_t* outPieceType, int32_t* outCorrectType) {
    const float* holePos;
    Actor* controller;
    f32 pos[3];

    (void)eaterClientId;
    if (!VileSync_IsLiveAuthority()) {
        return false;
    }
    holePos = VileHoles_GetPosition(holeId);
    controller = VileSync_FindActor(ACTOR_138_VILE_GAME_CTRL);
    if (holePos == NULL || controller == NULL) {
        return false;
    }
    pos[0] = holePos[0];
    pos[1] = 0.0f; // piece positions are stored with y = 0 (see chMrVileMinigame_newPiece)
    pos[2] = holePos[1];
    // A successful consume broadcasts the eaten hole state itself (the state change
    // fires OnVileHoleStateChange on this authority client); the piece type / correctness
    // are reported back so the caller can confirm the eat to the requester.
    return VileSync_ConsumeRemote(controller, pos, outPieceType, outCorrectType);
}

void VileSync_PlayLocalEatFeedback(int32_t pieceType, int32_t correctType) {
    if (gsworld_getMap() != MAP_10_BGS_MR_VILE) {
        return;
    }
    func_8028F6B8(BS_INTR_17, (pieceType != YUMBLIE) ? ASSET_3F7_MODEL_GRUMBLIE : ASSET_3F6_MODEL_YUMBLIE);
    if (!correctType) {
        func_8028F66C(BS_INTR_18_CROC_ATE_WRONG);
    }
}

void VileSync_ApplyVileUpdate(const float position[3], float pitch, float yaw, float roll, uint8_t animMode) {
    Actor* vile = VileSync_FindActor(ACTOR_13A_MR_VILE);

    if (vile != NULL) {
        ActorLocal_MrVile* local = (ActorLocal_MrVile*)&vile->local;

        vile->position[0] = position[0];
        vile->position[1] = position[1];
        vile->position[2] = position[2];
        vile->pitch = pitch;
        vile->yaw = yaw;
        vile->roll = roll;
        if (local->action != animMode) {
            chMrVile_setAction(vile, animMode);
        }
    }
}

bool VileSync_GatherGameSnapshot(VileGameSnapshot* dst) {
    Actor* controller = VileSync_FindActor(ACTOR_138_VILE_GAME_CTRL);
    int32_t i;

    if (controller == NULL || !VileSync_GatherGame(controller, dst)) {
        return false;
    }
    for (i = 0; i < VILE_HOLE_COUNT; i++) {
        Actor* yumblie = VileSync_FindHoleActor(i);
        s32 type = 0;

        dst->holes[i].state = (yumblie != NULL) ? (uint8_t)VileSync_GetYumblieState(yumblie, &type) : 1;
        dst->holes[i].type = (uint8_t)type;
    }
    return true;
}

void VileSync_ApplyGameSnapshot(const VileGameSnapshot* src) {
    Actor* controller = VileSync_FindActor(ACTOR_138_VILE_GAME_CTRL);
    int32_t i;

    if (controller == NULL) {
        return;
    }
    VileSync_ApplyGame(controller, src);

    // Correct any hole whose up/down disposition drifted from the snapshot. In-sync
    // holes are left alone so the event stream keeps driving their animations.
    for (i = 0; i < VILE_HOLE_COUNT; i++) {
        Actor* yumblie = VileSync_FindHoleActor(i);
        s32 curType;
        s32 curState;
        bool snapUp;
        bool curUp;

        if (yumblie == NULL) {
            continue;
        }
        curState = VileSync_GetYumblieState(yumblie, &curType);
        snapUp = (src->holes[i].state == 2) || (src->holes[i].state == 3);
        curUp = (curState == 2) || (curState == 3);
        if (snapUp && !curUp) {
            VileSync_ApplyYumblieState(yumblie, 2, src->holes[i].type);
        } else if (!snapUp && curUp) {
            VileSync_ApplyYumblieState(yumblie, 4, curType);
        }
    }
}

void VileSync_OnAuthorityChanged(void) {
    Actor* controller;
    VileGameSnapshot idle;

    SeqGate_Reset(&gVileSeq);
    if (VileSync_IsLiveAuthority()) {
        return;
    }
    // We are not (or no longer) the live authority: wind a locally running or just-ended
    // round back to idle. If a new authority exists, its snapshots rebuild from there.
    controller = VileSync_FindActor(ACTOR_138_VILE_GAME_CTRL);
    if (controller != NULL && controller->state != 1 && VileSync_GatherGame(controller, &idle)) {
        idle.gameState = 1;
        VileSync_ApplyGame(controller, &idle);
    }
}
