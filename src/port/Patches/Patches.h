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

// Animate + despawn the specific note door whose open flag was just received over the
// network, if it is spawned in the current map. progressFlag is the FILEPROG_*_NOTE_DOOR_*
// open flag. No-op for non-note-door flags or when that door isn't present.
void port_notedoor_remoteOpen(int32_t progressFlag);

// Animate the lair entrance door whose podium a teammate just completed — the same open
// animation the completer's cutscene drives, minus all camera/warp. progressFlag is the
// entrance-open "cutscene seen" fileprog (0x28-0x30, 0xE2). No-op for other flags; if the door
// isn't spawned here, the arm waits and expires once the persistent open flag syncs (spawn-time
// snap-open covers it from then on). Defined in LAIR/actor_spawninit.c.
void port_leveldoor_remoteOpen(int32_t progressFlag);

// Breakable lair objects (cobwebs, brickwalls, ice ball, grates, statue eye, rareware box,
// crypt gate). Replay the break (effect + despawn) for the object whose "broken" flag was
// just received over the network, if it's spawned in the current map. progressFlag is the
// FILEPROG_* broken/open flag. No-op for unknown flags or when that object isn't present.
// Defined in core2/quiz/game.c, next to the shared collision-die handler it reuses.
void port_breakable_remoteBreak(int32_t progressFlag);

// Non-persistent breakables (glass windows etc.) that break + despawn with no synced flag. The
// break is broadcast by spawn position (these objects are static, so their position is identical
// on every client), and replayed on teammates in the same map. Defined in core2/quiz/game.c.
// broadcastBreak is C->C++ (sends the packet); remoteBreakAt is C++->C (replays the break).
void port_breakable_broadcastBreak(int32_t markerId, int32_t x, int32_t y, int32_t z);
void port_breakable_remoteBreakAt(int32_t markerId, int32_t x, int32_t y, int32_t z);

// Variant of broadcastBreak for breakables that replay their own break by polling
// port_breakable_isBroken each update (CC grates, whose break/rise state machine the generic
// handler can't reproduce). Records + broadcasts the break but tells the receiver not to invoke
// the generic remote-break handler. Defined in port BreakObject.cpp.
void port_breakable_recordBreak(int32_t markerId, int32_t x, int32_t y, int32_t z);

// In-memory (never saved) record of which non-persistent breakables the team has broken this
// session, so the object stays broken on (re)load — checked at spawn. Keyed by (map, marker,
// spawn position); cleared on save load. Defined in port BreakObject.cpp.
int32_t port_breakable_isBroken(int32_t map, int32_t markerId, int32_t x, int32_t y, int32_t z);

// RBB egg-toll paths (bridges that extend in stages as eggs are paid). The extension stage is
// actor-local with no flag, so it's synced explicitly: onAdvance records + broadcasts a stage,
// getStage restores it at spawn (temporary in-memory persistence), remoteApply extends the matching
// toll live. Tolls are identified by their stable secondaryId. onAdvance/getStage are in
// EggToll.cpp; remoteApply is in RBB/ch/eggtoll1.c.
void port_eggToll_onAdvance(int32_t map, int32_t secondaryId, int32_t stage);
int32_t port_eggToll_getStage(int32_t map, int32_t secondaryId);
void port_eggToll_remoteApply(int32_t map, int32_t secondaryId, int32_t stage);

// Generic multi-step puzzle sync (live + temp-persist + team-state) for puzzles whose progress is
// actor-local with no flag (BGS Tanktup legs, croctus feed chain, nested pink eggs). Each sub-step
// is a bit; progress is an OR-merged bitmask keyed by (current map, puzzleId). orBits records +
// broadcasts a newly-done step; get returns the current mask so an actor can replay teammates'
// steps it hasn't applied. Defined in port PuzzleStep.cpp.
#define ANCHOR_PUZZLE_BGS_TANKTUP 1
#define ANCHOR_PUZZLE_BGS_CROCTUS 2
#define ANCHOR_PUZZLE_BGS_PINKEGG 3
// CC clanker teeth: bits 0-2 = token tooth (unk0 1) egg count, bits 3-5 = jiggy tooth (unk0 2).
#define ANCHOR_PUZZLE_CC_CLANKER_TEETH 4
// GV Jinxy/Banjo door: 1 bit. Cured Jinxy opens the door via a transient map flag; this persists
// it (in-memory + team-state) so it stays open across reloads for a teammate who can't cure Jinxy.
#define ANCHOR_PUZZLE_GV_JINXY_DOOR 5
// MM Juju totem: bits 0-3 = a prefix of how many of the 4 segments have been knocked off.
#define ANCHOR_PUZZLE_MM_JUJU 6
// TTC Nipper: bits 0-2 = a prefix of how many of the 3 hits he's taken (all 3 = dead, shell open).
#define ANCHOR_PUZZLE_TTC_NIPPER 7
// TTC Blubber: bit 0 = first gold bullion delivered, bit 1 = second (jiggy spawned, Blubber leaves).
#define ANCHOR_PUZZLE_TTC_BLUBBER 8
// TTC treasure hunt: bits 0-5 = a prefix of the 6 beak-busted X steps (bit 5 = treasure dug up).
#define ANCHOR_PUZZLE_TTC_XHUNT 9
// FP xmas tree ice: 1 bit, set when the tree-top ice shatters (star minigame complete). Recorded in
// the tree-interior map (where the ice lives); the FP hub tree reads it via getForMap.
#define ANCHOR_PUZZLE_FP_TREE_ICE 10
// FP bear cubs' presents: bit 0 = blue delivered, bit 1 = green, bit 2 = red. The received level
// flags (0x11-0x13) stay local (Anchor_ScopedFlagExcluded); bearcub.c records/replays via these.
#define ANCHOR_PUZZLE_FP_PRESENTS 11
// FP snowman buttons: bit per button (actorTypeSpecificField 1-3 -> bits 0-2). Pressing all three
// spawns JIGGY_2D; snowmanbutton.c records its own press and replays a teammate's silently.
#define ANCHOR_PUZZLE_FP_SNOWBUTTONS 12
// FP Sir Slushes: positional, not a count — each slush is keyed by its (stationary) spawn position,
// synced through the puzzle-pos set (below), so killing all of them team-wide spawns JIGGY_31.
#define ANCHOR_PUZZLE_FP_SLUSHES 13

// Remote teammates' carried-collectible display copies (defined in level_collectible.c). A
// teammate's PLAYER_UPDATE reports the carried collectible's marker id (0 = none); their throw
// arrives via the CARRY_THROW packet and replays the same ballistic arc locally. Display only —
// flags, spends, and quest progress ride their own sync paths. reset drops all tracked markers
// and is called from actorArray_free (they're about to dangle).
void port_remoteCarry_setCarried(uint32_t clientId, int32_t markerId, float offset[3], float yawOffset);
void port_remoteCarry_throw(uint32_t clientId, int32_t markerId, float start[3], float target[3]);
void port_remoteCarry_reset(void);
// Anchor dummy players: forget every stand-in actor marker. Called from actorArray_free —
// that path frees all actors/markers wholesale without firing OnActorDestroy, so the tracked
// markers are about to dangle (a freed marker resolves to non-null garbage, not nullptr).
void port_anchorDummies_onActorsFreed(void);
void port_puzzleStep_orBits(int32_t puzzleId, int32_t bits);
int32_t port_puzzleStep_get(int32_t puzzleId);
// Same as get, but for an explicit map key — for the rare puzzle whose recorder and consumer live
// in different maps (FP tree ice: recorded inside the tree, also read from the FP hub).
int32_t port_puzzleStep_getForMap(int32_t map, int32_t puzzleId);

// PUZZLE_POS: positional companion to PUZZLE_STEP for puzzles whose sub-steps are distinct world
// objects identified by their fixed spawn position rather than a small index (FP's Sir Slushes).
// Keyed by (current map, puzzleId, spawn-position hash); a member is recorded + broadcast on mark
// and stays for the session (temp-persist, like the other puzzle stores). isMarked replays a
// teammate's step at the matching object. Rides the same PUZZLE_STEP packet (see PuzzleStep.cpp).
void port_puzzlePos_mark(int32_t puzzleId, int32_t x, int32_t y, int32_t z);
int32_t port_puzzlePos_isMarked(int32_t puzzleId, int32_t x, int32_t y, int32_t z);

// FP twinkly (Christmas-light) minigame: release our hold on the single-runner claim once the run
// ends (completed or failed), so a teammate can take their turn. No-op offline / if we don't own it.
// The start side is gated by VB_FP_TWINKLY_START (HookHandlers.cpp). Defined there too.
void port_fpTwinkly_release(void);

// PUZZLE_COUNT: shared monotonic delivery counters keyed by (current map, counterId) — for
// progress that is a count, not distinct steps, where concurrent deliveries must all land
// (delta-synced; see PuzzleStep.cpp). add() records + broadcasts; get() reads the team total.
#define ANCHOR_COUNT_CCW_EYRIE_FED 0
#define ANCHOR_COUNT_CCW_NABNUT_ACORNS 1
void port_puzzleCount_add(int32_t counterId, int32_t delta);
int32_t port_puzzleCount_get(int32_t counterId);

// MM/BGS huts: break + dropped-bundle sync with temporary persistence. record stores + broadcasts
// the (spawn position -> bundle index) a hut dropped (idempotent, first smasher wins); get returns
// the recorded index for a hut at this position, or -1. The hut actor breaks + drops live for a
// teammate, and restores broken (re-dropping only non-tracked loot) on reload. Defined in
// port HutSmash.cpp.
void port_hutSmash_record(int32_t x, int32_t y, int32_t z, int32_t loot);
int32_t port_hutSmash_get(int32_t x, int32_t y, int32_t z);
// Number of huts already smashed in the current map (team-wide, via the shared record). MM uses
// this as the smash-order index so a teammate's smashes advance the sequence instead of every
// client counting its own huts from zero.
int32_t port_hutSmash_countForCurrentMap(void);

// RBB jiggy-cage crane: a transient timed minigame (no flag, not persisted). The lower (stage 2)
// and raise (stage 4) are broadcast to same-map teammates and replayed silently (no camera /
// hourglass). broadcast is in JiggyCrane.cpp; remoteApply is in RBB/crane_jiggycage.c.
void port_jiggyCrane_broadcast(int32_t stage);
void port_jiggyCrane_remoteApply(int32_t stage);

#ifdef __cplusplus
}
#endif

#endif
