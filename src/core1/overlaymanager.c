#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"

#if 0
/* .data */
static struct overlay_address_map_s sOverlayAdressMap[] = {
    MAKE_SEGMENT_ENTRY(core2, gs),
    MAKE_DUMMY_SEGMENT_ENTRY(emptyLvl, coshow),
    MAKE_SEGMENT_ENTRY(CC, whale),
    MAKE_SEGMENT_ENTRY(MMM, haunted),
    MAKE_SEGMENT_ENTRY(GV, desert),
    MAKE_SEGMENT_ENTRY(TTC, beach),
    MAKE_SEGMENT_ENTRY(MM, jungle),
    MAKE_SEGMENT_ENTRY(BGS, swamp),
    MAKE_SEGMENT_ENTRY(RBB, ship),
    MAKE_SEGMENT_ENTRY(FP, snow),
    MAKE_SEGMENT_ENTRY(CCW, tree),
    MAKE_SEGMENT_ENTRY(SM, training),
    MAKE_SEGMENT_ENTRY(cutscenes, intro),
    MAKE_SEGMENT_ENTRY(lair, witch),
    MAKE_SEGMENT_ENTRY(fight, battle),
};

static s32 sNumOverlays = sizeof(sOverlayAdressMap) / sizeof(sOverlayAdressMap[0]);
#endif

// [port] On PC all overlay code is statically linked — no dynamic loading needed.
// We just track which overlay ID is "loaded" so overlay_init() can find the right
// function table entry.
static enum overlay_e sLoadedOverlay = OVERLAY_0_CORE2;

/* .code */
static struct overlay_address_map_s *__overlayManager_getLargestOverlayAdressMap(void) {
#if 0
    int i;
    struct overlay_address_map_s *largest_overlay = &sOverlayAdressMap[1];

    for (i = 1; i < sNumOverlays; i++) {
        if (largest_overlay->ram_end - largest_overlay->ram_start < (u32)(sOverlayAdressMap[i].ram_end - sOverlayAdressMap[i].ram_start)) {
            largest_overlay = &sOverlayAdressMap[i];
        }
    }
    return largest_overlay;
#endif
    return NULL;
}

// returns always 0
static s32 __overlayManager_stub1(void) {
    return 0;
}

static s32 __overlayManager_getUknownSize(void) {
#if 0
    int unused;
    struct overlay_address_map_s *largest_overlay;
    s32 sp1C;
    s32 sp18;

    largest_overlay = __overlayManager_getLargestOverlayAdressMap();
    sp18 = func_802546DC();
    sp1C = __overlayManager_stub1();

    return (u8 *) gFramebuffers + sp1C - largest_overlay->ram_end + sp18;
#endif
    return 0;
}

static void __overlayManager802511C4(void) {
#if 0
    s32 sp24;
    s32 sp20;
    s32 sp1C;
    s32 heap_size;
    u32 tmp_v0;

    sp24 = __overlayManager_getUknownSize();
    heap_size = heap_get_size();
    sp20 = func_802546DC();
    sp1C = heap_size - sp20;

    if (sp24 < 0) {
        overlayManager_debug();
        tmp_v0 = sp1C + sp24;
        while (tmp_v0 & 0xF) { tmp_v0--; }
    }
#endif
}

enum overlay_e overlayManager_getLoadedID(void) {
    return sLoadedOverlay;
}

bool overlayManager_isOverlayLoaded(enum overlay_e id) {
    return sLoadedOverlay == id;
}

bool overlayManager_load(enum overlay_e id) {
    // [port] On PC, all overlay code is statically linked. Just track the ID.
    if (id == OVERLAY_0_CORE2)
        return false;
    if (id == sLoadedOverlay)
        return false;

    sLoadedOverlay = id;
#if 0
    {
        s32 rom_addr = (s32)(sOverlayAdressMap + id);
        overlay_load(
            id,
            ((struct overlay_address_map_s *)rom_addr)->ram_start,
            ((struct overlay_address_map_s *)rom_addr)->ram_end,
            ((struct overlay_address_map_s *)rom_addr)->rom_start,
            ((struct overlay_address_map_s *)rom_addr)->rom_end,
            ((struct overlay_address_map_s *)rom_addr)->code_start,
            ((struct overlay_address_map_s *)rom_addr)->code_end,
            ((struct overlay_address_map_s *)rom_addr)->data_start,
            ((struct overlay_address_map_s *)rom_addr)->data_end,
            ((struct overlay_address_map_s *)rom_addr)->bss_start,
            ((struct overlay_address_map_s *)rom_addr)->bss_end
        );
    }
#endif
    return true;
}

void overlayManager_clearLoadedId(void) {
    sLoadedOverlay = OVERLAY_0_CORE2;
}

void overlayManager_loadCore2(void) {
    overlayManager_clearLoadedId();
#if 0
    overlay_load(0,
        core2_VRAM, core2_VRAM_END,
        (u32) core2_ROM_START, (u32) core2_ROM_END,
        core2_TEXT_START, core2_TEXT_END,
        core2_DATA_START, core2_RODATA_END,
        core2_BSS_START, core2_BSS_END
    );
    __overlayManager802511C4();
#endif
}

void overlayManager_debug(void) {}
