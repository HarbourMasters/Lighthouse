// Sprite display data cache — replaces the ROM-based assetCacheCurrentIndex
// approach that doesn't work in the port (assetcache_get() bypasses the ROM cache,
// so index 0 would be reused for every sprite). Keyed on BKSprite pointer instead.

#include <libultraship.h>

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Patches/Patches.h"
#include "port/ShipInit.hpp"

extern "C" {

#include "structs.h"
#include "core1/sprite.h"
#include "enums.h"

void codeAEDA0_setSpriteDrawMode(s32 arg0);
void func_80338308(s32 arg0, s32 arg1);
BKSpriteDisplayData* func_80344A1C(BKSprite* arg0);

void* assetcache_get(enum asset_e assetId);
void assetcache_release(void* asset);
enum asset_e print_getCurrentMapBoldFontTexture(void);
char* ResourceMgr_ReloadByAssetId(uint32_t assetId);

enum level_e level_get(void);
s32 gcpausemenu_levelToMenuPage(enum level_e level);
extern s32 gFramebufferWidth;

extern BKSprite* print_sFontSpriteAssets[]; // font alphamask sprites ([0] = dialog font)
extern s32 print_sDialogFontGlyphCount;     // reachable glyph count: byte range 0x21 .. 0x21+count-1

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

// Recompute the dialog-font glyph count from the currently loaded slot-0 sprite. print_init
// sets this once from the base font, but a language pack can swap in an extended dialog font
// (more glyphs than the base 62) at runtime.
void port_refreshDialogFontGlyphCount(void) {
    if (print_sFontSpriteAssets[0] != NULL) {
        print_sDialogFontGlyphCount = sprite_getFramePtr(print_sFontSpriteAssets[0], 0)->chunkCnt;
    }
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

BKSprite* port_loadFilledBanner(s32 bannerAssetId, s32 fillId) {
    // Reload (not assetcache_get) so the cached, already-baked buffer isn't filled twice.
    BKSprite* banner = (BKSprite*)ResourceMgr_ReloadByAssetId((uint32_t)bannerAssetId);
    if (banner == NULL) {
        return NULL;
    }
    if (banner->type != SPRITE_TYPE_RGBA32) {
        return banner; // not a mask sprite; nothing to fill
    }

    if (fillId < 0) {
        fillId = print_getCurrentMapBoldFontTexture();
    }
    if (fillId == ASSET_708_SPRITE_EGG_PROJECTILE) {
        fillId = SPRITE_BOLD_FONT_FILL_TEXTURE; // sheet 13: default banner fill (JP)
    }

    BKSprite* fill = (BKSprite*)assetcache_get((enum asset_e)fillId);
    if (fill == NULL && fillId == SPRITE_BOLD_FONT_FILL_TEXTURE) {
        // Fallback if not using JP base
        fillId = ASSET_708_SPRITE_EGG_PROJECTILE;
        fill = (BKSprite*)assetcache_get((enum asset_e)fillId);
    }
    if (fill == NULL) {
        return banner;
    }

    BKSpriteFrame* bannerFrame = sprite_getFramePtr(banner, 0);
    BKSpriteTextureBlock* fillChunk = (BKSpriteTextureBlock*)(sprite_getFramePtr(fill, 0) + 1);
    const u8* fillData = (const u8*)(((uintptr_t)(fillChunk + 1) + 7) & ~(uintptr_t)7);

    BKSpriteTextureBlock* tb = (BKSpriteTextureBlock*)(bannerFrame + 1);
    for (s32 chunkIdx = 0; chunkIdx < bannerFrame->chunkCnt; chunkIdx++) {
        const s32 cw = tb->w;
        const s32 ch = tb->h;
        const s32 baseX = tb->x;
        const s32 baseY = tb->y;
        u8* px = (u8*)(((uintptr_t)(tb + 1) + 7) & ~(uintptr_t)7);

        for (s32 y = 0; y < ch; y++) {
            for (s32 x = 0; x < cw; x++) {
                const s32 gx = baseX + x; // banner-global coords (strips stack via baseY)
                const s32 gy = baseY + y;
                // JP 0x802f41e4 mapping: fillX = (fillW - chunkW/2)/2 + gx*0.5; fillY = gy.
                s32 fx = (s32)(((f32)fillChunk->w - (f32)cw * 0.5f) * 0.5f + (f32)gx * 0.5f);
                s32 fy = gy;
                fx = (fx < 0) ? 0 : ((fx > fillChunk->w - 1) ? fillChunk->w - 1 : fx);
                fy = (fy < 0) ? 0 : ((fy > fillChunk->h - 1) ? fillChunk->h - 1 : fy);

                const u8* fpx = fillData + (fx + fy * fillChunk->w) * 2; // RGBA16, N64 big-endian
                const u16 pixel = (u16)((fpx[0] << 8) | fpx[1]);
                s32 r5 = (pixel >> 11) & 0x1F;
                s32 g5 = (pixel >> 6) & 0x1F;
                s32 b5 = (pixel >> 1) & 0x1F;

                u8* maskPx = px + (x + y * cw) * 4;
                const s32 intensity = maskPx[2]; // mask byte[2] = intensity
                const u8 alpha = maskPx[3];      // mask byte[3] = alpha
                // 5-bit channel * 8-bit intensity / 31 expands to 8-bit exactly (31 * 255 / 31 == 255).
                // Dividing first truncates to 0..8, blacking out any intensity below 0x1F.
                r5 = (r5 * intensity) / 0x1F;
                g5 = (g5 * intensity) / 0x1F;
                b5 = (b5 * intensity) / 0x1F;
                maskPx[0] = (u8)r5;
                maskPx[1] = (u8)g5;
                maskPx[2] = (u8)b5;
                maskPx[3] = alpha;
            }
        }

        tb = (BKSpriteTextureBlock*)(px + (size_t)cw * ch * 4);
    }

    assetcache_release(fill);
    return banner;
}

// World-name pause-menu banner (JP ships these natively; a language pack can re-point them too)
static BKSprite* sPauseBanner = NULL;
static s32 sPauseBannerPage = -1;
static s32 sPauseBannerAlpha = 0;
#define PAUSE_BANNER_LAST_PAGE 11

s32 port_pauseBannerUpdate(s32 page_id) {
    // Draw a pre-rendered world-name banner when one exists for this page. The JP cart
    // ships them, and a language pack can supply its own; if none resolves,
    // port_loadFilledBanner returns NULL below and the caller falls back to drawing the
    // world name as bold-font text.
    s32 page = page_id;
    if (page < 0 || page > PAUSE_BANNER_LAST_PAGE) {
        page = -1;
    }
    if (page != sPauseBannerPage) {
        if (sPauseBanner != NULL) {
            assetcache_release(sPauseBanner);
            sPauseBanner = NULL;
        }
        sPauseBannerPage = page;
        sPauseBannerAlpha = 0;
        if (page >= 0) {
            s32 fillId = (page != 0 && page == gcpausemenu_levelToMenuPage(level_get())) ? 0x6e7 : -1;
            sPauseBanner = port_loadFilledBanner(SPRITE_WORLD_NAME_TOTAL + page, fillId);
        }
    }
    if (sPauseBannerAlpha < 100) {
        sPauseBannerAlpha += 10;
        if (sPauseBannerAlpha > 100) {
            sPauseBannerAlpha = 100;
        }
    }
    return sPauseBanner != NULL;
}

BKSprite* port_pauseBannerGetDraw(s32 headerY, f32* outX, f32* outY, f32* outW, f32* outH) {
    if (sPauseBanner == NULL) {
        return NULL;
    }
    BKSpriteFrame* bf = sprite_getFramePtr(sPauseBanner, 0);
    f32 bs = sPauseBannerAlpha / 100.0f;
    f32 bw = bf->w * bs;
    f32 bh = bf->h * bs;

    // Scale down banners wider than viewport
    f32 maxBw = gFramebufferWidth * 0.92f;
    if (bw > maxBw) {
        f32 fit = maxBw / bw;
        bw *= fit;
        bh *= fit;
    }
    *outX = gFramebufferWidth * 0.5f - bw * 0.5f;
    *outY = (f32)headerY - bh * 0.5f;
    *outW = bw;
    *outH = bh;
    return sPauseBanner;
}

// Release the banner held for the current page
void port_pauseBannerFree(void) {
    if (sPauseBanner != NULL) {
        assetcache_release(sPauseBanner);
        sPauseBanner = NULL;
    }
    sPauseBannerPage = -1;
    sPauseBannerAlpha = 0;
}

} // extern "C"

static void RegisterSpriteRestoreAlphaCompare_Init() {
    COND_VB_SHOULD(VB_SPRITE_RESTORE_ALPHA_COMPARE, EVENT_PRIORITY_NORMAL, true, { *should = true; });
}

static RegisterShipInitFunc sSpriteRestoreAlphaCompareInit(RegisterSpriteRestoreAlphaCompare_Init);
