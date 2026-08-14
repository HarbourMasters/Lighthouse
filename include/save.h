#ifndef __SAVE_H__
#define __SAVE_H__

#include "port/Rando/Types.h"

typedef enum {
    FILE_TYPE_SAVE_VANILLA,
    FILE_TYPE_SAVE_RANDO,
    FILE_TYPE_PRESET,
} FileType;

typedef struct {
    int32_t seedId;
    RandoSaveFlag randoSaveFlag[RANDO_INF_MAX];
    RandoSaveCheck randoSaveCheck[RC_MAX];
    RandoSaveOption randoSaveOption[RO_MAX];
} RandoSaveData;

// Note collection retention: per-map bitfield of collected music notes, keyed by
// (mapId, spawn-order noteIndex). Independent of rando check data so it works for
// vanilla and romhacks. Level attribution is derived dynamically via map_getLevel.
#define NOTE_RETENTION_MAP_SLOTS 0x100             // covers all map_e ids with headroom
#define NOTE_RETENTION_NOTES_PER_MAP 128           // max notes addressable per map
#define NOTE_RETENTION_BYTES_PER_MAP (NOTE_RETENTION_NOTES_PER_MAP / 8)

typedef struct {
    u8 collected[NOTE_RETENTION_MAP_SLOTS][NOTE_RETENTION_BYTES_PER_MAP];
} NoteRetentionSaveData;

// Jinjo collection retention: per-level bitmask of collected jinjo colours, using the
// same 5-bit layout as ITEM_12_JINJOS. Lets individually collected jinjos persist across
// level visits instead of resetting on every entry (vanilla requires all 5 in one go).
#define JINJO_RETENTION_LEVEL_SLOTS 0x20   // covers level_e ids (1..0xD) with headroom

typedef struct {
    u8 collected[JINJO_RETENTION_LEVEL_SLOTS];
} JinjoRetentionSaveData;

typedef struct {
    FileType fileType;
    uint64_t fileCreatedAt;
    RandoSaveData randoSaveData;
    NoteRetentionSaveData noteRetention;
    JinjoRetentionSaveData jinjoRetention;
} ShipSaveData;

typedef struct{
    u8 magic;
    u8 slotIndex;
    u8 data[0x70];
    u8 padding[0x2];
    u32 checksum;
    ShipSaveData shipSaveData;
}SaveData;

typedef struct {
    u32 snsItems;
    u8 padding[0x18];
    u32 checksum;
}GlobalData;

#endif
