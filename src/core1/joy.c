// BanjoDecomp: joy.c
#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "port/ShipUtils.h" // BK_LOG_*, port_shapeControllerInput
#include "variables.h"
#include "core1/joy.h"

#include "version.h"
#include "checksums.h"

#include "port/DevTools/ThreadWatchdog.h"
#include "port/OS/OS.h"
#include "port/Patches/Patches.h"

#define JOY_THREAD_STACK_SIZE 0x200

extern s32 D_803727F4;
extern s32 D_80276574;

s32 D_80275D30 = VER_SELECT(0xC3A68832, 0xED7BCDB7, 0, 0); // CCW_DATA_CRC2
s32 D_80275D34 = VER_SELECT(0xDDC3A724, 0xF82DC7AC, 0, 0); // FIGHT_DATA_CRC2

/* Number of held frames for certain L+R button combos */
#define NUM_L_R_COMBOS (5)
static s32 sHeldFramesCombos[MAXCONTROLLERS][NUM_L_R_COMBOS];

static u8 sOsContBitPattern;

//static s32 sHeldFrames[MAXCONTROLLERS][JOY_BUTTON_COUNT];
s32 sHeldFrames[MAXCONTROLLERS][JOY_BUTTON_COUNT];
static s32 sHeldFramesPrimary[JOY_BUTTON_COUNT];

//static struct CachedInputs sCachedInputs[MAXCONTROLLERS];
struct CachedInputs sCachedInputs[MAXCONTROLLERS];

static OSMesg sSiEventQueueBuf;
static OSMesg sSiReplyQueueBuf;
//static OSContPad sInputs[MAXCONTROLLERS];
OSContPad sInputs[MAXCONTROLLERS];
static OSContPad sInputsPrimary;
static OSMesgQueue sSiEventQueue;
static OSMesgQueue sSiReplyQueue;

/* Timer for consecutive unchanged input, in seconds */
static f32 sInputUnchangedTimer[MAXCONTROLLERS];

static OSContStatus sOsContStatus;
static u8 pad_D_80281320[0x8];
static volatile s32 sJoyBusy;
static OSThread sJoyThread;
//STACK(sJoyThreadStack, 0x200);
u8 sJoyThreadStack[JOY_THREAD_STACK_SIZE];
static f32 sSecsPostTransition;

static OSMesgQueue D_802816E8;
static OSMesg D_80281700[5];
static s32 D_80275D38 = 0;

/**
 * Clamps and normalises joystick input axis
 * 
 * `min` is effectively the deadzone
 */
f32 controller_clampAndNormaliseJoyAxis(s32 input, s32 min, s32 max)
{
    f32 unit = 1 / 80.f;

    // [port] Original N64 anti-tamper: if code/data CRC checksums don't match,
    // halve stick sensitivity (0.00625 instead of 0.0125). On PC the CRCs never
    // match since code is recompiled, so stick output was always halved (max 0.5
    // instead of 1.0), making it impossible to reach the 0.7 threshold used by
    // gameSelect.c and other input consumers. Bypassed.
#if ANTI_TAMPER
    if (gChecksumsCore2.text_checksum2 != D_803727F4 || gChecksumsCore2.data_checksum2 != D_80276574)
        // Reduce stick range by half
        unit = 1 / 160.f;
#endif

    if (input > 0)
    {
        input = input > max ? max : input < min ? min : input;
        input = ((input - min) * 80) / (max - min);
    }
    else if (input < 0)
    {
        input = input < -max ? -max : input > -min ? -min : input;
        input = ((input + min) * 80) / (max - min);
    }

    return unit *= input;
}

void controller_copyFaceButtons(s32 controllerIdx, s32 dst[6])
{
    dst[FACE_BUTTON(BUTTON_A)]       = sHeldFrames[controllerIdx][JOY_BUTTON_A];
    dst[FACE_BUTTON(BUTTON_B)]       = sHeldFrames[controllerIdx][JOY_BUTTON_B];
    dst[FACE_BUTTON(BUTTON_C_LEFT)]  = sHeldFrames[controllerIdx][JOY_BUTTON_C_LEFT];
    dst[FACE_BUTTON(BUTTON_C_DOWN)]  = sHeldFrames[controllerIdx][JOY_BUTTON_C_DOWN];
    dst[FACE_BUTTON(BUTTON_C_UP)]    = sHeldFrames[controllerIdx][JOY_BUTTON_C_UP];
    dst[FACE_BUTTON(BUTTON_C_RIGHT)] = sHeldFrames[controllerIdx][JOY_BUTTON_C_RIGHT];
}

void controller_copyFaceButtonsPrimary(s32 controllerIdx, s32 dst[6])
{
    dst[FACE_BUTTON(BUTTON_A)]       = sHeldFramesPrimary[JOY_BUTTON_A];
    dst[FACE_BUTTON(BUTTON_B)]       = sHeldFramesPrimary[JOY_BUTTON_B];
    dst[FACE_BUTTON(BUTTON_C_LEFT)]  = sHeldFramesPrimary[JOY_BUTTON_C_LEFT];
    dst[FACE_BUTTON(BUTTON_C_DOWN)]  = sHeldFramesPrimary[JOY_BUTTON_C_DOWN];
    dst[FACE_BUTTON(BUTTON_C_UP)]    = sHeldFramesPrimary[JOY_BUTTON_C_UP];
    dst[FACE_BUTTON(BUTTON_C_RIGHT)] = sHeldFramesPrimary[JOY_BUTTON_C_RIGHT];
}

/**
 * Returns the number of frames a certain combo has been held for
 */
s32 controller_getHeldFramesForCombo(s32 controllerIdx, s32 comboIdx)
{
    return sHeldFramesCombos[controllerIdx][comboIdx];
}

s32 controller_copySideButtons(s32 controllerIdx, s32 dst[3])
{
    dst[SIDE_BUTTON(BUTTON_Z)] = sHeldFrames[controllerIdx][JOY_BUTTON_Z];
    dst[SIDE_BUTTON(BUTTON_L)] = sHeldFrames[controllerIdx][JOY_BUTTON_L];
    dst[SIDE_BUTTON(BUTTON_R)] = sHeldFrames[controllerIdx][JOY_BUTTON_R];
    return 0;
}

s32 controller_copySideButtonsPrimary(s32 controllerIdx, s32 dst[3])
{
    dst[SIDE_BUTTON(BUTTON_Z)] = sHeldFramesPrimary[JOY_BUTTON_Z];
    dst[SIDE_BUTTON(BUTTON_L)] = sHeldFramesPrimary[JOY_BUTTON_L];
    dst[SIDE_BUTTON(BUTTON_R)] = sHeldFramesPrimary[JOY_BUTTON_R];
    return 0;
}

/**
 * Returns number of seconds of consecutive unchanged input
 */
f32 controller_getInputUnchangedTimer(s32 controllerIdx)
{
    return sInputUnchangedTimer[controllerIdx];
}

s32 controller_getStartButton(s32 controllerIdx)
{
    return sHeldFrames[controllerIdx][JOY_BUTTON_START];
}

s32 controller_getStartButtonSafe(s32 controllerIdx)
{
    if (globalTimer_getTime() < 2)
        return 0;

    return sHeldFrames[controllerIdx][JOY_BUTTON_START];
}

void controller_copyDpadButtons(s32 controllerIdx, s32 dst[4])
{
    dst[DPAD_BUTTON(BUTTON_D_UP)]    = sHeldFrames[controllerIdx][JOY_BUTTON_D_UP];
    dst[DPAD_BUTTON(BUTTON_D_DOWN)]  = sHeldFrames[controllerIdx][JOY_BUTTON_D_DOWN];
    dst[DPAD_BUTTON(BUTTON_D_LEFT)]  = sHeldFrames[controllerIdx][JOY_BUTTON_D_LEFT];
    dst[DPAD_BUTTON(BUTTON_D_RIGHT)] = sHeldFrames[controllerIdx][JOY_BUTTON_D_RIGHT];
}

void controller_copyJoystick(s32 controllerIdx, f32 dst[2])
{
    if (func_802E4A08())
    {
        dst[0] = sCachedInputs[controllerIdx].currJoystick.x;
        dst[1] = sCachedInputs[controllerIdx].currJoystick.y;
    }
    else
    {
        dst[0] = controller_clampAndNormaliseJoyAxis(sInputs[controllerIdx].stick_x, 7, 59);
        dst[1] = controller_clampAndNormaliseJoyAxis(sInputs[controllerIdx].stick_y, 7, 61);
    }
}

// [port] Raw right-stick analog values, normalized to [-1, 1]. Positive Y is
// "up", matching the left stick's forward convention.
void controller_getRightStick(s32 controller_index, f32 dst[2]){
    dst[0] = (f32)sInputs[controller_index].right_stick_x / 127.0f;
    dst[1] = (f32)sInputs[controller_index].right_stick_y / 127.0f;
    if(dst[0] < -1.0f) dst[0] = -1.0f;
    if(dst[0] >  1.0f) dst[0] =  1.0f;
    if(dst[1] < -1.0f) dst[1] = -1.0f;
    if(dst[1] >  1.0f) dst[1] =  1.0f;
}


#define INCREMENT_OR_CLEAR_IF(dst, test) dst = (test) ? ((dst) + 1) : (0)

void joy_update(void)
{
    s32 j;
    s32 i;

    /*demo*/
    s32  curr_demo_lag;
    u16  maybe_start_btn;
    bool demo_is_over;

    /*btn tmps*/
    u32 prev_buttons;
    u32 curr_buttons;

    if (func_8023E000() == 3)
        func_802E4384();

    osSetThreadPri(0, 0x29);

    // [port] Re-read the latched transaction so the pad data is stable for the
    // whole tick; the SI completes reads at its own cadence, and the shaping
    // below rewrites this buffer in place.
    osContGetReadData(sInputs);

    // [port] Control-scheme input shaping: right stick -> C-button conversion,
    // Free Look right-stick handling, and the Modern-scheme Talon Trot combo.
    // Implemented in Enhancements/ControlSchemes.cpp.
    port_shapeControllerInput(&sInputs[0]);

    if (port_mirror_active()) sInputs[0].stick_x = -sInputs[0].stick_x;

    sInputsPrimary.stick_x = sInputs[0].stick_x;
    sInputsPrimary.stick_y = sInputs[0].stick_y;
    sInputsPrimary.button  = sInputs[0].button;

    if (getGameMode() == GAME_MODE_6_FILE_PLAYBACK
        || getGameMode() == GAME_MODE_7_ATTRACT_DEMO
        || getGameMode() == GAME_MODE_8_BOTTLES_BONUS
        || getGameMode() == GAME_MODE_A_SNS_PICTURE
        || getGameMode() == GAME_MODE_9_BANJO_AND_KAZOOIE
    )
    {
        maybe_start_btn = START_BUTTON;

        if (gctransition_done())
            sSecsPostTransition += time_getDelta();

        if (sSecsPostTransition < 1.0 || getGameMode() == GAME_MODE_9_BANJO_AND_KAZOOIE)
            maybe_start_btn = 0;

//      demo_is_over = demo_readInput(&sInputs, &curr_demo_lag) == 0;
        demo_is_over = demo_readInput(sInputs, &curr_demo_lag) == 0;

        if ((sInputsPrimary.button & maybe_start_btn) || demo_is_over)
        {
            if (sInputsPrimary.button & maybe_start_btn)
                // Finished demo via start button
                volatileFlag_set(VOLATILE_FLAG_64_DEMO_FINISHED_VIA_START_BTN, TRUE);
            else
                // Finished demo via demo done (no more inputs left in demo)
                volatileFlag_set(VOLATILE_FLAG_63_DEMO_FINISHED_VIA_DEMO_DONE, TRUE);
        }

        time_setDeltaReal_frames(curr_demo_lag);
        // [port] Override display pacing for maps that ran slow on N64.
        port_setDemoViCount(port_getDemoDisplayViCount(curr_demo_lag));
    }
    else
    {
        // [port] Use the VI divisor from cutscene framerate actors so Game.cpp
        // paces the display correctly for slower cutscenes.
        s32 viDiv = viMgr_func_8024BFA0();
        port_setDemoViCount((viDiv > 2) ? viDiv : 0);
    }

    curr_demo_lag = time_getDeltaReal_frames();
    randf();

    for (i = 0; i < MAXCONTROLLERS; i++)
    {
        if ((sInputs[i].button & L_TRIG) && (sInputs[i].button & R_TRIG))
        {
            // Handle L+R+_ button combos
            INCREMENT_OR_CLEAR_IF(sHeldFramesCombos[i][0], sInputs[i].button     & D_CBUTTONS);
            INCREMENT_OR_CLEAR_IF(sHeldFramesCombos[i][1], sInputs[i].button     & Z_TRIG);
            INCREMENT_OR_CLEAR_IF(sHeldFramesCombos[i][2], sInputs[i].button     & A_BUTTON);
            INCREMENT_OR_CLEAR_IF(sHeldFramesCombos[i][3], sInputs[i].button     & B_BUTTON);
            INCREMENT_OR_CLEAR_IF(sHeldFramesCombos[i][4], sInputsPrimary.button & B_BUTTON);

            // Clear all other inputs when L+R are pressed

            for (j = 0; j < JOY_BUTTON_COUNT; j++)
                sHeldFrames[i][j] = 0;

            for (j = 0; j < JOY_BUTTON_COUNT && i == 0; j++)
                sHeldFramesPrimary[j] = 0;

            sCachedInputs[i].currButtons = 0;
            sCachedInputs[i].prevButtons = 0;
            sCachedInputs[i].newlyPressedButtons = 0;
            sCachedInputs[i].newlyReleasedButtons = 0;
            sCachedInputs[i].currJoystick.x = 0.0f;
            sCachedInputs[i].currJoystick.y = 0.0f;
            sCachedInputs[i].prevJoystick.x = 0.0f;
            sCachedInputs[i].prevJoystick.y = 0.0f;

            continue;
        }

        for (j = 0; j < NUM_L_R_COMBOS; j++)
            sHeldFramesCombos[i][j] = 0;

        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_A],       sInputs[i].button & A_BUTTON);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_B],       sInputs[i].button & B_BUTTON);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_C_LEFT],  sInputs[i].button & L_CBUTTONS);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_C_DOWN],  sInputs[i].button & D_CBUTTONS);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_C_UP],    sInputs[i].button & U_CBUTTONS);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_C_RIGHT], sInputs[i].button & R_CBUTTONS);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_Z],       sInputs[i].button & Z_TRIG);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_L],       sInputs[i].button & L_TRIG);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_R],       sInputs[i].button & R_TRIG);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_D_UP],    sInputs[i].button & U_JPAD);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_D_DOWN],  sInputs[i].button & D_JPAD);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_D_LEFT],  sInputs[i].button & L_JPAD);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_D_RIGHT], sInputs[i].button & R_JPAD);
        INCREMENT_OR_CLEAR_IF(sHeldFrames[i][JOY_BUTTON_START],   sInputs[i].button & START_BUTTON);

        if (i == 0)
        {
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_A],       sInputsPrimary.button & A_BUTTON);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_B],       sInputsPrimary.button & B_BUTTON);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_C_LEFT],  sInputsPrimary.button & L_CBUTTONS);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_C_DOWN],  sInputsPrimary.button & D_CBUTTONS);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_C_UP],    sInputsPrimary.button & U_CBUTTONS);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_C_RIGHT], sInputsPrimary.button & R_CBUTTONS);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_Z],       sInputsPrimary.button & Z_TRIG);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_L],       sInputsPrimary.button & L_TRIG);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_R],       sInputsPrimary.button & R_TRIG);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_D_UP],    sInputsPrimary.button & U_JPAD);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_D_DOWN],  sInputsPrimary.button & D_JPAD);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_D_LEFT],  sInputsPrimary.button & L_JPAD);
            INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_D_RIGHT], sInputsPrimary.button & R_JPAD);
            // INCREMENT_OR_CLEAR_IF(sHeldFramesPrimary[JOY_BUTTON_START],   sInputsPrimary.button & START_BUTTON);

            // TODO resolve fake
            sHeldFramesPrimary[JOY_BUTTON_START] = sInputsPrimary.button & START_BUTTON ? sHeldFramesPrimary[JOY_BUTTON_START] + 1LL/*fake*/ : 0;
        }

        prev_buttons = ((u16)sCachedInputs[i].currButtons);
        curr_buttons = (u16)sInputs[i].button;

        sCachedInputs[i].currButtons          =  curr_buttons;
        sCachedInputs[i].prevButtons          =  prev_buttons;
        sCachedInputs[i].newlyPressedButtons  = ~prev_buttons &  curr_buttons;
        sCachedInputs[i].newlyReleasedButtons =  prev_buttons & ~curr_buttons;
        sCachedInputs[i].prevJoystick.x       = sCachedInputs[i].currJoystick.x;
        sCachedInputs[i].prevJoystick.y       = sCachedInputs[i].currJoystick.y;
        sCachedInputs[i].currJoystick.x       = controller_clampAndNormaliseJoyAxis(sInputs[i].stick_x, 7, 59);
        sCachedInputs[i].currJoystick.y       = controller_clampAndNormaliseJoyAxis(sInputs[i].stick_y, 7, 61);

        if (sCachedInputs[i].newlyPressedButtons
            || sCachedInputs[i].prevJoystick.x != sCachedInputs[i].currJoystick.x
            || sCachedInputs[i].prevJoystick.y != sCachedInputs[i].currJoystick.y
        )
        {
            sInputUnchangedTimer[i] = 0.0f;
        }
        else
        {
            sInputUnchangedTimer[i] += time_getDelta();
        }
    }

    CALL_EVENT(OnControllerUpdate);
    osSetThreadPri(0, 0x14);
}

void joy_readData()
{
    controller_func_8024F35C(0);

//  if (!sOsContStatus.errno)
//      osContGetReadData(sInputs);
}

void joy_main(void *null)
{
    while (1)
    {
        osRecvMesg(&sSiEventQueue, 0, 1);

        if (OS_ThreadShouldExit()) { // [port] cooperative shutdown
            return;
        }
        ThreadWatchdog_Beat(WATCHDOG_JOY); // [port] one beat per SI completion

        if (sJoyBusy == TRUE)
            joy_readData();
        else
//          osSendMesg(&sSiReplyQueue, 0, 0);
            osSendMesg32(&sSiReplyQueue, 0, 0);
    }
}

void joy_thread_init(void)
{
    osCreateMesgQueue(&sSiEventQueue, &sSiEventQueueBuf, 1);
    osCreateMesgQueue(&sSiReplyQueue, &sSiReplyQueueBuf, 1);
//  osCreateThread(&sJoyThread, CONTROLLER_THREAD_ID, joy_main, NULL, STACK_START(sJoyThreadStack), CONTROLLER_THREAD_PRI);
    osCreateThread(&sJoyThread, CONTROLLER_THREAD_ID, joy_main, NULL, sJoyThreadStack + JOY_THREAD_STACK_SIZE, CONTROLLER_THREAD_PRI);
//  osSetEventMesg(OS_EVENT_SI, &sSiEventQueue, &sSiEventQueueBuf);
    osSetEventMesg(OS_EVENT_SI, &sSiEventQueue, OS_MESG_PTR(&sSiEventQueueBuf));
    osContInit(&sSiEventQueue, &sOsContBitPattern, &sOsContStatus);
    osContSetCh(1);
    joy_reset();
    thread5_enableControllerTimer();
    osStartThread(&sJoyThread);
}

bool joy_contHasErr(void)
{
//  return BOOL(sOsContStatus.errno);
    return BOOL(sOsContStatus.err_no);
}

void joy_spawnNoControllerOverlay(void)
{
    if (joy_contHasErr())
        chOverlayNoController_spawn(0, 0);
}

void joy_noControllerOverlay_8024F180(void)
{
    if (joy_contHasErr())
        chOverlayNoController_func_802DD040(0, 0);
}

void joy_getStartReadData(void)
{
    if (sJoyBusy == 0)
    {
        controller_func_8024F35C(1);
        osContStartReadData(&sSiEventQueue);
    }
}

void joy_waitForEventAndUpdate(void)
{
    osRecvMesg(&sSiEventQueue, NULL, 1);
    joy_update();
}

/**
 * Clear data for all controllers
 */
void joy_reset(void)
{
    s32 i, j;

    for(i = 0; i < MAXCONTROLLERS; i++)
    {
        sCachedInputs[i].currButtons = 0;
        sCachedInputs[i].prevButtons = 0;
        sCachedInputs[i].newlyPressedButtons = 0;
        sCachedInputs[i].newlyReleasedButtons = 0;
        sCachedInputs[i].currJoystick.x = 0.0f;
        sCachedInputs[i].currJoystick.y = 0.0f;
        sCachedInputs[i].prevJoystick.x = 0.0f;
        sCachedInputs[i].prevJoystick.y = 0.0f;

        for(j = 0; j < NUM_L_R_COMBOS; j++)
            sHeldFramesCombos[i][j] = 0;

        for(j = 0; j < JOY_BUTTON_COUNT; j++)
            sHeldFrames[i][j] = 0;

        sInputUnchangedTimer[i] = 0.0f;
    }
}

void joy_copyInputs(s32 controllerIdx, struct CachedInputs *dst)
{
    bk_memcpy(dst, &sCachedInputs[controllerIdx], sizeof(struct CachedInputs));
}

void joy_setHeldFramesForZ(s32 controllerIdx, s32 heldFrames)
{
    sHeldFrames[controllerIdx][JOY_BUTTON_Z] = heldFrames;
}

OSMesgQueue *si_getReplyQueue(void)
{
    return &sSiReplyQueue;
}

OSMesgQueue *si_getEventQueue(void)
{
    return &sSiEventQueue;
}

// [port] Watchdog diagnostics: the SI event-registration lock queue
// (func_8024F450 parks here), so blocked waits get a name.
OSMesgQueue *joy_getSiLockQueue(void){
    return &D_802816E8;
}

void controller_func_8024F35C(s32 arg0)
{
    if (!arg0)
        func_8024F4AC();
    else
        func_8024F450();

    if (arg0 || D_802816E8.validCount == 1)
        sJoyBusy = arg0; 
}

bool joy_getBusy(void)
{
    return sJoyBusy;
}

int joy_8024F3C4(int controllerIdx)
{
    return sInputs[controllerIdx].button
         + sInputs[controllerIdx].stick_x
         + sInputs[controllerIdx].stick_y;
}

OSContPad *joy_getInputsPrimary(void)
{
    return &sInputsPrimary;
}

/* initilizes D_802816E8 message queue */
void func_8024F400(void)
{
    D_80275D38 = TRUE;
//  osCreateMesgQueue(&D_802816E8, D_80281700, ARRLEN(D_80281700));
    osCreateMesgQueue(&D_802816E8, D_80281700, 5);
    OS_SetQueueBlocking(&D_802816E8, 1);
//  osSendMesg(&D_802816E8, 0, OS_MESG_NOBLOCK);
    osSendMesgPtr(&D_802816E8, NULL, OS_MESG_NOBLOCK);
}

void func_8024F450(void)
{
    if (!D_80275D38)
        func_8024F400();

    osRecvMesg(&D_802816E8, NULL, OS_MESG_BLOCK);
//  osSetEventMesg(OS_EVENT_SI, &sSiEventQueue, &sSiEventQueueBuf);
    osSetEventMesg(OS_EVENT_SI, &sSiEventQueue, OS_MESG_PTR(&sSiEventQueueBuf));
}

void func_8024F4AC(void)
{
//  osSendMesg(&D_802816E8, NULL, OS_MESG_NOBLOCK);
    osSendMesgPtr(&D_802816E8, NULL, OS_MESG_NOBLOCK);
}
