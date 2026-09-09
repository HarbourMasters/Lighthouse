#ifndef BANJO_KAZOOIE_CORE1_OVERLAYMANAGER_H
#define BANJO_KAZOOIE_CORE1_OVERLAYMANAGER_H

#include <ultra64.h>
#include "enums.h"

#define MAKE_OVERLAY_SEGMENT_EXTERNS(segname) \
    extern u8 segname##_VRAM[]; \
    extern u8 segname##_VRAM_END[]; \
    extern u8 segname##_ROM_START[]; \
    extern u8 segname##_ROM_END[]; \
    extern u8 segname##_TEXT_START[]; \
    extern u8 segname##_TEXT_END[]; \
    extern u8 segname##_DATA_START[]; \
    extern u8 segname##_DATA_END[]; \
    extern u8 segname##_RODATA_START[]; \
    extern u8 segname##_RODATA_END[]; \
    extern u8 segname##_BSS_START[]; \
    extern u8 segname##_BSS_END[];

#define MAKE_SEGMENT_ENTRY(segname, realname) \
    {#realname, segname##_VRAM, segname##_VRAM_END, (u32) segname##_ROM_START, (u32) segname##_ROM_END, segname##_TEXT_START, segname##_TEXT_END, segname##_DATA_START, segname##_RODATA_END, segname##_BSS_START, segname##_BSS_END}

#define MAKE_DUMMY_SEGMENT_ENTRY(segname, realname) \
    {#realname, segname##_VRAM, segname##_VRAM_END, (u32) segname##_ROM_START, (u32) segname##_ROM_END, NULL, NULL, NULL, NULL, NULL, NULL}

struct overlay_address_map_s {
    char *name;
    u8 *ram_start, *ram_end;
    u32 rom_start;
    u32 rom_end;
    u8 *code_start, *code_end;
    u8 *data_start, *data_end;
    u8 *bss_start, *bss_end;
};

// [port] All overlay code is statically linked on PC, so these ROM segments
// do not exist. Kept for reference alongside sOverlayAdressMap in overlaymanager.c.
#if 0
MAKE_OVERLAY_SEGMENT_EXTERNS(core2)
MAKE_OVERLAY_SEGMENT_EXTERNS(emptyLvl)
MAKE_OVERLAY_SEGMENT_EXTERNS(CC)
MAKE_OVERLAY_SEGMENT_EXTERNS(MMM)
MAKE_OVERLAY_SEGMENT_EXTERNS(GV)
MAKE_OVERLAY_SEGMENT_EXTERNS(TTC)
MAKE_OVERLAY_SEGMENT_EXTERNS(MM)
MAKE_OVERLAY_SEGMENT_EXTERNS(BGS)
MAKE_OVERLAY_SEGMENT_EXTERNS(RBB)
MAKE_OVERLAY_SEGMENT_EXTERNS(FP)
MAKE_OVERLAY_SEGMENT_EXTERNS(CCW)
MAKE_OVERLAY_SEGMENT_EXTERNS(SM)
MAKE_OVERLAY_SEGMENT_EXTERNS(cutscenes)
MAKE_OVERLAY_SEGMENT_EXTERNS(lair)
MAKE_OVERLAY_SEGMENT_EXTERNS(fight)
#endif

enum overlay_e overlayManager_getLoadedID(void);
bool overlayManager_isOverlayLoaded(enum overlay_e id);
bool overlayManager_load(enum overlay_e id);
void overlayManager_clearLoadedId(void);
void overlayManager_loadCore2(void);
void overlayManager_debug(void);

#endif
