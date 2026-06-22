#ifndef PORT_NOTE_RETENTION_H
#define PORT_NOTE_RETENTION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif
