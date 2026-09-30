#ifndef PORT_PATCHES_H
#define PORT_PATCHES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Threaded rendering (Game.cpp)

// Carries the interpolation pair with a submitted display list.
void port_thread5_onSubmit(void* taskData);
// Runs fn on the window thread when the tick is on its own thread.
void port_runOnRenderThread(void (*fn)(void*), void* arg);
// Waits for in-flight display lists before freeing or reading what they use.
void port_pipelineSyncPoint(void);

// Frame Pacing (FramePacingPatches.cpp)

int port_getDemoViCount(void);
void port_setDemoViCount(int viCount);
int port_getDemoDisplayViCount(int rawViCount);
void port_tickCutsceneStutter(void);
int port_getCutsceneExtraVis(void);

// Localization (Localization.cpp)

int port_pauseMenuNeedsRefresh(void); // language or Return-to-Lair CVar changed while menu open
void port_pauseMenuRebuild(void);     // free + recreate + replay the main menu open
void port_setPrintScale(float scale);

// Dialog font (AltDialogFont.cpp, SpritePatches.cpp)

void port_dialogFontHd_rebuild(void);        // rebuild the HD glyph textures for the current base
void port_refreshDialogFontGlyphCount(void); // re-read the reachable glyph count from the active font

// Framebuffer (FramebufferPatches.cpp)

void port_freezeReadback(int freeze);
void port_requestReadback(void);
int port_consumeReadbackRequest(void);
int port_getPauseFramebufferId(void);
int port_capturePauseFramebuffer(void);
void port_getPauseFramebufferSize(int* w, int* h);
int port_pauseConsumeRecaptureRequest(void);
int port_shouldCaptureTransition(void);

void port_patchPictureModel(void* model_bin, int32_t min_xy, int32_t max_xy, int32_t min_z, int32_t max_z,
                            uint32_t from);
int32_t port_getTransitionGpuFbId(void);
void port_captureTransitionFb(void* gfx_ptr);
void port_patchTransitionModel(void* model_bin);

// Sprite Display Cache (SpritePatches.cpp)

void port_spriteDisplayCache_clear(void);

// Save (SaveEnhancements.cpp)

void port_syncBottlesBonusIndex(void);

// Camera (CameraPatches.cpp)

float port_wsCameraYawScale(void);
float port_wsCameraPitchScale(void);

// Input

float port_getRumbleScale(void);

// Gameplay

int port_scalePlayerDamage(int damage);

// Graphics (GraphicsPatches.cpp)

int port_getDrawDistanceSetting(void); // configured multiplier; safe during map load
int port_getDrawDistanceLevel(void);   // render-time multiplier; clamped to 1x outside normal gameplay
int port_shouldDisableLOD(void);
int port_shouldDisableCulling(void);
int port_isDemoPlayback(void);
float port_drawDistanceMul(void);
void port_applyModelDrawDistanceCull(int* fadeFlag, float* cullMult, float* cullDist);
int port_spriteSizeCulled(float depth, float size, float baseThreshold, int disableFlag);
float port_hudOrthoShift(float refX);
void port_modelRenderResetTLUT(Gfx** gfx);

// Vertex-animated models (AnimVertexPatches.cpp)

// Recycles the per-draw vertex arena; once per tick, before the display list is built.
void port_animVtx_beginTick(void);
// Copies the just-posed vertices somewhere only this draw points at, and repoints
// segment 0x01 at the copy.
void port_modelRender_snapshotAnimVertices(Gfx** gfx, void* vertices, int32_t count);

// Mirror (MirrorPatches.cpp)

int port_mirror_active(void);
void port_mirror_beginScene(void);
void port_mirror_endScene(void);
void port_mirror_undoProjection(Gfx** gfx, Mtx** mtx);
void port_viewport_applyMirror(Gfx** gfx, Mtx** mtx);
void port_mirror_markCapture(void);
int port_mirror_shouldFlipPauseBg(void);

// Mirror per-model exclusion (counter-mirror text-bearing objects)
void port_mirror_setExclude(void);
void port_mirror_clearExclude(void);
int port_mirror_bakeCounterScale(void);
void port_mirror_patchTextActors(void);

// Volatile flag checks

int port_isInCharacterParade(void);

// Audio engine lock

void port_lockAudio(void);
void port_unlockAudio(void);
void port_audioIntMaskEnter(void);
void port_audioIntMaskExit(void);

// Romhacks

void* port_getRomhackResumeWarpFunc(void);
void romhack_RewriteActorSpawn(void* actorInfo, u32* flags);

// Attract-demo audio hold

void port_beginDemoAudioHold(void);
void port_tickDemoAudioHold(void);
int port_audioHeld(void);
void port_noteMainLoopAlive(void);
int port_audioStallHold(void);
int32_t port_audioPumpShouldWait(void);

// One-shot cues when a teammate's file-progress flag arrives
void port_progressFlag_remoteCue(int32_t progressFlag);
void port_notedoor_remoteOpen(int32_t progressFlag);
void port_leveldoor_remoteOpen(int32_t progressFlag);

void port_breakable_remoteBreak(int32_t progressFlag);

void port_breakable_broadcastBreak(int32_t markerId, int32_t x, int32_t y, int32_t z);
void port_breakable_remoteBreakAt(int32_t markerId, int32_t x, int32_t y, int32_t z);

void port_breakable_recordBreak(int32_t markerId, int32_t x, int32_t y, int32_t z);

int32_t port_breakable_isBroken(int32_t map, int32_t markerId, int32_t x, int32_t y, int32_t z);

int32_t port_cutsceneWarp_getReturnMap(void);

void port_eggToll_onAdvance(int32_t map, int32_t secondaryId, int32_t stage);
int32_t port_eggToll_getStage(int32_t map, int32_t secondaryId);
void port_eggToll_remoteApply(int32_t map, int32_t secondaryId, int32_t stage);

#define ANCHOR_PUZZLE_BGS_TANKTUP 1
#define ANCHOR_PUZZLE_BGS_CROCTUS 2
#define ANCHOR_PUZZLE_BGS_PINKEGG 3
// CC clanker teeth: bits 0-2 = token tooth egg count, bits 3-5 = jiggy tooth.
#define ANCHOR_PUZZLE_CC_CLANKER_TEETH 4
#define ANCHOR_PUZZLE_GV_JINXY_DOOR 5
// MM Juju totem: bits 0-3, count of the 4 segments knocked off.
#define ANCHOR_PUZZLE_MM_JUJU 6
// TTC Nipper: bits 0-2, count of hits taken (all 3 = dead, shell open).
#define ANCHOR_PUZZLE_TTC_NIPPER 7
// TTC Blubber: bit 0 = first bullion delivered, bit 1 = second (jiggy spawned, Blubber leaves).
#define ANCHOR_PUZZLE_TTC_BLUBBER 8
// TTC treasure hunt: bits 0-5, count of beak-busted X steps (bit 5 = treasure dug up).
#define ANCHOR_PUZZLE_TTC_XHUNT 9
// FP xmas tree ice: 1 bit, set when the tree-top ice shatters. Recorded in the tree-interior
// map; the FP hub tree reads it via getForMap.
#define ANCHOR_PUZZLE_FP_TREE_ICE 10
// FP bear cubs' presents: bit 0 = blue, bit 1 = green, bit 2 = red delivered.
#define ANCHOR_PUZZLE_FP_PRESENTS 11
// FP snowman buttons: bit per button; all three spawns JIGGY_2D.
#define ANCHOR_PUZZLE_FP_SNOWBUTTONS 12
#define ANCHOR_PUZZLE_FP_SLUSHES 13
// RBB engine-room fans: 1 bit. Transient map flag 0 (enginefan.c reads it to slow the fans); persisted across reloads.
#define ANCHOR_PUZZLE_RBB_ENGINE_FANS 14
// SM intro Bottles: bit 0 = the tutorial choice has been made.
#define ANCHOR_PUZZLE_SM_TUTORIAL 15

void port_remoteCarry_setCarried(uint32_t clientId, int32_t markerId, float offset[3], float yawOffset);
void port_remoteCarry_throw(uint32_t clientId, int32_t markerId, float start[3], float target[3]);
void port_remoteCarry_reset(void);
void port_anchorDummies_onActorsFreed(void);

// Connected, in a real room, syncing world state.
int32_t port_anchor_isWorldSyncActive(void);

// The puzzle mirrors below are home-scoped.
void port_puzzleStep_orBits(int32_t puzzleId, int32_t bits);
int32_t port_puzzleStep_get(int32_t puzzleId);
int32_t port_puzzleStep_getForMap(int32_t map, int32_t puzzleId);
int32_t port_puzzleStep_getForLevel(int32_t levelId, int32_t puzzleId);

void port_puzzlePos_mark(int32_t puzzleId, int32_t x, int32_t y, int32_t z);
int32_t port_puzzlePos_isMarked(int32_t puzzleId, int32_t x, int32_t y, int32_t z);

void port_fpTwinkly_release(void);

int32_t port_mapFlag_wasSetRemotely(int32_t index);

#define ANCHOR_COUNT_CCW_EYRIE_FED 0
#define ANCHOR_COUNT_CCW_NABNUT_ACORNS 1
void port_puzzleCount_add(int32_t counterId, int32_t delta);
int32_t port_puzzleCount_get(int32_t counterId);

void port_hutSmash_record(int32_t x, int32_t y, int32_t z, int32_t loot);
int32_t port_hutSmash_get(int32_t x, int32_t y, int32_t z);
int32_t port_hutSmash_countForCurrentLevel(void);

void port_jiggyCrane_broadcast(int32_t stage);
void port_jiggyCrane_remoteApply(int32_t stage);

// Rate-limited: a marker reached cube_removeProp with a propPtr outside its cube.
void port_warnPropNotInCube(int32_t index, int32_t propCnt);

// Rate-limited: a cube's node-prop split index passed 31, where the old :5 field wrapped.
void port_warnNodePropSplit(int32_t splitIndex, int32_t nodeCnt);

#ifdef __cplusplus
}
#endif

#endif
