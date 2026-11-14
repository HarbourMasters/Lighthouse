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
DEFINE_EVENT(OnFinalBossSpawned)
DEFINE_EVENT(OnFinalBossDefeated)
DEFINE_EVENT(OnFinalBossSpell, int32_t kind;) // 0 fireball, 1 final-spell fireball, 2 green blast
DEFINE_EVENT(OnFinalBossFireballThrow, float flightTime;)
DEFINE_EVENT(OnFinalBossFlightPad, bool spawned;)
DEFINE_EVENT(OnFinalBossBarrier)
DEFINE_EVENT(OnJinjoStatueSpawned, int32_t statueId;)
DEFINE_EVENT(OnJinjoStatueSlam, int32_t statueId;)
DEFINE_EVENT(OnJinjoStatueEgg, int32_t statueId; int32_t padIndex;)
DEFINE_EVENT(OnBossJinjoHitUpdate, Actor* jinjo;)
DEFINE_EVENT(OnSmMovesCompleted)
DEFINE_EVENT(OnFpTwinklyMinigameEnd)
DEFINE_EVENT(OnJigsawPodiumInput, s32 podiumId;)
DEFINE_EVENT(OnMumboTokenUpdate, Actor* actor;)
DEFINE_EVENT(OnMumboTokenIdResolve, s32* tokenId; s32 * position; s32 mapId;)
DEFINE_EVENT(OnAnimCtrlStart, AnimCtrl* ctrl;)
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
// Mr. Vile minigame (Anchor sync). Fired only on the client actually running the local
// logic; followers have the originating code paths suppressed via ShouldActorUpdate listeners
// and VB_VILE_* behaviors.
DEFINE_EVENT(OnVileHoleStateChange, ActorMarker* marker; f32 * position; s32 state; s32 pieceType;)
DEFINE_EVENT(OnVileGameStateChange, s32 state;)
DEFINE_EVENT(OnPuzzleStep, int32_t puzzleId; int32_t bits;)                                      // enum AnchorPuzzle
DEFINE_EVENT(OnBreakableBroken, int32_t markerId; int32_t x; int32_t y; int32_t z; bool replay;) // marker_e
DEFINE_EVENT(OnBreakableSetUp, Actor* actor;)
DEFINE_EVENT(OnFlowerpotRemaining, int32_t total; int32_t * remaining;)
DEFINE_EVENT(OnHutSetUp, Actor* actor;)
DEFINE_EVENT(OnHutSmashed, Actor* actor; int32_t * loot;)
DEFINE_EVENT(OnEggTollSetUp, Actor* actor;)
DEFINE_EVENT(OnEggTollPaid, Actor* actor;)
DEFINE_EVENT(OnCageSwitchState, Actor* actor; int32_t state;) // chcageupswitch_state_e
DEFINE_EVENT(OnLairWaterLevel, int16_t* levels; float current; float* height;)
DEFINE_EVENT(OnCutsceneWarp, int32_t map; int32_t progressFlag;)
DEFINE_EVENT(OnCutsceneWarpEnd, int32_t returnMap; int32_t cameraScript;)
DEFINE_EVENT(OnMmWitchSwitchCutscene)
DEFINE_EVENT(OnBottlesTutorialAnswer, Actor* bottles; int32_t * answer;) // 1 tutorial, 0 skip
DEFINE_EVENT(OnPuzzlePosMark, int32_t puzzleId; int32_t x; int32_t y; int32_t z;)
// FP's snowman-button and Sir Slush managers, just before their win calls the puzzle VBs with no actor.
DEFINE_EVENT(OnPuzzleWin, int32_t puzzleId;)
DEFINE_EVENT(OnPuzzleCountAdd, int32_t counterId; int32_t delta;) // enum AnchorPuzzleCount
DEFINE_EVENT(OnClankerUpdate, int32_t state;)                     // maClankerState_e
DEFINE_EVENT(OnWaterRise, int32_t map; int32_t waterId; int32_t targetDy; f32 duration;)
