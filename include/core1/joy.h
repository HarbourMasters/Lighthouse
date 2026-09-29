#ifndef BANJO_KAZOOIE_CORE1_JOY_H
#define BANJO_KAZOOIE_CORE1_JOY_H
#include "libultraship/libultra/message.h"
#include "libultraship/libultra/controller.h"

#define CONTROLLER_THREAD_ID 7
#define CONTROLLER_THREAD_PRI 40

enum joy_face_buttons_e
{
    /* -1 */ JOY_BUTTON_nil = -1,
    /*  0 */ JOY_BUTTON_A,
    /*  1 */ JOY_BUTTON_B,
    /*  2 */ JOY_BUTTON_C_LEFT,
    /*  3 */ JOY_BUTTON_C_DOWN,
    /*  4 */ JOY_BUTTON_C_UP,
    /*  5 */ JOY_BUTTON_C_RIGHT,
    /*  6 */ JOY_BUTTON_Z,
    /*  7 */ JOY_BUTTON_L,
    /*  8 */ JOY_BUTTON_R,
    /*  9 */ JOY_BUTTON_D_UP,
    /*  A */ JOY_BUTTON_D_DOWN,
    /*  B */ JOY_BUTTON_D_LEFT,
    /*  C */ JOY_BUTTON_D_RIGHT,
    /*  D */ JOY_BUTTON_START,
    JOY_BUTTON_COUNT
};

struct CachedInputs
{
    s16 currButtons;
    s16 prevButtons;
    u16 newlyPressedButtons;
    u16 newlyReleasedButtons;
    struct { f32 x; f32 y; } prevJoystick;
    struct { f32 x; f32 y; } currJoystick;
};

f32 controller_clampAndNormaliseJoyAxis(s32 input, s32 min, s32 max);
void controller_copyFaceButtons(s32 controllerIdx, s32 dst[6]);
void controller_copyFaceButtonsPrimary(s32 controllerIdx, s32 dst[6]);
s32 controller_getHeldFramesForCombo(s32 controllerIdx, s32 comboIdx);
s32 controller_copySideButtons(s32 controllerIdx, s32 dst[3]);
s32 controller_copySideButtonsPrimary(s32 controllerIdx, s32 dst[3]);
f32 controller_getInputUnchangedTimer(s32 controllerIdx);
s32 controller_getStartButton(s32 controllerIdx);
s32 controller_getStartButtonSafe(s32 controllerIdx);
void controller_copyDpadButtons(s32 controllerIdx, s32 dst[4]);
void controller_copyJoystick(s32 controllerIdx, f32 dst[2]);
// [port] Raw right-stick analog values, normalized to [-1, 1] (y positive = up).
void controller_getRightStick(s32 controller_index, f32 dst[2]);
void joy_update(void);
void joy_readData();
void joy_main(void *null);
void joy_thread_init(void);
bool joy_contHasErr(void);
void joy_spawnNoControllerOverlay(void);
void joy_noControllerOverlay_8024F180(void);
void joy_getStartReadData(void);
void joy_waitForEventAndUpdate(void);
void joy_reset(void);
void joy_copyInputs(s32 controllerIdx, struct CachedInputs *dst);
void joy_setHeldFramesForZ(s32 controllerIdx, s32 heldFrames);
OSMesgQueue *si_getReplyQueue(void);
OSMesgQueue *si_getEventQueue(void);
void controller_func_8024F35C(s32 arg0);
bool joy_getBusy(void);
int joy_8024F3C4(int controllerIdx);
OSContPad *joy_getInputsPrimary(void);
void func_8024F400(void);
void func_8024F450(void);
void func_8024F4AC(void);

#endif
