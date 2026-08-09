#pragma once
#include <stdint.h>
#include <stdbool.h>

// Final Grunty fight sync (Anchor).
//
// First client in the fight map with the boss spawned claims NET_ACTIVITY_FINAL_BOSS and runs
// the fight; everyone else mirrors it.
//
// FIGHT_UPDATE: authority -> map, per-frame transform/state stream, sequence-guarded.
// FIGHT_STATE: authority -> joining client, world catch-up snapshot.
// FIGHT_EVENT: one-shot; authority -> map for spells/pads/barrier/statues/eggs/jinjo
// attacks, follower -> authority for boss hits and statue eggs.

// FIGHT_EVENT ids ('ev' field).
enum FightSyncEvent {
    // authority -> followers
    FIGHT_EV_SPELL = 0,    // a = kind (0 fireball, 1 final fireball, 2 green blast); p/v/w = spawn vectors
    FIGHT_EV_PAD_SPAWN,    // flight pad appears (end of phase 2)
    FIGHT_EV_PAD_DESPAWN,  // flight pad removed (during phase 3 barrier cast)
    FIGHT_EV_BARRIER,      // spell barrier spawns on Grunty
    FIGHT_EV_STATUE_SPAWN, // a = ch_bossjinjo_e statue id (1-4 jinjos, 5 jinjonator)
    FIGHT_EV_EGG_FED,      // a = statue id (1-4 jinjos, 5 jinjonator), b = jinjonator pad index
    FIGHT_EV_JINJO_ATTACK, // a = jinjo statue id whose jinjo just slammed Grunty
    // followers -> authority
    FIGHT_EV_BOSS_HIT, // a = the phase the hitting client believed Grunty was in
    FIGHT_EV_EGG,      // a = statue id, b = jinjonator pad index
};

// FIGHT_STATE catch-up snapshot for a client joining mid-fight.
typedef struct FightWorldSnapshot {
    uint8_t pad;          // flight pad present
    uint8_t barrier;      // spell barrier up on Grunty
    uint8_t statue[4];    // jinjo statue bases (ids 1-4): 0xFF absent, else eggs fed (3 = broken)
    uint8_t jinjoGone[4]; // statue broken and its jinjo has already slammed into Grunty
    uint8_t jbase;        // jinjonator pedestal present
    uint8_t jpads[4];     // remaining eggs per jinjonator pedestal pad
} FightWorldSnapshot;

#ifdef __cplusplus
extern "C" {
#endif

// --- hooks called by the FINALE actors (implemented in FightSync.c) --------------------

// Boss lifecycle: spawn resets sync state; defeat drops every client to local simulation.
void FightSync_OnBossSpawned(void);
void FightSync_OnBossDefeated(void);

// True = a remote authority drives this boss; chfinalboss_update should skip its brain.
bool FightSync_BossFollowerTick(void* boss /* Actor* */);

// Forward follower input to the authority; true = handled remotely, skip local apply.
bool FightSync_ForwardBossHit(int32_t phase);
bool FightSync_ForwardEgg(int32_t statue_id, int32_t pad_index);

// Authority world replication (acts only as the live authority).
void FightSync_ReplicateEgg(int32_t statue_id, int32_t pad_index);
void FightSync_OnSpellSpawned(int32_t kind);
void FightSync_OnFlightPadSpawned(void);
void FightSync_OnFlightPadDespawned(void);
void FightSync_OnBarrierSpawned(void);
void FightSync_OnStatueSpawned(int32_t statue_id);
void FightSync_OnJinjoSlam(int32_t statue_id);

// --- FINALE entry points used by FightSync.c (implemented in the decomp actors) --------

void chjinjonatorbase_netApplyEgg(int32_t pad_index);
bool chjinjonatorbase_netGetPads(uint8_t pads[4]);

// --- C -> network (bridges implemented in the packet .cpp files) -----------------------

void FightSync_SendUpdate(const float pos[3], float yaw, int32_t state, int32_t phase, int32_t mirror, int32_t vuln);
// v0/v1/v2 may be NULL for events that carry no vectors.
void FightSync_SendEvent(int32_t ev, int32_t a, int32_t b, const float v0[3], const float v1[3], const float v2[3]);
void FightSync_SendSnapshot(uint32_t clientId);

uint32_t FightSyncSeq_Next(void);
bool FightSyncSeq_Accept(uint32_t seq);
void FightSyncSeq_Reset(void);

// --- network -> sync layer (implemented in FightSync.c) --------------------------------

void FightSync_OnAuthorityChanged(void);

// Fill the stream fields from the live boss; false if there's no boss to stream.
bool FightSync_GatherUpdate(float pos[3], float* yaw, int32_t* state, int32_t* phase, int32_t* mirror, int32_t* vuln);

void FightSync_ApplyUpdate(const float pos[3], float yaw, int32_t state, int32_t phase, int32_t mirror, int32_t vuln);
void FightSync_ApplyEvent(int32_t ev, int32_t a, int32_t b, const float v0[3], const float v1[3], const float v2[3]);

// Gather on the authority (false = nothing to send) / apply on a joining follower.
bool FightSync_GatherWorld(FightWorldSnapshot* snap);
void FightSync_ApplyWorld(const FightWorldSnapshot* snap);

#ifdef __cplusplus
}
#endif
