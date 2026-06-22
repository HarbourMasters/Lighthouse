#ifndef PORT_JINJO_RETENTION_H
#define PORT_JINJO_RETENTION_H

// Jinjo Collection Retention is implemented entirely with port event listeners
// (collection, level-load seeding, spawn suppression) and needs no engine-side hooks.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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
#endif

#endif
