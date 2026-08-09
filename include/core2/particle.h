#ifndef _PARTICLE_EMITTER_H_
#define _PARTICLE_EMITTER_H_

#include "structs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PART_EMIT_NO_OPA   0x20
#define PART_EMIT_NO_DEPTH 0x10
#define PART_EMIT_NO_LOOP  0x8

#define PART_EMIT_ROTATABLE 0x1


void particleEmitter_setAlpha(ParticleEmitter *self, s32 alpha);
void particleEmitter_setSfx(ParticleEmitter *self, enum sfx_e sfx_id, s32 arg2);
void func_802EFA04(ParticleEmitter *self, f32);
void particleEmitter_setParticleCallback(ParticleEmitter *self, void (*arg1)(ParticleEmitter *self, f32 pos[3]));
void particleEmitter_func_802EFA20(ParticleEmitter *self, f32, f32);
void func_802EFA34(ParticleEmitter *self, f32);
void func_802EFA40(ParticleEmitter *self, f32 (*)[3]);
void particleEmitter_func_802EFA78(ParticleEmitter *self, s32 arg1);
void func_802EFF5C(ParticleEmitter *self, f32, f32, f32);
void func_802EFF7C(ParticleEmitter *self, f32, f32, f32);
void func_802EFF9C(ParticleEmitter *self, f32);
void partEmitMgr_freeEmitter(ParticleEmitter *self);


#ifdef __cplusplus
}
#endif

#endif
