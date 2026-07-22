#ifndef PORT_PATCHES_H
#define PORT_PATCHES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Frame Pacing (FramePacingPatches.cpp)

int port_getDemoViCount(void);
void port_setDemoViCount(int viCount);
int port_getDemoDisplayViCount(int rawViCount);
int port_getCutsceneExtraVis(void);
int port_getInterpolationFpsCap(void);

// Localization (Localization.cpp)

int port_pauseMenuNeedsRefresh(void); // language or Return-to-Lair CVar changed while menu open
void port_pauseMenuRebuild(void);     // free + recreate + replay the main menu open
void port_setPrintScale(float scale);

// Framebuffer (FramebufferPatches.cpp)

void port_setViBlack(int active);
int port_isViBlack(void);
void port_freezeReadback(int freeze);
void port_requestReadback(void);
int port_consumeReadbackRequest(void);
int port_getPauseFramebufferId(void);
int port_capturePauseFramebuffer(void);
void port_getPauseFramebufferSize(int* w, int* h);
int port_pauseConsumeRecaptureRequest(void);
int port_shouldCaptureTransition(void);

int32_t port_getAuxGpuFbId(void);
void port_readAuxFbToCpu(void* gfx_ptr);
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

void port_camera_applyWsYawFix(float rotation[3]);

// Input

float port_getRumbleScale(void);

// Gameplay

int port_scalePlayerDamage(int damage);

// Graphics (GraphicsPatches.cpp)

int port_getDrawDistanceLevel(void);
int port_shouldDisableLOD(void);
float port_drawDistanceMul(void);
void port_applyModelDrawDistanceCull(int* fadeFlag, float* cullMult, float* cullDist);
int port_spriteSizeCulled(float depth, float size, float baseThreshold, int disableFlag);
float port_hudOrthoShift(float refX);

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

// Attract-demo audio hold

void port_beginDemoAudioHold(void);
// Note doors (defined in LAIR/actor_spawninit.c)

// Animate + despawn the note door whose open flag was just received, if spawned in our map.
void port_notedoor_remoteOpen(int32_t progressFlag);

// Animate the lair entrance door whose podium a teammate just completed, minus camera/warp.
// If not spawned here, the arm expires once the persistent open flag syncs.
void port_leveldoor_remoteOpen(int32_t progressFlag);

// Breakable lair objects (cobwebs, brickwalls, ice ball, grates, statue eye, rareware box,
// crypt gate). Replay the break for the object whose flag was just received, if spawned here.
// Defined in core2/quiz/game.c.
void port_breakable_remoteBreak(int32_t progressFlag);

// Non-persistent breakables (glass windows etc.) with no synced flag; matched by spawn position.
// broadcastBreak sends, remoteBreakAt replays. Defined in core2/quiz/game.c.
void port_breakable_broadcastBreak(int32_t markerId, int32_t x, int32_t y, int32_t z);
void port_breakable_remoteBreakAt(int32_t markerId, int32_t x, int32_t y, int32_t z);

// Variant of broadcastBreak for breakables that replay their own break by polling
// port_breakable_isBroken (CC grates). Records + broadcasts but skips the generic handler.
void port_breakable_recordBreak(int32_t markerId, int32_t x, int32_t y, int32_t z);

// In-memory record of which non-persistent breakables the team broke this session, checked at
// spawn. Keyed by (map, marker, spawn position); cleared on save load.
int32_t port_breakable_isBroken(int32_t map, int32_t markerId, int32_t x, int32_t y, int32_t z);

// RBB egg-toll paths: extension stage is actor-local with no flag, synced explicitly by
// secondaryId. onAdvance/getStage in EggToll.cpp; remoteApply in RBB/ch/eggtoll1.c.
void port_eggToll_onAdvance(int32_t map, int32_t secondaryId, int32_t stage);
int32_t port_eggToll_getStage(int32_t map, int32_t secondaryId);
void port_eggToll_remoteApply(int32_t map, int32_t secondaryId, int32_t stage);

// Generic multi-step puzzle sync for progress that's actor-local with no flag (BGS Tanktup legs,
// croctus feed, nested pink eggs). Each step is a bit in an OR-merged mask keyed by (map,
// puzzleId). orBits records + broadcasts; get reads the mask. Defined in port PuzzleStep.cpp.
#define ANCHOR_PUZZLE_BGS_TANKTUP 1
#define ANCHOR_PUZZLE_BGS_CROCTUS 2
#define ANCHOR_PUZZLE_BGS_PINKEGG 3
// CC clanker teeth: bits 0-2 = token tooth egg count, bits 3-5 = jiggy tooth.
#define ANCHOR_PUZZLE_CC_CLANKER_TEETH 4
// GV Jinxy/Banjo door: persists the cured-Jinxy door-open across reloads for other teammates.
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
// FP Sir Slushes: positional, keyed by spawn position via the puzzle-pos set below.
#define ANCHOR_PUZZLE_FP_SLUSHES 13
// RBB engine-room fans: 1 bit. The propeller-slowdown switch sets transient map flag 0 (enginefan.c
// reads it to slow the fans). It syncs live but resets on reload; persist it (in-memory + team-state)
// so the fans stay slowed for a returning player / joining teammate, like the GV Jinxy door.
#define ANCHOR_PUZZLE_RBB_ENGINE_FANS 14

// Remote teammates' carried-collectible display copies. PLAYER_UPDATE reports the carried marker
// id; CARRY_THROW replays the throw arc. Display only. reset drops tracked markers (called from
// actorArray_free before they dangle).
void port_remoteCarry_setCarried(uint32_t clientId, int32_t markerId, float offset[3], float yawOffset);
void port_remoteCarry_throw(uint32_t clientId, int32_t markerId, float start[3], float target[3]);
void port_remoteCarry_reset(void);
// Anchor dummy players: forget every stand-in actor marker. Called from actorArray_free, which
// frees actors/markers without firing OnActorDestroy.
void port_anchorDummies_onActorsFreed(void);
void port_puzzleStep_orBits(int32_t puzzleId, int32_t bits);
int32_t port_puzzleStep_get(int32_t puzzleId);
// Same as get, for an explicit map key (FP tree ice: recorded in the tree, read from the hub).
int32_t port_puzzleStep_getForMap(int32_t map, int32_t puzzleId);

// PUZZLE_POS: positional companion to PUZZLE_STEP for sub-steps identified by fixed spawn
// position rather than an index (FP's Sir Slushes). Rides the same PUZZLE_STEP packet.
void port_puzzlePos_mark(int32_t puzzleId, int32_t x, int32_t y, int32_t z);
int32_t port_puzzlePos_isMarked(int32_t puzzleId, int32_t x, int32_t y, int32_t z);

// FP twinkly minigame: release the single-runner claim once our run ends, so a teammate can go.
// Start side is gated by VB_FP_TWINKLY_START (HookHandlers.cpp).
void port_fpTwinkly_release(void);

// True if the given map-specific flag's most recent write came from the network, not local —
// used by VB_DOOR_OPEN_CAMERA to keep the camera-lock on whoever triggered it. See ScopedFlag.cpp.
int32_t port_mapFlag_wasSetRemotely(int32_t index);

// PUZZLE_COUNT: shared monotonic delivery counters keyed by (map, counterId), for progress
// that's a count rather than distinct steps. add() records + broadcasts; get() reads the total.
#define ANCHOR_COUNT_CCW_EYRIE_FED 0
#define ANCHOR_COUNT_CCW_NABNUT_ACORNS 1
void port_puzzleCount_add(int32_t counterId, int32_t delta);
int32_t port_puzzleCount_get(int32_t counterId);

// MM/BGS huts: break + dropped-bundle sync. record stores the (position -> bundle index) a hut
// dropped (first smasher wins); get returns it or -1. Defined in port HutSmash.cpp.
void port_hutSmash_record(int32_t x, int32_t y, int32_t z, int32_t loot);
int32_t port_hutSmash_get(int32_t x, int32_t y, int32_t z);
// Number of huts already smashed in the current map, team-wide — used as the smash-order index.
int32_t port_hutSmash_countForCurrentMap(void);

// RBB jiggy-cage crane: transient timed minigame, not persisted. Lower/raise broadcast to
// same-map teammates and replay silently. broadcast in JiggyCrane.cpp; remoteApply in
// RBB/crane_jiggycage.c.
void port_jiggyCrane_broadcast(int32_t stage);
void port_jiggyCrane_remoteApply(int32_t stage);

#ifdef __cplusplus
}
#endif

#endif
