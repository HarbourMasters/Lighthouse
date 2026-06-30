#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"
#include "port/Patches/GeoCull.h"

#define CVAR_DRAW_DISTANCE CVAR_ENHANCEMENT("Graphics.DrawDistance")
#define CVAR_DISABLE_LOD CVAR_ENHANCEMENT("Graphics.DisableLOD")

static int sDrawDistanceLevel = 0;
static int sDisableLOD = 0;

extern "C" {
#include "enums.h"
#include "functions.h"

extern s32 gFramebufferWidth;
extern s32 gFramebufferHeight;
float GameEngine_GetAspectRatio(void);

extern Gfx D_80369238[];
extern s8 D_80380F20[0x80];
extern s32 D_80380AE8;
void* print_getCurrentFontPalette(u8 letterId);

// Widescreen HUD edge anchoring (centered-ortho HUD geometry).
float port_hudOrthoShift(float refX) {
    float halfW = (f32)gFramebufferWidth * 0.5f;
    float extraHalf = (f32)gFramebufferHeight * 0.5f * GameEngine_GetAspectRatio() - halfW;
    if (extraHalf < 0.0f) {
        extraHalf = 0.0f; // narrower than 4:3 (e.g. pillarboxed): never pull inward
    }
    if (refX < halfW) {
        return -extraHalf; // left-anchored
    }
    if (refX > halfW) {
        return extraHalf; // right-anchored
    }
    return 0.0f; // centered
}

// Ortho-quad HUD count renderer (used by the lives counter).
void port_drawLivesCount(Gfx** gfx, Mtx** mtx, Vtx** vtx, char* str, f32 baseX, f32 screenY) {
    s32 savedFont;
    f32 cursorX;
    f32 leftMargin;
    s32 i;

    if (str == NULL || str[0] == '\0') {
        return;
    }

    savedFont = D_80380AE8;
    D_80380AE8 = 1; // bold font slot, for print_getBoldFontLetterSprite / palette lookup
    leftMargin = 0.0f;

    // Same widescreen edge-anchor shift the heads use, kept in screen space.
    cursorX = baseX + port_hudOrthoShift(baseX);

    gSPDisplayList((*gfx)++, D_80369238);
    viewport_setRenderViewportAndOrthoMatrix(gfx, mtx);
    gDPSetCombineMode((*gfx)++, G_CC_DECALRGBA, G_CC_DECALRGBA);
    gDPSetPrimColor((*gfx)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);

    for (i = 0; str[i] != '\0'; i++) {
        s32 glyphId = D_80380F20[(u8) str[i]];
        s32 fontType;
        BKSpriteTextureBlock* glyph;
        uintptr_t texData;
        f32 drawX, drawY;
        f32 glyphW, glyphH;
        f32 centerX, centerY;
        s32 vy, vx;

        if (glyphId < 0) {
            continue;
        }
        glyph = print_getBoldFontLetterSprite(glyphId, &fontType);
        if (glyph == NULL) {
            continue;
        }

        if (leftMargin == 0.0f) {
            leftMargin = -(f32) glyph->x * 0.5f;
        }
        drawX = cursorX + leftMargin;
        drawY = screenY - (f32) glyph->h * 0.5f;

        texData = (uintptr_t)(glyph + 1);
        while (texData % 8) {
            texData++;
        }

        const char* hdPath = NULL;
        CALL_EVENT(ResolveSpriteHdPath, glyph, &hdPath);
        if (hdPath != NULL) {
            texData = (uintptr_t) hdPath;
        }
        const char* boldHdPath = NULL;
        CALL_EVENT(ResolveBoldFontHd, glyph, &boldHdPath);
        if (boldHdPath != NULL) {
            texData = (uintptr_t) boldHdPath;
        }

        if (fontType == SPRITE_TYPE_RGBA32) {
            gDPLoadTextureTile((*gfx)++, texData, G_IM_FMT_RGBA, G_IM_SIZ_32b, glyph->w, glyph->h, 0, 0,
                               glyph->x - 1, glyph->y - 1, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK,
                               G_TX_NOLOD, G_TX_NOLOD);
        } else if (fontType == SPRITE_TYPE_IA8) {
            gDPLoadTextureTile((*gfx)++, texData, G_IM_FMT_IA, G_IM_SIZ_8b, glyph->w, glyph->h, 0, 0, glyph->x - 1,
                               glyph->y - 1, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                               G_TX_NOLOD);
        } else if (fontType == SPRITE_TYPE_I8) {
            gDPLoadTextureTile((*gfx)++, texData, G_IM_FMT_I, G_IM_SIZ_8b, glyph->w, glyph->h, 0, 0, glyph->x - 1,
                               glyph->y - 1, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                               G_TX_NOLOD);
        } else if (fontType == SPRITE_TYPE_I4) {
            gDPLoadTextureTile_4b((*gfx)++, texData, G_IM_FMT_I, glyph->w, glyph->h, 0, 0, glyph->x - 1,
                                  glyph->y - 1, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                                  G_TX_NOLOD);
        } else if (fontType == SPRITE_TYPE_CI8) {
            void* pal = print_getCurrentFontPalette((u8) glyphId);
            gDPLoadTLUT_pal256((*gfx)++, pal);
            gDPLoadTextureTile((*gfx)++, texData, G_IM_FMT_CI, G_IM_SIZ_8b, glyph->w, glyph->h, 0, 0, glyph->x - 1,
                               glyph->y - 1, 0, G_TX_CLAMP, G_TX_CLAMP, G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD,
                               G_TX_NOLOD);
            gDPSetTextureLUT((*gfx)++, G_TT_RGBA16);
        } else {
            continue;
        }

        glyphW = (f32) glyph->x - 1.0f;
        glyphH = (f32) glyph->y - 1.0f;
        centerX = drawX - (f32) gFramebufferWidth * 0.5f;
        centerY = drawY - (f32) gFramebufferHeight * 0.5f - 0.5f;

        gSPVertex((*gfx)++, (uintptr_t)*vtx, 4, 0);
        for (vy = 0; vy < 2; vy++) {
            for (vx = 0; vx < 2; vx++) {
                (*vtx)->v.ob[0] = (s16)(s32)((centerX + glyphW * (f32) vx) * 4.0f);
                (*vtx)->v.ob[1] = (s16)(s32)((centerY + glyphH * (f32) vy) * -4.0f);
                (*vtx)->v.ob[2] = -0xA;
                (*vtx)->v.tc[0] = (s16)(s32)(glyphW * (f32) vx * 64.0f);
                (*vtx)->v.tc[1] = (s16)(s32)(glyphH * (f32) vy * 64.0f);
                (*vtx)->v.cn[0] = 0xFF;
                (*vtx)->v.cn[1] = 0xFF;
                (*vtx)->v.cn[2] = 0xFF;
                (*vtx)->v.cn[3] = 0xFF;
                (*vtx)++;
            }
        }
        gSP1Quadrangle((*gfx)++, 0, 1, 3, 2, 0);

        cursorX += (f32) glyph->x;
    }

    gDPPipeSync((*gfx)++);
    gDPSetTextureLUT((*gfx)++, G_TT_NONE);
    gDPPipelineMode((*gfx)++, G_PM_NPRIMITIVE);
    viewport_setRenderViewportAndPerspectiveMatrix(gfx, mtx);

    D_80380AE8 = savedFont;
}

int port_getDrawDistanceLevel(void) {
    int level = CVarGetInteger(CVAR_ENHANCEMENT("Graphics.DrawDistance"), 0);
    if (IsDemoMode() && getGameMode() != GAME_MODE_4_PAUSED) {
        level = 0;
    }
    return level;
}

float port_drawDistanceMul(void) {
    int lvl = port_getDrawDistanceLevel();
    if (lvl >= 4) {
        return 1e9f;
    }
    if (lvl > 0) {
        static const float scale[] = { 1.0f, 1.25f, 1.5811f, 2.2361f };
        return scale[lvl];
    }
    return 1.0f;
}

void port_applyModelDrawDistanceCull(int* fadeFlag, float* cullMult, float* cullDist) {
    int lvl = port_getDrawDistanceLevel();
    if (lvl >= 4) {
        *fadeFlag = 0;
        *cullMult = 1e30f;
        *cullDist = 1e30f;
    } else if (lvl > 0) {
        static const float cullDistScale[] = { 1.0f, 1.25f, 1.5811f, 2.2361f };
        *cullMult *= cullDistScale[lvl];
        *cullDist *= cullDistScale[lvl];
    }
}

int port_spriteSizeCulled(float depth, float size, float baseThreshold, int disableFlag) {
    if (disableFlag) {
        return 0;
    }
    int lvl = port_getDrawDistanceLevel();
    if (lvl >= 4) {
        return 0;
    }
    float scale = 1.0f;
    if (lvl > 0) {
        static const float spriteCullScale[] = { 1.0f, 1.6667f, 2.1082f, 2.9814f };
        scale = spriteCullScale[lvl];
    }
    return (3000.0f * scale < depth) && (((size / depth) * scale) < baseThreshold);
}

int port_shouldDisableLOD(void) {
    return sDisableLOD;
}
}

// ============================================================================
// LEVEL OCCLUSION — extend draw distance to camera-area portal geometry
// ============================================================================
//
// Some distant level geometry is hidden by BK's camera-area portal culling rather than the
// distance/LOD culls the prop draw-distance enhancement covers. Disabling that culling
// wholesale floods the render buffer, so at the maxed draw-distance level we force just the
// specific chunks known to suffer from it. Currently the only one is the Mumbo's Mountain
// stonehenge: a single "outside areas {1,2}" CAMERA command in the opaque map model.

static void OnGeoCull_LevelOcclusion(IEvent* event) {
    auto* ev = reinterpret_cast<OnGeoCull*>(event);
    if (ev->type != OCCLUSION_CMD_CAMERA) {
        return;
    }
    if (gsworld_getMap() == MAP_2_MM_MUMBOS_MOUNTAIN && ev->offset == 0x2CD0 &&
        ev->modelBin == (const void*)mapModel_getModelBin(0)) {
        *ev->forceDraw = true;
    }
}

void RegisterLevelOcclusion_Init() {
    bool maxed = CVarGetInteger(CVAR_DRAW_DISTANCE, 0) >= 4;
    GeoCull_SetConsumer(GEOCULL_CONSUMER_ENHANCEMENT, maxed);
    COND_HOOK(OnGeoCull, EVENT_PRIORITY_NORMAL, maxed, OnGeoCull_LevelOcclusion);
}

static RegisterShipInitFunc sInitLevelOcclusion(RegisterLevelOcclusion_Init, { CVAR_DRAW_DISTANCE });

static const int kCubeWidthByLevel[] = { 4, 6, 8, 10, 18 };

static void RegisterDrawDistanceGraphics_Init() {
    COND_HOOK(DrawDistanceCubeWidth, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_DRAW_DISTANCE, 0) > 0,
              [](IEvent* event) {
                  int lvl = port_getDrawDistanceLevel();
                  if (lvl <= 0) {
                      return;
                  }
                  auto* ev = (DrawDistanceCubeWidth*)event;
                  int width = (lvl <= 4) ? kCubeWidthByLevel[lvl] : 4;
                  if (width > ev->mapWidth) {
                      width = ev->mapWidth;
                  }
                  *ev->width = width;
              });
}

static RegisterShipInitFunc drawDistanceGraphicsInit(RegisterDrawDistanceGraphics_Init, { CVAR_DRAW_DISTANCE });

static void RefreshDrawDistanceCVars() {
    sDrawDistanceLevel = CVarGetInteger(CVAR_DRAW_DISTANCE, 0);
    sDisableLOD = CVarGetInteger(CVAR_DISABLE_LOD, 0);
}

static RegisterShipInitFunc drawDistanceCVarCache(RefreshDrawDistanceCVars, { CVAR_DRAW_DISTANCE, CVAR_DISABLE_LOD });
