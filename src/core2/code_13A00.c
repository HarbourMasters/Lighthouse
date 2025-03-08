#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "core2/ba/physics.h"

extern void enableAndPlaySoundEffectWithVolume(void *, f32);
extern void setSoundEffectParameters(void *, s32, f32);
extern f32  func_80294500(void);


void func_8029AE74(s32 arg0);
void func_8029AEE4(s32 arg0);

/* .bss */
Struct5Ds *D_8037D190;
u8 D_8037D194;

/* .code */
void func_8029A990(void){
    f32 plyr_pos[3];
    ParticleEmitter *pCtrl;

    if(map_get() == MAP_2C_MMM_BATHROOM)
        func_8029C304(0);
    else
        func_8029C304(1);

    _player_getPosition(plyr_pos);
    plyr_pos[1] = func_80294500();
    pCtrl = createBubbleParticleEmitter(plyr_pos, 25.0f);
    particleEmitter_setParticleVelocityRange(pCtrl,
        -350.0f, 300.0f, -350.0f,
        350.0f, 500.0f, 350.0f
    );
    particleEmitter_emitN(pCtrl, 10);
}

void func_8029AA3C(void){
    f32 sp34[3];
    f32 sp30;
    ParticleEmitter *pCtrl;
    if(map_get() == MAP_2C_MMM_BATHROOM)
        func_8029C304(0);
    else
        func_8029C304(2);
    
    sp30 = ml_map_f(baphysics_get_horizontal_velocity(), 0.0f, 500.0f, 70.0f, 250.0f);
    func_8028E9C4(D_8037D194, sp34);
    sp34[1] = func_80294500();
    pCtrl = createBubbleParticleEmitter(sp34, 8.0f);
    particleEmitter_setParticleVelocityRange(pCtrl,
        -sp30, 300.0f, -sp30,
        sp30, 350.0f,sp30
    );
    particleEmitter_emitN(pCtrl, 5);
}

void func_8029AB14(s32 arg0, s32 arg1){
    int sp24 = (arg1 == 4) || (arg1 == 0xb);
    int sp20 = player_inWater();
    if(sp24){
        func_8029AA3C();
    }

    switch(player_getTransformation()){
        case TRANSFORM_3_PUMPKIN: //L8029AB9C
            if(arg1 == 7){
                playSoundEffectById(arg0, arg1);
            }
            if(sp24 || sp20){
                playSoundEffectById(arg0, 0xe);
            }
            else{
                playSoundEffectById(arg0, 0xd);
            }
            break;
        case TRANSFORM_4_WALRUS://L8029ABEC
            if(sp24){
                playSoundEffectById(arg0, 0x4);
            }
            else if(sp20){
                playSoundEffectById(arg0, 0xf);
            }
            else{
                playSoundEffectById(arg0, 0xd);
            }
            break;
        default: //L8029AC28
            break;
    }
}

void func_8029AC34(s32 arg0, s32 arg1){
    int sp1C;
    sp1C = (arg1 == 4) || (arg1 == 0xb || player_inWater());
    if(sp1C){
        func_8029A990();
    }

    switch(player_getTransformation()){
        case TRANSFORM_3_PUMPKIN:
        case TRANSFORM_4_WALRUS:
        case TRANSFORM_5_CROC:
            if(sp1C)
                FUNC_8030E624(SFX_10_BANJO_LANDING_07, 0.9f, 13000);
            else
                FUNC_8030E624(SFX_2F_ORANGE_SPLAT, 1.4f, 8000);
            break;
        default:
            break;
    }
}

void func_8029ACD4(void){
    f32 plyr_pos[3];

    _player_getPosition(plyr_pos);
    setSoundEffectPosition(D_8037D190, plyr_pos);
    setSoundEffectParameters( D_8037D190, func_80294660(), func_80294500());
}

void func_8029AD28(f32 arg0, s32 arg1){
    AnimCtrl *plyr_animCtrl = baanim_getAnimCtrlPtr();
    if(anctrl_isAt(plyr_animCtrl, arg0)){
        func_8029AE74(arg1);
    }
}


void func_8029AD68(f32 arg0, s32 arg1){
    AnimCtrl *plyr_animCtrl = baanim_getAnimCtrlPtr();
    if(anctrl_isAt(plyr_animCtrl, arg0)){
        func_8029AEE4(arg1);
    }
}

void func_8029ADA8(void){
    freeSoundEffect(D_8037D190);
}

void func_8029ADCC(void){
    D_8037D190 = createSoundEffect();
    setSoundEffectCallback(D_8037D190, func_8029AC34);
    setSoundEffectCallback2(D_8037D190, func_8029AB14);
}

void func_8029AE1C(void){
    func_8029ACD4();
    enableSoundEffect(D_8037D190);
}

void func_8029AE48(void){
    func_8029ACD4();
    disableSoundEffect(D_8037D190);
}

void func_8029AE74(s32 arg0){
    enum bsgroup_e player_movement_group;
    func_8029ACD4();
    D_8037D194 = arg0;
    player_movement_group = player_movementGroup();
    if(player_movement_group == BSGROUP_6_TURBO_TALON_TRAINERS || player_movement_group == BSGROUP_8_TROT || player_movement_group == BSGROUP_9_LONG_LEG){
        enableAndPlaySoundEffectWithVolume(D_8037D190, 0.2f);
    }
    else{
        enableAndPlaySoundEffect(D_8037D190);
    }
}

void func_8029AEE4(s32 arg0){
    func_8029ACD4();
    D_8037D194 = arg0;
    disableAndPlaySoundEffect(D_8037D190);
}

void func_8029AF1C(void){
    D_8037D190 = defragSoundEffect(D_8037D190);
}
