#ifndef BANJO_KAZOOIE_CORE1_PFSMANAGER_H
#define BANJO_KAZOOIE_CORE1_PFSMANAGER_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct pfs_manager_face_buttons_s {
    bool button_a;
    bool button_b;
    bool button_c_left;
    bool button_c_up;
    bool button_c_down;
    bool button_c_right;
} PfsManagerFaceButtons;

typedef struct pfs_manager_side_buttons_s {
    bool button_z;
    bool button_l;
    bool button_r;
} PfsManagerSideButtons;

typedef struct pfs_manager_controller_data {
    union { s32 face_button[6]; PfsManagerFaceButtons face_button2; };
    union { s32 side_button[3]; PfsManagerSideButtons side_button2; };
    s32 unk24[4];
    s32 start_button;
} PfsManagerControllerData;

typedef struct {
    s16 unk0;
    s16 unk2;
    u16 unk4;
    u16 unk6;
    f32 unk8[2];
    f32 joystick[2];
}Struct_core1_10A00_1;

f32 pfsManager_calculateStickValue(s32 arg0, s32 arg1, s32 arg2);
void controller_copyFaceButtons(s32 controller_index, s32 dst[6]);
void pfsManager_getFirstControllerFaceButtonState(s32 controller_index, s32 dst[6]);
s32 pfsManager_getButtonPressCount(s32 arg0, s32 arg1);
s32 pfsManager_getSideButtonState(s32 controller_index, s32 dst[3]);
s32 pfsManager_getFirstControllerSideButtonState(s32 controller_index, s32 dst[3]);
f32 pfsManager_getJoystickIdleTime(s32 controller_index);
s32 controller_getStartButton(s32 controller_index);
s32 pfsManager_getStartButtonState(s32 controller_index);
void pfsManager_getUnknownButtonState(s32 controller_index, s32 dst[4]);
void controller_getJoystick(s32 controller_index, f32 dst[2]);
void pfsManager_update(void);
void pfsManager_readData();
void pfsManager_entry(void *arg);
void pfsManager_init(void);
bool pfsManager_contErr(void);
void pfsManager_checkControllerError(void);
void pfsManager_handleControllerError(void);
void pfsManager_getStartReadData(void);
void pfsManager_receiveMesg(void);
void pfsManager_resetControllerData(void);
void pfsManager_copyControllerData(s32 arg0, Struct_core1_10A00_1 *arg1);
void pfsManager_setZButtonState(s32 controller_index, s32 arg1);
OSMesgQueue * pfsManager_getFrameReplyQ(void);
OSMesgQueue *pfsManager_getFrameMesgQ(void);
void pfsManager_setBusyState(s32 arg0);
bool pfsManager_isBusy(void);
int pfsManager_getControllerInputSum(int arg0);
OSContPad *pfsManager_getControllerData(void);
void pfsManager_initMesgQueue(void);
void pfsManager_waitForMesg(void);
void pfsManager_sendMesg(void);

#ifdef __cplusplus
}
#endif

#endif
