#ifndef PORT_PATCHES_H
#define PORT_PATCHES_H

#include <stdint.h>
#include <stdbool.h>

#include "port/Enhancements/Events/Hooks/Events.h"

#ifdef __cplusplus
extern "C" {
#endif

// Threaded rendering (Game.cpp)

// Carries the interpolation pair with a submitted display list.
void Graphics_OnThread5Submit(void* taskData);
void Graphics_ReleaseRcpTask(void);
int Graphics_RenderServicePending(void);
// Runs fn on the window thread when the tick is on its own thread.
void Graphics_RunOnRenderThread(void (*fn)(void*), void* arg);
void Graphics_ServiceRenderRequests(void);
// Waits for in-flight display lists before freeing or reading what they use.
void Graphics_PipelineSyncPoint(void);
void Graphics_PushFrame(Gfx* data);

// Frame Pacing (FramePacingPatches.cpp)

int FramePacingPatches_GetDemoViCount(void);
void FramePacingPatches_SetDemoViCount(int viCount);
unsigned FramePacingPatches_GetDemoViSerial(void);
int FramePacingPatches_WaitDemoViSerial(unsigned seen, int timeoutUs);
int FramePacingPatches_GetDemoDisplayViCount(int rawViCount);
void FramePacingPatches_TickCutsceneStutter(void);
int FramePacingPatches_GetCutsceneExtraVis(void);

// Localization (Localization.cpp)

int Localization_PauseMenuNeedsRefresh(void); // language or Return-to-Lair CVar changed while menu open
void Localization_RebuildPauseMenu(void);     // free + recreate + replay the main menu open
void Localization_SetPrintScale(float scale);

// Dialog font (AltDialogFont.cpp, SpritePatches.cpp)

void AltDialogFont_RebuildHd(void);                   // rebuild the HD glyph textures for the current base
void SpritePatches_RefreshDialogFontGlyphCount(void); // re-read the reachable glyph count from the active font
int SpritePatches_DialogFontUsesShiftedCodes(void);   // format codes shifted past the extended (PAL) glyph range

// Framebuffer (FramebufferPatches.cpp)

void Framebuffer_FreezeReadback(int freeze);
void Framebuffer_RequestReadback(void);
int Framebuffer_ConsumeReadbackRequest(void);
void Framebuffer_ReadbackGPU(int bufferIndex);
int Framebuffer_GetPauseId(void);
int Framebuffer_CapturePause(void);
void Framebuffer_GetPauseSize(int* w, int* h);
int Framebuffer_ConsumePauseRecapture(void);
int TransitionPatches_ShouldCapture(void);

void Framebuffer_PatchPictureModel(void* model_bin, int32_t min_xy, int32_t max_xy, int32_t min_z, int32_t max_z,
                                   uint32_t from);
int32_t Framebuffer_GetTransitionGpuId(void);
void Framebuffer_CaptureTransition(void* gfx_ptr);
void Framebuffer_PatchTransitionModel(void* model_bin);

// Sprite Display Cache (SpritePatches.cpp)

void SpritePatches_ClearDisplayCache(void);
BKSpriteDisplayData* SpritePatches_GetOrCreateDisplayData(BKSprite* sprite);

// JP pause-menu world-name banner (SpritePatches.cpp)

s32 SpritePatches_UpdatePauseBanner(s32 page_id);
BKSprite* SpritePatches_GetPauseBannerDraw(s32 headerY, f32* outX, f32* outY, f32* outW, f32* outH);
void SpritePatches_FreePauseBanner(void);

// Camera (CameraPatches.cpp)

float CameraPatches_GetWidescreenYawScale(void);
float CameraPatches_GetWidescreenPitchScale(void);

// Input

float Ship_GetRumbleScale(void);

// Graphics (GraphicsPatches.cpp)

int GraphicsPatches_GetDrawDistanceSetting(void); // configured multiplier; safe during map load
int GraphicsPatches_GetDrawDistanceLevel(void);   // render-time multiplier; clamped to 1x outside normal gameplay
int GraphicsPatches_IsLodDisabled(void);
float GraphicsPatches_GetDrawDistanceMultiplier(void);
void GraphicsPatches_ApplyModelDrawDistanceCull(bool* fadeFlag, float* cullMult, float* cullDist);
int GraphicsPatches_IsSpriteSizeCulled(float depth, float size, float baseThreshold, int disableFlag);
float GraphicsPatches_GetHudOrthoShift(float refX);
void GraphicsPatches_ResetModelTLUT(Gfx** gfx);

// Vertex-animated models (AnimVertexPatches.cpp)

// Recycles the per-draw vertex arena; once per tick, before the display list is built.
void AnimVertexPatches_BeginTick(void);
// Copies the just-posed vertices somewhere only this draw points at, and repoints
// segment 0x01 at the copy.
void AnimVertexPatches_SnapshotVertices(Gfx** gfx, void* vertices, int32_t count);

// Mirror (MirrorPatches.cpp)

int MirrorPatches_IsActive(void);
void MirrorPatches_BeginScene(void);
void MirrorPatches_EndScene(void);
void MirrorPatches_UndoProjection(Gfx** gfx, Mtx** mtx);
void MirrorPatches_ApplyViewport(Gfx** gfx, Mtx** mtx);
void MirrorPatches_MarkCapture(void);
int MirrorPatches_ShouldFlipPauseBg(void);

// Mirror per-model exclusion (counter-mirror text-bearing objects)
void MirrorPatches_SetExclude(void);
void MirrorPatches_ClearExclude(void);
int MirrorPatches_BakeCounterScale(void);
void MirrorPatches_PatchTextActors(void);

// Volatile flag checks

int GameFixes_IsInCharacterParade(void);

// Audio engine lock

void AudioSync_Lock(void);
void AudioSync_Unlock(void);
void AudioSync_IntMaskEnter(void);
void AudioSync_IntMaskExit(void);

// Volume sliders (AudioVolume.cpp)

float AudioVolume_GetMusic(void);
float AudioVolume_GetSfx(void);

// Note doors (NoteDoorNumbers.cpp)

int NoteDoorNumbers_GetCost(int doorIdx);
void NoteDoorNumbers_EnableComposer(void);

// Romhacks

void romhack_RewriteActorSpawn(void* actorInfo, u32* flags);

// Attract-demo audio hold

void GameEngine_BeginDemoAudioHold(void);
void GameEngine_TickDemoAudioHold(void);
int GameEngine_IsAudioHeld(void);
void AudioSync_NoteMainLoopAlive(void);
int AudioSync_IsStallHeld(void);
int32_t OS_AudioPumpShouldWait(void);

void actorDespawn_beginDefer(void);
void actorDespawn_endDefer(void);

// Anchor

enum AnchorFlagSpace {
    ANCHOR_FLAGSPACE_FILE_PROGRESS = 0,
    ANCHOR_FLAGSPACE_VOLATILE = 1,
    ANCHOR_FLAGSPACE_LEVEL_SPECIFIC = 2,
    ANCHOR_FLAGSPACE_MAP_SPECIFIC = 3,
    ANCHOR_FLAGSPACE_RANDO_INF = 4,
};
enum AnchorCollectibleSpace {
    ANCHOR_COLLECTIBLE_JIGGY = 0,
    ANCHOR_COLLECTIBLE_HONEYCOMB = 1,
    ANCHOR_COLLECTIBLE_MUMBO = 2,
    ANCHOR_COLLECTIBLE_NOTE = 3,
    ANCHOR_COLLECTIBLE_JINJO = 4,
    ANCHOR_COLLECTIBLE_WORM = 5,
    ANCHOR_COLLECTIBLE_ACORN = 6,
    ANCHOR_COLLECTIBLE_PRESENT_BLUE = 7,
    ANCHOR_COLLECTIBLE_PRESENT_GREEN = 8,
    ANCHOR_COLLECTIBLE_PRESENT_RED = 9,
    ANCHOR_COLLECTIBLE_GOLD = 10,
    ANCHOR_COLLECTIBLE_ORANGE = 11,
};

// One-shot cues when a teammate's file-progress flag arrives
void progressFlag_remoteCue(int32_t progressFlag);
void notedoor_remoteOpen(int32_t progressFlag);
void leveldoor_remoteOpen(int32_t progressFlag);

void breakable_remoteBreak(int32_t progressFlag);

void breakable_remoteBreakAt(int32_t markerId, int32_t x, int32_t y, int32_t z);

int32_t cutsceneWarp_getReturnMap(void);

void eggToll_remoteApply(int32_t map, int32_t secondaryId, int32_t stage);

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
// map; the FP hub tree reads it via VB_PUZZLE_STEP_SET_ON_MAP.
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

void remoteCarry_setCarried(uint32_t clientId, int32_t markerId, float offset[3], float yawOffset);
void remoteCarry_throw(uint32_t clientId, int32_t markerId, float start[3], float target[3]);
void remoteCarry_reset(void);

// Connected, in a real room, syncing world state.
int32_t Anchor_IsWorldSyncActive(void);

// The puzzle mirrors below are home-scoped.
int32_t PuzzleStep_GetBitsForLevel(int32_t levelId, int32_t puzzleId);

#define ANCHOR_COUNT_CCW_EYRIE_FED 0
#define ANCHOR_COUNT_CCW_NABNUT_ACORNS 1

void jiggyCrane_remoteApply(int32_t stage);

// Rate-limited: a marker reached cube_removeProp with a propPtr outside its cube.
void PerfPatches_WarnPropNotInCube(int32_t index, int32_t propCnt);

// Rate-limited: a cube's node-prop split index passed 31, where the old :5 field wrapped.
void PerfPatches_WarnNodePropSplit(int32_t splitIndex, int32_t nodeCnt);

#ifdef __cplusplus
}
#endif

#endif
