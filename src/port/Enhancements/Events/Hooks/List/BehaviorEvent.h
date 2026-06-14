#pragma once

#include <libultraship/bridge/eventsbridge.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "prop.h"

#ifdef __cplusplus
}
#endif

DEFINE_EVENT(OnBeakSwimVelocitySet, f32* velocity;)
DEFINE_EVENT(OnBoggyRaceSetSpeed, f32* speed;)
DEFINE_EVENT(OnBootLogosCheck, bool* skipLogos;)
DEFINE_EVENT(OnFurnaceFunDialog, s32* lifeThreshold;)
DEFINE_EVENT(OnGruntyJinjonatorComplete)
DEFINE_EVENT(OnIntroCutsceneCheck, bool* skipIntro;)
DEFINE_EVENT(OnMumboTokenUpdate, Actor* actor;)
DEFINE_EVENT(OnMumboTokenIdResolve, s32* tokenId; s32 * position; s32 mapId;)
DEFINE_EVENT(OnPlayerAnimChange, AssetID anim_id; f32 duration; AnimControl control; f32 start_position;
             f32 subrange_end; bool smooth;)
DEFINE_EVENT(OnPlayerAnimReset)
DEFINE_EVENT(OnPlayerTransformChange, Transformation tf_id;)
DEFINE_EVENT(OnPlayerAnimSubRangeChange, f32 duration; f32 end_position;)
DEFINE_EVENT(OnWaterPyramidTimer, s32* timer;)
// Mr. Vile minigame (Anchor sync). Fired only on the client actually running the local
// logic; followers have the originating code paths suppressed via VB_VILE_* behaviors.
DEFINE_EVENT(OnVileHoleStateChange, ActorMarker* marker; f32 * position; s32 state; s32 pieceType;)
DEFINE_EVENT(OnVileGameStateChange, s32 state;)
