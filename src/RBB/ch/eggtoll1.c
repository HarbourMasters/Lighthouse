// BanjoDecomp: eggtoll1.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

extern void timed_exitStaticCamera(f32);

/* typedefs and declarations */
#include "eggtoll1.h" // [port] the typedefs and states moved there


void chRBBEggToll_setState(Actor *this, s32 next_state);
void chRBBEggToll_update(Actor* this);

/* .data */
ActorInfo chRBBEggToll = {
    MARKER_182_RBB_EGG_TOLL, ACTOR_172_RBB_EGG_TOLL, ASSET_402_MODEL_EGG_TOLL,
    0x0, NULL,
    chRBBEggToll_update, NULL, func_80325340,
    0, 0, 0.0f, 0
};
chRBBEggTollInfo chRBBEggTollInfoTable[4] = {
    {0x0, 0x2, 0x4, 3.0f,   550, 200, -300},
    {0x1, 0x4, 0x0, 4.0f,  -600,   0,    0},
    {0x2, 0x6, 0x0, 4.0f,  -700,   0,    0},
    {0x3, 0x8, 0x0, 4.0f, -1010,   0,    0}
};

/* .code */
void RBB_func_803863F0(Actor *actor, s32 arg1){
    ActorLocal_ChRBBEggToll * local = (ActorLocal_ChRBBEggToll *) &actor->local;
    void *temp_a0;
    f32 sp3C[3];
    f32 sp30[3];

    temp_a0 = func_8034C528(local->tollId + 0x190);
    if(temp_a0){
        if(local->tollNum == 0){
            sp3C[2] = 0.0f;
            sp30[0] = 0.0f;
            sp30[1] = 0.0f;
            sp3C[0] = sp3C[1] = sp3C[2];
            sp30[2] = (f32) chRBBEggTollInfoTable[local->tollId].unk8;
        }
        else if(local->tollNum == 1){
            sp3C[0] = 0.0f;
            sp3C[1] = 0.0f;
            sp3C[2] = (f32) chRBBEggTollInfoTable[local->tollId].unk8;
            sp30[1] = 0.0f;
            sp30[0] = 0.0f;
            sp30[2] = (f32) chRBBEggTollInfoTable[local->tollId].unkA;
        }
        else{
            sp3C[0] = 0.0f;
            sp3C[1] = 0.0f;
            sp3C[2] = (f32) chRBBEggTollInfoTable[local->tollId].unkA;
            sp30[1] = 0.0f;
            sp30[0] = 0.0f;
            sp30[2] = (f32) chRBBEggTollInfoTable[local->tollId].unkC;

        }
        if(arg1){
            func_8034DDF0(temp_a0, sp3C, sp30, chRBBEggTollInfoTable[local->tollId].unk4, 1);
//          func_8034E174(temp_a0);
            if (EventSystem_Should(VB_PLAY_PUZZLE_JINGLE, true, actor)) func_8034E174(temp_a0);
        }
        else{
            func_8034DDF0(temp_a0, sp3C, sp30, 0.0f, 1);
        }
    }
}

void func_803865A4(ActorMarker *marker, s32 arg1){
    chRBBEggToll_setState(marker_getActor(marker), arg1);
}

void func_803865D0(ActorMarker *marker){
    Actor* actor = marker_getActor(marker);
    ActorLocal_ChRBBEggToll * local = (ActorLocal_ChRBBEggToll *) &actor->local;
    RBB_func_803863F0(actor, 1);
//  timed_setStaticCameraToNode(0.0f, chRBBEggTollInfoTable[local->tollId].cameraNode);
    if (EventSystem_Should(VB_PLAY_PUZZLE_CUTSCENE, true, actor)) timed_setStaticCameraToNode(0.0f, chRBBEggTollInfoTable[local->tollId].cameraNode);
    timedFunc_set_2(0.75 * chRBBEggTollInfoTable[local->tollId].unk4, (GenFunction_2) func_803865A4, (uintptr_t)actor->marker, 4);
//  timed_exitStaticCamera(0.75 * chRBBEggTollInfoTable[local->tollId].unk4);
//  func_80324E38(0.75 * chRBBEggTollInfoTable[local->tollId].unk4, 0);
    if (EventSystem_Should(VB_PLAY_PUZZLE_CUTSCENE, true, actor)) {
        timed_exitStaticCamera(0.75 * chRBBEggTollInfoTable[local->tollId].unk4);
        func_80324E38(0.75 * chRBBEggTollInfoTable[local->tollId].unk4, 0);
    }
//  timed_playSfx(chRBBEggTollInfoTable[local->tollId].unk4, SFX_7F_HEAVYDOOR_SLAM, 0.8f, 0x7FD0);
    if (EventSystem_Should(VB_PLAY_PUZZLE_JINGLE, true, actor)) timed_playSfx(chRBBEggTollInfoTable[local->tollId].unk4, SFX_7F_HEAVYDOOR_SLAM, 0.8f, 0x7FD0);
}

void chRBBEggToll_setState(Actor *this, s32 next_state){
    ActorLocal_ChRBBEggToll *local = (ActorLocal_ChRBBEggToll *) &this->local;
    

    if(next_state == CH_RBB_EGG_TOLL_STATE_2_FIRST_EGG_PAID){
        coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 28000);
        local = (ActorLocal_ChRBBEggToll *) &this->local;
        local->unkC = 3;
    }
    if(next_state == CH_RBB_EGG_TOLL_STATE_3_TOLL_COMPLETE){
        local->tollNum++;
        CALL_EVENT(OnEggTollPaid, this); // [port]
//      coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 28000);
        if (EventSystem_Should(VB_PLAY_PUZZLE_JINGLE, true, this)) coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 28000);
//      func_80324E38(0.0f, 3);
        if (EventSystem_Should(VB_PLAY_PUZZLE_CUTSCENE, true, this)) func_80324E38(0.0f, 3);
//      timedFunc_set_2(0.5f, (GenFunction_2) coMusicPlayer_playMusic, COMUSIC_2D_PUZZLE_SOLVED_FANFARE, 28000);
        if (EventSystem_Should(VB_PLAY_PUZZLE_JINGLE, true, this)) timedFunc_set_2(0.5f, (GenFunction_2) coMusicPlayer_playMusic, COMUSIC_2D_PUZZLE_SOLVED_FANFARE, 28000);
        timedFunc_set_1(1.0f,  (GenFunction_1) func_803865D0, (uintptr_t)this->marker);
    }//L803867D4

    if(next_state == CH_RBB_EGG_TOLL_STATE_4_UNK){
        if(local->tollNum == 1 && chRBBEggTollInfoTable[local->tollId].eggCost2 > 0){
            chRBBEggToll_setState(this, CH_RBB_EGG_TOLL_STATE_1_AWAITING_TOLL);
            return;
        }
    }

    this->state = next_state;
}

void func_8038685C(ActorMarker *marker){
    Actor *actor = marker_getActor(marker);
    ActorLocal_ChRBBEggToll *local = (ActorLocal_ChRBBEggToll *) &actor->local;

    if(actor->state == CH_RBB_EGG_TOLL_STATE_1_AWAITING_TOLL){
        local->eggsPaid++;
        if( chRBBEggTollInfoTable[local->tollId].eggCost1 == local->eggsPaid 
            || chRBBEggTollInfoTable[local->tollId].eggCost1 + chRBBEggTollInfoTable[local->tollId].eggCost2 == local->eggsPaid
        ){
            chRBBEggToll_setState(actor, CH_RBB_EGG_TOLL_STATE_3_TOLL_COMPLETE);
        }else{
            chRBBEggToll_setState(actor, CH_RBB_EGG_TOLL_STATE_2_FIRST_EGG_PAID);
        }
    }
}

void chRBBEggToll_update(Actor *this){
    ActorLocal_ChRBBEggToll *local = (ActorLocal_ChRBBEggToll *) &this->local;

    if(!this->volatile_initialized){
        this->volatile_initialized = true;
        if(this->state == CH_RBB_EGG_TOLL_STATE_0_NOT_INIT){
            local->eggsPaid = 0;
            local->tollId = 0;
            local->tollNum = 0;
            local->unkC = 0;
            local->tollId = (this->secondaryId == 0x15)? 0: local->tollId;
            local->tollId = (this->secondaryId == 0x13)? 1: local->tollId;
            local->tollId = (this->secondaryId == 0x14)? 2: local->tollId;
            local->tollId = (this->secondaryId == 0x0B)? 3: local->tollId;
            chRBBEggToll_setState(this, CH_RBB_EGG_TOLL_STATE_1_AWAITING_TOLL);
            CALL_EVENT(OnEggTollSetUp, this); // [port]
        }
        RBB_func_803863F0(this, 0);
    }//L803869F4
    if(this->state == CH_RBB_EGG_TOLL_STATE_2_FIRST_EGG_PAID){
        if(--local->unkC <= 0){
            chRBBEggToll_setState(this, CH_RBB_EGG_TOLL_STATE_1_AWAITING_TOLL);
        }

    }
}
