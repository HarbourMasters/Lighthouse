#include "functions.h"
#include "variables.h"
#include <ultra64.h>

/* .data */
f32 D_80365D90 = 30.0f;

/* .code */
void ncDynamicCamD_init(void) {
  func_802BE244(4.0f, 16.0f);
  func_802BE230(1.4f, 14.0f);
}

void ncDynamicCamD_end(void) {}

void ncDynamicCamD_func_802BF2C0(f32 arg0) { D_80365D90 = arg0; }

void ncDynamicCamD_update(void) {
  f32 sp54[3];
  f32 sp48[3];
  f32 sp3C[3];
  f32 sp30[3];
  f32 sp2C;

  player_getPosition(sp3C);
  sp3C[1] += 120.0f;
  ncDynamicCamera_getPosition(sp54);
  ml_vec3f_yaw_between(sp3C, sp54, &sp2C);
  ml_vec3f_rotate_direction(sp30, 0.0f, sp2C, 0.0f, 0.0f, 200.0f);
  sp3C[0] += sp30[0];
  sp3C[1] += sp30[1];
  sp3C[2] += sp30[2];
  func_802BE190(sp3C);
  ncDynamicCamera_getPosition(sp54);
  player_getPosition(sp3C);
  sp3C[1] += D_80365D90;
  ml_vec3f_angles_between_points(sp3C[0], sp3C[1], sp3C[2], sp54[0], sp54[1],
                                 sp54[2], &sp48[0], &sp48[1]);
  sp48[0] = mlNormalizeAngle(-sp48[0]);
  func_802BD904(sp48);
}
