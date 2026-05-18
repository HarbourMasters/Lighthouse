#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "enums.h"

extern void func_802BE720(void);
extern f32 modelRender_func_8033A244(f32);
extern void modelRender_func_8033A280(f32);

typedef struct chjiggy_s{
    u32 unk0;
    u32 index;
} ActorLocal_Jiggy;

Actor *spawnQueue_actor_s32(f32, f32, f32);
void spawnJiggyDestroyedEffects(u32 x, u32 y, u32 z, u32 arg3);
Actor *chjiggy_draw(ActorMarker *this, Gfx **gdl, Mtx **mptr, Vtx **arg3);
void chjiggy_update_2(Actor * arg0);
void chjiggy_update(Actor *this);
enum jiggy_e chjiggy_getJiggyId(Actor *this);



/* .data */
ActorAnimationInfo JIGGY_ANIMATIONS[] = {
    {0, 0.0f},
    {0, 0.0f},
    {0, 0.0f}
};

ActorInfo chJiggy = {
    MARKER_52_JIGGY, ACTOR_46_JIGGY, ASSET_35F_MODEL_JIGGY, 
    1, JIGGY_ANIMATIONS, 
    chjiggy_update, chjiggy_update_2, chjiggy_draw,
    0, 0, 0.9f, 0
}; 

/* .code */
enum jiggy_e getJiggyId(Actor *this){
    s32 id;
    s32 sp18[3];
    
    id = gsworld_getMap();

    sp18[0] = (s32)this->position[0];
    sp18[1] = (s32)this->position[1]; 
    sp18[2] = (s32)this->position[2]; 
    id = func_80307164(sp18);
    if( id < 0){
        return 0;
    }
    else{
        return func_80306DBC(id) + 1;
    }
}

void playJiggyDestroyedSoundsAndReset(ActorMarker * arg0, u32 arg1){
    func_8030E6D4(SFX_30_MAGIC_POOF);
    coMusicPlayer_playMusic(COMUSIC_3C_MINIGAME_LOSS, 0x7FF8);
    mapSpecificFlags_set(arg1, 1);
    marker_despawn(arg0);
}

void spawnJiggyDestroyedEffects(u32 x, u32 y, u32 z, u32 arg3){
    __spawnQueue_add_4((GenFunction_4)spawnQueue_actor_s32, ACTOR_4C_STEAM, x, y, z);
    __spawnQueue_add_4((GenFunction_4)spawnQueue_actor_s32, ACTOR_14F_DESTROYED_JIGGY, x, y, z);
    mapSpecificFlags_set(arg3, 1);
}

void resetFlag(u32 arg0){
    mapSpecificFlags_set(arg0, 0);
}

void destroyJiggy(Actor *this, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5, enum volatile_flags_e arg6){
    if( !mapSpecificFlags_get(arg1)
        && mapSpecificFlags_get(arg2)
        && item_getCount(ITEM_0_HOURGLASS_TIMER) == 0
    ){
        player_setModelVisible(1);
        actor_collisionOff(this);
        gcStaticCamera_activate(arg3);
        volatileFlag_setAndTriggerDialog_0(arg6);
        timedFunc_set_4(0.6f, (GenFunction_4)spawnJiggyDestroyedEffects, (s32)this->position[0], (s32)this->position[1], (s32)this->position[2], arg4);
        timedFunc_set_2(0.6f, (GenFunction_2)playJiggyDestroyedSoundsAndReset, (uintptr_t)this->marker, arg5);
        timedFunc_set_0(1.0f, (GenFunction_0)func_802BE720);
        timedFunc_set_1(3.9f, (GenFunction_1)resetFlag, arg4);
        mapSpecificFlags_set(arg1, 1);
    }
}

void chjiggy_updateRotation(Actor *this){
    f32 delta = time_getDelta();
    this->yaw += delta * 230.0f;
    if(360.0f <= this->yaw){
        this->yaw -= 360.0f;
    }
    this->yaw_ideal = this->yaw;
}

//chjiggy_draw
Actor *chjiggy_draw(ActorMarker *this, Gfx **gdl, Mtx **mptr, Vtx **arg3){
    Actor * thisActor = marker_getActor(this);
    ActorLocal_Jiggy *local = (ActorLocal_Jiggy *)&thisActor->local;
    u32 jiggyId;

    if(!local->unk0){
        jiggyId = chjiggy_getJiggyId(thisActor);
        if((jiggyId == JIGGY_1C_CC_RINGS) || (jiggyId == JIGGY_1D_CC_SLOW_SAWBLADES)){
            modelRender_func_8033A280(10.0f);
            modelRender_func_8033A244(30000.0f);
        }
        thisActor = actor_draw(this, gdl, mptr, arg3);
    }
    return thisActor;
}

void chjiggy_update_2(Actor * arg0){
    func_80343DEC(arg0);
    chjiggy_updateRotation(arg0);
}

//chjiggy_update
void chjiggy_update(Actor *this){
    ActorLocal_Jiggy *local = (ActorLocal_Jiggy *)&this->local;
    int i;

    if(this->marker->unk14_21){
        for(i = 0; i < 4; i++){
            if(randf() < 0.015){
                commonParticle_add(this->marker, i + 5, func_80329904);
                commonParticle_new(8, 1);
            }
        }
    }//L802C7E44
    switch(this->state){
        case 1: //L802C7E68
            local->unk0 = 0;
            if(local->index == 0)
                local->index = getJiggyId(this);
            
            if(jiggyscore_isCollected(local->index)){
                marker_despawn(this->marker);
            }
            else{
                subaddie_set_state(this, 2);
                switch(chjiggy_getJiggyId(this)){
                    case JIGGY_17_CC_CLANKER_RAISED: //L802C7EF8
                    case JIGGY_49_CCW_EYRIE:// L802C7EF8
                        this->marker->unk40_21 = 1;
                        break;
                    case JIGGY_36_LAIR_TTC_WITCH_SWITCH:// L802C7F0C
                        this->unk44_14 = func_80341F2C(0x20A);
                        this->unk48 = 0.0f;
                        this->unk4C = 300.0f;
                        this->marker->unk2C_2 = 1;
                        this->unk54 = 0.0f;
                        func_80343DEC(this);
                        chjiggy_updateRotation(this);
                        break;
                    case JIGGY_3E_GV_GRABBA:// L802C7F6C
                    case JIGGY_4D_CCW_FLOWER:// L802C7F6C
                        this->unk124_6 = 0;
                        break;
                    case JIGGY_41_GV_MAZE:// L802C7F7C
                        this->marker->unk14_10 = 30;
                        break;
                    case JIGGY_13_TTC_LOCKUP: //L802C7F94
                        this->marker->unk14_10 = 40;
                        break;
                }
            }
            break;
        case 2: //L802C7FAC
            chjiggy_updateRotation(this);
            switch(chjiggy_getJiggyId(this)){
                case JIGGY_20_BGS_ELEVATED_WALKWAY: //L802C7FE8
                    destroyJiggy(this, 4, 3, 0xD, 5, 2, VOLATILE_FLAG_AE_BGS_WALKWAY_JIGGY_MISSED);
                    break;
                case JIGGY_25_BGS_MAZE://L802C8018
                    destroyJiggy(this, 0xd, 0xc, 0x1e, 9, 0xb, VOLATILE_FLAG_AF_BGS_MAZE_JIGGY_MISSED);
                    break;
                case JIGGY_2F_FP_XMAS_TREE://L802C8048
                    if (levelSpecificFlags_get(LEVEL_FLAG_29_FP_XMAS_TREE_COMPLETE)) {
                        actor_collisionOn(this);
                    }
                    else {
                        actor_collisionOff(this);
                    }
                    break;
            }
            break;
    }//L802C8074
}

enum jiggy_e chjiggy_getJiggyId(Actor *this){
    ActorLocal_Jiggy *local = (ActorLocal_Jiggy *)&this->local;
    return local->index;
}

void chjiggy_hide(Actor * this){
    ActorLocal_Jiggy *local = (ActorLocal_Jiggy *)&this->local;
    local->unk0 = 1;
    actor_collisionOff(this);
}

void chjiggy_setJiggyId(Actor *this, u32 id){
    ActorLocal_Jiggy *local = (ActorLocal_Jiggy *)&this->local;
    local->index = id;
}
