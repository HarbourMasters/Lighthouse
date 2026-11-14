#ifndef __CORE2_BA_ANIM_H__
#define __CORE2_BA_ANIM_H__

typedef enum baanim_update_type_e {
    BAANIM_UPDATE_0_NONE,
    BAANIM_UPDATE_1_NORMAL,
    BAANIM_UPDATE_2_SCALE_HORZ,
    BAANIM_UPDATE_3_SCALE_VERT
} AnimUpdateType;

/* [port] Moved out of anim.c for Anchor's player updates */
typedef struct {
    f32 velocity_min; //velocity_min
    f32 velocity_max; //velocity_max
    f32 duration_min; //duration_min
    f32 duration_max; //duration_max
    f32 duration_scale; //duration_scale
    u8  scalable_duration; //scalable_duration
} BaAnimScale;

extern f32 baAnimMinDuration;
extern f32 baAnimMaxDuration;
extern BaAnimScale baAnimScale;

#endif
