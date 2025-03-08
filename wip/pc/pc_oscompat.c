#ifdef LIGHTHOUSE_P

#include "pc_oscompat.h"

#ifdef __cplusplus
extern "C" {
#endif

//os function placeholders
OSIntMask osSetIntMask(OSIntMask){ return 0; }
void osInitialize(void) {}
void osCreateThread(OSThread *t, OSId id, void (*entry)(void *), void *arg, void *sp, OSPri p) {}
s32 osAiSetFrequency(u32 frequency){ return 0; }
void osStopThread(OSThread *t) {}
void osStartThread(OSThread *t) {}
void osSpTaskLoad(OSTask *intp){}
void osSpTaskStartGo(OSTask *tp){}
u32 osDpGetStatus() { return 0; }
void osSpTaskYield(void){}
u32	__osGetSR(void){ return 0; }
u32 func_8025C29C(u32 *seed){ return 0; }
void osDestroyThread(OSThread *t){}
void osSetThreadPri(OSThread *t, OSPri pri){}
u32 ___osGetSR(void){ return 0; }
s32 osContSetCh(u8 ch){ return 0; }
void func_80253010(void *dest, void *src, s32 size){}
void func_80253034(void *framebuffers, int value, int size){}
void overlayManagerloadCore2(void){}
int overlayManagergetLoadedId(void) { return 0; }
bool overlayManagerload(enum overlay_e overlay_id) { return 0; }

//gu function placeholders
void guRotate(Mtx *m, float a, float x, float y, float z){}
void guOrtho(Mtx *m, float l, float r, float b, float t, float n, float f, float scale){}
void guMtxIdentF(float mf[4][4]) {}
void guMtxF2L(float mf[4][4], Mtx *m) {}
void guTranslate(Mtx *m, float x, float y, float z) {}
float gu_sqrtf(float val) {
    return sqrtf(val);
}


int defaultHuft = 0x803FBE00;
u8 n_aspMainTextStart[] = {0};
u8 n_aspMainTextEnd[] = {0};
u8 gSPF3DEX_fifoTextStart[] = {0};
u8 gSPF3DEX_fifoTextEnd[] = {0};
u8 gSPF3DEX_fifoDataStart[] = {0};
u8 gSPL3DEX_fifoDataStart[] = {0};
u8 gSPF3DEX_fifoDataEnd[] = {0};
u8 gSPL3DEX_fifoTextStart[] = {0};
u8 gSPL3DEX_fifoTextEnd[] = {0};
s32 osCicId = 0;
u8 D_8000E800 = 0;
u8 n_aspMainDataStart[] = {0};
u8 n_aspMainDataEnd[] = {0};
s32 osTvType = 1;

#ifdef __cplusplus
}
#endif

#endif
