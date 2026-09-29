// BanjoDecomp: core2/code_1550.c
/**
 * @file anim/commoncache.c
 * @brief This file controls a cache of common animations consisting of 
 *    player move animations. This main difference between this cache and 
 *    the normal anim/cache.c is that these assets default to persist even
 *    after they become stall, and are cleaned up much later in memory 
 *    defragmentation.
 */
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#define ANIMATION_ASSETS_END        0x02CA
#define ANIMATION_ASSETS_BEGIN      0x0000

typedef struct animation_file_cache_s{
    BKAnimationFileBin *ptr;
    u16 exp_timer:15;
    u16 persist:1;
    u8  pad6[2];
}AnimationFileCache;

/* .data */
s16 animcommoncache_persistantList[] = {
    ASSET_1_ANIM_BSCROUCH_ENTER,
    ASSET_3_ANIM_BSWALK,
    ASSET_5_ANIM_BSPUNCH,
    ASSET_C_ANIM_BSWALK_RUN,
    ASSET_E_ANIM_BSTURN,
    ASSET_17_ANIM_BSBFLAP,
    ASSET_18_ANIM_BSBFLAP_ENTER,
    ASSET_19_ANIM_BSBPECK_ENTER,
    ASSET_1A_ANIM_BSBPECK,
    ASSET_1C_ANIM_BSBBARGE,
    ASSET_1D_ANIM_BSBBUSTER,
    0
};

/* .bss */
AnimationFileCache animcommoncache_list[ANIMATION_ASSETS_END - ANIMATION_ASSETS_BEGIN];

/* .code */
void __animcommoncache_initPersistent(void) {
    s16 *i_ptr;

    for (i_ptr = animcommoncache_persistantList; *i_ptr != 0; i_ptr++) {
        animcommoncache_list[*i_ptr].persist = TRUE;
    }
}

void __animcommoncache_loadAll(void) {
    s32 i;
    for (i = ANIMATION_ASSETS_BEGIN; i < ANIMATION_ASSETS_END; i++) {
        if (animcommoncache_list[i].persist) {
            animcommoncache_get(i);
        }
    }
}

BKAnimationFileBin *animcommoncache_get(enum asset_e asset_id) {
    if((u32)asset_id >= 0x2CA){ // [port] bounds check — 0xFFFE sentinel truncated in 15-bit bitfields overflows array
        return NULL;
    }
    if (!animcommoncache_list[asset_id].ptr) {
        animcommoncache_list[asset_id].ptr = (BKAnimationFileBin *) assetcache_get(asset_id);
    }

    animcommoncache_list[asset_id].exp_timer = 30;

    return animcommoncache_list[asset_id].ptr;
}

void animcommoncache_free(void) {
    int i;

    for (i = ANIMATION_ASSETS_BEGIN; i < ANIMATION_ASSETS_END; i++) {
        if (animcommoncache_list[i].ptr) {
            assetcache_release(animcommoncache_list[i].ptr);
        }
    }
}

void animcommoncache_init(void) {
    int i = 0;
    for (i = ANIMATION_ASSETS_BEGIN; i < ANIMATION_ASSETS_END; i++) {
        animcommoncache_list[i].ptr = NULL;
        animcommoncache_list[i].exp_timer = 0;
        animcommoncache_list[i].persist = 0;
    }

    __animcommoncache_initPersistent();
    __animcommoncache_loadAll();
}

void animcommoncache_flushStale(bool persistant) {
    int i;

    if (persistant) {
        for (i = ANIMATION_ASSETS_BEGIN; i < ANIMATION_ASSETS_END; i++) {
            if (animcommoncache_list[i].ptr && animcommoncache_list[i].persist && (animcommoncache_list[i].exp_timer < 30)) {
                assetcache_release(animcommoncache_list[i].ptr);
                animcommoncache_list[i].ptr = NULL;
                animcommoncache_list[i].persist = 0;
            }
        }
    } else {
        for (i = ANIMATION_ASSETS_BEGIN; i < ANIMATION_ASSETS_END; i++) {
            if (animcommoncache_list[i].ptr && !animcommoncache_list[i].persist && (animcommoncache_list[i].exp_timer < 30)) {
                assetcache_release(animcommoncache_list[i].ptr);
                animcommoncache_list[i].ptr = NULL;
                if (func_80254BC4(1)) {
                    break;
                }
            }
        }
    }
}

void animcommoncache_update(void) {
    int i;

    for(i = ANIMATION_ASSETS_BEGIN; i < ANIMATION_ASSETS_END; i++) {
        if (animcommoncache_list[i].ptr && !animcommoncache_list[i].persist) {
            if (--animcommoncache_list[i].exp_timer == 0) {
                assetcache_release(animcommoncache_list[i].ptr);
                animcommoncache_list[i].ptr = NULL;
            }
        }
    }
}
