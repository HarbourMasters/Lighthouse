#include <algorithm>
#include <cstring>
#include <vector>

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Engine.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"
#include "port/Patches/GeoCull.h"

#define CVAR_DRAW_DISTANCE CVAR_ENHANCEMENT("Graphics.DrawDistance")
#define CVAR_DISABLE_LOD CVAR_ENHANCEMENT("Graphics.DisableLOD")
#define CVAR_FP_LOBBY_DOOR_TILE CVAR_ENHANCEMENT("Fixes.FPLobbyDoorTile")

static const int kMaxDrawDistanceMul = 6;
static int sDrawDistanceCubeWidth(int mul) {
    return 4 * mul;
}

static int sDrawDistanceLevel = 1;
static int sDisableLOD = 0;

extern "C" {
#include <ultra64.h>
#include "enums.h"
#include "functions.h"
#include "libultraship/libultra/gbi.h"
#include "port/Romhack/RomhackConfig.h"

extern s32 gFramebufferWidth;
extern s32 gFramebufferHeight;

// Romhack model display lists can leave a palette (TLUT) mode enabled and leak it
// into the next model's draw.
void port_modelRenderResetTLUT(Gfx** gfx) {
    if (!port_isRomhack()) {
        return;
    }
    gDPSetTextureLUT((*gfx)++, G_TT_NONE);
}

// Widescreen HUD edge anchoring (centered-ortho HUD geometry).

static const float kHudCenterBand = 32.0f;

float port_hudOrthoShift(float refX) {
    float halfW = (f32)gFramebufferWidth * 0.5f;
    float extraHalf = (f32)gFramebufferHeight * 0.5f * GameEngine_GetAspectRatio() - halfW;
    if (extraHalf < 0.0f) {
        extraHalf = 0.0f; // narrower than 4:3 (e.g. pillarboxed): never pull inward
    }
    if (refX < halfW - kHudCenterBand) {
        return -extraHalf; // left-anchored
    }
    if (refX > halfW + kHudCenterBand) {
        return extraHalf; // right-anchored
    }
    return 0.0f; // centered
}

int port_getDrawDistanceSetting(void) {
    return sDrawDistanceLevel;
}

int port_getDrawDistanceLevel(void) {
    int mul = port_getDrawDistanceSetting();
    if (IsDemoMode() && getGameMode() != GAME_MODE_4_PAUSED) {
        mul = 1;
    }
    return mul;
}

float port_drawDistanceMul(void) {
    int level = port_getDrawDistanceLevel();
    if (level <= 1) {
        return 1.0f;
    }
    return (float)level + 0.1f; // Nudge
}

void port_applyModelDrawDistanceCull(int* fadeFlag, float* cullMult, float* cullDist) {
    float mul = port_drawDistanceMul();
    *cullMult *= mul;
    *cullDist *= mul;
}

int port_spriteSizeCulled(float depth, float size, float baseThreshold, int disableFlag) {
    if (disableFlag) {
        return 0;
    }
    float scale = port_drawDistanceMul();
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
    // Offset into the opaque map model's geo command list (the "outside areas {1,2}" CAMERA
    // command). Read it off the Occlusion Debugger, which reports the same geo-relative key.
    if (gsworld_getMap() == MAP_2_MM_MUMBOS_MOUNTAIN && ev->offset == 0x2C98 &&
        ev->modelBin == (const void*)mapModel_getModelBin(0)) {
        *ev->forceDraw = true;
    }
}

void RegisterLevelOcclusion_Init() {
    bool maxed = CVarGetInteger(CVAR_DRAW_DISTANCE, 1) >= kMaxDrawDistanceMul;
    GeoCull_SetConsumer(GEOCULL_CONSUMER_ENHANCEMENT, maxed);
    COND_HOOK(OnGeoCull, EVENT_PRIORITY_NORMAL, maxed, OnGeoCull_LevelOcclusion);
}

static RegisterShipInitFunc sInitLevelOcclusion(RegisterLevelOcclusion_Init, { CVAR_DRAW_DISTANCE });

static void RegisterDrawDistanceGraphics_Init() {
    COND_HOOK(DrawDistanceCubeWidth, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_DRAW_DISTANCE, 1) > 1,
              [](IEvent* event) {
                  int mul = port_getDrawDistanceLevel();
                  if (mul <= 1) {
                      return;
                  }
                  auto* ev = (DrawDistanceCubeWidth*)event;
                  int width = sDrawDistanceCubeWidth(mul);
                  if (width > ev->mapWidth) {
                      width = ev->mapWidth;
                  }
                  *ev->width = width;
              });
}

static RegisterShipInitFunc drawDistanceGraphicsInit(RegisterDrawDistanceGraphics_Init, { CVAR_DRAW_DISTANCE });

static void RefreshDrawDistanceCVars() {
    int mul = CVarGetInteger(CVAR_DRAW_DISTANCE, 1);
    if (mul < 1) {
        mul = 1;
    }
    if (mul > kMaxDrawDistanceMul) {
        mul = kMaxDrawDistanceMul;
    }
    sDrawDistanceLevel = mul;
    sDisableLOD = CVarGetInteger(CVAR_DISABLE_LOD, 0);
}

static RegisterShipInitFunc drawDistanceCVarCache(RefreshDrawDistanceCVars, { CVAR_DRAW_DISTANCE, CVAR_DISABLE_LOD });

// ============================================================================
// STALE TILE DESCRIPTOR — Freezeezy Peak lobby door trim
// ============================================================================

// F3DEX opcodes as used by BK64.
static const uint8_t kOpVtx = 0x04;
static const uint8_t kOpTri2 = 0xB1;
static const uint8_t kOpTri1 = 0xBF;
static const uint8_t kOpSetTileSize = 0xF2;
static const uint8_t kOpSetTile = 0xF5;
static const uint8_t kOpSetTImg = 0xFD;

static const uint32_t kTxClampBit = 0x2; // G_TX_CLAMP
static const uint8_t kRenderTile = 0;    // G_TX_RENDERTILE

// Torch tags a G_SETTIMG that references an extracted texture by putting 0xFF in
// the top byte of w1 and the texture index in the low 24 bits.
static const uint32_t kTexMarker = 0xFF;

static const char kFPLobbySuffix[] = "GL_FP_LOBBY_OPA";

static bool sPathEndsWith(const char* path, const char* suffix) {
    if (path == NULL) {
        return false;
    }
    size_t plen = strlen(path);
    size_t slen = strlen(suffix);
    return plen >= slen && strcmp(path + plen - slen, suffix) == 0;
}

static void OnModelDisplayListLoad_StaleTile(IEvent* event) {
    auto* ev = reinterpret_cast<OnModelDisplayListLoad*>(event);
    if (ev->dlWords == NULL || ev->texSizes == NULL || !sPathEndsWith(ev->path, kFPLobbySuffix)) {
        return;
    }

    // Walk the list tracking live render-tile state, flagging any G_SETTILE whose
    // clamp causes a larger texture to be drawn through a smaller tile.
    std::vector<uint32_t> toPatch;
    int curTex = -1;
    uint32_t liveTile = UINT32_MAX;
    uint32_t tileW = 0, tileH = 0;

    for (uint32_t i = 0; i + 1 < ev->dlWordCount; i += 2) {
        uint32_t w0 = ev->dlWords[i];
        uint32_t w1 = ev->dlWords[i + 1];
        uint8_t op = (uint8_t)(w0 >> 24);

        if (op == kOpSetTImg) {
            if (((w1 >> 24) & 0xFF) == kTexMarker) {
                curTex = (int)(w1 & 0x00FFFFFF);
            }
        } else if (op == kOpSetTile) {
            if (((w1 >> 24) & 0x7) == kRenderTile) {
                liveTile = i;
            }
        } else if (op == kOpSetTileSize) {
            if (((w1 >> 24) & 0x7) == kRenderTile) {
                uint32_t uls = (w0 >> 12) & 0xFFF;
                uint32_t ult = w0 & 0xFFF;
                uint32_t lrs = (w1 >> 12) & 0xFFF;
                uint32_t lrt = w1 & 0xFFF;
                tileW = ((lrs - uls) / 4) + 1;
                tileH = ((lrt - ult) / 4) + 1;
            }
        } else if (op == kOpVtx || op == kOpTri1 || op == kOpTri2) {
            if (curTex < 0 || liveTile == UINT32_MAX || tileW == 0 || tileH == 0) {
                continue;
            }
            if ((uint32_t)curTex >= ev->texCount) {
                continue;
            }
            uint32_t declW = ev->texSizes[curTex].width;
            uint32_t declH = ev->texSizes[curTex].height;
            if (declW <= tileW && declH <= tileH) {
                continue;
            }
            uint32_t tileWord = ev->dlWords[liveTile + 1];
            uint32_t cms = (tileWord >> 8) & 0x3;
            uint32_t cmt = (tileWord >> 18) & 0x3;
            if (((cms & kTxClampBit) == 0) && ((cmt & kTxClampBit) == 0)) {
                continue;
            }
            // Only safe to swap clamp for wrap when the mask makes the wrap period
            // exactly the texture size.
            uint32_t masks = (tileWord >> 4) & 0xF;
            uint32_t maskt = (tileWord >> 14) & 0xF;
            if (masks == 0 || maskt == 0 || (1u << masks) != declW || (1u << maskt) != declH) {
                continue;
            }
            if (std::find(toPatch.begin(), toPatch.end(), liveTile) == toPatch.end()) {
                toPatch.push_back(liveTile);
            }
        }
    }

    for (size_t n = 0; n < toPatch.size(); n++) {
        ev->dlWords[toPatch[n] + 1] &= ~((kTxClampBit << 18) | (kTxClampBit << 8));
    }
}

static void RegisterStaleTileFix_Init() {
    COND_HOOK(OnModelDisplayListLoad, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_FP_LOBBY_DOOR_TILE, 0),
              OnModelDisplayListLoad_StaleTile);
}

static RegisterShipInitFunc staleTileFixInit(RegisterStaleTileFix_Init, { CVAR_FP_LOBBY_DOOR_TILE });

// ============================
// INHERITED RENDER STATE
// ============================

static void ApplySnowRenderState(Gfx** gfx) {
    gSPClearGeometryMode((*gfx)++, G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG | G_LIGHTING | G_TEXTURE_GEN |
                                       G_TEXTURE_GEN_LINEAR | G_LOD | G_SHADING_SMOOTH);
    gSPSetGeometryMode((*gfx)++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH);
    gDPSetAlphaCompare((*gfx)++, G_AC_NONE);
    gDPSetPrimColor((*gfx)++, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetEnvColor((*gfx)++, 0xFF, 0xFF, 0xFF, 0xFF);
    gDPSetBlendColor((*gfx)++, 0x00, 0x00, 0x00, 0x80);
}

static void RegisterSnowRenderState_Init() {
    COND_VB_SHOULD(VB_SNOW_RENDER_STATE, EVENT_PRIORITY_NORMAL, true, {
        ApplySnowRenderState(va_arg(args, Gfx**));
        *should = false;
    });
}

static RegisterShipInitFunc sSnowRenderStateInit(RegisterSnowRenderState_Init);

static void RegisterModelDrawDistFadeAlpha_Init() {
    COND_VB_SHOULD(VB_MODEL_DRAWDIST_FADE_ALPHA, EVENT_PRIORITY_NORMAL, true, {
        s32* alpha = va_arg(args, s32*);
        *alpha = std::clamp(*alpha, 0, 0xFF);
    });
}

static RegisterShipInitFunc sModelDrawDistFadeAlphaInit(RegisterModelDrawDistFadeAlpha_Init);
