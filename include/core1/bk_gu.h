#ifndef BANJO_KAZOOIE_CORE1_BK_GU_H
#define BANJO_KAZOOIE_CORE1_BK_GU_H

#include <ultra64.h>

/*
 * For some reasons, some of the gu functions were duplicated (some of them even twice)
 * in the code base. It's even more curious, that some of the duplicated functions call
 * original gu functions, even if they are also duplicated.
 * 
 * The duplicated functions are prefixed to avoid possible name collisions.
 */

void core1_3250_guMtxF2L(float mf[4][4], Mtx *m);
void core1_3250_guMtxIdentF(float mf[4][4]);
void core1_3250_guScaleF(float mf[4][4], float x, float y, float z);
void core1_3250_guLookAtReflectF(float mf[4][4], LookAt *l, 
		      float xEye, float yEye, float zEye,
		      float xAt,  float yAt,  float zAt,
		      float xUp,  float yUp,  float zUp);
void core1_3250_guLookAtReflect (Mtx *m, LookAt *l, float xEye, float yEye, float zEye,
	       float xAt,  float yAt,  float zAt,
	       float xUp,  float yUp,  float zUp);
void core1_3250_guScale(Mtx *m, float x, float y, float z);
void core1_3250_guRotateRPYF(float mf[4][4], float r, float p, float h);

long core1_7F60_guFToFix32(float x);
void core1_7F60_guMtxF2L_slow(float mf[4][4], Mtx *m);
void core1_7F60_guMtxF2L(float mf[4][4], Mtx *m);
void core1_7F60_guFustrumF(float mf[4][4], float arg1, float arg2, float arg3, float arg4, float arg5, float arg6, float arg7);
void core1_7F60_guMtxIdentF(float mf[4][4]);
void core1_7F60_guMtxCatF(float lmf[4][4], float rmf[4][4], float dst[4][4]);
void core1_7F60_guNormalize(float *arg0, float *arg1, float *arg2);
void core1_7F60_guPerspectiveF(float mf[4][4], u16 *perspNorm, float fovy, float aspect, float near, float far, float scale);
void core1_7F60_guPerspective(Mtx *m, u16 *perspNorm, float fovy, float aspect, float near, float far, float scale);
void core1_7F60_guRotateF(float mf[4][4], float arg1, float arg2, float arg3, float arg4);
void core1_7F60_guRotateRollF(float mf[4][4], float arg1);
void core1_7F60_guRotatePitchF(float mf[4][4], float arg1);
void core1_7F60_guRotateYawF(float mf[4][4], float arg1);
void core1_7F60_guScaleF(float mf[4][4], float arg1, float arg2, float arg3);
void core1_7F60_guTranslateF(float mf[4][4], float arg1, float arg2, float arg3);

#endif
