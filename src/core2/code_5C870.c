#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include "gc/gctransition.h"
#include "bk_time.h"

//game.c

extern void releaseBoldFontTextures(void);
extern void releaseFontTextures(void);
extern void timedFuncQueue_update(void);
extern void clearSecondaryTrack(s32);
extern void comusic_clearSecondaryTrack(void);
extern void comusic_fadeAllTracksWithArgs(s32, s32, s32);
extern void fadeAllTracks(s32);
extern void resetGameState(void);
extern void initializeAudioSystem(void);
extern void updateGameMode(void);
extern void updateAudioSystem(void);
extern void comusicPlayer_update(void);

enum transition_e { TRANSITION_0_NONE };

void game_setFramebufferIndex(s32 frame_buffer_indx);
void game_setMap(s32 map, s32 exit);
void game_setTransition(s32 arg0);
void game_setMapWithoutTransition(s32 map, s32 exit);
void game_setTransitionAndReset(s32 transition);
bool game_isSpecialMode(void);

f32 resetGameStateVariables(void);
extern void clearTimedFunctionQueue(void);

/* .data */
s16 specialModeData[] = {
    0x159A, 0x02BC, 0x21C8, 0x00C8, 0x14ED, 0x03A2, 0x1951, 0x0003,
    '**',   ' B',   'AN',   'JO',   ' K',   'AZ',   'OO',   'IE',
    ' (',   'c)',   ' R',   'AR',   'E ',   'Lt',   'd ',   '19',
    '98',   ' *',   '*\0',  0,      0,      0,      0,      0,

};

/* .bss */
struct {
  s32 unk0;
  s32 game_mode; // game_mode
  f32 unk8;
  s32 unkC; // freeze_scene_flag (used for pause menu)
  f32 unk10;
  u8 transition;
  u8 map;
  u8 exit;
  u8 unk17; // reset_on_map_load
  u8 unk18;
  u8 unk19;
  u8 unk1A;
  u8 unk1B;
  u8 unk1C;
} gameState;

void game_resetViewport(void) {
  viewport_setPosition_f3(0.0f, 0.0f, 0.0f);
  viewport_setRotation_f3(-30.0f, 30.0f, 0.0f);
  viewport_moveAlongZAxis(3000.0f);
  viewport_update();
}

void game_defragAll(void) {
  int i;

  assetCache_clearAllDependencies();
  dummy_func_80254464();
  for (i = 0; i < 0xF; i++) {
    defragTextRenderer(5);
    modelRender_defrag();
    mapSavestate_defrag_all();
    gctransition_defrag();
    printbuffer_defrag();
    initializeParticleSystem();
    initializeGameSettings();
    assetCache_defrag();
    timedFuncQueue_defrag();
    comusic_defrag();
  }
}

void game_loadMap(enum map_e map, s32 exit, s32 reset_on_load) {
  if (reset_on_load || level_get() != map_getLevel(map)) {
    setAudioMode(1);
    initializeGameMode();
    loadMapData(map); // load_map_asm
    finalizeAudioSettings(map);
  } else {
    setAudioMode(1);
    finalizeAudioSettings(map);
  }
  initializeGameTimers();
  setMapExit(map, exit, 0);
  game_resetViewport();
  resetGameFlags();
}

void game_update(s32 arg0) {
  updateGameTimers();
  updateAudioSystem();
  comusicPlayer_update();
  if (arg0 != 0) {
    game_defragAll();
  }
}

void game_renderFrame(Gfx **gdl, Mtx **mptr, Vtx **vptr, s32 framebuffer_idx,
                 s32 arg4) {
  Mtx *m_start = *mptr;
  Vtx *v_start = *vptr;

  scissorBox_SetForGameMode(gdl, framebuffer_idx);
  gameState.unkC = FALSE;
  renderGameFrame(gdl, mptr, vptr);
  if (!arg4) {
    enableTextFlag();
    game_setFramebufferIndex(getActiveFramebuffer());
    disableTextFlag();
    renderAndResetText(gdl);
  }
  if (gameState.game_mode == GAME_MODE_A_SNS_PICTURE &&
      gameState.unk19 != 6 && gameState.unk19 != 5) {
    gctransition_draw(gdl, mptr, vptr);
  }

  if (gameState.game_mode == GAME_MODE_8_BOTTLES_BONUS ||
      gameState.game_mode == GAME_MODE_A_SNS_PICTURE) {
    initializeAudioChannels(gdl, mptr, vptr);
  }

  if (!game_isFrozen() && func_80335134()) {
    updateParticleSystem(gdl, mptr, vptr);
  }

  gcpausemenu_draw(gdl, mptr, vptr);
  if (!game_isFrozen()) {
    dummy_func_8025AFC0(gdl, mptr, vptr);
  }

  gcdialog_draw(gdl, mptr, vptr);
  if (!game_isFrozen()) {
    itemPrint_draw(gdl, mptr, vptr);
  }

  printbuffer_draw(gdl, mptr, vptr);

  if (gameState.game_mode != GAME_MODE_A_SNS_PICTURE ||
      gameState.unk19 == 6 || gameState.unk19 == 5) {
    gctransition_draw(gdl, mptr, vptr);
  }
  finishFrame(gdl);
  osWritebackDCache(m_start, sizeof(Mtx) * (*mptr - m_start));
  osWritebackDCache(v_start, sizeof(Vtx) * (*vptr - v_start));
}

void game_setFramebufferIndex(s32 frame_buffer_indx) {
  framebufferdraw_setBufferIndex(frame_buffer_indx);
}

void game_noop(void) { return; }

//_set_game_mode
void game_setMode(enum game_mode_e next_mode, s32 arg1) {
  s32 prev_mode = gameState.game_mode;
  s32 sp20;
  s32 sp1C;

  if (((gameState.game_mode == GAME_MODE_3_NORMAL || game_isSpecialMode()) &&
       next_mode != GAME_MODE_4_PAUSED) ||
      (gameState.game_mode == GAME_MODE_4_PAUSED &&
       next_mode != GAME_MODE_3_NORMAL)) {
    clearTimedFunctionQueue();
  }

  if (gameState.game_mode == GAME_MODE_4_PAUSED &&
      next_mode != GAME_MODE_4_PAUSED) {
    gcpausemenu_free();
  }

  // L802E3C84
  if (next_mode == GAME_MODE_8_BOTTLES_BONUS ||
      next_mode == GAME_MODE_A_SNS_PICTURE) {
    allocateImageBuffer();
  } else {
    freeImageBuffer();
  } // L802E3CB4

  gameState.game_mode = next_mode;

  if (next_mode == 2) {
    updateGraphicsTasks(3);
  } else if (next_mode == GAME_MODE_3_NORMAL || game_isSpecialMode()) {
    if (prev_mode != GAME_MODE_4_PAUSED) {
      updateGraphicsTasks(2);
    } // L802E3D18
    if (arg1) {
      sp20 = FALSE;
      if (next_mode == GAME_MODE_3_NORMAL) {
        if (volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE)) {
          sp20 = TRUE;
          sp1C = 7;
        } else if (isGamePaused() && level_get() != LEVEL_C_BOSS &&
                   level_get() != LEVEL_B_SPIRAL_MOUNTAIN &&
                   level_get() != LEVEL_6_LAIR &&
                   level_get() != LEVEL_D_CUTSCENE) {
          sp20 = TRUE;
          sp1C = 1;
        }
      } else if (game_isSpecialMode()) { // L802E3DBC
        sp20 = TRUE;
        sp1C = getMapExitId(gameState.map, gameState.exit);
      }

      if (sp20)
        gctransition_8030BEA4(sp1C);
      else
        gctransition_8030BD4C();
    }
    initializeGameData();
    gameState.unk10 = 0.0f;
  } else if (next_mode == GAME_MODE_4_PAUSED) { // L802E3E24
    fadeAllTracks(0);
    FUNC_8030E624(SFX_C9_PAUSEMENU_ENTER, 1.1f, 32750);
    pfsManager_update();
    comusic_fadeAllTracksWithArgs(0, 2000, 3);
    comusic_setSecondaryTrack(COMUSIC_6F_PAUSE_SCREEN);
    gcpausemenu_init();
  } // L802E3E6C
}

void game_changeMode(enum game_mode_e mode) {
  s32 sp34;
  s32 sp30;
  s32 map;
  s32 sp28;
  s32 prev_mode;

  sendGfxTaskMessage();
  sp34 = gameState.unk18;
  sp30 = gameState.unk17;
  map = gameState.map;
  sp28 = gameState.exit;
  prev_mode = gameState.unk0;
  game_setMode(GAME_MODE_2_UNKNOWN, 0);
  if (!volatileFlag_getAndSet(VOLATILE_FLAG_21, 0) ||
      map_getLevel(map_get()) == map_getLevel(gameState.map)) {
    if (!volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE))
      mapSavestate_save(map_get());
  }
  game_update(1);
  game_loadMap(map, sp28, sp34);
  mapSavestate_apply(map);
  gameState.unk0 = prev_mode;
  game_setMode(mode, sp30);
  jiggylist_map_actors();
  initializeGameData();
}

s32 game_getPreviousMode(void) { return gameState.unk0; }

void game_draw(s32 arg0) {
  Gfx *gfx;
  Gfx *gfx_start;
  Gfx *sp2C;
  Mtx *mtx;
  Vtx *vtx;

  if (arg0) {
    scissorBox_setDefault();
  }

  getGraphicsStacks(&gfx, &mtx, &vtx);

  if (gameState.unkC == 1) {
    getGraphicsStacks(&gfx, &mtx, &vtx);
  }

  gfx_start = gfx;
  game_renderFrame(&gfx, &mtx, &vtx, getActiveFramebuffer(), arg0);

  if (gameState.unkC == 0) {
    sp2C = gfx;
    viMgr_waitForFrameAndProcess();
    enqueueGfxTaskWithFlag(gfx_start, sp2C);

    if (arg0) {
      scissorBox_setDefault();
    }
  }
}

void game_setMapAndTransition(s32 map, s32 exit, s32 transition) {
  game_setMap(map, exit);
  game_setTransitionAndReset(transition);
  game_setTransition(1);
}

// take me there
void game_setMapWithTransition(enum map_e map, s32 exit, s32 transition) {
  game_setMapWithoutTransition(map, exit);
  game_setTransitionAndReset(transition);
  game_setTransition(1);
}

void game_setMap(s32 map, s32 exit) {
  gameState.unk18 = 1;
  gameState.map = map;
  gameState.exit = exit;
}

void game_setTransition(s32 arg0) { gameState.transition = arg0; }

void game_setMapWithoutTransition(s32 map, s32 exit) {
  gameState.unk18 = 0;
  gameState.map = map;
  gameState.exit = exit;
}

void game_setTransitionAndReset(s32 transition) {
  gameState.unk17 = transition;
  gameState.unk19 = 0;
  if (transition && !gctransition_8030BDC0()) {
    gctransition_8030BE60();
  }
}

void game_setTransitionWithArgs(s32 arg0, s32 arg1) {
  gameState.unk17 = arg0;
  gameState.unk19 = arg1;
  if (arg0 && !gctransition_8030BDC0()) {
    gctransition_8030BEA4(arg1);
  }
}

void game_reset(void) {
  game_setMode(GAME_MODE_2_UNKNOWN, 0);
  defragManager_free();
  cleanupTextRenderer();
  if (!game_isSpecialMode())
    releaseFontAssets();
  timedFuncQueue_free();
  freeAllStruct4Es();
  modelRender_free();
  depthBuffer_stub();
  game_update(0);
  setAudioMode(0);
  initializeGameMode();
  resetDebugFlags();
  animCache_free();
  comusicPlayer_free();
  releaseSoundEffects();
}

void game_initialize(enum map_e map_id) {
  gameState.transition = TRANSITION_0_NONE;
  gameState.unk19 = gameState.unk18 = 0;
  gameState.map = gameState.exit = gameState.unk17 = 0;
  gameState.unk1B = gameState.unk1A = 0;
  gameState.unkC = 0;
  gameState.unk1C = 0;
  savedata_init();
  sns_save_and_update_global_data();
  initializeSoundSystem();
  comusicPlayer_init();
  resetGameModeFlags();
  timedFuncQueue_init();
  initializeStruct4Es();
  resetDialogBin();
  if (!game_isSpecialMode())
    initializePrintBuffer();
  initializeTextRenderer();
  defragManager_init();
  modelRender_init();
  depthBuffer_init(1);
  animCache_init();
  viewport_reset();
  viewport_setNearAndFar(1.0f, 10000.0f);
  rand_reset();
  scissorBox_setDefault();
  sendViMessage();
  time_reset();
  resetFrameCount();
  resetDebugFlags2();
  gameState.game_mode = GAME_MODE_2_UNKNOWN;
  gameState.unk8 = 0.0f;
  time_setDeltaReal_sec(0.0f);
  time_setDeltaReal_frames(0);
  loadMapData(map_id);
  finalizeAudioSettings(map_id);
  game_defragAll();
  game_loadMap(map_id, 0, 0);
  gameState.unk0 = 0;
  game_setMode(GAME_MODE_3_NORMAL, 1);
}

void game_updateTime(void) {
  if (gameState.unk8 == 0.0f) {
    time_setDeltaReal_sec(0.0f);
  } else {
    clearGameStateVariables();
    ;
    time_setDeltaReal_frames((s32)(resetGameStateVariables() * 60.0f + 0.5));
  }
  resetGameFlags();

  gameState.unk8 += time_getDelta();
}

bool game_updateState(void) {
  s32 sp1C;
  u8 temp_v0;

  viewport_debug();
  rand_shuffle();
  if (!gctransition_8030BDC0()) {
    temp_v0 = gameState.transition;
    gameState.transition = TRANSITION_0_NONE;
    switch (temp_v0) { /* switch 1 */
    case 9:            /* switch 1 */
      if ((gameState.game_mode == GAME_MODE_7_ATTRACT_DEMO) ||
          (gameState.game_mode == GAME_MODE_8_BOTTLES_BONUS) ||
          (gameState.game_mode == GAME_MODE_A_SNS_PICTURE) ||
          (gameState.game_mode == GAME_MODE_9_BANJO_AND_KAZOOIE)) {
        freeDemoAndRestoreItems();
      }
      gcparade_8031ABF8();
      game_changeMode(GAME_MODE_3_NORMAL);
      return FALSE;

    case 10: /* switch 1 */
      if ((gameState.game_mode == GAME_MODE_7_ATTRACT_DEMO) ||
          (gameState.game_mode == GAME_MODE_8_BOTTLES_BONUS) ||
          (gameState.game_mode == GAME_MODE_A_SNS_PICTURE) ||
          (gameState.game_mode == GAME_MODE_9_BANJO_AND_KAZOOIE)) {
        freeDemoAndRestoreItems();
      }
      gcparade_8031ABA0();
      game_changeMode(GAME_MODE_3_NORMAL);
      return FALSE;

    case 1: /* switch 1 */
      if ((gameState.game_mode == GAME_MODE_7_ATTRACT_DEMO) ||
          (gameState.game_mode == GAME_MODE_8_BOTTLES_BONUS) ||
          (gameState.game_mode == GAME_MODE_A_SNS_PICTURE) ||
          (gameState.game_mode == GAME_MODE_9_BANJO_AND_KAZOOIE)) {
        freeDemoAndRestoreItems();
      }
      game_changeMode(GAME_MODE_3_NORMAL);
      return FALSE;

    case 6: /* switch 1 */
      loadDemoAndSetFlags(gameState.map, gameState.exit);
      game_changeMode(GAME_MODE_7_ATTRACT_DEMO);
      return FALSE;

    case 12: /* switch 1 */
      loadDemoAndSetFlags(gameState.map, gameState.exit);
      game_changeMode(GAME_MODE_A_SNS_PICTURE);
      return FALSE;

    case 7: /* switch 1 */
      loadDemoAndSetFlags(gameState.map, gameState.exit);
      game_changeMode(GAME_MODE_8_BOTTLES_BONUS);
      return FALSE;

    case 8: /* switch 1 */
      loadDemoAndSetFlags(gameState.map, gameState.exit);
      game_changeMode(GAME_MODE_9_BANJO_AND_KAZOOIE);
      return FALSE;

    case 11: /* switch 1 */
      game_changeMode(gameState.game_mode);
      return FALSE;

    case 2: /* switch 1 */
      system_scheduleModeChange(1);
      return FALSE;

    case 3: /* switch 1 */
      system_scheduleModeChange(4);
      return FALSE;
    case 0:
      break;
    }
  }
  if (gameState.unk1A != 0) {
    game_setMode(gameState.unk1A - 1, gameState.unk1B);
    gameState.unk1A = 0;
  }
  sp1C = updateGameTimers2();
  updateGameMode();
  updateAudioSystem();
  comusicPlayer_update();
  switch (gameState.game_mode) {
  case GAME_MODE_8_BOTTLES_BONUS:
  case GAME_MODE_A_SNS_PICTURE:
    initializeAudioSystem();
    /* fallthrough */
  case GAME_MODE_7_ATTRACT_DEMO:
  case GAME_MODE_9_BANJO_AND_KAZOOIE:
    resetGameState();
    if ((controller_getStartButton(0) == 1) && (gameState.unk0 != 0)) {
      game_setMode(GAME_MODE_1_UNKNOWN, 0U);
    }
    break;
  case GAME_MODE_3_NORMAL: /* switch 2 */
    gameState.unk10 += time_getDelta();
    if ((pfsManager_getStartButtonState(0) == 1) && func_8028F070() &&
        (isPlayerInWater() == 0) && !gctransition_8030BDC0() &&
        gctransition_done() && (level_get() != 0) && (0.6 < gameState.unk10) &&
        gcpausemenu_80314B00() && !player_isDead() && updateGameSelectText() &&
        levelSpecificFlags_validateCRC1() && dummy_func_80320248()) {
      game_setMode(GAME_MODE_4_PAUSED, 0U);
    } else if ((controller_getStartButton(0) == 1) && (gameState.unk0 != 0)) {
      game_setMode(GAME_MODE_1_UNKNOWN, 0U);
    } else if (sp1C == 0) {
      game_setMode(GAME_MODE_3_NORMAL, 1U);
    }
    break;

  case GAME_MODE_4_PAUSED: /* switch 2 */
    if (gcPauseMenu_update() || cutscenetrigger_update()) {
      FUNC_8030E624(SFX_C9_PAUSEMENU_ENTER, 0.899316, 32736);
      fadeAllTracks(1);
      comusic_fadeAllTracksWithArgs(-1, 2000, 3);
      comusic_clearSecondaryTrack();
      clearSecondaryTrack(1);
      game_setMode(GAME_MODE_3_NORMAL, 0U);
    }
    break;
  }
  if ((gameState.game_mode == GAME_MODE_3_NORMAL) ||
      (game_isSpecialMode() != 0)) {
    timedFuncQueue_update();
    releaseFontTextures();
  }
  gctransition_update();
  if (game_isSpecialMode() == 0) {
    releaseBoldFontTextures();
  }
  return TRUE;
}

void game_setGameMode(enum game_mode_e mode, s32 arg1) { game_setMode(mode, arg1); }

s32 game_defrag(void) {
  heap_resetDefragFlag(); // reset defragged flag in memory.c
  if (!level_get())
    return NULL;

  glspline_defrag();
  animCache_defrag();
  defragAllParticleEmitters();
  ncCameraNodeList_defrag();
  modelRender_defrag();
  resetPlayerState();
  partEmitMgr_defrag();
  mapModel_defrag();
  cubeList_defrag();
  actorArray_defrag();
  spawnQueue_defrag();
  defragMemory();
  printbuffer_defrag();
  gcdialog_defrag();
  if (gameState.game_mode == GAME_MODE_4_PAUSED)
    gcpausemenu_defrag();
  switch (overlayManagergetLoadedId()) {
  case OVERLAY_2_WHALE:
    defragGameResources();
    break;
  case OVERLAY_D_WITCH:
    initializeParticleSystem();
    break;
  }
  return heap_isDefragFlagSet(); // returns defrag flag in memory.c
}

void game_freeze(void) { gameState.unkC = TRUE; }

int game_isFrozen(void) { return gameState.unkC; }

s32 game_getMode(void) { return gameState.game_mode; }

bool game_isSpecialMode(void) {
  return (gameState.game_mode == GAME_MODE_6_FILE_PLAYBACK) ||
         (gameState.game_mode == GAME_MODE_5_UNKNOWN) ||
         (gameState.game_mode == GAME_MODE_7_ATTRACT_DEMO) ||
         (gameState.game_mode == GAME_MODE_8_BOTTLES_BONUS) ||
         (gameState.game_mode == GAME_MODE_9_BANJO_AND_KAZOOIE) ||
         (gameState.game_mode == GAME_MODE_A_SNS_PICTURE);
}

void game_enableSpecialMode(void) { gameState.unk1C = 1; }

void game_disableSpecialMode(void) { gameState.unk1C = 0; }

u8 isSpecialModeEnabled(void) { return gameState.unk1C; }

s32 getSpecialModeData1(s32 arg0) { return specialModeData[8 * (arg0 - 0x80) + 0]; }

s32 getSpecialModeData2(s32 arg0) { return specialModeData[8 * (arg0 - 0x80) + 1]; }

s32 getSpecialModeData3(s32 arg0) { return specialModeData[8 * (arg0 - 0x80) + 2]; }

s32 getSpecialModeData4(s32 arg0) { return specialModeData[8 * (arg0 - 0x80) + 3]; }

s32 getSpecialModeData5(s32 arg0) { return specialModeData[8 * (arg0 - 0x80) + 4]; }

s32 getSpecialModeData6(s32 arg0) { return specialModeData[8 * (arg0 - 0x80) + 5]; }

s32 getSpecialModeData7(s32 arg0) { return specialModeData[8 * (arg0 - 0x80) + 6]; }

s32 getSpecialModeData8(s32 arg0) { return specialModeData[8 * (arg0 - 0x80) + 7]; }

f32 game_getTime(void) { return gameState.unk8; }
