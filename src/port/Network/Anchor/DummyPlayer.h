#pragma once
#include "functions.h"
#include "core2/ba/model.h"
#include "variables.h"
#include "port/Enhancements/Cosmetics/PlayerColors.h"

class DummyPlayer {
public:
    DummyPlayer();
    Actor* dummy_80291AAC(ActorMarker* marker, Gfx** gfx, Mtx** mtx, Vtx** vtx);
    AssetID dummy_getModelId(void);
    BKModelBin* dummy_getModelBin(void);
    PlayerModelDirection dummy_getDirection(void);
    f32 dummy_80292230(void);
    f32 dummy_getYaw(void);
    s32 dummy_func_802985F0(void);
    s32 dummy_isVisible(void);
    void Draw(Gfx** gfx, Mtx** mtx, Vtx** vtx);
    void _dummy_preDraw(int arg0);
    void _dummy_updateModelYaw(void);
    void dummy_80291A50(s32 arg0, f32 dst[3]);
    void dummy_80292048(s32 arg0, f32 arg1, f32 arg2, f32 arg3);
    void dummy_80292078(s32 arg0, f32 arg1);
    void dummy_80292158(f32 arg0);
    void dummy_8029217C(f32 arg0);
    void dummy_802921D4(f32 arg0[3]);
    void dummy_8029223C(f32 arg0[3]);
    void dummy_80292260(f32 arg0[3]);
    void dummy_802924B8(f32 arg0[3]);
    void dummy_802924E8(f32 arg0[3]);
    void dummy_80292554(f32 arg0[3]);
    void dummy_80292578(f32 arg0[3]);
    void dummy_defrag(void);
    void dummy_detachActor(void);
    void dummy_free(void);
    void dummyAnim_reset();
    void dummy_func_8029DBF0(void);
    void dummy_getPosition(f32 arg0[3]);
    void dummy_reset(void);
    void dummy_set(enum asset_e asset_id);
    void dummy_setDirection(enum player_model_direction_e direction);
    void dummy_setDisplacement(f32 arg0[3]);
    void dummy_setEnvAlpha(s32 alpha);
    void dummy_setPitch(f32 pitch);
    void dummy_setPoisition(f32 pos[3]);
    void dummy_setPostDraw(void (*draw_func)(Gfx** gfx, Mtx** mtx, Vtx** vtx));
    void dummy_setRoll(f32 roll);
    void dummy_setScale(f32 scale);
    void dummy_setTransformation(Transformation transform);
    Transformation dummy_getTransformation();
    // Bottles-bonus effect mask (D_803635EC bitfield).
    void dummy_setBottlesBonus(s32 mask);
    void dummy_setVisible(s32 arg0);
    void dummy_setYDisplacement(f32 arg0);
    void dummy_setYaw(f32 yaw);
    void dummy_update(void);
    void dummy_updateModel(void);
    // void func_80254008(void);
    void modelAppendages_loadAppendage(void);
    // void modelRender_func_8033A280(f32);
    //  anim
    void dummyAnim_init(void);
    void dummyAnim_free(void);
    void dummyAnim_update(void);
    void dummyAnim_playForDuration(AssetID anim_id, f32 duration, AnimControl control, f32 start_position,
                                   f32 subrange_end, bool smooth);
    bool dummyAnim_isAnimID(enum asset_e anim_id);
    bool dummyAnim_isStopped(void);
    // anim scale — set by network packets; mirrored from local player for clone test
    void dummyAnim_setUpdateType(s32 state);
    void dummyAnim_setVelocity(f32 vel[3]);
    void dummyAnim_setVelocityMapRanges(f32 vel_min, f32 vel_max, f32 dur_min, f32 dur_max);
    void dummyAnim_setScalableDuration(f32 scale, bool scalable);
    void dummyAnim_setDurationRange(f32 min, f32 max);
    void dummyAnim_setEndAndDuration(f32 end_position, f32 duration);
    void setModelSubStates(bool kazooie, bool squint, bool wink, bool mouth1, bool mouth2, f32 eyeBlendUpper,
                           f32 eyeBlendLower);
    // eye/mouth
    void dummy_setEyeState(bool squint, bool wink, bool isHat);
    AnimCtrl* dummy_getAnimCtrl();
    ActorMarker* dummy_getMarker() const {
        return dummyMarker;
    }
    void dummy_despawnActor(void);
    // Cosmetics: which Anchor client this stand-in belongs to, and the model colors they chose.
    void dummy_setOwner(uint32_t clientId) {
        dummyOwnerId = clientId;
    }
    void dummy_setColors(const BKPlayerColorSet& colors) {
        dummyColors = colors;
    }

private:
    uint32_t PlayerID;

    // extern s32 osCicId;

    /* .data */
    Vec3fArray* dummy_D_80363780 = NULL;

    /* .bss */
    BKModelBin* dummyBin = nullptr; // dummyPtr
    AssetID dummyId;                // dummy asset_id
    u8 dummyEnvAlpha;
    f32 dummyEnvColor[3];
    PlayerModelDirection dummyDirection;
    u8 dummyIsVisible;
    f32 dummyScale;
    f32 dummyPitch;
    f32 dummyRoll;
    f32 dummyYaw;
    f32 dummyPosition[3];
    AnimUpdateType dummyAnimUpdateType;
    f32 dummyAnimMinDuration;
    f32 dummyAnimMaxDuration;
    struct {
        f32 velocity_min;
        f32 velocity_max;
        f32 duration_min;
        f32 duration_max;
        f32 duration_scale;
        u8 scalable_duration;
    } dummyAnimScale;
    AnimCtrl* dummyAnimCtrl = nullptr;
    f32 dummyVelocity[3];
    ActorMarker* dummyMarker = nullptr;
    f32 dummy_D_8037C100[3];
    f32 dummy_D_8037C110[3];
    f32 dummyDisplacement[3];
    void (*dummyPostDrawMethod)(Gfx** gfx, Mtx** mtx, Vtx** vtx);
    f32 dummy_D_8037C130[2][4];
    struct {
        u8 unk0;
        f32 unk4[3];
    } dummy_D_8037C150;

    f32 dummy_D_8037D230;
    u8 dummy_D_8037D234;
    u8 dummy_kazooieLower;
    u8 dummy_kazooieFeet;
    u8 dummy_kazooieTurbos;
    u8 dummy_kazooieUpper;
    u8 dummy_kazooieBoots;
    u8 dummy_D_8037D23A;
    f32 dummy_banjoLeftEye;
    f32 dummy_banjoRightEye;
    Transformation dummy_transformation;
    s32 dummyBottlesBonus = 0;
    uint32_t dummyOwnerId = 0;
    BKPlayerColorSet dummyColors = {};
};