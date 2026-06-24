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

// Record + broadcast a local note pickup. Call from the actual collection, not proximity.
// marker is an ActorMarker* (void* to keep this header free of engine types).
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

#ifdef __cplusplus
}

// Helpers shared by both retention .cpp files (defined in Retention.cpp).
namespace retention {
// Active save slot index (0-3) for the current game, or -1 (default/demo file).
int32_t activeSlot();
// False during demos, Bottles bonus games, and rando files — where the vanilla retention
// systems must stay out of the way (rando has its own retention).
bool systemActive();
} // namespace retention
#endif

#endif
