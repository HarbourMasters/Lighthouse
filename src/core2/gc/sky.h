// [port] sky.c's typedefs, moved here for RomhackConfig.cpp.
#ifndef BANJO_KAZOOIE_CORE2_GC_SKY_H
#define BANJO_KAZOOIE_CORE2_GC_SKY_H

typedef struct {
    s16 model_id;
    // u8 pad2[0x2];
    f32 scale;
    f32 rotation_speed;
}SkyInfo;

typedef struct {
    s16 map;
    // u8 pad2[2];
    SkyInfo sky_list[3];
}MapSkyInfo;

#endif
