// Sprite display data cache — replaces the ROM-based assetCacheCurrentIndex
// approach that doesn't work in the port (assetcache_get() bypasses the ROM cache,
// so index 0 would be reused for every sprite). Keyed on BKSprite pointer instead.

#include <libultraship.h>

extern "C" {

#include "structs.h"
#include "core1/sprite.h"

void codeAEDA0_setSpriteDrawMode(s32 arg0);
void func_80338308(s32 arg0, s32 arg1);
BKSpriteDisplayData* func_80344A1C(BKSprite* arg0);

#define SPRITE_DISPLAY_CACHE_SIZE 256

typedef struct {
    BKSprite* sprite;
    BKSpriteDisplayData* displayData;
} SpriteDisplayCacheEntry;

static SpriteDisplayCacheEntry sSpriteDisplayCache[SPRITE_DISPLAY_CACHE_SIZE];
static s32 sSpriteDisplayCacheCount = 0;

void port_spriteDisplayCache_clear(void) {
    sSpriteDisplayCacheCount = 0;
}

BKSpriteDisplayData* port_getOrCreateDisplayData(BKSprite* sprite) {
    s32 i;
    if (sprite == NULL) {
        return NULL;
    }
    for (i = 0; i < sSpriteDisplayCacheCount; i++) {
        if (sSpriteDisplayCache[i].sprite == sprite) {
            return sSpriteDisplayCache[i].displayData;
        }
    }
    if (sSpriteDisplayCacheCount < SPRITE_DISPLAY_CACHE_SIZE) {
        codeAEDA0_setSpriteDrawMode(-1);
        func_80338308(sprite_getUnk8(sprite), sprite_getUnkA(sprite));
        BKSpriteDisplayData* dd = func_80344A1C(sprite);
        sSpriteDisplayCache[sSpriteDisplayCacheCount].sprite = sprite;
        sSpriteDisplayCache[sSpriteDisplayCacheCount].displayData = dd;
        sSpriteDisplayCacheCount++;
        return dd;
    }
    codeAEDA0_setSpriteDrawMode(-1);
    func_80338308(sprite_getUnk8(sprite), sprite_getUnkA(sprite));
    return func_80344A1C(sprite);
}

} // extern "C"
