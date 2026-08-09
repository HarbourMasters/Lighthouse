#ifndef PORT_RETENTION_H
#define PORT_RETENTION_H

// Shared header for the vanilla collectible-retention systems (notes and jinjos).

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- Note retention ---

void port_noteRetention_beginMapLoad(int32_t mapId);
void port_noteRetention_onActorsFreed(void);

// Live slot's note-retention bytes for Anchor team-state sync (size 0 / null if no slot).
void port_noteRetention_getSizeAndPtr(int32_t* size, uint8_t** addr);

void port_noteRetention_applyRemoteCollect(int32_t mapId, int32_t noteIndex, int32_t sameMap);

// Record + broadcast a local note pickup (marker is ActorMarker*, void* to avoid engine types).
void port_noteRetention_onLocalNoteCollected(void* marker);

// Force retention on/off regardless of the user CVar (Anchor uses this while connected).
void port_noteRetention_setForced(int32_t forced);

// --- Jinjo retention ---

// Live slot's jinjo-retention bytes for Anchor team-state sync (size 0 / null if no slot).
void port_jinjoRetention_getSizeAndPtr(int32_t* size, uint8_t** addr);

void port_jinjoRetention_applyRemoteCollect(int32_t map, int32_t bit, int32_t sameMap);

// Record + broadcast a local jinjo pickup. Call from the actual collection, not proximity.
void port_jinjoRetention_onLocalJinjoCollected(int32_t markerId);

// Force retention on/off regardless of the user CVar (Anchor uses this while connected).
void port_jinjoRetention_setForced(int32_t forced);

// --- CCW carried-collectible live-despawn sync (worms for Eyrie, acorns for Nabnut) ---
//
// Carried count is ITEM_22/23. Object identity is a hash of its fixed spawn position. `kind` =
// ANCHOR_COLLECTIBLE_WORM/_ACORN; marker is ActorMarker* (void* to avoid engine types).

void port_carriedSync_beginMapLoad(int32_t mapId);

// Registers a world object, attaching spawn-position identity to the marker. Sets *suppress
// non-zero if a teammate already collected it.
void port_carriedSync_register(int32_t kind, void* marker, int32_t x, int32_t y, int32_t z, int32_t* suppress);

void port_carriedSync_onLocalCollect(int32_t kind, void* marker);

// Broadcast a local spend (feeding Eyrie/Nabnut): -1 to the shared pool.
void port_carriedSync_onLocalSpend(int32_t kind);

// Apply a teammate's pickup (id = spawn-position hash): marks it collected.
void port_carriedSync_applyRemoteCollect(int32_t kind, int32_t mapId, int32_t id, int32_t sameMap);

// True if a teammate collected this object (caller should despawn).
int32_t port_carriedSync_consumeRemoteDespawn(int32_t kind, void* marker);

int32_t port_carriedSync_collectedCount(int32_t kind);

void setCollectedBits(int32_t level, u8 bits);
uint8_t jinjoBitFromActor(int32_t actorId);
uint8_t collectedBits(int32_t level);

#ifdef __cplusplus
}

namespace retention {
// Active save slot index (0-3) for the current game, or -1 (default/demo file).
int32_t activeSlot();
// False during demos, Bottles bonus games, and rando files.
bool systemActive();
} // namespace retention
#endif

#endif
