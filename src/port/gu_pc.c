// BanjoDecomp: (port-specific, no decomp origin)
#include <math.h>
#include <string.h>
#include "libultraship/libultra/types.h"

void guMtxF2L(float mf[4][4], Mtx* m) {
#ifdef GBI_FLOATS
    // [port] With GBI_FLOATS, Mtx is MtxF (float[4][4]) — just copy
    memcpy(m->mf, mf, sizeof(float) * 16);
#else
    unsigned int r, c;
    s32 tmp1;
    s32 tmp2;
    s32* m1 = &m->m[0][0];
    s32* m2 = &m->m[2][0];
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 2; c++) {
            tmp1 = mf[r][2 * c] * 65536.0f;
            tmp2 = mf[r][2 * c + 1] * 65536.0f;
            *m1++ = (tmp1 & 0xffff0000) | ((tmp2 >> 0x10) & 0xffff);
            *m2++ = ((tmp1 << 0x10) & 0xffff0000) | (tmp2 & 0xffff);
        }
    }
#endif
}

void guMtxL2F(float mf[4][4], Mtx* m) {
#ifdef GBI_FLOATS
    // [port] With GBI_FLOATS, Mtx is MtxF (float[4][4]) — just copy
    memcpy(mf, m->mf, sizeof(float) * 16);
#else
    unsigned int r, c;
    u32 tmp1;
    u32 tmp2;
    u32* m1;
    u32* m2;
    s32 stmp1, stmp2;
    m1 = (u32*)&m->m[0][0];
    m2 = (u32*)&m->m[2][0];
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 2; c++) {
            tmp1 = (*m1 & 0xffff0000) | ((*m2 >> 0x10) & 0xffff);
            tmp2 = ((*m1++ << 0x10) & 0xffff0000) | (*m2++ & 0xffff);
            stmp1 = *(s32*)&tmp1;
            stmp2 = *(s32*)&tmp2;
            mf[r][c * 2 + 0] = stmp1 / 65536.0f;
            mf[r][c * 2 + 1] = stmp2 / 65536.0f;
        }
    }
#endif
}

void guMtxIdentF(f32 mf[4][4]) {
    unsigned int r, c;
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 4; c++) {
            if (r == c) {
                mf[r][c] = 1.0f;
            } else {
                mf[r][c] = 0.0f;
            }
        }
    }
}

void guMtxIdent(Mtx* m) {
    guMtxIdentF(m->mf);
}

void guTranslateF(float m[4][4], float x, float y, float z) {
    guMtxIdentF(m);
    m[3][0] = x;
    m[3][1] = y;
    m[3][2] = z;
}
void guTranslate(Mtx* m, float x, float y, float z) {
    float mf[4][4];
    guTranslateF(mf, x, y, z);
    guMtxF2L(mf, m);
}
#if 0
void guScaleF(float mf[4][4], float x, float y, float z) {
    guMtxIdentF(mf);
    mf[0][0] = x;
    mf[1][1] = y;
    mf[2][2] = z;
    mf[3][3] = 1.0;
}
void guScale(Mtx* m, float x, float y, float z) {
    float mf[4][4];
    guScaleF(mf, x, y, z);
    guMtxF2L(mf, m);
}
#endif
void guNormalize(f32* x, f32* y, f32* z) {
    f32 tmp = 1.0f / sqrtf(*x * *x + *y * *y + *z * *z);
    *x = *x * tmp;
    *y = *y * tmp;
    *z = *z * tmp;
}

void guRotateF(float m[4][4], float a, float x, float y, float z) {
    // Lighthouse TODO bring over M_PIf from 2ship
    static float D_80097F90 = M_PI / 180.0f;
    float sine;
    float cosine;
    float ab;
    float bc;
    float ca;
    float t;
    float xs;
    float ys;
    float zs;

    guNormalize(&x, &y, &z);

    a = a * D_80097F90;

    sine = sinf(a);
    cosine = cosf(a);

    ab = x * y * (1 - cosine);
    bc = y * z * (1 - cosine);
    ca = z * x * (1 - cosine);

    guMtxIdentF(m);

    xs = x * sine;
    ys = y * sine;
    zs = z * sine;

    t = x * x;
    m[0][0] = (1 - t) * cosine + t;
    m[2][1] = bc - xs;
    m[1][2] = bc + xs;
    t = y * y;
    m[1][1] = (1 - t) * cosine + t;
    m[2][0] = ca + ys;
    m[0][2] = ca - ys;
    t = z * z;
    m[2][2] = (1 - t) * cosine + t;
    m[1][0] = ab - zs;
    m[0][1] = ab + zs;
}

void guRotate(Mtx* m, float a, float x, float y, float z) {
    float mf[4][4];
    guRotateF(mf, a, x, y, z);
    guMtxF2L(mf, m);
}

void guOrthoF(float m[4][4], float l, float r, float b, float t, float n, float f, float scale) {
    int i;
    int j;
    guMtxIdentF(m);
    m[0][0] = 2 / (r - l);
    m[1][1] = 2 / (t - b);
    m[2][2] = -2 / (f - n);
    m[3][0] = -(r + l) / (r - l);
    m[3][1] = -(t + b) / (t - b);
    m[3][2] = -(f + n) / (f - n);
    m[3][3] = 1;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            m[i][j] *= scale;
        }
    }
}

void guOrtho(Mtx* m, float l, float r, float b, float t, float n, float f, float scale) {
    float mf[4][4];
    guOrthoF(mf, l, r, b, t, n, f, scale);
    guMtxF2L(mf, m);
}