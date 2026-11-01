// [port] mapModel.c's typedef, moved here for RomhackConfig.cpp.
#ifndef BANJO_KAZOOIE_CORE2_MAP_MAPMODEL_H
#define BANJO_KAZOOIE_CORE2_MAP_MAPMODEL_H

typedef struct {
    s16 map_id; //enum map_e
    s16 opa_model_id; //enum asset_e level_model_id
    s16 xlu_model_id; //enum asset_e level2_model_id
    s16 cube_min_offset[3]; // added to the cube bounds computed from the opaque model (see mapModel_getCubeBounds)
    s16 cube_max_offset[3];
    // u8 pad12[0x2];
    f32 scale;
}MapModelDescription;

#endif
