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

DEFINE_EVENT(OnMapLoad, GameMap prevMap; GameMap nextMap; s32 exit;);

DEFINE_EVENT(ViewportFrustumUpdate, float* frustumX; float* frustumY;);

DEFINE_EVENT(OnActorTick, Actor* actor;);
DEFINE_EVENT(OnPropTick, ActorMarker* marker; float* position;);
DEFINE_EVENT(OnSpritePropTick, int32_t assetId; float* position;);
DEFINE_EVENT(OnNametagDraw, Actor* actor; const char* label; float yOffset;);

DEFINE_EVENT(OnWorldDraw, Gfx** gfx; Mtx * *mtx; Vtx * *vtx;);
DEFINE_EVENT(OnPlayerDraw, Gfx** gfx; Mtx * *mtx; Vtx * *vtx;);

DEFINE_EVENT(OnReset);