#ifndef PORT_RETENTION_H
#define PORT_RETENTION_H

// Shared header for the vanilla collectible-retention systems (notes, jinjos and consumables).

#include <stdint.h>

#include "port/UI/cvar_prefixes.h"
#include "save.h"

#define CVAR_NOTE_RETENTION CVAR_ENHANCEMENT("Gameplay.NoteRetention")

#ifdef __cplusplus
extern "C" {
#endif

// --- Note retention ---

// Live slot's note-retention bytes for Anchor team-state sync (size 0 / null if no slot).
void NoteRetention_GetSizeAndPtr(int32_t* size, uint8_t** addr);

// Recompute the live note counter from the bitfield, deferred to normal play. Call after anything
// overwrites the bitfield wholesale (team-state sync).
void NoteRetention_RequestReseed(void);

void NoteRetention_ApplyRemoteCollect(int32_t mapId, int32_t noteIndex, int32_t sameMap);

// Force retention on/off regardless of the user CVar (Anchor uses this while connected).
void NoteRetention_SetForced(int32_t forced);

// --- Jinjo retention ---

// Live slot's jinjo-retention bytes for Anchor team-state sync (size 0 / null if no slot).
void JinjoRetention_GetSizeAndPtr(int32_t* size, uint8_t** addr);

// Same as the note variant, for ITEM_12_JINJOS.
void JinjoRetention_RequestReseed(void);

void JinjoRetention_ApplyRemoteCollect(int32_t map, int32_t bit, int32_t sameMap);

// Force retention on/off regardless of the user CVar (Anchor uses this while connected).
void JinjoRetention_SetForced(int32_t forced);

void setCollectedBits(int32_t level, u8 bits);
uint8_t jinjoBitFromActor(int32_t actorId);
uint8_t collectedBits(int32_t level);

#ifdef __cplusplus
}

namespace retention {
// False during demos, Bottles bonus games, and rando files.
bool systemActive();
} // namespace retention
#endif

#endif
