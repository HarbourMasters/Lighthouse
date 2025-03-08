#ifndef _PARTICLE_EMITTER_H_
#define _PARTICLE_EMITTER_H_


#ifdef __cplusplus
extern "C" {
#endif


#define PART_EMIT_NO_OPA   0x20
#define PART_EMIT_NO_DEPTH 0x10
#define PART_EMIT_NO_LOOP  0x8

#define PART_EMIT_ROTATABLE 0x1


void particleEmitter_setAlpha(ParticleEmitter *this, s32 alpha);
void particleEmitter_setSfx(ParticleEmitter *this, enum sfx_e sfx_id, s32 arg2);
void particleEmitter_setMinHeight(ParticleEmitter *this, f32);
void particleEmitter_setParticleCallback(ParticleEmitter *this, void (*arg1)(ParticleEmitter *this, f32 pos[3]));
void particleEmitter_setSfxPitchRange(ParticleEmitter *this, f32, f32);
void particleEmitter_setMaxHeight(ParticleEmitter *this, f32);
void particleEmitter_setPositionOffset(ParticleEmitter *this, f32 (*)[3]);
void particleEmitter_setDrawLayer(ParticleEmitter *this, s32 arg1);
void particleEmitter_setWaveAmplitude(ParticleEmitter *this, f32, f32, f32);
void particleEmitter_setWaveFrequency(ParticleEmitter *this, f32, f32, f32);
void particleEmitter_setWaveLength(ParticleEmitter *this, f32);
void partEmitMgr_freeEmitter(ParticleEmitter *this);

#ifdef __cplusplus
}
#endif


#endif
