#ifndef PORT_RETENTION_H
#define PORT_RETENTION_H

// Shared header for the vanilla collectible-retention systems (notes and jinjos). Each
// persists which individual collectibles the player has picked up so they don't respawn on
// revisit; both stay out of the way on demos, Bottles bonus games, and rando files.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- Note retention ---

void port_noteRetention_beginMapLoad(int32_t mapId);
void port_noteRetention_onActorsFreed(void);

// Live slot's note-retention bytes for Anchor team-state sync (size 0 / null if no slot).
void port_noteRetention_getSizeAndPtr(int32_t* size, uint8_t** addr);

// Apply a remote note pickup (record bit; if sameMap, credit count + despawn our note).
void port_noteRetention_applyRemoteCollect(int32_t mapId, int32_t noteIndex, int32_t sameMap);

// Record + broadcast a local note pickup (marker is ActorMarker*, void* to avoid engine types).
void port_noteRetention_onLocalNoteCollected(void* marker);

// Force retention on/off regardless of the user CVar (Anchor uses this while connected).
void port_noteRetention_setForced(int32_t forced);

// --- Jinjo retention ---

// Live slot's jinjo-retention bytes for Anchor team-state sync (size 0 / null if no slot).
void port_jinjoRetention_getSizeAndPtr(int32_t* size, uint8_t** addr);

// Apply a remote jinjo pickup (record bit for collector's level; if sameMap, HUD + despawn).
void port_jinjoRetention_applyRemoteCollect(int32_t map, int32_t bit, int32_t sameMap);

// Record + broadcast a local jinjo pickup. Call from the actual collection, not proximity.
void port_jinjoRetention_onLocalJinjoCollected(int32_t markerId);

// Force retention on/off regardless of the user CVar (Anchor uses this while connected).
void port_jinjoRetention_setForced(int32_t forced);

// --- CCW carried-collectible live-despawn sync (worms for Eyrie, acorns for Nabnut) ---
//
// Shared pool: carried count (ITEM_22/23) is delta-synced via COLLECT_ITEM. Each object's
// identity is a hash of its fixed spawn position (stable across clients); the picked-up set
// persists for the session so it also suppresses re-visits. `kind` = ANCHOR_COLLECTIBLE_WORM/
// _ACORN; marker is ActorMarker* (void* to avoid engine types in this header).

// Single map-load entry point (currently a no-op; identity/state aren't per-map).
void port_carriedSync_beginMapLoad(int32_t mapId);

// Registers a world object, attaching spawn-position identity to the marker. *suppress is
// set non-zero if a teammate already collected it — caller should not present it.
void port_carriedSync_register(int32_t kind, void* marker, int32_t x, int32_t y, int32_t z,
                               int32_t* suppress);

// Record + broadcast a local pickup (marks the object collected and fires the COLLECT_ITEM).
void port_carriedSync_onLocalCollect(int32_t kind, void* marker);

// Broadcast a local spend (feeding Eyrie/Nabnut): a -1 to the shared pool, no object identity.
void port_carriedSync_onLocalSpend(int32_t kind);

// Apply a teammate's pickup (id = spawn-position hash): marks it collected so our matching
// object despawns via consumeRemoteDespawn, and later visits suppress it.
void port_carriedSync_applyRemoteCollect(int32_t kind, int32_t mapId, int32_t id, int32_t sameMap);

// True if a teammate collected this object (caller should despawn). Pull-based so we never
// dereference a marker the engine may have already freed.
int32_t port_carriedSync_consumeRemoteDespawn(int32_t kind, void* marker);

// Team's session collected count for this kind. Quest actors with a fixed spend target
// (Blubber's gold, FP cubs' presents) use this to rebuild the transient pool count on entry.
int32_t port_carriedSync_collectedCount(int32_t kind);

#ifdef __cplusplus
}

// Helpers shared by both retention .cpp files (defined in Retention.cpp).
namespace retention {
// Active save slot index (0-3) for the current game, or -1 (default/demo file).
int32_t activeSlot();
// False during demos, Bottles bonus games, and rando files (rando has its own retention).
bool systemActive();
} // namespace retention
#endif

#endif
