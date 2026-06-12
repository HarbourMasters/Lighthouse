#include "Anchor.h"
#include "port/Nametag/Nametag.h"

extern "C" {
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
void func_802D729C(Actor* actor, f32 arg1);
}

#include "bk_math.h"
#include "port/Patches/Patches.h"

DummyPlayer::DummyPlayer(){};

void DummyPlayer::dummy_setTransformation(Transformation transform) {
    dummy_transformation = transform;
}

Transformation DummyPlayer::dummy_getTransformation() {
    return dummy_transformation;
}

void DummyPlayer::dummy_getPosition(f32 arg0[3]) {
    ml_vec3f_copy(arg0, dummyPosition);
}

void DummyPlayer::dummy_setPoisition(f32 pos[3]) {
    ml_vec3f_copy(dummyPosition, pos);
}

void DummyPlayer::dummy_80291A50(s32 arg0, f32 dst[3]) {
    vec3fArray_get_vec3f(dummy_D_80363780, arg0, dst);
    if (ml_isZero_vec3f(dst)) {
        dummy_getPosition(dst);
    }
}

Actor* dummy_80291AAC(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx) {
    return NULL;
}

static void _dummy_preDraw(int arg0) {
    // baMarker_get()->unk14_21 = 1;
}

void DummyPlayer::dummy_func_8029DBF0(void) {
    s32 temp_s0;
    switch (dummy_getModelId()) {
        case ASSET_34D_MODEL_BANJOKAZOOIE_LOW_POLY:  // L8029DC24
        case ASSET_34E_MODEL_BANJOKAZOOIE_HIGH_POLY: // L8029DC24
            temp_s0 = (s32)ml_interpolate_f(dummy_modelEyeBlendUpper, 1.0f, 8.0f);
            func_8033A45C(0x1B, temp_s0);
            func_8033A45C(0x1D, temp_s0);
            func_8033A45C(0x1F, temp_s0);
            func_8033A45C(0x21, temp_s0);
            temp_s0 = (s32)ml_interpolate_f(dummy_modelEyeBlendLower, 1.0f, 8.0f);
            func_8033A45C(0x1A, temp_s0);
            func_8033A45C(0x1C, temp_s0);
            func_8033A45C(0x1E, temp_s0);
            func_8033A45C(0x20, temp_s0);
            break;

        case ASSET_34F_MODEL_BANJO_TERMITE: // L8029DCCC
        case ASSET_359_MODEL_BANJO_WALRUS:  // L8029DCCC
        case ASSET_36F_MODEL_BANJO_PUMPKIN: // L8029DCCC
        case ASSET_374_MODEL_BANJO_CROC:    // L8029DCCC
            func_8033A45C(0x1B, (s32)ml_interpolate_f(dummy_modelEyeBlendUpper, 1.0f, 6.0f));
            func_8033A45C(0x1A, (s32)ml_interpolate_f(dummy_modelEyeBlendLower, 1.0f, 6.0f));
            break;

        case ASSET_356_MODEL_BANJO_WISHYWASHY: // L8029DD2C
            func_8033A45C(1, (s32)ml_interpolate_f(dummy_modelEyeBlendLower, 1.0f, 4.0f));
            break;
    }
}

AnimCtrl* DummyPlayer::dummy_getAnimCtrl() {
    return dummyAnimCtrl;
}

void DummyPlayer::dummy_setEyeState(bool squint, bool wink, bool isHat) {
    dummy_modelSquint = squint;
    dummy_modelWink = wink;
    dummy_D_8037D230 = 1.0f;
    dummy_D_8037D234 = isHat;
}

void DummyPlayer::func_8029DD6C(void) {
    s32 temp_s0; // [port] must hold values > 1 for geo selector branches

    modelRender_func_8033A1FC();
    switch (dummy_getModelId()) {
        case ASSET_34D_MODEL_BANJOKAZOOIE_LOW_POLY:
        case ASSET_34E_MODEL_BANJOKAZOOIE_HIGH_POLY:
            func_8033A45C(1, dummy_kazooieVisible);
            func_8033A45C(9, dummy_kazooieVisible);
            func_8033A45C(0xC, dummy_kazooieVisible);
            func_8033A45C(0xF, dummy_kazooieVisible);
            func_8033A45C(2, dummy_modelWink);
            func_8033A45C(0xA, dummy_modelWink);
            func_8033A45C(0xD, dummy_modelWink);
            func_8033A45C(0x10, dummy_modelWink);
            func_8033A45C(8, dummy_modelSquint);
            func_8033A45C(0xB, dummy_modelSquint);
            func_8033A45C(0xE, dummy_modelSquint);
            func_8033A45C(0x11, dummy_modelSquint);
            temp_s0 = dummy_modelMouth1 + 1;
            func_8033A45C(0x12, temp_s0);
            func_8033A45C(0x14, temp_s0);
            func_8033A45C(0x16, temp_s0);
            func_8033A45C(0x18, temp_s0);
            func_8033A45C(0x13, temp_s0);
            func_8033A45C(0x15, temp_s0);
            func_8033A45C(0x17, temp_s0);
            func_8033A45C(0x19, temp_s0);
            temp_s0 = dummy_modelMouth2 + 1;
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
            temp_s0 = dummy_modelMouth1 + 1;
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

// void DummyPlayer::dummyPlayer_setEnvColor(s32 r, s32 g, s32 b) {
//
// }

void DummyPlayer::Draw(Gfx** gfx, Mtx** mtx, Vtx** vtx) {
    f32 rotation[3];
    f32 plyr_pos[3]; // sp44
    f32 sp38[3];
    s32 env_color[3];

    if (!dummyIsVisible)
        return;

    //_dummy_updateModelYaw();
    dummy_getPosition(plyr_pos);
    plyr_pos[1] += 2.0f;
    ml_vec3f_assign(rotation, dummyPitch, dummyYaw, dummyRoll);
    func_8029A47C(env_color);
    ml_vec3f_copy(sp38, dummy_D_8037C100);

    plyr_pos[0] += dummyDisplacement[0];
    plyr_pos[1] += dummyDisplacement[1];
    plyr_pos[2] += dummyDisplacement[2];

    sp38[0] += dummyDisplacement[0];
    sp38[1] += dummyDisplacement[1];
    sp38[2] += dummyDisplacement[2];

    if (dummyBin) {
        anctrl_drawSetup(dummyAnimCtrl, dummyPosition, 1);
        func_8029DD6C();
        modelRender_setEnvColor(env_color[0], env_color[1], env_color[2], dummyEnvAlpha);
        modelRender_func_8033A280(2.0f);
        // modelRender_preDraw((GenFunction_1)_dummy_preDraw, 0);
        modelRender_setRefPoints(dummy_D_80363780);
        modelRender_setDepthMode(MODEL_RENDER_DEPTH_FULL);
        if (dummy_D_8037C150.unk0) {
            dummy_D_8037C150.unk0 = 0;
            modelRender_draw(gfx, mtx, dummy_D_8037C150.unk4, rotation, dummyScale, sp38, dummyBin);
        } else {
            modelRender_draw(gfx, mtx, plyr_pos, rotation, dummyScale, sp38, dummyBin);
        }
    } // L80291CD4

    if (dummyPostDrawMethod) {
        dummyPostDrawMethod(gfx, mtx, vtx);
    }
}

s32 DummyPlayer::dummy_func_802985F0(void) {
    switch (dummy_transformation) {
        case TRANSFORM_2_TERMITE: // 80298624
            return ASSET_34F_MODEL_BANJO_TERMITE;
        case TRANSFORM_3_PUMPKIN: // 8029862C
            return ASSET_36F_MODEL_BANJO_PUMPKIN;
        case TRANSFORM_5_CROC: // 80298634
            return ASSET_374_MODEL_BANJO_CROC;
        case TRANSFORM_4_WALRUS: // 8029863C
            return ASSET_359_MODEL_BANJO_WALRUS;
        case TRANSFORM_6_BEE: // 80298644
            return ASSET_362_MODEL_BANJO_BEE;
        case TRANSFORM_7_WISHWASHY: // 8029864C
            return ASSET_356_MODEL_BANJO_WISHYWASHY;
        case TRANSFORM_1_BANJO: // 80298654
        default:
            if (port_shouldDisableLOD()) {
                return ASSET_34E_MODEL_BANJOKAZOOIE_HIGH_POLY;
            }
            return ASSET_34D_MODEL_BANJOKAZOOIE_LOW_POLY;
    }
}

void DummyPlayer::dummy_updateModel(void) {
    dummy_set(static_cast<AssetID>(dummy_func_802985F0()));
}

void DummyPlayer::dummy_reset(void) {
    // Clear stale actor reference — the game engine owns actor lifetime and may have
    // already freed it (e.g. after a map transition). Don't dereference the old pointer.
    // dummyActor = NULL;
    if (dummyAnimCtrl) {
        dummyAnim_free();
        dummyAnimCtrl = NULL;
    }
    if (dummy_D_80363780) {
        vec3fArray_free(dummy_D_80363780);
        dummy_D_80363780 = NULL;
    }

    f32 plyr_pos[3];
    int i;
    dummyEnvAlpha = 0xFF;
    dummyBin = NULL;
    dummyId = ASSET_0_NONE;
    dummyPostDrawMethod = NULL;
    dummy_D_80363780 = vec3fArray_new();
    vec3fArray_clearValues(dummy_D_80363780);
    ml_vec3f_clear(dummy_D_8037C100);
    ml_vec3f_clear(dummy_D_8037C110);
    ml_vec3f_clear(dummyDisplacement);
    dummyYaw = dummyRoll = dummyPitch = 0.0f;
    dummy_D_8037C150.unk0 = 0;
    dummy_setVisible(true);
    dummy_setScale(1.0f);
    dummyDirection = PLAYER_MODEL_DIR_NONE;
    dummy_setDirection(PLAYER_MODEL_DIR_BANJO);
    // Always load the model. The local player's model loader skips this during the
    // level-intro flythrough (func_8028ADB4) and re-runs it when the intro ends, but
    // nothing re-runs model setup for dummies — gating here left the dummy permanently
    // invisible whenever it was registered while entering a level from the lair.
    dummy_updateModel();
    dummy_getPosition(plyr_pos);
    dummyAnim_init();
    dummyAnim_reset();
}

void DummyPlayer::dummy_detachActor(void) {
    // dummyActor = nullptr;
}

void DummyPlayer::dummy_free(void) {
    /*if (dummyActor) {
        if (dummyActor->unk104) {
            Actor *shadow = marker_getActor(dummyActor->unk104);
            shadow->unk104 = NULL;
            shadow->despawn_flag = true;
            dummyActor->unk104 = NULL;
        }
        if (dummyActor->marker) {
            marker_despawn(dummyActor->marker);
        }
        dummyActor = NULL;
    }*/
    if (dummyBin) {
        assetcache_release(dummyBin);
        dummyBin = NULL;
    }
    dummyId = ASSET_0_NONE;
    if (dummy_D_80363780) {
        vec3fArray_free(dummy_D_80363780);
        dummy_D_80363780 = NULL;
    }
    if (dummyAnimCtrl) {
        dummyAnim_free();
        dummyAnimCtrl = NULL;
    }
}

void DummyPlayer::dummyAnim_reset() {
    dummy_D_8037D230 = 0;
    dummy_D_8037D234 = 0;
    dummy_kazooieVisible = 0;
    dummy_modelWink = 0;
    dummy_modelSquint = 0;
    dummy_modelEyeBlendUpper = 0.0f;
    dummy_modelEyeBlendLower = 0.0f;
    dummy_modelMouth1 = 0;
    dummy_modelMouth2 = 0;
    dummy_D_8037D23A = 0;
}

void DummyPlayer::dummy_update(void) {
    dummyAnim_update();

    // if (dummyActor && !dummyActor->despawn_flag) {
    //     dummyActor->position[0] = dummyPosition[0];
    //     dummyActor->position[1] = dummyPosition[1];
    //     dummyActor->position[2] = dummyPosition[2];
    //     func_802D729C(dummyActor, 1.0f);
    // }
}

BKModelBin* DummyPlayer::dummy_getModelBin(void) {
    return dummyBin;
}

AssetID DummyPlayer::dummy_getModelId(void) {
    return dummyId;
}

void DummyPlayer::dummy_setEnvAlpha(s32 alpha) {
    dummyEnvAlpha = alpha;
}

void DummyPlayer::dummy_set(enum asset_e asset_id) {
    if (asset_id != dummyId) {
        if (dummyBin) {
            core1_15B30_sendMesg3ToRenderThread();
            assetcache_release(dummyBin);
            dummyBin = NULL;
        }
        dummyId = asset_id;
        if (dummyId)
            dummyBin = static_cast<BKModelBin*>(assetcache_get(dummyId));
    }
}

void DummyPlayer::dummy_80292048(s32 arg0, f32 arg1, f32 arg2, f32 arg3) {
    dummy_D_8037C130[arg0][2] = arg1;
    dummy_D_8037C130[arg0][3] = arg2;
    dummy_D_8037C130[arg0][1] = arg3;
}

void DummyPlayer::dummy_80292078(s32 arg0, f32 arg1) {
    dummy_D_8037C130[arg0][0] = arg1;
}

void DummyPlayer::dummy_setDirection(enum player_model_direction_e direction) {
    dummyDirection = direction;
}

void DummyPlayer::dummy_setScale(f32 scale) {
    dummyScale = scale;
}

void DummyPlayer::dummy_80292158(f32 arg0) {
    dummy_D_8037C100[1] = arg0;
    dummy_8029217C(arg0);
}

void DummyPlayer::dummy_8029217C(f32 arg0) {
    dummy_D_8037C110[1] = arg0;
}

void DummyPlayer::dummy_setPostDraw(void (*draw_func)(Gfx** gfx, Mtx** mtx, Vtx** vtx)) {
    dummyPostDrawMethod = draw_func;
}

void DummyPlayer::dummy_setDisplacement(f32 arg0[3]) {
    ml_vec3f_copy(dummyDisplacement, arg0);
}

void DummyPlayer::dummy_setYDisplacement(f32 arg0) {
    dummyDisplacement[1] = arg0;
}

void DummyPlayer::dummy_setVisible(s32 arg0) {
    dummyIsVisible = arg0;
}

void DummyPlayer::dummy_802921D4(f32 arg0[3]) {
    if (player_getWaterState() == BSWATERGROUP_0_NONE) {
        dummy_D_8037C150.unk0 = 1;
        TUPLE_COPY(dummy_D_8037C150.unk4, arg0)
    }
}

f32 DummyPlayer::dummy_getYaw(void) {
    return dummyYaw;
}

f32 DummyPlayer::dummy_80292230(void) {
    return dummy_D_8037C100[1];
}

void DummyPlayer::dummy_8029223C(f32 arg0[3]) {
    dummy_80291A50(8, arg0);
}

void DummyPlayer::dummy_80292260(f32 arg0[3]) {
    dummy_80291A50(7, arg0);
}

void DummyPlayer::dummy_802924B8(f32 arg0[3]) {
    dummy_80291A50(0xA, arg0);
}

PlayerModelDirection DummyPlayer::dummy_getDirection(void) {
    return dummyDirection;
}

void DummyPlayer::dummy_802924E8(f32 arg0[3]) {
    switch (dummy_transformation) {
        case TRANSFORM_5_CROC:
            dummy_80291A50(5, arg0);
            break;
        case TRANSFORM_4_WALRUS: // L80292520
            dummy_80291A50(0xB, arg0);
            break;
        default: // L80292530
            dummy_80291A50(0x9, arg0);
            break;
    }
}

s32 DummyPlayer::dummy_isVisible(void) {
    return dummyIsVisible;
}

void DummyPlayer::dummy_80292554(f32 arg0[3]) {
    dummy_80291A50(0x9, arg0);
}

void DummyPlayer::dummy_80292578(f32 arg0[3]) {
    dummy_80291A50(0xA, arg0);
}

void DummyPlayer::dummy_defrag(void) {
    if (dummy_D_80363780) {
        dummy_D_80363780 = vec3fArray_defrag(dummy_D_80363780);
    }
}

// anim

void DummyPlayer::dummyAnim_init(void) {
    dummyAnimCtrl = anctrl_new(1);
    func_80287784(dummyAnimCtrl, 0);
    anctrl_setSmoothTransition(dummyAnimCtrl, false);
    // func_8028746C(dummyAnimCtrl, __baanim_applyBottlesBonus);
    // AnimModifyFunction = NULL;
    anctrl_drawSetup(dummyAnimCtrl, dummyPosition, 1);
    dummyAnimUpdateType = BAANIM_UPDATE_0_NONE;
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

void DummyPlayer::dummyAnim_free(void) {
    anctrl_free(dummyAnimCtrl);
}

void DummyPlayer::dummyAnim_update(void) {
    f32 horiz_speed;
    f32 temp;
    f32 scale;

    // Apply velocity-scaled duration using this dummy's own state and velocity,
    // mirroring the logic in __baanim_update_scaleToHorizontalVelocity /
    // __baanim_update_scaleToVerticalVelocity in ba_anim.c.
    switch (dummyAnimUpdateType) {
        case BAANIM_UPDATE_2_SCALE_HORZ:
            scale = (dummyAnimScale.scalable_duration != 0) ? dummyAnimScale.duration_scale : 1.0f;
            horiz_speed = gu_sqrtf(dummyVelocity[0] * dummyVelocity[0] + dummyVelocity[2] * dummyVelocity[2]);
            temp = ml_mapRange_f(horiz_speed, dummyAnimScale.velocity_min, dummyAnimScale.velocity_max,
                                 dummyAnimScale.duration_min * scale, dummyAnimScale.duration_max * scale);
            anctrl_setDuration(dummyAnimCtrl, ml_clamp_f(temp, dummyAnimMinDuration, dummyAnimMaxDuration));
            break;
        case BAANIM_UPDATE_3_SCALE_VERT:
            temp = ml_mapRange_f(mlAbsF(dummyVelocity[1]), dummyAnimScale.velocity_min, dummyAnimScale.velocity_max,
                                 dummyAnimScale.duration_min, dummyAnimScale.duration_max);
            anctrl_setDuration(dummyAnimCtrl, ml_clamp_f(temp, dummyAnimMinDuration, dummyAnimMaxDuration));
            break;
        default:
            break;
    }
    anctrl_update(dummyAnimCtrl);
}

void DummyPlayer::setModelSubStates(bool kazooie, bool squint, bool wink, bool mouth1, bool mouth2, f32 eyeBlendUpper,
                                    f32 eyeBlendLower) {
    dummy_kazooieVisible = kazooie;
    dummy_modelSquint = squint;
    dummy_modelWink = wink;
    dummy_modelMouth1 = mouth1;
    dummy_modelMouth2 = mouth2;
    dummy_modelEyeBlendUpper = eyeBlendUpper;
    dummy_modelEyeBlendLower = eyeBlendLower;
}

void DummyPlayer::dummyAnim_setUpdateType(s32 state) {
    dummyAnimUpdateType = static_cast<AnimUpdateType>(state);
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

void DummyPlayer::dummyAnim_setScalableDuration(f32 scale, bool scalable) {
    dummyAnimScale.duration_scale = scale;
    dummyAnimScale.scalable_duration = scalable;
}

void DummyPlayer::dummyAnim_setDurationRange(f32 min, f32 max) {
    dummyAnimMinDuration = min;
    dummyAnimMaxDuration = max;
}

void DummyPlayer::dummyAnim_setEndAndDuration(f32 end_position, f32 duration) {
    if (!dummyAnimCtrl)
        return;
    anctrl_setSubRange(dummyAnimCtrl, 0.0f, end_position);
    anctrl_setDuration(dummyAnimCtrl, duration);
    anctrl_setPlaybackType(dummyAnimCtrl, ANIMCTRL_ONCE);
}

void DummyPlayer::dummyAnim_playForDuration(AssetID anim_id, f32 duration, AnimControl control, f32 start_position,
                                            f32 subrange_end, bool smooth) {
    if (!dummyAnimCtrl)
        return;
    anctrl_reset(dummyAnimCtrl);
    anctrl_setSmoothTransition(dummyAnimCtrl, smooth);
    anctrl_setIndex(dummyAnimCtrl, anim_id);
    anctrl_setDuration(dummyAnimCtrl, duration);
    f32 sub_start = (start_position >= 0.0f) ? start_position : 0.0f;
    anctrl_setSubRange(dummyAnimCtrl, sub_start, subrange_end);
    anctrl_setStart(dummyAnimCtrl, sub_start);
    anctrl_setPlaybackType(dummyAnimCtrl, control);
    anctrl_start(dummyAnimCtrl, "DummyPlayer.cpp", 564);
}

bool DummyPlayer::dummyAnim_isAnimID(enum asset_e anim_id) {
    if (!dummyAnimCtrl)
        return false;
    return anctrl_getIndex(dummyAnimCtrl) == anim_id;
}

bool DummyPlayer::dummyAnim_isStopped(void) {
    if (!dummyAnimCtrl)
        return true;
    return anctrl_isStopped(dummyAnimCtrl);
}
