#include <ultra64.h>

#include "core1/core1.h"
#include "functions.h"
#include "gc/gctransition.h"
#include "variables.h"
#include "version.h"

#include "bk_bool.h"

#ifdef LIGHTHOUSE_P
#include "compat.h"
s32 gFramebufferWidth = DEFAULT_FRAMEBUFFER_WIDTH;
s32 gFramebufferHeight = DEFAULT_FRAMEBUFFER_HEIGHT;
u16 gFramebuffers[2][DEFAULT_FRAMEBUFFER_WIDTH * DEFAULT_FRAMEBUFFER_HEIGHT];
#endif

//main.c, core1.c, bk_engine.c


#define MAIN_THREAD_STACK_SIZE 0x17F0

#if VERSION == VERSION_PAL
extern s32 D_80000300;
#endif

s32 sScheduledModeChange = 0;
s32 sUnusedVar = 0;
u32 sGlobalTimer = 0;
u32 sDebugVar_8027561C[] = {0x9, 0x4, 0xA, 0x3, 0xB, 0x2, 0xC,
                            0x5, 0x0, 0x1, 0x6, 0xD, -1}; // never used
u32 sDataCRC1 = VER_SELECT(0xAD019D3C, 0xA371A8F3, 0, 0); // SM_DATA_CRC_1
u32 sDataCRC2 = VER_SELECT(0xD381B72F, 0xD0709154, 0, 0); // SM_DATA_CRC_2
char sDebugVar_80275658[] = VER_SELECT("HjunkDire:218755", "HjunkDire:300875",
                                       "HjunkDire:", "HjunkDire:");

/* .bss */
u32 sCurrentMode;
u8 pad_8027A138[0x400];
u64 sDebugVar1;                              // never used
u64 sDebugVar2;                              // never used
u8 sMainThreadStack[MAIN_THREAD_STACK_SIZE]; // The real size of the stack is
                                             // unclear yet, maybe there are
                                             // some out-optimized debug
                                             // variables below the stack
OSThread sMainThread;
s32 sBootMap;
static bool sDisableInput;
static u64 sDebugVar3;                       // never used

extern u8 core2_TEXT_START[];

void initialize_system(s32 arg0) {
  #ifndef LIGHTHOUSE_P
  bzero(&sCurrentMode, core2_TEXT_START - (u8 *)&sCurrentMode);
  #endif
  osWritebackDCacheAll();
  sns_find_and_parse_payload();
  osInitialize();
  initThread_create();
}

void system_flushHeapQueue(void) {
  assetCache_clear();
  heap_free_queue_flush(); // heap_flush_free_queue
}

void system_setGameMode(s32 arg0) {
  sendGfxTaskMessage();
  viMgr_clearFramebuffers();
  if (sCurrentMode == 4) {
    freeResources();
  }
  if (sCurrentMode == 3) {
    game_reset();
  }
  system_flushHeapQueue();
  sCurrentMode = arg0;
  if (sCurrentMode == 3) {
    game_initialize(sBootMap);
  }
  if (sCurrentMode == 4) {
    dummy_func_802E35D0();
  }
  ucode_stub1();
}

u32 globalTimer_getTimeMasked(u32 mask) { return sGlobalTimer & mask; }

s32 globalTimer_getTime(void) { return sGlobalTimer; }

void globalTimer_reset(void) { sGlobalTimer = 0; }

enum map_e getSpecialBootMap(void) {
  return (DEBUG_use_special_bootmap()) ? MAP_80_GL_FF_ENTRANCE
                                       : MAP_91_FILE_SELECT;
}

enum map_e getDefaultBootMap(void) { return MAP_1F_CS_START_RAREWARE; }

void system_setDefaultBootMap(void) {
  setBootMap(getDefaultBootMap());
  system_scheduleModeChange(3);
}

void system_setSpecialBootMap(void) {
  setBootMap(getSpecialBootMap());
  system_scheduleModeChange(3);
}

void core1_init(void) {
#if VERSION == VERSION_PAL
  osTvType = 0;
#endif
  ucode_load();
  setBootMap(getDefaultBootMap());
  rarezip_init(); // initialize decompressor's huft table
  viMgr_init();
  overlayManagerloadCore2();
  sDebugVar3 = sDebugVar1;
  heap_init();
  initGfxTaskQueue();
  dummy_func_8025AFB0();
  allocUnusedBlock();
  assetCache_init();
  pfsManager_init();
  baMotor_init();
  audioManager_init();
  graphicsCache_init();
  ml_init();
  gctransition_reset();
  sCurrentMode = 0;
  sGlobalTimer = 0;
  system_setGameMode(3);
}

void globalTimer_incTimer(void) { sGlobalTimer++; }

void globalTimer_decTimer(void) { sGlobalTimer--; }

void mainLoop(void) {
  s32 x, y;
  s32 r, g, b, a;
  u16 tmp;
  u16 rgba;
  s32 offset;

  if ((globalTimer_getTime() & 0x7f) == 0x11)
    sns_write_payload_over_heap();
  system_flushHeapQueue();

  if (sCurrentMode != 3 || game_getMode() != GAME_MODE_4_PAUSED)
    globalTimer_incTimer();

  if (!sDisableInput)
    pfsManager_update();
  sDisableInput = FALSE;

  baMotor_updateRumbleTimer();

/*     if (!mapSpecificFlags_validateCRC1()) {
      eeprom_writeBlocks(0, 0, 0x80397AD0, 0x40);
    } */

  if (!mapSpecificFlags_validateCRC1()) {
      eeprom_writeBlocks(0, 0, (void *)0x80397AD0, 0x40);
  }

  switch (sCurrentMode) {
  case 4:
    updateGameStateData();
    break;
  case 3:
    heap_free_temp_blocks_if_needed();
    heap_incrementCounter();
    spawnQueue_updateActorList();
    if (game_updateState())
      game_draw(0);
    spawnQueue_flush();
    break;
  } // L8023DE34

  if (sScheduledModeChange) {
    system_setGameMode(sScheduledModeChange - 1);
    sScheduledModeChange = 0;
  } // L8023DE54
  if (!updateGameSelectText() || !levelSpecificFlags_validateCRC1() ||
      !dummy_func_80320240()) {
    s32 offset;
    // render weird CRC failure image
    for (y = 0x1e; y < gFramebufferHeight - 0x1e; y++) { // L8023DEB4
      for (x = 0x14; x < 0xeb; x++) {
        tmp = ((8 * globalTimer_getTime()) + ((x * x) + (y * y)));

        r = _SHIFTL(x >> 3, 11, 5);
        g = _SHIFTL(y >> 3, 6, 5);
        b = _SHIFTL(tmp >> 3, 1, 5);
        a = 1;

        rgba = b | r | g | a;

        offset = ((gFramebufferWidth - 0xFF) / 2) + x + (y * gFramebufferWidth);
        gFramebuffers[0][offset] = (s32)rgba;
        gFramebuffers[1][offset] = (s32)rgba;
      }
    }
  } // L8023DF70
}

void mainThread_entry(void *arg) {
  core1_init();
  sns_write_payload_over_heap();

  while (1) {
    mainLoop();
  }
}

void system_scheduleModeChange(s32 arg0) { sScheduledModeChange = arg0 + 1; }

s32 system_getCurrentMode(void) { return sCurrentMode; }

void setBootMap(enum map_e map_id) { sBootMap = map_id; }

void mainThread_create(void) {
  osCreateThread(&sMainThread, 6, mainThread_entry, NULL,
                 sMainThreadStack + MAIN_THREAD_STACK_SIZE, 20);
}

OSThread *mainThread_get(void) { return &sMainThread; }

void disableInput_set(void) { sDisableInput = TRUE; }
