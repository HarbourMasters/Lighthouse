#include "VileSync.h"
#include "port/Network/Anchor/Authority.h"

#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "prop.h"

// Implemented in src/BGS (see [port] Anchor sync entry points there).
extern void chyumblie_netApplyState(Actor* actor, s32 state, s32 piece_type);
extern s32 chyumblie_netGetState(Actor* actor, s32* piece_type);
extern void chvile_netApplyUpdate(Actor* actor, const f32 position[3], f32 pitch, f32 yaw, f32 roll, u8 anim_mode);
extern s32 chvile_netGetAnimMode(Actor* actor);
extern bool chvilegame_netGather(Actor* actor, VileGameSnapshot* dst);
extern void chvilegame_netApply(Actor* actor, const VileGameSnapshot* src);
extern bool chvilegame_netConsumeRemote(Actor* actor, f32 position[3], s32* out_piece_type, s32* out_correct_type);
extern void chvilegame_netPlayEatFeedback(s32 piece_type, s32 correct_type);

static uint32_t sOutgoingSeq = 0;
static uint32_t sLastAcceptedSeq = 0;

uint32_t VileSync_NextOutgoingSeq(void) {
    return ++sOutgoingSeq;
}

bool VileSync_AcceptIncomingSeq(uint32_t seq) {
    if (seq <= sLastAcceptedSeq) {
        return false;
    }
    sLastAcceptedSeq = seq;
    return true;
}

void VileSync_ResetSeq(void) {
    sOutgoingSeq = 0;
    sLastAcceptedSeq = 0;
}

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

// --- Application layer ---------------------------------------------------------------

void VileSync_ApplyHoleState(int32_t holeId, int32_t holeState, int32_t pieceType, uint32_t eaterClientId) {
    Actor* yumblie = VileSync_FindHoleActor(holeId);

    (void)eaterClientId; // attribution is cosmetic for now; scores arrive via snapshot
    if (yumblie == NULL || yumblie->state == holeState) {
        return;
    }
    chyumblie_netApplyState(yumblie, holeState, pieceType);
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
    pos[1] = 0.0f; // piece positions are stored with y = 0 (see chvilegame_new_piece)
    pos[2] = holePos[1];
    // A successful consume broadcasts the eaten hole state itself (the state change
    // fires OnVileHoleStateChange on this authority client); the piece type / correctness
    // are reported back so the caller can confirm the eat to the requester.
    return chvilegame_netConsumeRemote(controller, pos, outPieceType, outCorrectType);
}

void VileSync_PlayLocalEatFeedback(int32_t pieceType, int32_t correctType) {
    if (gsworld_getMap() != MAP_10_BGS_MR_VILE) {
        return;
    }
    chvilegame_netPlayEatFeedback(pieceType, correctType);
}

void VileSync_ApplyVileUpdate(const float position[3], float pitch, float yaw, float roll, uint8_t animMode) {
    Actor* vile = VileSync_FindActor(ACTOR_13A_MR_VILE);

    if (vile != NULL) {
        chvile_netApplyUpdate(vile, position, pitch, yaw, roll, animMode);
    }
}

bool VileSync_GatherGameSnapshot(VileGameSnapshot* dst) {
    Actor* controller = VileSync_FindActor(ACTOR_138_VILE_GAME_CTRL);
    int32_t i;

    if (controller == NULL || !chvilegame_netGather(controller, dst)) {
        return false;
    }
    for (i = 0; i < VILE_HOLE_COUNT; i++) {
        Actor* yumblie = VileSync_FindHoleActor(i);
        s32 type = 0;

        dst->holes[i].state = (yumblie != NULL) ? (uint8_t)chyumblie_netGetState(yumblie, &type) : 1;
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
    chvilegame_netApply(controller, src);

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
        curState = chyumblie_netGetState(yumblie, &curType);
        snapUp = (src->holes[i].state == 2) || (src->holes[i].state == 3);
        curUp = (curState == 2) || (curState == 3);
        if (snapUp && !curUp) {
            chyumblie_netApplyState(yumblie, 2, src->holes[i].type);
        } else if (!snapUp && curUp) {
            chyumblie_netApplyState(yumblie, 4, curType);
        }
    }
}

void VileSync_OnAuthorityChanged(void) {
    Actor* controller;
    VileGameSnapshot idle;

    VileSync_ResetSeq();
    if (VileSync_IsLiveAuthority()) {
        return;
    }
    // We are not (or no longer) the live authority: wind a locally running or just-ended
    // round back to idle. If a new authority exists, its snapshots rebuild from there.
    controller = VileSync_FindActor(ACTOR_138_VILE_GAME_CTRL);
    if (controller != NULL && controller->state != 1 && chvilegame_netGather(controller, &idle)) {
        idle.gameState = 1;
        chvilegame_netApply(controller, &idle);
    }
}
