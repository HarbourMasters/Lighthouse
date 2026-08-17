// BanjoDecomp: code_0.c
#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include "port/ShipUtils.h" // gPortResetPending
#include "version.h"
#include "gc/gctransition.h"

#define MAIN_THREAD_STACK_SIZE 0x17F0

#if VERSION == VERSION_PAL
    extern s32 D_80000300;
#endif

s32 D_80275610 = 0; // always set to 0
s32 D_80275614 = 0;
s32 gGlobalTimer = 0;
u32 sDebugVar_8027561C[] = { 0x9, 0x4, 0xA, 0x3, 0xB, 0x2, 0xC, 0x5, 0x0,  0x1, 0x6, 0xD,  -1 }; // never used
s32 D_80275650 = VER_SELECT(0xAD019D3C, 0xA371A8F3, 0, 0); //SM_DATA_CRC2
s32 D_80275654 = VER_SELECT(0xD381B72F, 0xD0709154, 0, 0); //MM_DATA_CRC2
char sDebugVar_80275658[] = VER_SELECT("HjunkDire:218755", "HjunkDire:300875", "HjunkDire:", "HjunkDire:");

u32 D_8027A130; // always set to 3
u8 pad_8027A138[0x400];
u64 sDebugVar_8027A538; // never used
u64 sDebugVar_8027A540; // never used
u8 sMainThreadStack[MAIN_THREAD_STACK_SIZE]; // The real size of the stack is unclear yet, maybe there are some out-optimized debug variables below the stack
OSThread sMainThread;
s32 gBootMap;
bool sDisableInput;
u64 sDebugVar_8027BEF0; // never used

extern u8 core1_BSS_START[];
extern u8 core1_BSS_END[];

void core1_main(void *arg) {
#if 0
    bzero(core1_BSS_START, core1_BSS_END - core1_BSS_START);
    osWritebackDCacheAll();
    osInitialize();
#endif
    sns_find_and_parse_payload();

    initThread_create();
}

void func_8023DA74(void) {
    func_8033BD6C();
    func_80255198(); //heap_flush_free_queue
}

void func_8023DA9C(s32 next_state) {
    core1_15B30_sendMesg3ToRenderThread();
    viMgr_clearFramebuffers();

    if (D_8027A130 == 4) {
        func_802E3580();
    }

    if (D_8027A130 == 3) {
        func_802E4170();
    }

    func_8023DA74();
    D_8027A130 = next_state;

    if (D_8027A130 == 3) {
        func_802E4214(gBootMap);
    }

    if (D_8027A130 == 4) {
        dummy_func_802E35D0();
    }

    ucode_stub1();
}

s32 globalTimer_getTimeMasked(s32 mask) {
    return gGlobalTimer & mask;
}

s32 globalTimer_getTime(void){
    return gGlobalTimer;
}

void globalTimer_reset(void) {
    gGlobalTimer = 0;
}

enum map_e getSpecialBootMap(void) {
    return DEBUG_use_special_bootmap() ? MAP_80_GL_FF_ENTRANCE : MAP_91_FILE_SELECT;
}

// [port] Boot sequence: 0=Default (Rareware logo), 1=Authentic (same), 2=File Select
enum map_e getDefaultBootMap(void) {
    int seq = port_getBootSequence();
    if (seq == 2) // BOOTSEQUENCE_FILESELECT
        return MAP_91_FILE_SELECT;
    return MAP_1F_CS_START_RAREWARE;
}

void func_8023DBAC(void) {
    setBootMap(getDefaultBootMap());
    func_8023DFF0(3);
}

void func_8023DBDC(void) {
    setBootMap(getSpecialBootMap());
    func_8023DFF0(3);
}

void core1_init(void) {
#if VERSION == VERSION_PAL
     osTvType = OS_TV_TYPE_PAL;
#endif
    // ucode_load();
    setBootMap(getDefaultBootMap());
    // rarezip_init();
    viMgr_init();
    overlayManager_loadCore2();
    sDebugVar_8027BEF0 = sDebugVar_8027A538;
    // [port] Irrelevant and replaced with system malloc
    // heap_init();
    core1_15B30_init();
    // core1_1D590_func_8025AFB0();
    // [port] Irrelevant and replaced with system malloc
    // allocUnusedBlock();
    assetCache_init();
    pfsManager_init();
    baMotor_init();
    audioManager_init();
    graphicsCache_init();
    ml_init();
    gctransition_reset();
    D_8027A130 = 0;
    gGlobalTimer = 0;
    func_8023DA9C(3);
}

void globalTimer_incTimer(void) {
    gGlobalTimer++;
}

void globalTimer_decTimer(void) {
    gGlobalTimer--;
}

void mainLoop(void) {

    if((globalTimer_getTime() & 0x7f) == 0x11)
        sns_write_payload_over_heap();
    func_8023DA74();

    if ((D_8027A130 != 3) || (getGameMode() != GAME_MODE_4_PAUSED)) {
        globalTimer_incTimer();
    }
    
    if (!sDisableInput) {
        pfsManager_update();
    }

    sDisableInput = FALSE;

    baMotor_80250C08();

// [port] Don't need this
#if 0
    if(!mapSpecificFlags_validateCRC1()){
        eeprom_writeBlocks(0, 0, (void *) PHYS_TO_K0(0x00397AD0), EEPROM_MAXBLOCKS);
    }
#endif

    /* D_8027A130 seems to be always 3 during the game, it is only ever changed when calling
     * func_8023DA9C, which only happens in core1_init and after this switch statement, when the
     * following if statement is true (which is never be the case, see below).
     */
    switch (D_8027A130) {
        case 4:
            func_802E35D8();
            break;

        case 3:
            func_80255524();
            func_80255ACC();
            spawnQueue_func_802C3A18();
            if (func_802E4424()) {
                game_draw(FALSE);
            }
            spawnQueue_flush();
            break;
    }

    /* This if block is never executed, because D_80275610 seems to be 0 forever.
     * D_80275610 is only changed in func_8023DFF0, which is only called in a few places, that
     * never execute:
     * - func_8023DBAC and func_8023DBDC: Never gets called
     * - func_802E35D8: Only would be called in the above switch block if D_8027A130 was 4 (never happens, see above)
     * - func_802E4424: When D_8037E8E0.transition was 2 or 3, which is never the case
     */
    if (D_80275610) {
        func_8023DA9C(D_80275610 - 1);
        D_80275610 = 0;
        gPortResetPending = 0; // [port] clear flag so normal audio spin-waits work again
    }

// [port] This doesn't break anything, but better safe than sorry
#if 0
    s32 x, y;
    s32 r, g, b, a;
    u16 tmp;
    u16 rgba;
    s32 offset;

    if (!func_8032056C() || !levelSpecificFlags_validateCRC1() || !dummy_func_80320240()) {
        //render weird CRC failure image
        for (y = 30; y < gFramebufferHeight - 30; y++) {
            for (x = 20; x < 235; x++) {
                tmp = ((globalTimer_getTime() << 3) + x * x + y * y);

                r = _SHIFTL(x >> 3, 11, 5);
                g = _SHIFTL(y >> 3, 6, 5);
                b = _SHIFTL(tmp >> 3, 1, 5);
                a = 1;

                rgba = b | r | g | a;
                offset = ((gFramebufferWidth - 0xFF) / 2) + x + (y * gFramebufferWidth);

                gFramebuffers[0][offset] = (s32) rgba;
                gFramebuffers[1][offset] = (s32) rgba;
            }
        }
    }
#endif

    port_actorDespawn_beginDefer();
    CALL_EVENT(GameFrameUpdate);
    port_actorDespawn_endDefer();
}

void mainThread_entry(void *arg) {
    core1_init();
    sns_write_payload_over_heap();

    while (1) {
        mainLoop();
    }
}

// Calling with 3 causes the game to soft reboot, every other value causes a black screen and hang
void func_8023DFF0(s32 arg0) {
    D_80275610 = arg0 + 1;
}

s32 func_8023E000(void) {
    return D_8027A130;
}

void setBootMap(enum map_e map_id) {
    gBootMap = map_id;
}

void mainThread_create(void) {
    osCreateThread(&sMainThread, MAINTHREAD_ID, mainThread_entry, NULL, sMainThreadStack + MAIN_THREAD_STACK_SIZE, MAINTHREAD_PRI);
}

OSThread *mainThread_get(void) {
    return &sMainThread;
}

void disableInput_set(void) {
    sDisableInput = TRUE;
}