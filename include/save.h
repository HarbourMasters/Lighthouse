#ifndef __SAVE_H__
#define __SAVE_H__

typedef enum {
    FILE_TYPE_SAVE_VANILLA,
} FileType;

typedef struct {
    FileType fileType;
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
