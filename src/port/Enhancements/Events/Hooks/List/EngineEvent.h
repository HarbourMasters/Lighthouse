#pragma once

#include <libultraship/bridge/eventsbridge.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "prop.h"

#ifdef __cplusplus
}
#endif

DEFINE_EVENT(GameFrameUpdate);
DEFINE_EVENT(FrameDrawEnd);
DEFINE_EVENT(OnControllerRead, void* pad;);
DEFINE_EVENT(OnControllerUpdate);

DEFINE_EVENT(OnMapLoad, GameMap prevMap; GameMap nextMap; s32 exit;);
DEFINE_EVENT(OnMapLoaded, GameMap prevMap; GameMap nextMap; s32 exit;);
DEFINE_EVENT(OnMapLoadStub);

DEFINE_EVENT(OnDialogLoaded, s32 textId; char* text;);
DEFINE_EVENT(OnDialogUpdate, s32 textId; s32 state; s32 activeZoombox; BKDialog * *dialogs; u8 * stringCounts;
             s8 * stringCmds; u8 * stringIndices; char** strings; void** zoomboxes; u8 * pageState;);

DEFINE_EVENT(OnModelLoad, s32 modelId; void* modelInfo; s32 * reload;);

// Declared texture dimensions from a model's BKTextureInfo table.
typedef struct {
    u8 width;
    u8 height;
} ModelTexSize;

DEFINE_EVENT(OnModelDisplayListLoad, const char* path; u32 * dlWords; u32 dlWordCount; const ModelTexSize* texSizes;
             u16 texCount;);
DEFINE_EVENT(OnModelBinBuilt, const char* path; void* bin;);
DEFINE_EVENT(OnModelDraw, s32 modelId; void* modelBin;);
DEFINE_EVENT(OnModelRenderBegin, Gfx** gfx;);
DEFINE_EVENT(OnModelRenderEnd, Gfx** gfx;);
DEFINE_EVENT(ViewportFrustumUpdate, float* frustumX; float* frustumY;);
DEFINE_EVENT(OnViewportProjection, Gfx** gfx; Mtx * *mtx;);
DEFINE_EVENT(CameraRotationAuthored, s32 source; s32 id; const f32* position; f32 * rotation;);
DEFINE_EVENT(OnDynamicCameraUpdate, s32 state;);
DEFINE_EVENT(MapModelXluScale, s32 map; f32 * scale;);
DEFINE_EVENT(OnTransitionModelScale, Gfx** gfx; Mtx * *mtx; s32 uid; f32 * scale;);
DEFINE_EVENT(OnTransitionStateUpdate, s32 modelId; s32 uid; s32 substate;);
DEFINE_EVENT(DrawDistanceCubeWidth, int32_t mapWidth; int32_t * width;);
DEFINE_EVENT(CubeDrawRange, const int32_t* mapWidth; int32_t * min; int32_t * max;);
DEFINE_EVENT(MapUnderwaterTint, s32 map; s32 * tintMap;);

DEFINE_EVENT(OnActorTick, Actor* actor;);
DEFINE_EVENT(OnActorDrawBegin, Actor* actor;);
DEFINE_EVENT(OnActorDrawEnd, Actor* actor;);
DEFINE_EVENT(OnPropTick, ActorMarker* marker; float* position;);
DEFINE_EVENT(OnSpritePropTick, int32_t assetId; float* position;);
DEFINE_EVENT(OnNametagDraw, Actor* actor; const char* label; float yOffset;);
DEFINE_EVENT(LocalizeUiString, const char** str;);
DEFINE_EVENT(OnParadeNameDraw, const char* name; int32_t yPosition;);
DEFINE_EVENT(OnJinjoHeadDraw, s32 jinjoId;);
DEFINE_EVENT(OnBoldFontLetterBuilt, void* output; void* maskChunk; void* sphereChunk;);
DEFINE_EVENT(ResolveBoldFontHd, void* output; const char** path;);
DEFINE_EVENT(OnBoldFontReset);
DEFINE_EVENT(ResolveSpriteHdPath, const void* chunkAddr; const char** path;);
DEFINE_EVENT(ResolveAnimTextureHd, const void* textures; int32_t slot; int32_t frameOffset; uintptr_t * segment;);
DEFINE_EVENT(OnFileSelectInfoBuild, int32_t gamenum; char* upper; char* lower;);
DEFINE_EVENT(LocalizeFileSelectPrompt, int32_t promptId; void* zoombox;);
DEFINE_EVENT(OnFileSelectLanguageRefresh, int32_t gamenum; int32_t isSelected;);
DEFINE_EVENT(OnFileSelectPortrait, int32_t gamenum; void* zoombox;);
DEFINE_EVENT(LocalizeParade, int32_t paradeId; void** table; uint8_t * count;);
DEFINE_EVENT(ParadeCreditDialogId, int32_t index; int32_t * dialogId;);
DEFINE_EVENT(PresentDialogId, int32_t* dialogId;);
DEFINE_EVENT(ResolveBoldFontSlot, int32_t* slot; int32_t * letterId;);
DEFINE_EVENT(OnWorldDrawBegin);
DEFINE_EVENT(OnWorldDraw, Gfx** gfx; Mtx * *mtx; Vtx * *vtx;);
DEFINE_EVENT(OnPlayerDraw, Gfx** gfx; Mtx * *mtx; Vtx * *vtx;);
DEFINE_EVENT(OnHudDraw, Gfx** gfx; Mtx * *mtx; Vtx * *vtx;);
DEFINE_EVENT(OnPauseMenuDraw, Gfx** gfx; Mtx * *mtx; Vtx * *vtx; s32 menu; s32 headerY;);
DEFINE_EVENT(OnPauseMenuFree);
DEFINE_EVENT(OnZoomboxTextDraw, void* zoombox; f32 boxScale; f32 * scale; s32 * xOffset;);
DEFINE_EVENT(OnReset);