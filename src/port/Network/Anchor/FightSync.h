#pragma once
#include <stdint.h>
#include <stdbool.h>

// Final Grunty fight sync (Anchor).
//
// The first client in the fight map (MAP_90_GL_BATTLEMENTS) with the boss spawned claims
// NET_ACTIVITY_FINAL_BOSS and runs the vanilla fight; everyone else in the map follows:
// their boss actor skips its brain and mirrors the authority instead. The sync logic lives
// in FightSync.c; the FINALE actors only call the small FightSync_* hooks below.
//
// Protocol overview:
//  - FIGHT_UPDATE  authority -> map: Grunty transform + state/phase stream (per frame,
//                  sequence-guarded so a stale packet can't roll a newer one back)
//  - FIGHT_STATE   authority -> one joining client: world catch-up snapshot (statues with
//                  their egg progress, flight pad, spell barrier)
//  - FIGHT_EVENT   both directions, one shot each:
//      authority -> map:  spell spawns, flight pad spawn/despawn, spell barrier, jinjo
//                         statue spawns, accepted statue eggs (EGG_FED), jinjo attacks
//      follower  -> map:  boss hits (my player damaged Grunty) and statue eggs, which
//                         only the live authority applies
//
// The statue subsystem stays deterministic on every client by replicating accepted eggs
// (EGG_FED): each client runs the same counters, so pedestal pads lower, stone jinjos
// break and — for the jinjonator — the endgame release triggers locally everywhere. From
// the release (chfinalboss_setBossDefeated) onward the fight is a fixed script, so every
// client leaves follower mode and plays the ending with its own local simulation.

// FIGHT_EVENT ids ('ev' field).
enum FightSyncEvent {
    // authority -> followers
    FIGHT_EV_SPELL = 0,       // a = kind (0 fireball, 1 final fireball, 2 green blast); p/v/w = spawn vectors
    FIGHT_EV_PAD_SPAWN,       // flight pad appears (end of phase 2)
    FIGHT_EV_PAD_DESPAWN,     // flight pad removed (during phase 3 barrier cast)
    FIGHT_EV_BARRIER,         // spell barrier spawns on Grunty
    FIGHT_EV_STATUE_SPAWN,    // a = ch_bossjinjo_e statue id (1-4 jinjos, 5 jinjonator)
    FIGHT_EV_EGG_FED,         // a = statue id (1-4 jinjos, 5 jinjonator), b = jinjonator pad index
    FIGHT_EV_JINJO_ATTACK,    // a = jinjo statue id whose jinjo just slammed Grunty
    // followers -> authority
    FIGHT_EV_BOSS_HIT,        // a = the phase the hitting client believed Grunty was in
    FIGHT_EV_EGG,             // a = statue id, b = jinjonator pad index
};

// Catch-up snapshot of the fight's world objects for a client that enters the map
// mid-fight (FIGHT_STATE packet). The boss itself rides the FIGHT_UPDATE stream; this
// carries what past FIGHT_EVENTs already built. Applied incrementally on the follower
// since the replayed actors take frames to come into existence.
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

// Boss lifecycle: fresh spawn resets the sync state; the jinjonator release drops every
// client (authority and followers alike) back to its own local simulation for the ending.
void FightSync_OnBossSpawned(void);
void FightSync_OnBossDefeated(void);

// Follower gate for chfinalboss_update: true = a remote authority drives this boss, skip
// the local brain (the mirrored cosmetics and the latecomer catch-up run inside).
bool FightSync_BossFollowerTick(void* boss /* Actor* */);

// Follower input forwarding. When these return true the local apply must be skipped — the
// input was sent to the fight authority instead (phase = the phase the hitter saw).
bool FightSync_ForwardBossHit(int32_t phase);
bool FightSync_ForwardEgg(int32_t statue_id, int32_t pad_index);

// Authority world replication (each is a no-op unless we are the live authority).
void FightSync_ReplicateEgg(int32_t statue_id, int32_t pad_index);
void FightSync_OnSpellSpawned(int32_t kind); // reads the chfinalboss spell-spawn globals
void FightSync_OnFlightPadSpawned(void);
void FightSync_OnFlightPadDespawned(void);
void FightSync_OnBarrierSpawned(void);
void FightSync_OnStatueSpawned(int32_t statue_id);
void FightSync_OnJinjoSlam(int32_t statue_id);

// --- FINALE entry points used by FightSync.c (implemented in the decomp actors) --------

// chjinjonatorbase.c: apply one networked egg to a pedestal pad / read the pad counters
// (returns false, pads defaulted, when the pedestal isn't up).
void chjinjonatorbase_netApplyEgg(int32_t pad_index);
bool chjinjonatorbase_netGetPads(uint8_t pads[4]);

// --- C -> network (bridges implemented in the packet .cpp files) -----------------------

void FightSync_SendUpdate(const float pos[3], float yaw, int32_t state, int32_t phase, int32_t mirror);
// v0/v1/v2 may be NULL for events that carry no vectors.
void FightSync_SendEvent(int32_t ev, int32_t a, int32_t b, const float v0[3], const float v1[3],
                         const float v2[3]);
// Send the authority's world snapshot to one (newly arrived) client.
void FightSync_SendSnapshot(uint32_t clientId);

// Stream sequence guard (implemented in FightUpdate.cpp).
uint32_t FightSyncSeq_Next(void);
bool FightSyncSeq_Accept(uint32_t seq);
void FightSyncSeq_Reset(void);

// --- network -> sync layer (implemented in FightSync.c) --------------------------------

// Called whenever the fight's authority owner changes (claim, tie-break, disconnect,
// owner left the map). A promoted client re-enters its current phase cleanly.
void FightSync_OnAuthorityChanged(void);

// Fill the stream fields from the live boss. Returns false when there is no boss to
// stream (not in the fight map / boss absent) or the ending script has taken over.
bool FightSync_GatherUpdate(float pos[3], float* yaw, int32_t* state, int32_t* phase, int32_t* mirror);

// Follower-side appliers.
void FightSync_ApplyUpdate(const float pos[3], float yaw, int32_t state, int32_t phase, int32_t mirror);
void FightSync_ApplyEvent(int32_t ev, int32_t a, int32_t b, const float v0[3], const float v1[3],
                          const float v2[3]);

// World snapshot: gather on the authority (false = nothing to send: no live fight, or the
// ending script took over) / stage on a joining follower for the incremental catch-up.
bool FightSync_GatherWorld(FightWorldSnapshot* snap);
void FightSync_ApplyWorld(const FightWorldSnapshot* snap);

#ifdef __cplusplus
}
#endif
