#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "port/Network/Anchor/VileHoles.h"

// Mr. Vile minigame sync (Anchor).
//
// The client running the minigame (the one that talked to Mr. Vile) is the authority:
// it runs the vanilla yumblie/Mr. Vile logic and broadcasts state. Other clients in
// MAP_10_BGS_MR_VILE suppress their local random logic and apply what arrives.
//
// Protocol overview:
//  - VILE_HOLE_STATE   authority -> map: a hole changed state (appear/hide/eaten)
//  - VILE_EAT_REQUEST  non-authority -> map: local player chomped a hole; authority
//                      validates and answers with VILE_HOLE_STATE (eaten) on success
//  - VILE_UPDATE       authority -> map: Mr. Vile transform + anim mode stream
//  - VILE_GAME_STATE   authority -> map: periodic full snapshot; idempotent correction
//                      layer that also covers late joiners
//
// Authority packets carry a monotonic sequence number so a stale snapshot can never
// roll back a newer event (and vice versa).

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VileHoleSnapshot {
    uint8_t state; // chyumblie_state_e
    uint8_t type;  // chvilegame_piece_type_e (yumblie/grumblie)
} VileHoleSnapshot;

typedef struct VileGameSnapshot {
    uint8_t gameState; // vile game controller Actor state
    uint8_t round;     // controller unkC
    uint8_t maxRound;  // controller unkD
    uint8_t currentType;
    float typeChangeTimer;
    uint8_t playerScore;
    uint8_t vileScore;
    int32_t hourglassRemaining; // ITEM_0_HOURGLASS_TIMER ticks left
    VileHoleSnapshot holes[VILE_HOLE_COUNT];
} VileGameSnapshot;

// In VILE_HOLE_STATE packets with an eaten state, identifies who ate the piece.
#define VILE_EATER_MR_VILE 0u

// Called whenever the vile minigame's authority owner changes (claim, release, tie-break
// loss, disconnect). Resets sequence counters and, on clients that are not the live
// authority, winds a locally running round back to idle.
void VileSync_OnAuthorityChanged(void);

// Sequence counter for outgoing authority packets.
uint32_t VileSync_NextOutgoingSeq(void);
// Returns false (and does not update) if seq is not newer than the last accepted one.
bool VileSync_AcceptIncomingSeq(uint32_t seq);
// Reset both counters; call on map load / authority change.
void VileSync_ResetSeq(void);

// Drive the yumblie actor at holeId into holeState with the given piece type.
// eaterClientId is only meaningful for the eaten state (VILE_EATER_MR_VILE = Mr. Vile).
void VileSync_ApplyHoleState(int32_t holeId, int32_t holeState, int32_t pieceType, uint32_t eaterClientId);

// Authority-side: Validate a remote client's request to eat a yumblie/grumblie, and report back
// whether it's allowed, as well as whether it matches the required type.
bool VileSync_HandleEatRequest(int32_t holeId, uint32_t eaterClientId, int32_t* outPieceType, int32_t* outCorrectType);

// Requester-side: Replay the local croc's eat feedback (chomp animation + chomp SFX,
// plus the wrong-type reaction) once the authority confirms the eat succeeded.
void VileSync_PlayLocalEatFeedback(int32_t pieceType, int32_t correctType);

// Apply a streamed Mr. Vile transform + anim mode (chMrVile_setAction modes 101-104).
void VileSync_ApplyVileUpdate(const float position[3], float pitch, float yaw, float roll, uint8_t animMode);

// Fill dst from the live minigame controller. Returns false if there is nothing to
// snapshot (controller absent or game not running).
bool VileSync_GatherGameSnapshot(VileGameSnapshot* dst);

// Force local minigame state to match an authoritative snapshot.
void VileSync_ApplyGameSnapshot(const VileGameSnapshot* src);

#ifdef __cplusplus
}
#endif
