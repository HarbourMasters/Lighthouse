#ifndef __CORE2_BA_ANIM_H__
#define __CORE2_BA_ANIM_H__

typedef enum baanim_update_type_e {
    BAANIM_UPDATE_0_NONE,
    BAANIM_UPDATE_1_NORMAL,
    BAANIM_UPDATE_2_SCALE_HORZ,
    BAANIM_UPDATE_3_SCALE_VERT
} AnimUpdateType;

/* getters — used by DummyPlayer and network code to read current scale state */
void baanim_getDurationRange(f32 *min, f32 *max);
void baanim_getVelocityMapRanges(f32 *vel_min, f32 *vel_max, f32 *dur_min, f32 *dur_max);
f32  baanim_getDurationScale(void);
bool baanim_isScalableDuration(void);

#endif
