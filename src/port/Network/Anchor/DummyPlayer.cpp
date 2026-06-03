#include "Anchor.h"
#include "port/nametag.h"

extern "C" {
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
}

#include "bk_math.h"
#include "port/patches/Patches.h"

void _dummy_updateModelYaw(void){
    //switch(dummyDirection){
    //case PLAYER_MODEL_DIR_KAZOOIE:
    //    dummyYaw = mlNormalizeAngle(yaw_get() + 180.0f);
    //    break;
    //default:
    //    dummyYaw = yaw_get();
    //    break;
    //case PLAYER_MODEL_DIR_GLOBAL:
    //    break;
    //}
}

DummyPlayer::DummyPlayer() {};

void DummyPlayer::dummy_setTransformation(Transformation transform) {
    dummy_transformation = transform;
}

void DummyPlayer::dummy_getPosition(f32 arg0[3]){
    ml_vec3f_copy(arg0, dummyPosition);
}

//void DummyPlayer::dummy_getPosition(f32* dst){
//    f32 tmp1[3];
//    f32 tmp2[3];
//    dummy_80291A50(5,tmp1);
//    dummy_80291A50(6,tmp2);
//    ml_vec3f_add(dst, tmp1, tmp2);
//    ml_vec3f_scale(dst, 0.5);
//}

void DummyPlayer::dummy_setPoisition(f32 pos[3]) {
    ml_vec3f_copy(dummyPosition, pos);
}

void DummyPlayer::dummy_80291A50(s32 arg0, f32 dst[3]){
    func_8034A174(dummy_D_80363780, arg0, dst);
    if(ml_isZero_vec3f(dst)){
        dummy_getPosition(dst);
    }
}

Actor *dummy_80291AAC(ActorMarker *marker, Gfx **gfx, Mtx **mtx, Vtx **vtx){
    return NULL;
}

static void _dummy_preDraw(int arg0){
    //baMarker_get()->unk14_21 = 1;
}

void DummyPlayer::dummy_func_8029DBF0(void){
    s32 temp_s0;
    switch(dummy_getModelId()){
    case ASSET_34D_MODEL_BANJOKAZOOIE_LOW_POLY: //L8029DC24
    case ASSET_34E_MODEL_BANJOKAZOOIE_HIGH_POLY: //L8029DC24
        temp_s0 = (s32) ml_interpolate_f(dummy_D_8037D23C, 1.0f, 8.0f);
        func_8033A45C(0x1B, temp_s0);
        func_8033A45C(0x1D, temp_s0);
        func_8033A45C(0x1F, temp_s0);
        func_8033A45C(0x21, temp_s0);
        temp_s0 = (s32) ml_interpolate_f(dummy_D_8037D240, 1.0f, 8.0f);
        func_8033A45C(0x1A, temp_s0);
        func_8033A45C(0x1C, temp_s0);
        func_8033A45C(0x1E, temp_s0);
        func_8033A45C(0x20, temp_s0);
        break;

    case ASSET_34F_MODEL_BANJO_TERMITE: //L8029DCCC
    case ASSET_359_MODEL_BANJO_WALRUS: //L8029DCCC
    case ASSET_36F_MODEL_BANJO_PUMPKIN: //L8029DCCC
    case ASSET_374_MODEL_BANJO_CROC: //L8029DCCC
        func_8033A45C(0x1B, (s32) ml_interpolate_f(dummy_D_8037D23C, 1.0f, 6.0f));
        func_8033A45C(0x1A, (s32) ml_interpolate_f(dummy_D_8037D240, 1.0f, 6.0f));
        break;

    case ASSET_356_MODEL_BANJO_WISHYWASHY: //L8029DD2C
        func_8033A45C(1, (s32) ml_interpolate_f(dummy_D_8037D240, 1.0f, 4.0f));
        break;
    }
}

AnimCtrl* DummyPlayer::dummy_getAnimCtrl() {
    return dummyAnimCtrl;
}

void DummyPlayer::dummy_setEyeState(bool squint, bool wink, bool isHat) {
    dummy_D_8037D235 = squint;
    dummy_D_8037D236 = wink;
    dummy_D_8037D230 = 1.0f;
    dummy_D_8037D234 = isHat;
}

void DummyPlayer::func_8029DD6C(void) {
    s32 temp_s0; // [port] must hold values > 1 for geo selector branches

    func_8033A1FC();
    switch (dummy_getModelId()) {
    case ASSET_34D_MODEL_BANJOKAZOOIE_LOW_POLY:
    case ASSET_34E_MODEL_BANJOKAZOOIE_HIGH_POLY:
        func_8033A45C(1, dummy_D_8037D238);
        func_8033A45C(9, dummy_D_8037D238);
        func_8033A45C(0xC, dummy_D_8037D238);
        func_8033A45C(0xF, dummy_D_8037D238);
        func_8033A45C(2, dummy_D_8037D236);
        func_8033A45C(0xA, dummy_D_8037D236);
        func_8033A45C(0xD, dummy_D_8037D236);
        func_8033A45C(0x10, dummy_D_8037D236);
        func_8033A45C(8, dummy_D_8037D235);
        func_8033A45C(0xB, dummy_D_8037D235);
        func_8033A45C(0xE, dummy_D_8037D235);
        func_8033A45C(0x11, dummy_D_8037D235);
        temp_s0 = dummy_D_8037D237 + 1;
        func_8033A45C(0x12, temp_s0);
        func_8033A45C(0x14, temp_s0);
        func_8033A45C(0x16, temp_s0);
        func_8033A45C(0x18, temp_s0);
        func_8033A45C(0x13, temp_s0);
        func_8033A45C(0x15, temp_s0);
        func_8033A45C(0x17, temp_s0);
        func_8033A45C(0x19, temp_s0);
        temp_s0 = dummy_D_8037D239 + 1;
        func_8033A45C(0x22, temp_s0);
        func_8033A45C(0x24, temp_s0);
        func_8033A45C(0x26, temp_s0);
        func_8033A45C(0x28, temp_s0);
        func_8033A45C(0x23, temp_s0);
        func_8033A45C(0x25, temp_s0);
        func_8033A45C(0x27, temp_s0);
        func_8033A45C(0x29, temp_s0);
        break;
    case ASSET_359_MODEL_BANJO_WALRUS:
        func_8033A45C(3, dummy_D_8037D23A);
        break;
    case ASSET_374_MODEL_BANJO_CROC:
        temp_s0 = dummy_D_8037D237 + 1;
        func_8033A45C(4, temp_s0);
        func_8033A45C(5, temp_s0);
        func_8033A45C(6, temp_s0);
        func_8033A45C(7, temp_s0);
        break;
    }
    dummy_func_8029DBF0();
}

void DummyPlayer::dummy_setPitch(f32 pitch) {
    dummyPitch = pitch;
}

void DummyPlayer::dummy_setRoll(f32 roll) {
    dummyRoll = roll;
}

void DummyPlayer::dummy_setYaw(f32 yaw) {
    dummyYaw = yaw;
}

//void DummyPlayer::dummyPlayer_setEnvColor(s32 r, s32 g, s32 b) {
//
//}

void DummyPlayer::Draw(Gfx **gfx, Mtx **mtx, Vtx **vtx){
    f32 rotation[3];
    f32 plyr_pos[3]; //sp44
    f32 sp38[3];
    s32 env_color[3];

    if(!dummyIsVisible)
        return;

    //_dummy_updateModelYaw();
    dummy_getPosition(plyr_pos);
    plyr_pos[1] += 2.0f;
    ml_vec3f_assign(rotation, dummyPitch, dummyYaw, dummyRoll);
    func_8029A47C(env_color);
    ml_vec3f_copy(sp38, dummy_D_8037C100);

    plyr_pos[0] += dummyDisplacement[0];\
        plyr_pos[1] += dummyDisplacement[1];\
        plyr_pos[2] += dummyDisplacement[2];

    sp38[0] += dummyDisplacement[0];\
        sp38[1] += dummyDisplacement[1];\
        sp38[2] += dummyDisplacement[2];

    if(dummyBin){
        anctrl_drawSetup(dummyAnimCtrl, dummyPosition, 1);
        func_8029DD6C();
        modelRender_setEnvColor(env_color[0], env_color[1], env_color[2], dummyEnvAlpha);
        func_8033A280(2.0f);
        //modelRender_preDraw((GenFunction_1)_dummy_preDraw, 0);
        func_8033A450(dummy_D_80363780);
        modelRender_setDepthMode(MODEL_RENDER_DEPTH_FULL);
        if(dummy_D_8037C150.unk0){
            dummy_D_8037C150.unk0 = 0;
            modelRender_draw(gfx, mtx, dummy_D_8037C150.unk4, rotation, dummyScale, sp38, dummyBin);
        }
        else{
            modelRender_draw(gfx, mtx, plyr_pos, rotation, dummyScale, sp38, dummyBin);
        }
    }//L80291CD4

    if(dummyPostDrawMethod){
        dummyPostDrawMethod(gfx, mtx, vtx);
    }
}

s32 DummyPlayer::dummy_func_802985F0(void){
    switch(dummy_transformation)
    {
    case TRANSFORM_2_TERMITE: //80298624
        return ASSET_34F_MODEL_BANJO_TERMITE;
    case TRANSFORM_3_PUMPKIN: //8029862C
        return ASSET_36F_MODEL_BANJO_PUMPKIN;
    case TRANSFORM_5_CROC: //80298634
        return ASSET_374_MODEL_BANJO_CROC;
    case TRANSFORM_4_WALRUS: //8029863C
        return ASSET_359_MODEL_BANJO_WALRUS;
    case TRANSFORM_6_BEE: //80298644
        return ASSET_362_MODEL_BANJO_BEE;
    case TRANSFORM_7_WISHWASHY: //8029864C
        return ASSET_356_MODEL_BANJO_WISHYWASHY;
    case TRANSFORM_1_BANJO: //80298654
    default:
        if (port_shouldForceHighPolyBanjo()) {
            return ASSET_34E_MODEL_BANJOKAZOOIE_HIGH_POLY;
        }
        return ASSET_34D_MODEL_BANJOKAZOOIE_LOW_POLY;
    }
}

void DummyPlayer::dummy_updateModel(void){
    dummy_set(static_cast<AssetID>(dummy_func_802985F0()));
}

void DummyPlayer::dummy_reset(void){
    f32 plyr_pos[3];
    int i;
    for(i = 0; i < 2 ; i++){
        /*dummy_80292048(i, 0.0f, 0.0f, 0.0f);
        dummy_80292078(i, 0.0f);*/
    }
    dummyEnvAlpha = 0xFF;
    dummyBin = NULL;
    dummyId = ASSET_0_NONE;
    dummyPostDrawMethod = NULL;
    dummy_D_80363780 = func_8034A2C8();
    func_8034A130(dummy_D_80363780);
    ml_vec3f_clear(dummy_D_8037C100);
    ml_vec3f_clear(dummy_D_8037C110);
    ml_vec3f_clear(dummyDisplacement);
    dummyYaw = dummyRoll = dummyPitch = 0.0f;
    dummy_D_8037C150.unk0 = 0;
    dummy_setVisible(true);
    dummy_setScale(1.0f);
    dummyDirection = PLAYER_MODEL_DIR_NONE;
    dummy_setDirection(PLAYER_MODEL_DIR_BANJO);
    if(!func_8028ADB4())
        dummy_updateModel();
    dummy_getPosition(plyr_pos);
    __spawnQueue_add_4((GenFunction_4)spawnQueue_actor_f32,
        ACTOR_17_PLAYER_SHADOW, 
        reinterpret_cast(s32, plyr_pos[0]), 
        reinterpret_cast(s32, plyr_pos[1]), 
        reinterpret_cast(s32, plyr_pos[2])
    );
    dummyAnim_init();
    dummyAnim_reset();
}

void DummyPlayer::dummy_free(void) {
    assetcache_release(dummyBin);
    dummyBin = NULL;
    dummyId = ASSET_0_NONE;
    func_8034A2A8(dummy_D_80363780);
    dummy_D_80363780 = NULL;
    dummyAnim_free();
}

void DummyPlayer::dummyAnim_reset() {
    dummy_D_8037D230 = 0;
    dummy_D_8037D234 = 0;
    dummy_D_8037D238 = 0;
    dummy_D_8037D236 = 0;
    dummy_D_8037D235 = 0;
    dummy_D_8037D23C = 0.0f;
    dummy_D_8037D240 = 0.0f;
    dummy_D_8037D237 = 0;
    dummy_D_8037D239 = 0;
    dummy_D_8037D23A = 0;
}

void DummyPlayer::dummy_update(void) {
    f32 pos[3];
    player_getPosition(pos);
    pos[0] += 100;
    dummy_setPoisition(pos);
    dummy_setTransformation((Transformation)player_getTransformation());
    // Mirror model direction and apply the same yaw flip as _baModel_updateModelYaw:
    // PLAYER_MODEL_DIR_KAZOOIE adds 180° so Kazooie faces the direction of travel.
    {
        PlayerModelDirection dir = (PlayerModelDirection)baModel_getDirection();
        dummy_setDirection(dir);
        if (dir == PLAYER_MODEL_DIR_KAZOOIE) {
            dummy_setYaw(mlNormalizeAngle(player_getYaw() + 180.0f));
        } else if (dir != PLAYER_MODEL_DIR_GLOBAL) {
            dummy_setYaw(player_getYaw());
        }
        // PLAYER_MODEL_DIR_GLOBAL: yaw is set externally, don't overwrite it.
    }
    dummy_setRoll(roll_get());
    dummy_setPitch(pitch_get());
    // Mirror anim velocity-scale state from the local player.
    // For a real remote player these come from network packets instead.
    dummyAnimState = baanim_getUpdateType();
    baphysics_get_velocity(dummyVelocity);
    baanim_getVelocityMapRanges(
        &dummyAnimScale.velocity_min, &dummyAnimScale.velocity_max,
        &dummyAnimScale.duration_min, &dummyAnimScale.duration_max);
    dummyAnimScale.duration_scale   = baanim_getDurationScale();
    dummyAnimScale.scalable_duration = baanim_isScalableDuration();
    baanim_getDurationRange(&dummyAnimMinDuration, &dummyAnimMaxDuration);

    // For non-velocity-scaled states, mirror the player's live animation duration
    // directly each frame. This covers moves like feathery flap where the BS state
    // calls anctrl_setDuration on playerAnimCtrl each tick to step through a
    // slowdown table — no anctrl_start fires, so no event captures these changes.
    // Velocity-scaled states (walk/run) skip this: dummyAnim_update derives their
    // duration from velocity instead.
    // For real networking, send anctrl_getDuration(playerAnimCtrl) in the
    // per-tick PlayerUpdate packet and call dummyAnim_setLiveDuration() here.
    if (dummyAnimState != BAANIM_UPDATE_2_SCALE_HORZ &&
        dummyAnimState != BAANIM_UPDATE_3_SCALE_VERT) {
        anctrl_setDuration(dummyAnimCtrl, anctrl_getDuration(baanim_getAnimCtrlPtr()));
    }

    // Mirror animstate so Kazooie geometry selectors match the local player.
    // D_8037D238 is read by func_8033A45C(1/9/C/F, ...) inside func_8029DD6C to
    // show or hide Kazooie's model parts. Without this, Kazooie stays invisible
    // even while her animations play. The other fields control eye/mouth state.
    dummy_D_8037D238 = func_8029DFBC(); // Kazooie visibility (Kazooie popped out)
    dummy_D_8037D235 = func_8029DFA4(); // squint
    dummy_D_8037D236 = func_8029DFB0(); // wink
    dummy_D_8037D237 = func_8029DFE0(); // mouth
    dummy_D_8037D239 = func_8029DFEC(); // mouth 2
    dummy_D_8037D23C = func_8029DFC8(); // eye blend upper
    dummy_D_8037D240 = func_8029DFD4(); // eye blend lower
    dummyAnim_update();
//    f32 sp1C;
//    f32 temp_f0;
//
//    sp1C = dummy_D_8037C110[1] - dummy_D_8037C100[1];
//    temp_f0 = mlAbsF(sp1C);
//    if( temp_f0 < 0.01){
//        dummy_D_8037C100[1] = dummy_D_8037C110[1];
//    }
//    else{
//        if(5.0f < temp_f0){
//            temp_f0 = 1.0f;
//        }
//        if(0.0f < sp1C){
//            dummy_D_8037C100[1] += temp_f0;
//        }
//        else{
//            dummy_D_8037C100[1] -= temp_f0; 
//        }
//        {//L80291F7C
//            baMarker_get()->unk14_21 = 0;
//        }
//    }
}

BKModelBin* DummyPlayer::dummy_getModelBin(void){
    return dummyBin;
}

AssetID DummyPlayer::dummy_getModelId(void){
    return dummyId;
}

void DummyPlayer::dummy_setEnvAlpha(s32 alpha){
    dummyEnvAlpha = alpha;
}

void DummyPlayer::dummy_set(enum asset_e asset_id){
    if(asset_id != dummyId){
        if(dummyBin){
            func_80254008();
            assetcache_release(dummyBin);
            dummyBin = NULL;
        }
        dummyId = asset_id;
        if(dummyId)
            dummyBin = static_cast<BKModelBin*>(assetcache_get(dummyId));
    }
}

void DummyPlayer::dummy_80292048(s32 arg0, f32 arg1, f32 arg2, f32 arg3){
    dummy_D_8037C130[arg0][2] = arg1;
    dummy_D_8037C130[arg0][3] = arg2;
    dummy_D_8037C130[arg0][1] = arg3;
}

void DummyPlayer::dummy_80292078(s32 arg0, f32 arg1){
    dummy_D_8037C130[arg0][0] = arg1; 
}

void DummyPlayer::dummy_setDirection(enum player_model_direction_e direction){
    //if(direction != dummyDirection){
    //    if(direction == PLAYER_MODEL_DIR_KAZOOIE || PLAYER_MODEL_DIR_KAZOOIE == dummyDirection){
    //        //flip model
    //        yaw_setIdeal(mlNormalizeAngle(yaw_get() + 180.0f));
    //        yaw_applyIdeal();
    //    }
    //}
    dummyDirection = direction;
}

void DummyPlayer::dummy_setScale(f32 scale){
    //if(osCicId + -6103){
    //    scale = scale*0.25;
    //}
    dummyScale = scale;
}

void DummyPlayer::dummy_80292158(f32 arg0){
    dummy_D_8037C100[1] = arg0;
    dummy_8029217C(arg0);
}

void DummyPlayer::dummy_8029217C(f32 arg0){
    dummy_D_8037C110[1] = arg0;
}

void DummyPlayer::dummy_setPostDraw(void (*draw_func)(Gfx **gfx, Mtx **mtx, Vtx **vtx)){
    dummyPostDrawMethod = draw_func;
}

void DummyPlayer::dummy_setDisplacement(f32 arg0[3]){
    ml_vec3f_copy(dummyDisplacement, arg0);
}

void DummyPlayer::dummy_setYDisplacement(f32 arg0){
    dummyDisplacement[1] = arg0;
}

void DummyPlayer::dummy_setVisible(s32 arg0){
    dummyIsVisible = arg0;
}

void DummyPlayer::dummy_802921D4(f32 arg0[3]){
    if(player_getWaterState() == BSWATERGROUP_0_NONE){
        dummy_D_8037C150.unk0 = 1;
        TUPLE_COPY(dummy_D_8037C150.unk4, arg0)
    }
}

f32 DummyPlayer::dummy_getYaw(void) {
    return dummyYaw;
}

f32 DummyPlayer::dummy_80292230(void){
    return dummy_D_8037C100[1];
}

void DummyPlayer::dummy_8029223C(f32 arg0[3]){
    dummy_80291A50(8, arg0);
}

void DummyPlayer::dummy_80292260(f32 arg0[3]){
    dummy_80291A50(7, arg0);
}

void DummyPlayer::dummy_80292284(f32 arg0[3], s32 arg1){
    f32 sp44[3];
    f32 sp38[3];

    if(dummyMarker->unk14_21 && dummyIsVisible){
        switch(dummy_getModelId()){
        case ASSET_34D_MODEL_BANJOKAZOOIE_LOW_POLY:
        case ASSET_34E_MODEL_BANJOKAZOOIE_HIGH_POLY:
        case ASSET_34F_MODEL_BANJO_TERMITE: //802922E8
        case ASSET_359_MODEL_BANJO_WALRUS: //802922E8
        case ASSET_362_MODEL_BANJO_BEE:
        case ASSET_36F_MODEL_BANJO_PUMPKIN:
        case ASSET_374_MODEL_BANJO_CROC:
            func_8034A174(dummy_D_80363780, arg1 + 1, arg0);
            if(ml_isZero_vec3f(arg0)){
                dummy_getPosition(arg0);
            }

            arg0[1] += dummy_D_8037C130[arg1][0];
            if(dummy_D_8037C130[arg1][1] != 0.0f){
                func_80256E24(sp44, dummy_D_8037C130[arg1][2],  mlNormalizeAngle(yaw_get() + dummy_D_8037C130[arg1][3]), 0.0f, 0.0f, dummy_D_8037C130[arg1][1]);
                arg0[0] += sp44[0]; 
                arg0[1] += sp44[1]; 
                arg0[2] += sp44[2]; 
            }
            //baphysics_get_position_change(sp38);
            arg0[0] = arg0[0] + sp38[0];
            arg0[1] = arg0[1] + sp38[1];
            arg0[2] = arg0[2] + sp38[2];
            break;
        default: ////80292400
            dummy_getPosition(arg0);
            break;
        }
    }
    else{//L80292410
        dummy_getPosition(arg0);
        if(arg1){
            arg0[1] += 33.0f;
        }
        else{
            arg0[1] += 75.0f;
        }
    }
}

void DummyPlayer::dummy_802924B8(f32 arg0[3]){
    dummy_80291A50(0xA, arg0);
}

PlayerModelDirection DummyPlayer::dummy_getDirection(void){
    return dummyDirection;
}

void DummyPlayer::dummy_802924E8(f32 arg0[3]){
    switch(dummy_transformation){
    case TRANSFORM_5_CROC:
        dummy_80291A50(5, arg0);
        break;
    case TRANSFORM_4_WALRUS: //L80292520
        dummy_80291A50(0xB, arg0);
        break;
    default: //L80292530
        dummy_80291A50(0x9, arg0);
        break;
    }
}

s32 DummyPlayer::dummy_isVisible(void){
    return dummyIsVisible;
}

void DummyPlayer::dummy_80292554(f32 arg0[3]){
    dummy_80291A50(0x9, arg0);
}

void DummyPlayer::dummy_80292578(f32 arg0[3]){
    dummy_80291A50(0xA, arg0);
}

void DummyPlayer::dummy_defrag(void){
    if(dummy_D_80363780){
        dummy_D_80363780 = func_8034A348(dummy_D_80363780);
    }
}

// anim

void DummyPlayer::dummyAnim_init(void){
    dummyAnimCtrl = anctrl_new(1);
    func_80287784(dummyAnimCtrl, 0);
    anctrl_setSmoothTransition(dummyAnimCtrl, false);
    //func_8028746C(dummyAnimCtrl, __baanim_applyBottlesBonus);
    //AnimModifyFunction = NULL;
    anctrl_drawSetup(dummyAnimCtrl, dummyPosition, 1);
    dummyAnimState = BAANIM_UPDATE_0_NONE;
    //__baanim_setUpdateType(BAANIM_UPDATE_1_NORMAL);
    dummyAnimMinDuration = 0.01f;
    dummyAnimMaxDuration = 100.0f;
    dummyAnimScale.velocity_min = 0.0f;
    dummyAnimScale.velocity_max = 1000.0f;
    dummyAnimScale.duration_min = 0.1f;
    dummyAnimScale.duration_max = 10.0f;
    dummyAnimScale.scalable_duration = false;
    dummyAnimScale.scalable_duration = 0;
    dummyAnimScale.duration_scale = 1.0f;
}

void DummyPlayer::dummyAnim_free(void){
    anctrl_free(dummyAnimCtrl);
}

void DummyPlayer::dummyAnim_update(void){
    f32 horiz_speed;
    f32 temp;
    f32 scale;

    // Apply velocity-scaled duration using this dummy's own state and velocity,
    // mirroring the logic in __baanim_update_scaleToHorizontalVelocity /
    // __baanim_update_scaleToVerticalVelocity in ba_anim.c.
    // For the local clone, dummyVelocity is populated from baphysics each frame.
    // For a real remote player it comes from network packets.
    switch(dummyAnimState) {
        case BAANIM_UPDATE_2_SCALE_HORZ:
            scale = (dummyAnimScale.scalable_duration != 0) ? dummyAnimScale.duration_scale : 1.0f;
            horiz_speed = gu_sqrtf(dummyVelocity[0]*dummyVelocity[0] + dummyVelocity[2]*dummyVelocity[2]);
            temp = ml_mapRange_f(horiz_speed,
                dummyAnimScale.velocity_min, dummyAnimScale.velocity_max,
                dummyAnimScale.duration_min * scale, dummyAnimScale.duration_max * scale);
            anctrl_setDuration(dummyAnimCtrl, ml_clamp_f(temp, dummyAnimMinDuration, dummyAnimMaxDuration));
            break;
        case BAANIM_UPDATE_3_SCALE_VERT:
            temp = ml_mapRange_f(mlAbsF(dummyVelocity[1]),
                dummyAnimScale.velocity_min, dummyAnimScale.velocity_max,
                dummyAnimScale.duration_min, dummyAnimScale.duration_max);
            anctrl_setDuration(dummyAnimCtrl, ml_clamp_f(temp, dummyAnimMinDuration, dummyAnimMaxDuration));
            break;
        default:
            break;
    }
    anctrl_update(dummyAnimCtrl);
}

void DummyPlayer::dummyAnim_setUpdateType(s32 state) {
    dummyAnimState = state;
}

void DummyPlayer::dummyAnim_setVelocity(f32 vel[3]) {
    ml_vec3f_copy(dummyVelocity, vel);
}

void DummyPlayer::dummyAnim_setVelocityMapRanges(f32 vel_min, f32 vel_max, f32 dur_min, f32 dur_max) {
    dummyAnimScale.velocity_min = vel_min;
    dummyAnimScale.velocity_max = vel_max;
    dummyAnimScale.duration_min = dur_min;
    dummyAnimScale.duration_max = dur_max;
    dummyAnimScale.scalable_duration = false;
}

void DummyPlayer::dummyAnim_setScalableDuration(f32 scale) {
    dummyAnimScale.duration_scale = scale;
    dummyAnimScale.scalable_duration = true;
}

void DummyPlayer::dummyAnim_setDurationRange(f32 min, f32 max) {
    dummyAnimMinDuration = min;
    dummyAnimMaxDuration = max;
}

void DummyPlayer::dummyAnim_setEndAndDuration(f32 end_position, f32 duration) {
    anctrl_setSubRange(dummyAnimCtrl, 0.0f, end_position);
    anctrl_setDuration(dummyAnimCtrl, duration);
    anctrl_setPlaybackType(dummyAnimCtrl, ANIMCTRL_ONCE);
    // Note: do NOT call anctrl_start here — the animation should continue from
    // its current timer position, matching the player who also doesn't restart.
}

void DummyPlayer::dummyAnim_playForDuration(AssetID anim_id, f32 duration, AnimControl control, f32 start_position, bool smooth){
    anctrl_reset(dummyAnimCtrl);
    anctrl_setSmoothTransition(dummyAnimCtrl, smooth);
    anctrl_setIndex(dummyAnimCtrl, anim_id);
    anctrl_setDuration(dummyAnimCtrl, duration);
    if (start_position >= 0) {
        anctrl_setStart(dummyAnimCtrl, start_position);
    }
    anctrl_setPlaybackType(dummyAnimCtrl, control);
    anctrl_start(dummyAnimCtrl, "DummyPlayer.cpp", 564);
}

bool DummyPlayer::dummyAnim_isAnimID(enum asset_e anim_id){
    return anctrl_getIndex(dummyAnimCtrl) == anim_id;
}

bool DummyPlayer::dummyAnim_isStopped(void){
    return anctrl_isStopped(dummyAnimCtrl);
}
