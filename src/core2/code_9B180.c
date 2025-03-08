#include <ultra64.h>
#include "functions.h"
#include "variables.h"

typedef struct {
    u8 pad0[2];
    s16 unk2;
}Struct_core2_9B180_1;

typedef struct struct_core2_9B180_s{
    s16 unk0;
    // u8 pad2[0x2];
    Struct_core2_9B180_1 *unk4;
    void (*unk8)(struct struct_core2_9B180_s *);
    void (*unkC)(struct struct_core2_9B180_s *);
    void (*unk10)(struct struct_core2_9B180_s *);
}Struct_core2_9B180_0;

extern void musicTrack_setTempoTransition(u8, f32, f32);
extern void func_8030EDAC(f32, f32);

void deactivateSoundEvent(Struct_core2_9B180_0 *);
void deactivateSoundEvent2(Struct_core2_9B180_0 *);
void updatePlayerPositionY(Struct_core2_9B180_0 *);
void setAllMusicTrackTempos(Struct_core2_9B180_0 *);
void setMusicTrackTempo(Struct_core2_9B180_0 *);
void adjustSoundMode(Struct_core2_9B180_0 *);
void checkStartButtonAndReset(Struct_core2_9B180_0 *);
void resetSoundEvent(Struct_core2_9B180_0 *);
void checkAndSetPlayerPosition(Struct_core2_9B180_0 *);

/* .data */
Struct_core2_9B180_0 soundEventHandlers[6] = {
    {0x18A, NULL, deactivateSoundEvent, checkStartButtonAndReset, NULL}, 
    {0x018, NULL, deactivateSoundEvent2, resetSoundEvent, NULL}, 
    {0x18B, NULL, updatePlayerPositionY, checkAndSetPlayerPosition, NULL}, 
    {0x194, NULL, setAllMusicTrackTempos,          NULL, NULL}, 
    {0x193, NULL, setMusicTrackTempo,          NULL, NULL}, 
    {0x192, NULL, adjustSoundMode,          NULL, NULL}
};

u8 isSoundEventActive = 1;

/* .bss */
u8 soundModeFlag;
f32 playerPositionY;

/* .code */
void updatePlayerPositionY(Struct_core2_9B180_0 *arg0) {
    playerPositionY = (f32) arg0->unk4->unk2;
}

void checkAndSetPlayerPosition(Struct_core2_9B180_0 *arg0) {
    f32 sp1C[3];

    player_getPosition(sp1C);
    if (sp1C[1] < playerPositionY) {
        if (func_8028F66C(BS_INTR_2C) == 2) {
            arg0->unk4 = 0;
            FUNC_8030E624(SFX_96_HOTSAND_EEL_HISS, 1.0f, 32000);
            FUNC_8030E624(SFX_A_BANJO_LANDING_05, 1.0f, 32000);
            return;
        }
        sp1C[1] = playerPositionY;
        func_8028FAB0(sp1C);
        ncDynamicCamera_setState(6);
    }
}


void setMusicTrackTempo(Struct_core2_9B180_0 * arg0){
    musicTrack_setTempoTransition(0, 0.2f, 1.25f);
}

void adjustSoundMode(Struct_core2_9B180_0 * arg0){
    func_8030EDAC(0.50999999f, 1.0f);
    setSoundMode(0);
}

void setAllMusicTrackTempos(Struct_core2_9B180_0 * arg0) {
    s32 phi_s0;

    for(phi_s0 = 1; phi_s0 < 5; phi_s0++){
        musicTrack_setTempoTransition(phi_s0, 0.2f, 1.25f);
    }
}

void deactivateSoundEvent(Struct_core2_9B180_0 *arg0){
    if(isSoundEventActive){
        isSoundEventActive = 0;
        arg0 ->unk4 = 0;
    }
}

void checkStartButtonAndReset(Struct_core2_9B180_0 * ag0){
    if(pfsManager_getStartButtonState(0) == 1){
        func_8031D06C(0,0);
    }
}

void deactivateSoundEvent2(Struct_core2_9B180_0 *arg0){
    if(isSoundEventActive){
        isSoundEventActive = 0;
        arg0 ->unk4 = 0;
    }
}

void resetSoundEvent(Struct_core2_9B180_0 * arg0){
    func_8031D0C0(0, 0);
}

bool isActorPositionValid(s32 arg0) {
    f32 sp1C[3];

    if (nodeProp_findPositionFromActorId(arg0, &sp1C)) {
        return 1;
    }
    return 0;
}

void updateSoundEventState(s32 arg0, s32 arg1, s32 *arg2) {
    if (isActorPositionValid(arg0) && (*arg2 == 0)) {
        *arg2 = arg1;
    }
}

void initializeSoundEvents(void) {
    Struct_core2_9B180_0 *i_ptr;
    s32 sp28;

    setSoundMode(1);
    for(i_ptr = soundEventHandlers; i_ptr != &soundEventHandlers[6]; i_ptr++){
        i_ptr->unk4 = cubeList_findNodePropByActorIdAndPosition_s32(i_ptr->unk0, NULL);
        if(i_ptr->unk4 != 0 && i_ptr->unk8 != NULL){
            i_ptr->unk8(i_ptr);
        }
    }

    sp28 = 0;
    updateSoundEventState(0x19, 1, &sp28);
    updateSoundEventState(0x1A, 2, &sp28);
    updateSoundEventState(0x1B, 3, &sp28);
    updateSoundEventState(0x1C, 4, &sp28);
    updateSoundEventState(0x1D, 5, &sp28);
    sp28 = (sp28 == 0) ?  2 : sp28;
    viMgr_setFrameLimit(sp28);
}

void processSoundEvents(void) {
    Struct_core2_9B180_0 *i_ptr;

    if (soundModeFlag != 0) {
        for(i_ptr = soundEventHandlers; i_ptr != &soundEventHandlers[6]; i_ptr++){
            if(i_ptr->unk4 != 0 && i_ptr->unkC != NULL){
                i_ptr->unkC(i_ptr);
            }
        }
    }
}


void resetAllSoundEvents(void) {
    s32 i;
    Struct_core2_9B180_0 *i_ptr;

    for(i_ptr = soundEventHandlers; i_ptr != &soundEventHandlers[6]; i_ptr++){
        if(i_ptr->unk4 != 0 && i_ptr->unk10 != NULL){
            i_ptr->unk10(i_ptr);
        }
    }
    func_8030EDAC(0.0f, 1.0f);
    setSoundMode(1);
    for(i = 0; i < 5; i++){
        musicTrack_setTempoTransition(i, 0.0f, 1.0f);
    }
}

void setSoundModeFlag(s32 arg0, s32 arg1) {
    soundModeFlag = BOOL(arg1 == 2);
}
