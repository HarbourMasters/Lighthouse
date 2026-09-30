// BanjoDecomp: core1/code_7F60.c
#include <ultra64.h>
#include <string.h>
#include <math.h>
#include "core1/bk_gu.h"
#include "version.h"

extern f32 gu_sqrtf(f32);

s32 D_80275900 = VER_SELECT(0xCFADA290, 0xD39EA620, 0, 0); // LAIR_DATA_CRC2
s32 D_80275904 = VER_SELECT(0x934A83F1, 0xC77ED5C6, 0, 0); // GV_DATA_CRC2
//float core1_7F60_M_DTOR = M_DTOR;
float core1_7F60_M_DTOR = 3.14159265358979323846 / 180.0;

long core1_7F60_guFToFix32(float x) {
    return FTOFIX32(x);
}

void core1_7F60_guMtxF2L_slow(float mf[4][4], Mtx *m)
{
#ifdef GBI_FLOATS
	// [port] With GBI_FLOATS, Mtx is MtxF (float[4][4]) — just copy
	memcpy(m->mf, mf, sizeof(float) * 16);
#else
	int	i, j;
	int	e1,e2;
	int	*ai,*af;


	ai=(int *) &m->m[0][0];
	af=(int *) &m->m[2][0];

	for (i=0; i<4; i++)
	for (j=0; j<2; j++) {
		e1=core1_7F60_guFToFix32(mf[i][j*2]);
		e2=core1_7F60_guFToFix32(mf[i][j*2+1]);
		*(ai++) = ( e1 & 0xffff0000 ) | ((e2 >> 16)&0xffff);
		*(af++) = ((e1 << 16) & 0xffff0000) | (e2 & 0xffff);
	}
#endif
}

void core1_7F60_guMtxF2L(float mf[4][4], Mtx *m)
{
#ifdef GBI_FLOATS
  // [port] With GBI_FLOATS, Mtx is MtxF (float[4][4]) — just copy
  memcpy(m->mf, mf, sizeof(float) * 16);
#else
  int i;
  int j;
  int e1;
  int e2;
  int *ai;
  int *af;
    float * a2;
    
  ai = (int *) (&m->m[0][0]);
  af = (int *) (&m->m[2][0]);
  a2 = mf[0];
  for (i = 0; i < 8; i+=4)
    for (j = 0; j < 4; j++) {
		e1=(*(a2++))* 65536;
		e2=(*(a2++))* 65536;
      *(ai++) = (e1 & 0xffff0000) | ((e2 >> 16) & 0xffff);
      *(af++) = ((e1 <<  16) & 0xffff0000) | (e2 & 0xffff);
    }
#endif
}

void core1_7F60_guFustrumF(float mf[4][4], float arg1, float arg2, float arg3, float arg4, float arg5, float arg6, float arg7) {
    s32 i, j;

    core1_7F60_guMtxIdentF(mf);
    mf[0][0] = (2 * arg5) / (arg2 - arg1);
    mf[1][1] = (2 * arg5) / (arg4 - arg3);
    mf[2][0] = (arg2 + arg1) / (arg2 - arg1);
    mf[2][1] = (arg4 + arg3) / (arg4 - arg3);
    mf[2][2] = -(arg6 + arg5) / (arg6 - arg5);
    mf[2][3] = -1.0f;
    mf[3][2] = (-2 * arg6 * arg5) / (arg6 - arg5);
    mf[3][3] = 0.0f;
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			mf[i][j] *= arg7;
		}
	}
}

void core1_7F60_guMtxIdentF(float mf[4][4])
{
	int	i, j;

	for (i=0; i<4; i++)
	for (j=0; j<4; j++)
		if (i == j) mf[i][j] = 1.0;
		else mf[i][j] = 0.0;
}

void core1_7F60_guMtxCatF(float lmf[4][4], float rmf[4][4], float dst[4][4]) {
	s32 i, j, k;
    float sp1C[4][4];

	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			sp1C[i][j] = 0.0f;
			for(k = 0; k < 4; k++){
				sp1C[i][j] += lmf[i][k]*rmf[k][j];
			}
		}
	}
	for(i = 0; i < 4; i++){
		for(j = 0; j < 4; j++){
			dst[i][j] = sp1C[i][j];
		}
	}
}

void core1_7F60_guNormalize(float *arg0, float *arg1, float *arg2) {
    float inv_len;

    inv_len = 1.0f / gu_sqrtf((*arg0 * *arg0) + (*arg1 * *arg1) + (*arg2 * *arg2));
    *arg0 *= inv_len;
    *arg1 *= inv_len;
    *arg2 *= inv_len;
}

void core1_7F60_guPerspectiveF(float mf[4][4], u16 *perspNorm, float fovy, float aspect, float near, float far, float scale)
{
  float cot;
  float tmp;
  int i;
  int j;
  near = (near < 1.0f) ? (1.0f) : (near);
  far = (far < (near + 100.0f)) ? (near + 100.0f) : (far);
  tmp = ((2 * near) * far) / (near - far);
  if (((tmp * scale) > 32767) || ((tmp * scale) < (-32767)))
  {
    tmp = ((tmp * scale) > 32767) ? (32767) : (-32767);
    near = ((-(tmp / scale)) * far) / ((2 * far) - (tmp / scale));
  }
  guMtxIdentF(mf);
  fovy *= 3.141592654 / 180.0;
  cot = cosf(fovy / 2) / sinf(fovy / 2);
  mf[0][0] = cot / aspect;
  mf[1][1] = cot;
  mf[2][2] = (near + far) / (near - far);
  mf[2][3] = -1;
  if (!far)
  {
  }
  mf[3][2] = ((2 * near) * far) / (near - far);
  mf[3][3] = 0;
  for (i = 0; i < 4; i++)
  {
    for (j = 0; j < 4; j++)
    {
      mf[i][j] *= scale;
    }

  }

  if (perspNorm != ((u16 *) 0))
  {
    if ((near + far) <= 2.0)
    {
      *perspNorm = (u16) 0xFFFF;
    }
    else
    {
      *perspNorm = (u16) ((2.0 * 65536.0) / (near + far));
      if ((*perspNorm) <= 0)
      {
        *perspNorm = (u16) 0x0001;
      }
    }
  }
}


void core1_7F60_guPerspective(Mtx *m, u16 *perspNorm, float fovy, float aspect, float near, float far, float scale) {
    float sp28[4][4];

    core1_7F60_guPerspectiveF(sp28, perspNorm, fovy, aspect, near, far, scale);
    core1_7F60_guMtxF2L_slow(sp28, m);
}

void core1_7F60_guRotateF(float mf[4][4], float arg1, float arg2, float arg3, float arg4) {
    float sp34;
    float sp30;
    float sp2C;
    float sp28;
    volatile float sp24;

    core1_7F60_guNormalize(&arg2, &arg3, &arg4);
    arg1 *= core1_7F60_M_DTOR;
    sp34 = sinf(arg1);
    sp30 = cosf(arg1);
    sp2C = arg2 * arg3 * (1.0f - sp30);
    sp28 = arg3 * arg4 * (1.0f - sp30);
    sp24 = (arg4 * arg2 * (1.0f - sp30));
    core1_7F60_guMtxIdentF(mf);
    mf[0][0] = ((1.0f - (arg2 * arg2)) * sp30) + (arg2 * arg2);
    mf[2][1] = (sp28 - (arg2 * sp34));
    mf[1][2] = ((arg2 * sp34) + sp28);

    mf[1][1] = (((1.0f - (arg3 * arg3)) * sp30) + (arg3 * arg3));
    mf[2][0] = ((arg3 * sp34) + sp24);
    mf[0][2] = (sp24 - (arg3 * sp34));
	
    mf[2][2] = (((1.0f - (arg4 * arg4)) * sp30) + (arg4 * arg4));
    mf[1][0] = (sp2C - (arg4 * sp34));
    mf[0][1] = (arg4 * sp34) + sp2C;
}

void core1_7F60_guRotateRollF(float mf[4][4], float arg1){
	float c, s;
	arg1 *= core1_7F60_M_DTOR;
	core1_7F60_guMtxIdentF(mf);
	c = cosf(arg1);
	mf[1][1] = c;
	mf[2][2] = c;
	s = sinf(arg1);
	mf[1][2] = s;
	mf[2][1] = -s;
}

void core1_7F60_guRotatePitchF(float mf[4][4], float arg1){
	float c, s;
	arg1 *= core1_7F60_M_DTOR;
	core1_7F60_guMtxIdentF(mf);
	c = cosf(arg1);
	mf[0][0] = c;
	mf[2][2] = c;
	s = sinf(arg1);
	mf[2][0] = s;
	mf[0][2] = -s;
}

void core1_7F60_guRotateYawF(float mf[4][4], float arg1){
	float c, s;
	arg1 *= core1_7F60_M_DTOR;
	core1_7F60_guMtxIdentF(mf);
	c = cosf(arg1);
	mf[0][0] = c;
	mf[1][1] = c;
	s = sinf(arg1);
	mf[0][1] = s;
	mf[1][0] = -s;
}

void core1_7F60_guScaleF(float mf[4][4], float arg1, float arg2, float arg3){
	core1_7F60_guMtxIdentF(mf);
	mf[0][0] = arg1;
	mf[1][1] = arg2;
	mf[2][2] = arg3;
	mf[3][3] = 1.0f;
}

void core1_7F60_guTranslateF(float mf[4][4], float arg1, float arg2, float arg3){
	core1_7F60_guMtxIdentF(mf);
	mf[3][0] = arg1;
	mf[3][1] = arg2;
	mf[3][2] = arg3;
}
