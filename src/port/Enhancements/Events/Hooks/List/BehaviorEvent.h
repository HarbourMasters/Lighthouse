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
DEFINE_EVENT(OnMrVileSetSpeed, f32* speed;)
DEFINE_EVENT(OnFurnaceFunDialog, s32* lifeThreshold;)
// The quiz entering a gcquiz_state. zoomboxes holds its four GcZoombox*, and answerValues[i] == 1 marks the box
// with the right answer.
DEFINE_EVENT(OnQuizStateChange, s32 state; s32 questionType; s32 questionIndex; void** zoomboxes; s8 * answerValues;)
DEFINE_EVENT(OnQuizUpdate, s32 state; s32 questionType; s32 assetId; void** questionBin; u8 * answerStrCounts;
             s8 * answerValues; uintptr_t * answerOptions; void** zoomboxes;)
DEFINE_EVENT(OnLairRelease)
DEFINE_EVENT(OnRollInit)
DEFINE_EVENT(OnRollUpdate, s32* nextState;)
DEFINE_EVENT(OnGruntyJinjonatorComplete)
DEFINE_EVENT(OnIntroCutsceneCheck, bool* skipIntro;)
DEFINE_EVENT(OnMiscCutscenesCheck, bool* skipMiscCutscenes;)
DEFINE_EVENT(OnTooieJiggyCollect, f32* position;)
DEFINE_EVENT(OnJigsawPodiumInput, s32 podiumId;)
DEFINE_EVENT(OnJigsawPedestalLeave, s32 id;)
DEFINE_EVENT(OnMumboTokenUpdate, Actor* actor;)
DEFINE_EVENT(OnMumboTokenIdResolve, s32* tokenId; s32 * position; s32 mapId;)
DEFINE_EVENT(OnPlayerAnimChange, AssetID anim_id; f32 duration; AnimControl control; f32 start_position;
             f32 subrange_end; bool smooth;)
DEFINE_EVENT(OnPlayerAnimReset)
DEFINE_EVENT(OnPlayerTransformChange, Transformation tf_id;)
DEFINE_EVENT(OnPlayerAnimSubRangeChange, f32 duration; f32 end_position;)
DEFINE_EVENT(OnWaterPyramidTimer, s32* timer;)
// Fired per conditional geometry-cull command in the map model (camera-area portal, LOD
// band, frustum sphere). Listeners may set *forceDraw to draw a chunk the vanilla gate
// would skip. `type` is OCCLUSION_CMD_* (see GeoCull.h); `offset` is the command's byte
// offset within `modelBin` (a stable per-asset key). areaIds/areaCount apply to CAMERA.
DEFINE_EVENT(OnGeoCull, s32 type; s32 offset; const void* modelBin; const u8* areaIds; s32 areaCount; s32 detail0;
             s32 detail1; s32 drawnVanilla; bool* forceDraw;)
DEFINE_EVENT(OnJiggyCageStateChange, s32 state;)
DEFINE_EVENT(OnEggTollAdvance, s32 map; s32 secondaryId; s32 tollNum;)
DEFINE_EVENT(OnSwitchCutscene, s32 map; s32 progressFlag;)
DEFINE_EVENT(OnWaterRise, s32 map; s32 waterId; s32 targetDy; f32 duration;)
DEFINE_EVENT(OnTwinklyMinigameEnd)
DEFINE_EVENT(OnHutSmash, s32 x; s32 y; s32 z; s32 loot;)
DEFINE_EVENT(OnBreakableBroken, s32 markerId; s32 x; s32 y; s32 z; bool replay;)
DEFINE_EVENT(OnPuzzleStep, s32 puzzleId; s32 bits;)
DEFINE_EVENT(OnPuzzlePosMark, s32 puzzleId; s32 x; s32 y; s32 z;)
// Mr. Vile minigame (Anchor sync). Fired only on the client actually running the local
// logic; followers have the originating code paths suppressed via VB_VILE_* behaviors.
DEFINE_EVENT(OnVileHoleStateChange, ActorMarker* marker; f32 * position; s32 state; s32 pieceType;)
DEFINE_EVENT(OnVileGameStateChange, s32 state;)
