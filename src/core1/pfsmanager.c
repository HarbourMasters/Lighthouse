// BanjoDecomp: pfsmanager.c
#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "port/ShipUtils.h" // BK_LOG_*, port_shapeControllerInput
#include "variables.h"
#include "core1/pfsmanager.h"

#include "version.h"
#include "checksums.h"

#include "port/DevTools/ThreadWatchdog.h"
#include "port/OS/OS.h"
#include "port/Patches/Patches.h"

#define PFSMANAGER_THREAD_STACK_SIZE 0x200

extern s32 D_803727F4;
extern s32 D_80276574;

s32 D_80275D30 = VER_SELECT(0xC3A68832, 0xED7BCDB7, 0, 0); // CCW_DATA_CRC2
s32 D_80275D34 = VER_SELECT(0xDDC3A724, 0xF82DC7AC, 0, 0); // FIGHT_DATA_CRC2

static s32 D_80275D38 = 0;

/* .bss */
UNK_TYPE(s32) D_802810E0[4][5];
u8 pfsManagerBitPattern;
PfsManagerControllerData D_80281138[4];
PfsManagerControllerData D_80281218;
Struct_core1_10A00_1 D_80281250[4];
OSMesg pfsManagerContPollingMsqBuf;
OSMesg pfsManagerContReplyMsgBuf;
OSContPad pfsManagerContPadData[4];
OSContPad D_802812D0;
OSMesgQueue pfsManagerContPollingMsqQ;
OSMesgQueue pfsManagerContReplyMsgQ;
f32 D_80281308[4];
OSContStatus pfsManagerContStatus;
u8 pad_D_80281320[0x8];
volatile s32 pfsManagerBusy;
OSThread sPfsManagerThread;
u8 sPfsManagerThreadStack[PFSMANAGER_THREAD_STACK_SIZE];
f32 D_802816E0;
OSMesgQueue D_802816E8;
OSMesg D_80281700[5];
u8 pad_D_80281710[1];

f32 func_8024E420(s32 arg0, s32 arg1, s32 arg2) {
    f32 phi_f2;

    phi_f2 = 0.0125f;
    // [port] Original N64 anti-tamper: if code/data CRC checksums don't match,
    // halve stick sensitivity (0.00625 instead of 0.0125). On PC the CRCs never
    // match since code is recompiled, so stick output was always halved (max 0.5
    // instead of 1.0), making it impossible to reach the 0.7 threshold used by
    // gameSelect.c and other input consumers. Bypassed.
#if ANTI_TAMPER
    if ((gChecksumsCore2.text_checksum2 != D_803727F4) || (gChecksumsCore2.data_checksum2 != D_80276574)) {
        phi_f2 = 0.00625f;
    }
#endif
    if (arg0 > 0) {
        arg0 = (arg2 < arg0) ? arg2 : (arg0 < arg1) ? arg1 : arg0;
        arg0 = (s32) ((arg0 - arg1) * 0x50) / (s32) (arg2 - arg1);
    } else {
        if (arg0 < 0) {
            arg0 = (arg0 < -arg2) ? -arg2 : (-arg1 < arg0) ? -arg1 : arg0;
            arg0 = (s32) ((arg0 + arg1) * 0x50) / (s32) (arg2 - arg1);
        }
    }
    return phi_f2 *= arg0;
}

void controller_copyFaceButtons(s32 controller_index, s32 dst[6]){
    dst[FACE_BUTTON(BUTTON_A)]       = D_80281138[controller_index].face_button[FACE_BUTTON(BUTTON_A)];
    dst[FACE_BUTTON(BUTTON_B)]       = D_80281138[controller_index].face_button[FACE_BUTTON(BUTTON_B)];
    dst[FACE_BUTTON(BUTTON_C_LEFT)]  = D_80281138[controller_index].face_button[FACE_BUTTON(BUTTON_C_LEFT)];
    dst[FACE_BUTTON(BUTTON_C_DOWN)]  = D_80281138[controller_index].face_button[FACE_BUTTON(BUTTON_C_DOWN)];
    dst[FACE_BUTTON(BUTTON_C_UP)]    = D_80281138[controller_index].face_button[FACE_BUTTON(BUTTON_C_UP)];
    dst[FACE_BUTTON(BUTTON_C_RIGHT)] = D_80281138[controller_index].face_button[FACE_BUTTON(BUTTON_C_RIGHT)];
}

void pfsManager_getFirstControllerFaceButtonState(s32 controller_index, s32 dst[6]){
    dst[FACE_BUTTON(BUTTON_A)]       = D_80281218.face_button[FACE_BUTTON(BUTTON_A)];
    dst[FACE_BUTTON(BUTTON_B)]       = D_80281218.face_button[FACE_BUTTON(BUTTON_B)];
    dst[FACE_BUTTON(BUTTON_C_LEFT)]  = D_80281218.face_button[FACE_BUTTON(BUTTON_C_LEFT)];
    dst[FACE_BUTTON(BUTTON_C_DOWN)]  = D_80281218.face_button[FACE_BUTTON(BUTTON_C_DOWN)];
    dst[FACE_BUTTON(BUTTON_C_UP)]    = D_80281218.face_button[FACE_BUTTON(BUTTON_C_UP)];
    dst[FACE_BUTTON(BUTTON_C_RIGHT)] = D_80281218.face_button[FACE_BUTTON(BUTTON_C_RIGHT)];
}

s32 func_8024E5E8(s32 arg0, s32 arg1){
    return D_802810E0[arg0][arg1];
}

s32 controller_copySideButtons(s32 controller_index, s32 dst[3]){
    dst[SIDE_BUTTON(BUTTON_Z)]       = D_80281138[controller_index].side_button[SIDE_BUTTON(BUTTON_Z)];
    dst[SIDE_BUTTON(BUTTON_L)]       = D_80281138[controller_index].side_button[SIDE_BUTTON(BUTTON_L)];
    dst[SIDE_BUTTON(BUTTON_R)]       = D_80281138[controller_index].side_button[SIDE_BUTTON(BUTTON_R)];
    return 0;
}

s32 func_8024E640(s32 controller_index, s32 dst[3]){
    dst[SIDE_BUTTON(BUTTON_Z)]       = D_80281218.side_button[SIDE_BUTTON(BUTTON_Z)];
    dst[SIDE_BUTTON(BUTTON_L)]       = D_80281218.side_button[SIDE_BUTTON(BUTTON_L)];
    dst[SIDE_BUTTON(BUTTON_R)]       = D_80281218.side_button[SIDE_BUTTON(BUTTON_R)];
    return 0;
}

f32 func_8024E668(s32 controller_index){
    return D_80281308[controller_index];
}

s32 controller_getStartButton(s32 controller_index){
    return D_80281138[controller_index].start_button;
}

s32 func_8024E698(s32 controller_index){
    if(globalTimer_getTime() < 2){
        return 0;
    }
    
    return D_80281138[controller_index].start_button;
}

void func_8024E6E0(s32 controller_index, s32 dst[4]){
    dst[0] = D_80281138[controller_index].unk24[0];
    dst[1] = D_80281138[controller_index].unk24[1];
    dst[2] = D_80281138[controller_index].unk24[2];
    dst[3] = D_80281138[controller_index].unk24[3];
}

void controller_getJoystick(s32 controller_index, f32 dst[2]){
    if(func_802E4A08()){
        dst[0] = D_80281250[controller_index].joystick[0];
        dst[1] = D_80281250[controller_index].joystick[1];
    }
    else{
        dst[0] = func_8024E420(pfsManagerContPadData[controller_index].stick_x, 7, 0x3B);
        dst[1] = func_8024E420(pfsManagerContPadData[controller_index].stick_y, 7, 0x3D);
    }
}

// [port] Raw right-stick analog values, normalized to [-1, 1]. Positive Y is
// "up", matching the left stick's forward convention.
void controller_getRightStick(s32 controller_index, f32 dst[2]){
    dst[0] = (f32)pfsManagerContPadData[controller_index].right_stick_x / 127.0f;
    dst[1] = (f32)pfsManagerContPadData[controller_index].right_stick_y / 127.0f;
    if(dst[0] < -1.0f) dst[0] = -1.0f;
    if(dst[0] >  1.0f) dst[0] =  1.0f;
    if(dst[1] < -1.0f) dst[1] = -1.0f;
    if(dst[1] >  1.0f) dst[1] =  1.0f;
}

void pfsManager_update(void) {
    int j;
    int i;
    u32 sp5C;
    u32 s0;
    u32 temp_t6;
    u32 temp_v0_3;
    u32 var_a2;
    if (func_8023E000() == 3) {
        func_802E4384();
    }

    osSetThreadPri(0, 0x29);

    // [port] Re-read the latched transaction so the pad data is stable for the
    // whole tick; the SI completes reads at its own cadence, and the shaping
    // below rewrites this buffer in place.
    osContGetReadData(pfsManagerContPadData);

    // [port] Control-scheme input shaping: right stick -> C-button conversion,
    // Free Look right-stick handling, and the Modern-scheme Talon Trot combo.
    // Implemented in Enhancements/ControlSchemes.cpp.
    port_shapeControllerInput(&pfsManagerContPadData[0]);

    if (port_mirror_active()) pfsManagerContPadData[0].stick_x = -pfsManagerContPadData[0].stick_x;

    D_802812D0.stick_x = pfsManagerContPadData[0].stick_x;
    D_802812D0.stick_y = pfsManagerContPadData[0].stick_y;
    D_802812D0.button = pfsManagerContPadData[0].button;
    if ((getGameMode() == GAME_MODE_6_FILE_PLAYBACK) 
        || (getGameMode() == GAME_MODE_7_ATTRACT_DEMO)
        || (getGameMode() == GAME_MODE_8_BOTTLES_BONUS)
        || (getGameMode() == GAME_MODE_A_SNS_PICTURE)
        || (getGameMode() == GAME_MODE_9_BANJO_AND_KAZOOIE)
    ) {
        s0 = 0x1000;
        if (gctransition_done()) {
            D_802816E0 += time_getDelta();
        }
        if ((D_802816E0 < 1.0) || (getGameMode() == GAME_MODE_9_BANJO_AND_KAZOOIE)) {
            s0 = 0;
        }
        temp_t6 = demo_readInput(pfsManagerContPadData, &sp5C) == 0;
        if ((D_802812D0.button & s0) || temp_t6) {
            if (D_802812D0.button & s0) {
                volatileFlag_set(VOLATILE_FLAG_64, 1);
            } else {
                volatileFlag_set(VOLATILE_FLAG_63, 1);
            }
        }
        time_setDeltaReal_frames(sp5C);
        // [port] Override display pacing for maps that ran slow on N64.
        port_setDemoViCount(port_getDemoDisplayViCount(sp5C));
    } else {
        // [port] Use the VI divisor from cutscene framerate actors so Game.cpp
        // paces the display correctly for slower cutscenes.
        s32 viDiv = viMgr_func_8024BFA0();
        port_setDemoViCount((viDiv > 2) ? viDiv : 0);
    }
    sp5C = time_getDeltaReal_frames();
    randf();
    for (i = 0; i < 4; i++) {
        if ((pfsManagerContPadData[i].button & 0x20) && (pfsManagerContPadData[i].button & 0x10)) {
            D_802810E0[i][0] = (pfsManagerContPadData[i].button & 0x0004) ? (D_802810E0[i][0] + 1) : (0);
            D_802810E0[i][1] = (pfsManagerContPadData[i].button & 0x2000) ? (D_802810E0[i][1] + 1) : (0);
            D_802810E0[i][2] = (pfsManagerContPadData[i].button & 0x8000) ? (D_802810E0[i][2] + 1) : (0);
            D_802810E0[i][3] = (pfsManagerContPadData[i].button & 0x4000) ? (D_802810E0[i][3] + 1) : (0);
            D_802810E0[i][4] = (D_802812D0.button & 0x4000) ? (D_802810E0[i][4] + 1) : (0);
            for (j = 0; j < 0xE; j++)
            {
                ((s32 *) (&D_80281138[i]))[j] = 0;
            }

            for (j = 0; (j < 0xE) && (i == 0); j++)
            {
                ((s32 *) (&D_80281218))[j] = 0;
            }

            D_80281250[i].unk0 = 0;
            D_80281250[i].unk2 = 0;
            D_80281250[i].unk4 = 0;
            D_80281250[i].unk6 = 0;
            D_80281250[i].joystick[0] = 0.0f;
            D_80281250[i].joystick[1] = 0.0f;
            D_80281250[i].unk8[0] = 0.0f;
            D_80281250[i].unk8[1] = 0.0f;
            continue;
        }

        for (j = 0; j < 5; j++) {
            D_802810E0[i][j] = 0;
        }

        D_80281138[i].face_button[0] = (pfsManagerContPadData[i].button & 0x8000) ? (D_80281138[i].face_button[0] + 1) : (0);
        D_80281138[i].face_button[1] = (pfsManagerContPadData[i].button & 0x4000) ? (D_80281138[i].face_button[1] + 1) : (0);
        D_80281138[i].face_button[2] = (pfsManagerContPadData[i].button & 0x0002) ? (D_80281138[i].face_button[2] + 1) : (0);
        D_80281138[i].face_button[3] = (pfsManagerContPadData[i].button & 0x0004) ? (D_80281138[i].face_button[3] + 1) : (0);
        D_80281138[i].face_button[4] = (pfsManagerContPadData[i].button & 0x0008) ? (D_80281138[i].face_button[4] + 1) : (0);
        D_80281138[i].face_button[5] = (pfsManagerContPadData[i].button & 0x0001) ? (D_80281138[i].face_button[5] + 1) : (0);
        D_80281138[i].side_button[0] = (pfsManagerContPadData[i].button & 0x2000) ? (D_80281138[i].side_button[0] + 1) : (0);
        D_80281138[i].side_button[1] = (pfsManagerContPadData[i].button & 0x0020) ? (D_80281138[i].side_button[1] + 1) : (0);
        D_80281138[i].side_button[2] = (pfsManagerContPadData[i].button & 0x0010) ? (D_80281138[i].side_button[2] + 1) : (0);
        D_80281138[i].unk24[0] = (pfsManagerContPadData[i].button & 0x0800) ? (D_80281138[i].unk24[0] + 1) : (0);
        D_80281138[i].unk24[1] = (pfsManagerContPadData[i].button & 0x0400) ? (D_80281138[i].unk24[1] + 1) : (0);
        D_80281138[i].unk24[2] = (pfsManagerContPadData[i].button & 0x0200) ? (D_80281138[i].unk24[2] + 1) : (0);
        D_80281138[i].unk24[3] = (pfsManagerContPadData[i].button & 0x0100) ? (D_80281138[i].unk24[3] + 1) : (0);
        D_80281138[i].start_button = (pfsManagerContPadData[i].button & 0x1000) ? (D_80281138[i].start_button + 1) : (0);
        if (i == 0) {
            D_80281218.face_button[0] = (D_802812D0.button & 0x8000) ? (D_80281218.face_button[0] + 1) : (0);
            D_80281218.face_button[1] = (D_802812D0.button & 0x4000) ? (D_80281218.face_button[1] + 1) : (0);
            D_80281218.face_button[2] = (D_802812D0.button & 0x0002) ? (D_80281218.face_button[2] + 1) : (0);
            D_80281218.face_button[3] = (D_802812D0.button & 0x0004) ? (D_80281218.face_button[3] + 1) : (0);
            D_80281218.face_button[4] = (D_802812D0.button & 0x0008) ? (D_80281218.face_button[4] + 1) : (0);
            D_80281218.face_button[5] = (D_802812D0.button & 0x0001) ? (D_80281218.face_button[5] + 1) : (0);
            D_80281218.side_button[0] = (D_802812D0.button & 0x2000) ? (D_80281218.side_button[0] + 1) : (0);
            D_80281218.side_button[1] = (D_802812D0.button & 0x0020) ? (D_80281218.side_button[1] + 1) : (0);
            D_80281218.side_button[2] = (D_802812D0.button & 0x0010) ? (D_80281218.side_button[2] + 1) : (0);
            D_80281218.unk24[0] = (D_802812D0.button & 0x0800) ? (D_80281218.unk24[0] + 1) : (0);
            D_80281218.unk24[1] = (D_802812D0.button & 0x0400) ? (D_80281218.unk24[1] + 1) : (0);
            D_80281218.unk24[2] = (D_802812D0.button & 0x0200) ? (D_80281218.unk24[2] + 1) : (0);
            D_80281218.unk24[3] = (D_802812D0.button & 0x0100) ? (D_80281218.unk24[3] + 1) : (0);
            D_80281218.start_button = (D_802812D0.button & 0x1000) ? ((u64)D_80281218.start_button + 1) : (0);
        }
        temp_v0_3 = (u16)D_80281250[i].unk0;
        var_a2 = (u16)pfsManagerContPadData[i].button;
        D_80281250[i].unk0 = var_a2;
        D_80281250[i].unk2 = temp_v0_3;
        D_80281250[i].unk4 = (~temp_v0_3) & var_a2;
        D_80281250[i].unk6 = temp_v0_3 & (~var_a2);
        D_80281250[i].unk8[0] = D_80281250[i].joystick[0];
        D_80281250[i].unk8[1] = D_80281250[i].joystick[1];
        D_80281250[i].joystick[0] = func_8024E420(pfsManagerContPadData[i].stick_x, 7, 0x3B);
        D_80281250[i].joystick[1] = func_8024E420(pfsManagerContPadData[i].stick_y, 7, 0x3D);
        if ((D_80281250[i].unk4 != 0) 
            || (D_80281250[i].unk8[0] != D_80281250[i].joystick[0])
            || (D_80281250[i].unk8[1] != D_80281250[i].joystick[1])
        ) {
            D_80281308[i] = 0.0f;
        } else {
            D_80281308[i] += time_getDelta();
        }
    }

    CALL_EVENT(OnControllerUpdate);
    osSetThreadPri(0, 0x14);
}

void pfsManager_readData(){
    func_8024F35C(0);
}


void pfsManager_entry(void *arg) {
    do {
        osRecvMesg(&pfsManagerContPollingMsqQ, 0, 1);
        if (OS_ThreadShouldExit()) { // [port] cooperative shutdown
            return;
        }
        ThreadWatchdog_Beat(WATCHDOG_PFSMANAGER); // [port] one beat per SI completion
        if(pfsManagerBusy == true){
            pfsManager_readData();
        }
        else{
            osSendMesg32(&pfsManagerContReplyMsgQ, 0, 0);
        }
    } while (1);
}

void pfsManager_init(void) {
    osCreateMesgQueue(&pfsManagerContPollingMsqQ, &pfsManagerContPollingMsqBuf, 1);
    osCreateMesgQueue(&pfsManagerContReplyMsgQ, &pfsManagerContReplyMsgBuf, 1);
    osCreateThread(&sPfsManagerThread, 7, pfsManager_entry, NULL, sPfsManagerThreadStack + PFSMANAGER_THREAD_STACK_SIZE, 40);
    osSetEventMesg(OS_EVENT_SI, &pfsManagerContPollingMsqQ, OS_MESG_PTR(&pfsManagerContPollingMsqBuf));
    osContInit(&pfsManagerContPollingMsqQ, &pfsManagerBitPattern, &pfsManagerContStatus);
    osContSetCh(1);
    func_8024F224();
    thread5_enableControllerTimer();
    osStartThread(&sPfsManagerThread);
}

bool pfsManager_contErr(void) {
    return BOOL(pfsManagerContStatus.err_no);
}

void func_8024F150(void){
    if(pfsManager_contErr())
        chOverlayNoController_spawn(0,0);
}

void func_8024F180(void){
    if(pfsManager_contErr())
        chOverlayNoController_func_802DD040(0,0);
}

void pfsManager_getStartReadData(void){
    if(pfsManagerBusy == 0){
        func_8024F35C(1);
        osContStartReadData(&pfsManagerContPollingMsqQ);
    }
}

void func_8024F1F0(void){
    osRecvMesg(&pfsManagerContPollingMsqQ, NULL, 1);
    pfsManager_update();
}

void func_8024F224(void){
    s32 iCont, j;

    // for(iCont = 0; iCont < 4; iCont++){
    //     D_80281250[iCont].unk0 = 0;
    // }

    for(iCont = 0; iCont < 4; iCont++){
        D_80281250[iCont].unk0 = 0;
        D_80281250[iCont].unk2 = 0;
        D_80281250[iCont].unk4 = 0;
        D_80281250[iCont].unk6 = 0;
        D_80281250[iCont].joystick[0] = 0.0f;
        D_80281250[iCont].joystick[1] = 0.0f;
        D_80281250[iCont].unk8[0] = 0.0f;
        D_80281250[iCont].unk8[1] = 0.0f;
        for(j = 0; j < 5; j++){
            D_802810E0[iCont][j] = 0;
        }
        for(j = 0; j < 14; j++){
            D_80281138[iCont].face_button[j] = 0;
        }
        D_80281308[iCont] = 0.0f;
    }
}

void func_8024F2E4(s32 arg0, Struct_core1_10A00_1 *arg1){
    memcpy(arg1, D_80281250 + arg0, sizeof(Struct_core1_10A00_1));
}

void func_8024F328(s32 controller_index, s32 arg1){
    D_80281138[controller_index].side_button[SIDE_BUTTON(BUTTON_Z)] = arg1;
}

OSMesgQueue * pfsManager_getFrameReplyQ(void){
    return &pfsManagerContReplyMsgQ;
}

OSMesgQueue *pfsManager_getFrameMesgQ(void){
    return &pfsManagerContPollingMsqQ;
}

// [port] Watchdog diagnostics: the SI event-registration lock queue
// (func_8024F450 parks here), so blocked waits get a name.
OSMesgQueue *pfsManager_getSiLockQueue(void){
    return &D_802816E8;
}

void func_8024F35C(s32 arg0) {
    if(!arg0)
        func_8024F4AC();
    else
        func_8024F450();

    if(arg0 || D_802816E8.validCount == 1){
        pfsManagerBusy = arg0;
    }
}

bool pfsManager_isBusy(void){
    return pfsManagerBusy;
}

int func_8024F3C4(int arg0){
    return pfsManagerContPadData[arg0].button + pfsManagerContPadData[arg0].stick_x + pfsManagerContPadData[arg0].stick_y;
}

OSContPad *func_8024F3F4(void){
    return &D_802812D0;
}

/* initilizes D_802816E8 message queue */
void func_8024F400(void) {
    D_80275D38 = true;
    osCreateMesgQueue(&D_802816E8, D_80281700, 5);
    OS_SetQueueBlocking(&D_802816E8, 1);
    osSendMesgPtr(&D_802816E8, NULL, OS_MESG_NOBLOCK);
}

void func_8024F450(void){
    if(!D_80275D38)
        func_8024F400();
    osRecvMesg(&D_802816E8, NULL, OS_MESG_BLOCK);
    osSetEventMesg(OS_EVENT_SI, &pfsManagerContPollingMsqQ, OS_MESG_PTR(&pfsManagerContPollingMsqBuf));
}

void func_8024F4AC(void){
    osSendMesgPtr(&D_802816E8, NULL, OS_MESG_NOBLOCK);
}
