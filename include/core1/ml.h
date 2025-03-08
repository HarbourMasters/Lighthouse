#ifndef BANJO_KAZOOIE_CORE1_ML_H
#define BANJO_KAZOOIE_CORE1_ML_H


#ifdef __cplusplus
extern "C" {
#endif


f32 ml_acosf_deg(f32 x);
void ml_vec3f_sub_and_rotate(f32 vec1[3], f32 rotation[3], f32 vec2[3], f32 dst[3]);
f32 ml_vec3f_cos_between(f32 vec1[3], f32 vec2[3]);
void ml_vec3f_cross_product(f32 dst[3], f32 vec1[3], f32 vec2[3]);
void ml_vec3f_interpolate_fast(f32 dst[3], f32 start[3], f32 end[3], f32 t);
f32 ml_vec3f_dot_product(f32 vec1[3], f32 vec2[3]);
f32 ml_vec3f_distance(f32 vec1[3], f32 vec2[3]);
f32 ml_func_802560D0(f32 arg0[3], f32 arg1[3], f32 arg2[3]);
f32 ml_distanceSquared_vec3f(f32 vec1[3], f32 vec2[3]);
void ml_vec3f_project(f32 vec1[3], f32 vec2[3], f32 vec3[3]);
f32 ml_vec3f_length_unused(f32 vec[3]);
void ml_vec3f_normalize_copy(f32 arg0[3], f32 arg1[3]);
void ml_vec3f_normalize(f32 vec[3]);
void ml_vec2f_normalize(f32 vec[2]);
void ml_3f_normalize(f32 *x, f32 *y, f32 *z);
void ml_vec3f_set_length_copy(f32 dst[3], f32 src[3], f32 len);
void ml_vec3f_normalize_angles(f32 ptr[3]);
void ml_vec3f_wrap_angles(f32 vec[3]);
void ml_vec3f_pitch_rotate_copy(f32 dst[3], f32 src[3], f32 pitch);
void ml_vec3f_yaw_rotate_copy(f32 dst[3], f32 src[3], f32 yaw);
void ml_vec3f_roll_rotate_copy(f32 dst[3], f32 src[3], f32 roll);
void ml_vec3f_set_length(f32 vec[3], f32 length);
f32 ml_f_sin_of_angle_between_points_2D(f32 x1, f32 y1, f32 x2, f32 y2);
f32 ml_vec3f_sin_of_angle_between_vectors(f32 vec1[3], f32 vec2[3]);
f32 ml_vec3f_snap_to_grid(f32 vec[3], s32 val);
void ml_vec3f_rotate(f32 val1, f32 val2, f32 x, f32 y, f32 z, f32 *dstX, f32 *dstY, f32 *dstZ);
void ml_vec3f_rotate_direction(f32 dst[3], f32 theta, f32 phi, f32 x, f32 y, f32 z);
void ml_vec3f_rotate_and_translate(f32 vec1[3], f32 vec2[3], f32 vec3[3], f32 dst[3]);
f32 ml_acosf(f32 x);
f32 ml_vec2f_angle_between_points(f32 val1, f32 val2);
f32 ml_angle_between_points_2D(f32 val1, f32 val2, f32 val3, f32 val4);
f32 ml_vec3f_angle_between_points(f32 vec1[3], f32 vec2[3]);
void ml_vec3f_angles_between_points(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, f32 *o1, f32 *o2);
void ml_init(void);
void ml_free(void);
f32 ml_smoothstep(f32 val);
f32 ml_sin_half_pi(f32 val);
f32 ml_smoothstep_twice(f32 val);
f32 ml_smoothstep_range(f32 val1, f32 val2, f32 val3);
void ml_defrag(void);
bool ml_timer_update(f32 *timer, f32 delta);
void ml_find_closest_point_on_triangle(f32 dst[3], f32 arg1[3], f32 arg2[3][3]);
void ml_vec3f_project_onto_plane(f32 dst[3], f32 vec1[3], f32 vec2[3]);
void ml_vec3f_project_point_onto_plane(f32 arg0[3], f32 arg1[3], f32 arg2[3], f32 arg3[3]);
bool ml_vec3f_within_bounds(f32 vec[3], f32 x1, f32 z1, f32 x2, f32 z2);
f32 ml_fractional_part(f32 val1, f32 val2);
f32 ml_smoothstep_sin(f32 val1, f32 val2);
f32 ml_smoothstep_sin_2pi(f32 val1, f32 val2);
f32 ml_map_f(f32 val, f32 in_min, f32 in_max, f32 out_min, f32 out_max);
f32 ml_mapRange_f(f32 val, f32 in_min, f32 in_max, f32 out_min, f32 out_max);
f32 ml_interpolate_f(f32 arg0, f32 arg1, f32 arg2);
f32 ml_mapFunction_f(f32 a, f32 b, f32 c, f32 d, f32 e, f32 (*func)(f32));
f32 ml_mapSmoothstep(f32 a, f32 b, f32 c, f32 d, f32 e);
f32 ml_mapSmoothstepTwice(f32 a, f32 b, f32 c, f32 d, f32 e);
f32 ml_mapAbsRange_f(f32 val, f32 in_min, f32 in_max, f32 out_min, f32 out_max);
void ml_vec3f_reflect(f32 arg0[3], f32 arg1[3], f32 arg2[3]);
void ml_setLength_vec3f(f32 v[3], f32 a);
s32 ml_clamp_w(s32 val, s32 min, s32 max);
f32 ml_clamp_f(f32 val, f32 min, f32 max);
int ml_vec3f_yaw_between(f32 src[3], f32 target[3], f32 *yaw);
int ml_vec3f_yaw_towards(f32 target[3], f32 *yaw);
int ml_vec3f_angles_from_vector(f32 vec[3], f32 *arg1, f32 *arg2);
int ml_vec2f_angle_from_origin(f32 x, f32 y, f32 *dst);
bool ml_isZero_vec3f(f32 vec[3]);
bool ml_isNonzero_vec3f(f32 vec[3]);
bool ml_vec3f_not_on_vertical_axis(f32 vec[3]);
bool ml_vec3f_inside_box_f(f32 vec[3], f32 minX, f32 minY, f32 minZ, f32 maxX, f32 maxY, f32 maxZ);
bool ml_vec3f_inside_box_vec3f(f32 vec[3], f32 min[3], f32 max[3]);
bool ml_vec3w_inside_box_w(s32 vec[3], s32 minX, s32 minY, s32 minZ, s32 maxX, s32 maxY, s32 maxZ);
f32 ml_vec3f_horizontal_distance_zero_likely(f32 vec1[3], f32 vec2[3]);
f32 ml_vec3f_horizontal_distance_squared_zero_likely(f32 vec1[3], f32 vec2[3]);
f32 ml_vec3f_length(f32 vec1[3], f32 vec2[3]);
f32 ml_vec3f_length_sq(f32 vec1[3], f32 vec2[3]);
f32 ml_sin_deg(f32 angle_deg);
f32 ml_cos_deg(f32 angle_deg);
f32 mlNormalizeAngle(f32 angle);
f32 ml_remainder_f(f32 arg0, f32 arg1);
f32 ml_max_f(f32 arg0, f32 arg1);
f32 ml_min_f(f32 arg0, f32 arg1);
int ml_max_w(int arg0, int arg1);
int ml_min_w(int arg0, int arg1);
f32 mlAbsF(f32 arg0);
f32 ml_vec3f_abs_sum_xz(f32 arg0[3]);
int ml_abs_w(int arg0);
void ml_vec3f_set_yaw_length(f32 dst[3], f32 yaw, f32 length);
void ml_vec3f_rotate_and_project(f32 vec1[3], f32 arg1, f32 vec2[3], f32 *arg3, f32 *arg4, f32 *arg5);
void ml_vec3f_clear(f32 dst[3]);
void ml_vec3f_copy(f32 dst[3], f32 src[3]);
void ml_vec3f_diff_copy(f32 dst[3], f32 src1[3], f32 src2[3]);
void ml_vec3f_diff(f32 dst[3], f32 src[3]);
void ml_vec3f_assign(f32 dst[3], f32 x, f32 y, f32 z);
void ml_vec3f_add(f32 dst[3], f32 src1[3], f32 src2[3]);
void ml_vec3f_scale(f32 vec[3], f32 scale);
void ml_vec3f_scale_copy(f32 dst[3], f32 src[3], f32 scale);
void ml_vec3f_abs_sum(f32 vec1[3], f32 vec2[3]);
void ml_vec3w_to_vec3f(f32 dst[3], s32 src[3]);
void ml_vec3h_to_vec3f(f32 dst[3], s16 src[3]);
void ml_vec3f_to_vec3w(s32 dst[3], f32 src[3]);
void ml_vec3f_to_vec3h(s16 dst[3], f32 src[3]);
void ml_translate_y_local(f32 position[3], f32 rotation[3], f32 dy);
void ml_translate_z_local(f32 dst[3], f32 src[3], f32 dz);
void ml_translate_x_local(f32 dst[3], f32 src[3], f32 dx);
void ml_vec3f_adjust_rotation(f32 arg0, f32 arg1[3], f32 arg2[3], f32 arg3);
f32 ml_clamp_abs(f32 arg0, f32 arg1);
f32 mlDiffDegF(f32 arg0, f32 arg1);
bool ml_vec3f_point_within_horizontal_distance(f32 vec[3], f32 x, f32 z, f32 distance);
bool ml_vec3f_within_horizontal_distance(f32 vec1[3], f32 vec2[3], f32 distance);
bool ml_vec3w_within_horizontal_distance(s32 vec1[3], s32 vec2[3], s32 distance);
bool ml_vec3f_within_distance(f32 vec1[3], f32 vec2[3], f32 distance);
bool ml_stub_80259400(f32 x);
void ml_sub_delta_time(f32 *x);
void ml_vec3f_intersect_lines(f32 a0[3], f32 a1[3], f32 a2[3], f32 a3[3]);
f32 ml_vec3f_project_point_onto_line(f32 dst[3], f32 vec1[3], f32 vec2[3], f32 vec3[3]);
void ml_vec3f_closest_point_on_line_segment(f32 a0[3], f32 a1[3], f32 a2[3], f32 a3[3]);
s32 ml_getViewportYawWithOffset(f32 x);
bool ml_isViewportYawWithOffsetNormalized(f32 x);
void ml_vec3f_interpolate(f32 dst[3], f32 start[3], f32 end[3], f32 t);

#ifdef __cplusplus
}
#endif

#endif
