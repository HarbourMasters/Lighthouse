// BanjoDecomp: code_640.c
#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include "prop.h"
#include "actor.h"

#include "port/Patches/Patches.h"


extern ActorInfo chBellBuoy;
extern ActorInfo chRBBEggToll;
extern ActorInfo chCageUpSwitch;
extern ActorInfo chTNTDownSwitch;
extern ActorInfo chRowboat;
extern ActorInfo chRBBRearPropeller;
extern ActorInfo chRBBGreenPropellerSwitch;
extern ActorInfo chRBBSpinningSinglePipe1;
extern ActorInfo chRBBSpinningSinglePipe2;
extern ActorInfo chRBBSpinningDoublePipe1;
extern ActorInfo chRBBSpinningDoublePipe2;
extern ActorInfo chSpinningFlatPlatform1;
extern ActorInfo chSpinningFlatPlatform2;
extern ActorInfo chSpinningFlatPlatform3;
extern ActorInfo chRBBSmallCog;
extern ActorInfo chRBBMediumCog;
extern ActorInfo chRBBLargeCog;
extern ActorInfo chEngineRoomPropeller1;
extern ActorInfo chEngineRoomPropeller2;
extern ActorInfo chEngineRoomPropeller3;
extern ActorInfo chRBBGreyPropellerSwitch;
extern ActorInfo chRBBWhistleSwitch1;
extern ActorInfo chRBBWhistleSwitch2;
extern ActorInfo chRBBWhistleSwitch3;
extern ActorInfo chRBBWhistle1;
extern ActorInfo chRBBWhistle2;
extern ActorInfo chRBBWhistle3;
extern ActorInfo chRBBWhistleCtrl;
extern ActorInfo chGrimlet;
extern ActorInfo chAnchorSwitch; //anchorswitch
extern ActorInfo chSnorkel; //dolphin
extern ActorInfo chAnchor; //anchor
extern ActorInfo chAnchorCtrl;
extern ActorInfo chRarewareFlag; //rarewareflag
extern ActorInfo chBossBoomBoxLargest;
extern ActorInfo chBossBoomBoxLarge;
extern ActorInfo chBossBoomBoxMedium;
extern ActorInfo chBossBoomBoxSmall;
extern ActorInfo chBossBoomBoxCtrl;
extern ActorInfo chBoomBoxSlow;
extern ActorInfo chBoomBoxFast; //TNTpart_IDStruct;
extern ActorInfo chBoomBoxMinigameCtrl;
extern ActorInfo chRBBSpinningSinglePipeSfxCtrl;
extern ActorInfo chRBBSpinningDoublePipeSfxCtrl;
extern ActorInfo D_803900E0;
extern ActorInfo D_80390104; //captcabinwooddoor
extern ActorInfo D_80390128;
extern ActorInfo D_8039014C;
extern ActorInfo D_803901B8;
extern ActorInfo D_803901DC;
extern ActorInfo D_80390170;//skylight
extern ActorInfo D_80390194;//honeycombswitch



extern void func_802D3D54(Actor *);

void func_80386A7C(Actor *);
void func_80386BF8(Actor *arg0);
Actor *func_80386B9C(ActorMarker *marker, Gfx **gdl, Mtx **mptr, Vtx **arg3);


/* .data */
ActorAnimationInfo D_803900C0[4] = {
    {0, 0.0f},
    {0, 0.0f},
    {0xD4, 0.15f},
    {0xD5, 0.5f}
};

ActorInfo D_803900E0 = {
    0x107, 0x21D, 0x493, 0x1, NULL,
    func_802D3D54, actor_update_func_80326224, actor_drawFullDepth,
    0, 0, 0.0f, 0
};

ActorInfo D_80390104 = {
    0x108, 0x21C, 0x492, 0x1, NULL,
    func_802D3D54, actor_update_func_80326224, actor_drawFullDepth,
    0, 0, 0.0f, 0
};

ActorInfo D_80390128 = {
    0x22D, 0x266, 0x4BA, 0x1, NULL,
    func_802D3D54, actor_update_func_80326224, actor_drawFullDepth,
    0, 0, 0.0f, 0
};

ActorInfo D_8039014C = {
    0x22E, 0x267, 0x4BB, 0x1, NULL,
    func_802D3D54, actor_update_func_80326224, actor_drawFullDepth,
    0, 0, 0.0f, 0
};

ActorInfo D_80390170 = {
    0x235, 0x23F, 0x4E2, 0x1, NULL,
    func_802D3D54, actor_update_func_80326224, actor_drawFullDepth,
    0, 0, 0.0f, 0
};

ActorInfo D_80390194 = {
    0x15F, 0x18F, 0x42F, 0x1, D_803900C0,
    func_80386BF8, actor_update_func_80326224, actor_draw,
    0, 0, 0.0f, 0
};

ActorInfo D_803901B8 = {
    0x22F, 0x263, 0x4DB, 0x1, NULL,
    func_80386A7C, actor_update_func_80326224, func_80386B9C,
    0, 0, 0.0f, 0
};

ActorInfo D_803901DC = {
    0x230, 0x264, 0x4DE, 0x1, NULL,
    func_80386A7C, actor_update_func_80326224, func_80386B9C,
    0, 0, 0.0f, 0
};


/* .code */
s32 func_80386A30(f32 arg0[3]){
    s32 r; 
    
    r = (arg0[0] < 0.0f)? 0 : 1;

    return ((arg0[2] < 0.0f)? 0 : 2) + r;
}

void func_80386A7C(Actor *this){
    f32 viewport[3];
    s32 sp28;
    s32 temp_v0;

    func_802D3D74(this);
    this->depth_mode = 1;
    viewport_getPosition_vec3f(viewport);
    sp28 = func_80386A30(this->position);
    temp_v0 = func_80386A30(viewport);
    this->unk38_0 = 0;
    // [port] Extend the bespoke distance cull with draw distance; quadrant occlusion below stays.
    f32 cull = 8000.0f * port_drawDistanceMul();
    if(viewport[0] + cull < this->position_x || this->position_x < viewport[0] - cull)
        return;
        
    if( !(  (sp28 ^ temp_v0) & 2
            && (-5000.0f < viewport[0] && viewport[0] < 6000.0f)
            && (viewport[2] < -600.0f || 600.0f < viewport[2])
        )
        && ( sp28 ^ temp_v0) != 3
    ){
            this->unk38_0 = 1;
    }
}

Actor *func_80386B9C(ActorMarker *marker, Gfx **gdl, Mtx **mptr, Vtx **arg3){
    Actor *actor = marker_getActor(marker);

    return (actor->unk38_0) ? actor_drawFullDepth(marker, gdl, mptr, arg3): NULL;
}

void func_80386BF8(Actor *arg0){
    if(!mapSpecificFlags_get(RBB_BOAT_ROOM_SPECIFIC_FLAG_0_HONEYCOMB_SPAWNED) && honeycombscore_get(HONEYCOMB_F_RBB_BOAT_HOUSE))
        mapSpecificFlags_set(RBB_BOAT_ROOM_SPECIFIC_FLAG_0_HONEYCOMB_SPAWNED, true);
    func_802D4A9C(arg0, 0);
}

void RBB_func_80386C48(void){
    spawnableActorList_add(&chBellBuoy, actor_new, ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_14);
    spawnableActorList_add(&chRBBEggToll, actor_new, ACTOR_FLAG_NONE);
    spawnableActorList_add(&chCageUpSwitch, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chTNTDownSwitch, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chRowboat, actor_new, ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_14 | ACTOR_FLAG_UNKNOWN_27);
    spawnableActorList_add(&chRBBRearPropeller, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chRBBGreenPropellerSwitch, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chRBBSpinningSinglePipe1, actor_new, ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_13 | ACTOR_FLAG_UNKNOWN_14);
    spawnableActorList_add(&chRBBSpinningSinglePipe2, actor_new, ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_13 | ACTOR_FLAG_UNKNOWN_14);
    spawnableActorList_add(&chRBBSpinningDoublePipe1, actor_new, ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_13 | ACTOR_FLAG_UNKNOWN_14);
    spawnableActorList_add(&chRBBSpinningDoublePipe2, actor_new, ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_13 | ACTOR_FLAG_UNKNOWN_14);
    spawnableActorList_add(&chSpinningFlatPlatform1, actor_new, ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_13 | ACTOR_FLAG_UNKNOWN_14);
    spawnableActorList_add(&chSpinningFlatPlatform2, actor_new, ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_13 | ACTOR_FLAG_UNKNOWN_14);
    spawnableActorList_add(&chSpinningFlatPlatform3, actor_new, ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_13 | ACTOR_FLAG_UNKNOWN_14);
    spawnableActorList_add(&chRBBSmallCog, actor_new, ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_13 | ACTOR_FLAG_UNKNOWN_14);
    spawnableActorList_add(&chRBBMediumCog, actor_new, ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_13 | ACTOR_FLAG_UNKNOWN_14);
    spawnableActorList_add(&chRBBLargeCog, actor_new, ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_13 | ACTOR_FLAG_UNKNOWN_14);
    spawnableActorList_add(&chEngineRoomPropeller1, actor_new, ACTOR_FLAG_UNKNOWN_10);
    spawnableActorList_add(&chEngineRoomPropeller2, actor_new, ACTOR_FLAG_UNKNOWN_10);
    spawnableActorList_add(&chEngineRoomPropeller3, actor_new, ACTOR_FLAG_UNKNOWN_10);
    spawnableActorList_add(&chRBBGreyPropellerSwitch, actor_new, ACTOR_FLAG_NONE);
    spawnableActorList_add(&chRBBWhistleSwitch1, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chRBBWhistleSwitch2, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chRBBWhistleSwitch3, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chRBBWhistle1, actor_new, ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_11);
    spawnableActorList_add(&chRBBWhistle2, actor_new, ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_11);
    spawnableActorList_add(&chRBBWhistle3, actor_new, ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_11);
    spawnableActorList_add(&chRBBWhistleCtrl, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chGrimlet, actor_new, ACTOR_FLAG_UNKNOWN_1 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_8 | ACTOR_FLAG_UNKNOWN_11);
    spawnableActorList_add(&chAnchorSwitch, actor_new, ACTOR_FLAG_UNKNOWN_7); //anchorswitch
    spawnableActorList_add(&chSnorkel, actor_new, ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_6 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_11); //dolphin
    spawnableActorList_add(&chAnchor, actor_new, ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_11); //anchor
    spawnableActorList_add(&chAnchorCtrl, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chRarewareFlag, actor_new, ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_11); //rarewareflag
    spawnableActorList_add(&chBossBoomBoxLargest, actor_new, ACTOR_FLAG_UNKNOWN_2 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_11);
    spawnableActorList_add(&chBossBoomBoxLarge, actor_new, ACTOR_FLAG_UNKNOWN_2 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_11);
    spawnableActorList_add(&chBossBoomBoxMedium, actor_new, ACTOR_FLAG_UNKNOWN_2 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_11);
    spawnableActorList_add(&chBossBoomBoxSmall, actor_new, ACTOR_FLAG_UNKNOWN_2 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_5 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_11);
    spawnableActorList_add(&chBossBoomBoxCtrl, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chBoomBoxSlow, actor_new, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_25);
    spawnableActorList_add(&chBoomBoxFast, actor_new, ACTOR_FLAG_UNKNOWN_0 | ACTOR_FLAG_UNKNOWN_3 | ACTOR_FLAG_UNKNOWN_7 | ACTOR_FLAG_UNKNOWN_11 | ACTOR_FLAG_UNKNOWN_25);
    spawnableActorList_add(&chBoomBoxMinigameCtrl, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chRBBSpinningSinglePipeSfxCtrl, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&chRBBSpinningDoublePipeSfxCtrl, actor_new, ACTOR_FLAG_UNKNOWN_7);
    spawnableActorList_add(&D_803900E0, actor_new, ACTOR_FLAG_NONE);
    spawnableActorList_add(&D_80390104, actor_new, ACTOR_FLAG_UNKNOWN_9 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_15); //captcabinwooddoor
    spawnableActorList_add(&D_80390128, actor_new, ACTOR_FLAG_NONE);
    spawnableActorList_add(&D_8039014C, actor_new, ACTOR_FLAG_NONE);
    spawnableActorList_add(&D_803901B8, actor_new, ACTOR_FLAG_UNKNOWN_9 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_15);
    spawnableActorList_add(&D_803901DC, actor_new, ACTOR_FLAG_UNKNOWN_9 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_15);
    spawnableActorList_add(&D_80390170, actor_new, ACTOR_FLAG_UNKNOWN_9 | ACTOR_FLAG_UNKNOWN_10 | ACTOR_FLAG_UNKNOWN_15); //skylight
    spawnableActorList_add(&D_80390194, actor_new, ACTOR_FLAG_UNKNOWN_3); //honeycombswitch
}
