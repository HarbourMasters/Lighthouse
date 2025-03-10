#ifndef BANJO_KAZOOIE_CORE1_MLMTX_H
#define BANJO_KAZOOIE_CORE1_MLMTX_H

#ifdef __cplusplus
extern "C" {
#endif

void mlMtxGet(MtxF *dst);
MtxF *mlMtx_get_stack_pointer(void);
void mlMtxApply(Mtx *mPtr);
void mlMtxPop(void);
void mlMtx_push_duplicate(void);
void mlMtx_push_identity(void);
void mlMtx_push_translation(f32 x, f32 y, f32 z);
void mlMtx_push_mtx(f32* mtx);
void mlMtx_push_multiplied(f32* l_mtx);
void mlMtx_push_multiplied_2(MtxF * l_mtx, MtxF * r_mtx);
void mlMtxIdent(void);
void mlMtx_set_translation(f32 x, f32 y, f32 z);
void mlMtxSet(MtxF* arg0);
void mlMtxRotate(f32 a, f32 x, f32 y, f32 z);
void mlMtxRotPitch(f32 arg0);
void mlMtxRotYaw(f32 arg0);
void mlMtxRotRoll(f32 arg0);
void mlMtx_rotate_pitch_deg(f32 arg0);
void mlMtx_rotate_yaw_deg(f32 arg0);
void mlMtxRotatePYR(f32 pitch, f32 yaw, f32 roll);
void mlMtxScale_xyz(f32 x, f32 y, f32 z);
void mlMtxScale(f32 scale);
void mlMtx_set_translation_vec3f(f32 x, f32 y, f32 z);
void mlMtx_apply_vec3f(f32 dst[3], f32 src[3]);
void mlMtx_apply_vec3f_restricted(f32 dst[3], f32 src[3]);
void mlMtx_apply_f3(f32 dst[3], f32 x, f32 y, f32 z);
void mlMtx_apply_vec3s(s16 dst[3], s16 src[3]);
void mlMtx_apply_to_bounding_box(s32 arg0[3], s32 arg1[3], s32 arg2[3], s32 arg3[3]);
void mlMtxTranslate(f32 x, f32 y, f32 z);
void mlMtx_translate_vec3f(f32 x, f32 y, f32 z);
void mlMtx_transform(f32 arg0[3], f32 arg1[3], f32 rotation[3], f32 scale, f32 arg4[3]);
void mlMtx_transform_with_scale(f32 arg0[3], f32 rotation[3], f32 scale, f32 arg3[3]);
void mlMtx_inverse_transform_with_scale(f32 position[3], f32 rotation[3], f32 scale, f32 arg3[3]);
void mlMtx_translate_and_rotate(f32 position[3], f32 rotation[3]);
void mlMtx_inverse_translate_and_rotate(f32 position[3], f32 rotation[3]);
void mlMtx_translate_rotate_translate(f32 position[3], f32 rotation[3]);
void mlMtx_translate_inverse_rotate_translate(f32 position[3], f32 rotation[3]);
void mlMtx_rotate(f32 rotation[3]);
void mlMtx_rotate_xyz(f32 rotation[3]);
void mlMtx_inverse_rotate_xyz(f32 rotation[3]);

#ifdef __cplusplus
}
#endif

#endif
