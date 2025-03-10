#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include "core1/core1.h"
#include "core2/particle.h"

extern s32 sprite_getFrameCount(BKSprite_s *);
extern void func_80344720(s32 SpriteGfx, s32 frame, s32, f32[3], f32[3], f32[3],
                          Gfx **, Mtx **);
extern void func_80344424(s32 SpriteGfx, s32 frame, s32, f32[3], f32[3], f32,
                          Gfx **, Mtx **);

Gfx D_80368940[] = {
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG |
                          G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR |
                          G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_TEXTURE_GEN_LINEAR |
                        G_SHADING_SMOOTH),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetCombineMode(G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(
        Z_CMP | IM_RD | CVG_DST_FULL | ZMODE_OPA | FORCE_BL |
            GBL_c1(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA),
        Z_CMP | IM_RD | CVG_DST_FULL | ZMODE_OPA | FORCE_BL |
            GBL_c2(G_BL_CLR_IN, G_BL_A_IN, G_BL_CLR_MEM, G_BL_1MA)),
    gsSPEndDisplayList()};

Gfx D_80368978[] = {
    gsSPClearGeometryMode(G_ZBUFFER | G_SHADE | G_CULL_BOTH | G_FOG |
                          G_LIGHTING | G_TEXTURE_GEN | G_TEXTURE_GEN_LINEAR |
                          G_LOD | G_SHADING_SMOOTH),
    gsSPSetGeometryMode(G_SHADE | G_TEXTURE_GEN_LINEAR | G_SHADING_SMOOTH),
    gsSPTexture(0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON),
    gsDPSetCombineMode(G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM),
    gsDPSetCycleType(G_CYC_1CYCLE),
    gsDPSetRenderMode(G_RM_XLU_SURF, G_RM_XLU_SURF2),
    gsSPEndDisplayList()};

ParticleEmitter **partEmitMgr = NULL; // particlePtrArrayPtr
s32 partEmitMgrLength = 0;            // particlePtrArraySize

// particle
typedef struct particle {
  f32 acceleration[3];
  f32 fade;
  f32 frame;     // frame
  f32 framerate; // framerate
  f32 position[3];
  f32 rotation[3];
  f32 scale;          // size
  f32 initialSize_34; // initial_size
  f32 finalSizeDiff;  // delta_size
  f32 angluar_velocity[3];
  f32 age_48;
  f32 lifetime_4C;
  f32 velocity_50[3];
  u8 unk5C;
  // u8 pad5D[3];
} Particle;

/* .bss */
f32 particleSfxTimer;
u8 partEmitMgrEnable;

/* .code */
void particleEmitter_freeResources(ParticleEmitter *pthis) {
  assetCache_releaseBKSprite(&pthis->sprite_1C, &pthis->unk34);
  if (pthis->model_20)
    assetCache_releaseBKModelBin(&pthis->model_20);
}

int particleEmitter_checkCollision(ParticleEmitter *pthis, f32 (*arg1)[3],
                                   f32 (*arg2)[3], f32 (*arg3)[3], s32 arg4) {
  if (-100000.0 == pthis->unk74 && 100000.0 == pthis->unk78) {
    return findCollisionTriAlongPath3(arg1, arg2, arg3, 0);
  }

  if (100000.0 != pthis->unk78 && pthis->unk78 < (*arg2)[1]) {
    (*arg2)[1] = pthis->unk78;
    (*arg3)[2] = 0.0f;
    (*arg3)[0] = 0.0f;
    (*arg3)[1] = -1.0f;
    return 1;
  }

  if (pthis->unk74 != -100000.0 && (*arg2)[1] < pthis->unk74) {
    (*arg2)[1] = pthis->unk74;
    (*arg3)[2] = 0.0f;
    (*arg3)[0] = 0.0f;
    (*arg3)[1] = 1.0f;
    return 1;
  }

  return 0;
}

void particleEmitter_initParticle(ParticleEmitter *pthis, Particle *particle) {
  particle->acceleration[0] = randf2(pthis->particleAccerationRange_4C_min_x,
                                     pthis->particleAccerationRange_4C_max_x);
  particle->acceleration[1] = randf2(pthis->particleAccerationRange_4C_min_y,
                                     pthis->particleAccerationRange_4C_max_y);
  particle->acceleration[2] = randf2(pthis->particleAccerationRange_4C_min_z,
                                     pthis->particleAccerationRange_4C_max_z);
  particle->unk5C = pthis->unk64;

  particle->fade = (0.0f == pthis->fade_in) ? 1.0f : 0;
  particle->frame = randf2((f32)pthis->particleStartingFrameRange_84_min,
                           (f32)pthis->particleStartingFrameRange_84_max);
  particle->framerate = randf2(pthis->particleFramerateRange_8C_min,
                               pthis->particleFramerateRange_8C_max);

  particle->position[0] = pthis->postion_28[0];
  particle->position[1] = pthis->postion_28[1];
  particle->position[2] = pthis->postion_28[2];

  particle->position[0] =
      particle->position[0] + randf2(pthis->particleSpawnPositionRange_94_min_x,
                                     pthis->particleSpawnPositionRange_94_max_x);
  particle->position[1] =
      particle->position[1] + randf2(pthis->particleSpawnPositionRange_94_min_y,
                                     pthis->particleSpawnPositionRange_94_max_y);
  particle->position[2] =
      particle->position[2] + randf2(pthis->particleSpawnPositionRange_94_min_z,
                                     pthis->particleSpawnPositionRange_94_max_z);

  particle->initialSize_34 = particle->scale =
      randf2(pthis->particleStartingScaleRange_AC_min,
             pthis->particleStartingScaleRange_AC_max);
  if (0.0f == pthis->particleFinalScaleRange_B4_min &&
      0.0f == pthis->particleFinalScaleRange_B4_max)
    particle->finalSizeDiff = 0.0f;
  else
    particle->finalSizeDiff = randf2(pthis->particleFinalScaleRange_B4_min,
                                     pthis->particleFinalScaleRange_B4_max) -
                              particle->initialSize_34;

  particle->rotation[2] = 0.0f;
  particle->rotation[1] = 0.0f;
  particle->rotation[0] = 0.0f;

  particle->angluar_velocity[0] = randf2(pthis->unkBC[0], pthis->unkC8[0]);
  particle->angluar_velocity[1] = randf2(pthis->unkBC[1], pthis->unkC8[1]);
  particle->angluar_velocity[2] = randf2(pthis->unkBC[2], pthis->unkC8[2]);

  particle->age_48 = 0.0f;
  particle->lifetime_4C =
      randf2(pthis->particleLifeTimeRange[0], pthis->particleLifeTimeRange[1]) +
      0.001;
  if (!pthis->sphericalParticleVelocity_48) {
    particle->velocity_50[0] =
        randf2(pthis->particleVelocityRange_E4.cartisian_min_x,
               pthis->particleVelocityRange_E4.cartisian_max_x);
    particle->velocity_50[1] =
        randf2(pthis->particleVelocityRange_E4.cartisian_min_y,
               pthis->particleVelocityRange_E4.cartisian_max_y);
    particle->velocity_50[2] =
        randf2(pthis->particleVelocityRange_E4.cartisian_min_z,
               pthis->particleVelocityRange_E4.cartisian_max_z);
  } else {
    ml_vec3f_rotate_direction(
        particle->velocity_50,
        mlNormalizeAngle(
            randf2(pthis->particleVelocityRange_E4.spherical.pitch_min,
                   pthis->particleVelocityRange_E4.spherical.pitch_max)),
        mlNormalizeAngle(
            randf2(pthis->particleVelocityRange_E4.spherical.yaw_min,
                   pthis->particleVelocityRange_E4.spherical.yaw_max)),
        0.0f, 0.0f,
        randf2(pthis->particleVelocityRange_E4.spherical.radius_min,
               pthis->particleVelocityRange_E4.spherical.radius_max));
  }
}

void particleEmitter_applyWaveEffect(ParticleEmitter *pthis, f32 age,
                                     f32 arg2[3]) {
  int i;
  f32 tmp_f22;
  f32 tmp_f0;

  tmp_f22 = pthis->unk108 * 0.5;
  for (i = 0; i < 3; i++) {
    if (pthis->unk118[i] != 0.0f) {
      tmp_f0 = ml_remainder_f(pthis->unk10C[i] + age, pthis->unk108);
      tmp_f0 = mlAbsF(tmp_f0 - tmp_f22);
      tmp_f0 = ml_map_f(tmp_f0, 0.0f, tmp_f22, 1.0 - pthis->unk118[i],
                        pthis->unk118[i] + 1.0);
      arg2[i] *= tmp_f0;
    }
  }
}

void __particleEmitter_drawOnPass(ParticleEmitter *pthis, Gfx **gfx, Mtx **mtx,
                                  Vtx **vtx, u32 draw_pass) {
  f32 position[3];
  f32 flat_rotation[3];
  f32 scale[3];
  Particle *iPtr;

  if (reinterpret_cast(u32, draw_pass) != (pthis->draw_mode & 0x4) != 0)
    return;

  if (pthis->model_20 != NULL) {
    for (iPtr = pthis->pList_start_124; iPtr < pthis->pList_end_128; iPtr++) {
      position[0] = iPtr->position[0] + pthis->unk4[0];
      position[1] = iPtr->position[1] + pthis->unk4[1];
      position[2] = iPtr->position[2] + pthis->unk4[2];
      if (0.0f != pthis->fade_in || 1.0 != pthis->fade_out ||
          pthis->alpha != 0xff) {
        modelRender_setAlpha((s32)(iPtr->fade * pthis->alpha));
      } // L802EEF5C
      modelRender_setDepthMode((pthis->draw_mode & PART_EMIT_NO_DEPTH)
                                   ? MODEL_RENDER_DEPTH_NONE
                                   : MODEL_RENDER_DEPTH_FULL);
      modelRender_draw(gfx, mtx, position, iPtr->rotation, iPtr->scale, NULL,
                       pthis->model_20);
    }
    return;
  }

  if (pthis->unk34) { // L802EEFC4
    if (pthis->rgb[0] != 0xff || pthis->rgb[1] != 0xff || pthis->rgb[2] != 0xff ||
        pthis->alpha != 0xff) {
      func_803382E4((pthis->draw_mode & PART_EMIT_NO_DEPTH) ? 9 : 0xf);
      func_80338338(pthis->rgb[0], pthis->rgb[1], pthis->rgb[2]);
      func_803382B4((pthis->rgb[0] < 8) ? 0 : pthis->rgb[0] - 8,
                    (pthis->rgb[1] < 8) ? 0 : pthis->rgb[1] - 8,
                    (pthis->rgb[2] < 8) ? 0 : pthis->rgb[2] - 8,
                    (pthis->draw_mode & PART_EMIT_NO_OPA) ? 0xff : pthis->alpha);
      func_80338370();
      func_80335D30(gfx);
    } else if (pthis->draw_mode & PART_EMIT_NO_DEPTH) { // L802EF0C0
      __gSPDisplayList((*gfx)++, D_80368978);
    } else { // L802EF0EC
      __gSPDisplayList((*gfx)++, D_80368940);
    } // L802EF10C
    flat_rotation[0] = 90.0f;
    flat_rotation[1] = 0.0f;
    flat_rotation[2] = 0.0f;
    for (iPtr = pthis->pList_start_124; iPtr < pthis->pList_end_128; iPtr++) {
      gDPSetPrimColor((*gfx)++, 0, 0, pthis->rgb[0], pthis->rgb[1], pthis->rgb[2],
                      iPtr->fade * pthis->alpha);
      position[0] = iPtr->position[0] + pthis->unk4[0];
      position[1] = iPtr->position[1] + pthis->unk4[1];
      position[2] = iPtr->position[2] + pthis->unk4[2];

      scale[0] = iPtr->scale;
      scale[1] = iPtr->scale;
      scale[2] = iPtr->scale;
      if (0.0f != pthis->unk108) {
        particleEmitter_applyWaveEffect(pthis, iPtr->age_48, scale);
      }
      func_80344C2C(pthis->unk0_16);
      if (pthis->draw_mode & PART_EMIT_ROTATABLE) {
        func_80344720(pthis->unk34, (s32)iPtr->frame, 0, position, flat_rotation,
                      scale, gfx, mtx);
      } // L802EF2F8
      else {
        func_80344424(pthis->unk34, (s32)iPtr->frame, 0, position, scale,
                      iPtr->rotation[2], gfx, mtx);
      } // L802EF324
    } // L802EF338
    if (pthis->rgb[0] != 0xff || pthis->rgb[1] != 0xff || pthis->rgb[2] != 0xff ||
        pthis->alpha != 0xff) {
      func_8033687C(gfx);
    }
  }
}

void particleEmitter_draw(ParticleEmitter *pthis, Gfx **gdl, Mtx **mPtr,
                          Vtx **vPtr) {
  __particleEmitter_drawOnPass(pthis, gdl, mPtr, vPtr, 4);
  __particleEmitter_drawOnPass(pthis, gdl, mPtr, vPtr, 0);
}

void particleEmitter_emitInVolume(ParticleEmitter *pthis, f32 position_min[3],
                                  f32 position_max[3], s32 count) {
  for (count; count > 0; count--) {
    if (pthis->pList_end_128 < pthis->pList_capacity_12C) {
      particleEmitter_initParticle(pthis, pthis->pList_end_128);
      pthis->pList_end_128->position[0] =
          randf2(position_min[0], position_max[0]);
      pthis->pList_end_128->position[1] =
          randf2(position_min[1], position_max[1]);
      pthis->pList_end_128->position[2] =
          randf2(position_min[2], position_max[2]);
      pthis->pList_end_128++;
    }
  }
}

void particleEmitter_emitUniformLine(ParticleEmitter *pthis, f32 start[3],
                                     f32 end[3], s32 count) {
  f32 d_position[3];
  f32 temp_f0;
  s32 temp_s4;
  s32 i;

  count -= 1;
  d_position[0] = end[0] - start[0];
  d_position[1] = end[1] - start[1];
  d_position[2] = end[2] - start[2];
  for (i = 0; i <= count; i++) {
    if (pthis->pList_end_128 < pthis->pList_capacity_12C) {
      particleEmitter_initParticle(pthis, pthis->pList_end_128);
      pthis->pList_end_128->position[0] =
          start[0] + d_position[0] * ((f32)i / count);
      pthis->pList_end_128->position[1] =
          start[1] + d_position[1] * ((f32)i / count);
      pthis->pList_end_128->position[2] =
          start[2] + d_position[2] * ((f32)i / count);
      pthis->pList_end_128++;
    }
  }
}

void particleEmitter_emitN(ParticleEmitter *pthis, int n) {
  for (n; n > 0; n--) {
    if (pthis->pList_end_128 < pthis->pList_capacity_12C) {
      particleEmitter_initParticle(pthis, pthis->pList_end_128++);
    }
  }
}

int particleEmitter_getParticleCount(ParticleEmitter *pthis) {
  return (u32)(pthis->pList_end_128 - pthis->pList_start_124);
}

int particleEmitter_isDone(ParticleEmitter *pthis) {
  return (u32)(pthis->pList_end_128 - pthis->pList_start_124) < 1 &&
         pthis->doneSpawning_0_23 == TRUE;
}

void particleEmitter_free(ParticleEmitter *pthis) {
  particleEmitter_freeResources(pthis);
  bk_free(pthis);
}

ParticleEmitter *particleEmitter_new(u32 capacity) {
  ParticleEmitter *pthis =
      heap_malloc(capacity * sizeof(Particle) + sizeof(ParticleEmitter));
  f32 sp40[3];

  pthis->auto_free = 0;
  pthis->doneSpawning_0_23 = 1;
  pthis->draw_mode = 0;
  pthis->sprite_1C = NULL;
  pthis->unk0_16 = 0;
  pthis->model_20 = NULL;
  pthis->unk34 = 0;
  pthis->assetId_0_15 = 0;
  pthis->rgb[0] = 0xff;
  pthis->rgb[1] = 0xff;
  pthis->rgb[2] = 0xff;
  pthis->particleSpawnTimer_24 = 0.0f;
  pthis->spawnIntervalTimer_38 = 0.0f;
  sp40[0] = sp40[1] = sp40[2] = 0.0f;

  particleEmitter_setPositionOffset(pthis, &sp40);
  particleEmitter_setFade(pthis, 0.0f, 1.0f);
  particleEmitter_setDrawMode(pthis, 0);
  particleEmitter_setPosition(pthis, sp40);
  particleEmitter_setAccelerationRange(pthis, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                       0.0f);
  particleEmitter_setAlpha(pthis, 0xff);
  particleEmitter_setSpawnPositionRange(pthis, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f);
  particleEmitter_setParticleVelocityRange(pthis, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                           0.0f);
  particleEmitter_setSfx(pthis, 0, 0);
  particleEmitter_setBounceFactor(pthis, 0.9f);
  particleEmitter_setMinHeight(pthis, -100000.0f);
  particleEmitter_setMaxHeight(pthis, 100000.0f);
  particleEmitter_setParticleCallback(pthis, 0);
  particleEmitter_setCollisionCount(pthis, 0);
  particleEmitter_setSfxPitchRange(pthis, 1.0f, 1.0f);
  particleEmitter_setStartingFrameRange(pthis, 0, 0);
  particleEmitter_setParticleFramerateRange(pthis, 0.0f, 0.0f);
  particleEmitter_setStartingScaleRange(pthis, 1.0f, 1.0f);
  particleEmitter_setFinalScaleRange(pthis, 0.0f, 0.0f);
  particleEmitter_setAngularVelocityRange(pthis, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                          0.0f);
  particleEmitter_setSpawnIntervalRange(pthis, 0.0f, 5.0f);
  particleEmitter_setParticleLifeTimeRange(pthis, 0.0f, 5.0f);
  particleEmitter_setWaveAmplitude(pthis, 0.0f, 0.0f, 0.0f);
  particleEmitter_setWaveFrequency(pthis, 0.0f, 0.0f, 0.0f);
  particleEmitter_setWaveLength(pthis, 0.0f);
  particleEmitter_setCullDistance(pthis, 0.0f);
  pthis->unk100 = 0;
  pthis->unk104 = 0;
  pthis->pList_start_124 = (Particle *)(pthis + 1);
  pthis->pList_end_128 = (Particle *)(pthis + 1);
  pthis->pList_capacity_12C = &pthis->pList_start_124[capacity];
  return pthis;
}

void particleEmitter_setSprite(ParticleEmitter *pthis, enum asset_e sprite_id) {
  if (sprite_id != pthis->assetId_0_15) {
    pthis->assetId_0_15 = sprite_id;
    particleEmitter_freeResources(pthis);
    pthis->sprite_1C = assetCache_releaseSound(sprite_id, &pthis->unk34);
  }
}

void particleEmitter_setAccelerationRange(ParticleEmitter *pthis, f32 min_x,
                                          f32 min_y, f32 min_z, f32 max_x,
                                          f32 max_y, f32 max_z) {
  pthis->particleAccerationRange_4C_min_x = min_x;
  pthis->particleAccerationRange_4C_min_y = min_y;
  pthis->particleAccerationRange_4C_min_z = min_z;
  pthis->particleAccerationRange_4C_max_x = max_x;
  pthis->particleAccerationRange_4C_max_y = max_y;
  pthis->particleAccerationRange_4C_max_z = max_z;
}

void particleEmitter_setAlpha(ParticleEmitter *pthis, s32 arg1) {
  pthis->alpha = arg1;
}

void particleEmitter_setSfx(ParticleEmitter *pthis, enum sfx_e sfx_id,
                            s32 arg2) {
  pthis->sfx_id = sfx_id;
  pthis->unk7C = arg2;
}

void particleEmitter_setBounceFactor(ParticleEmitter *pthis, f32 arg1) {
  pthis->unk68 = arg1;
}

void particleEmitter_setMinHeight(ParticleEmitter *pthis, f32 arg1) {
  pthis->unk74 = arg1;
}

void particleEmitter_setParticleCallback(ParticleEmitter *pthis,
                                         void (*arg1)(ParticleEmitter *pthis,
                                                      f32 pos[3])) {
  pthis->particleCallback_80 = arg1;
}

void particleEmitter_setCollisionCount(ParticleEmitter *pthis, s32 arg1) {
  pthis->unk64 = arg1;
}

void particleEmitter_setSfxPitchRange(ParticleEmitter *pthis, f32 arg1,
                                      f32 arg2) {
  pthis->unk6C = arg1;
  pthis->unk70 = arg2;
}

void particleEmitter_setMaxHeight(ParticleEmitter *pthis, f32 arg1) {
  pthis->unk78 = arg1;
}

void particleEmitter_setPositionOffset(ParticleEmitter *pthis, f32 (*arg1)[3]) {
  pthis->unk4[0] = (*arg1)[0];
  pthis->unk4[1] = (*arg1)[1];
  pthis->unk4[2] = (*arg1)[2];
}

void particleEmitter_setFade(ParticleEmitter *pthis, f32 fade_in, f32 fade_out) {
  pthis->fade_in = fade_in;
  pthis->fade_out = fade_out;
}

void particleEmitter_setDrawMode(ParticleEmitter *pthis, s32 arg1) {
  pthis->draw_mode = arg1;
}

void particleEmitter_setDrawLayer(ParticleEmitter *pthis, s32 arg1) {
  pthis->unk0_16 = arg1;
}

void particleEmitter_setStartingFrameRange(ParticleEmitter *pthis, s32 arg1,
                                           s32 arg2) {
  pthis->particleStartingFrameRange_84_min = arg1;
  pthis->particleStartingFrameRange_84_max = arg2;
}

void particleEmitter_setParticleFramerateRange(ParticleEmitter *pthis, f32 arg1,
                                               f32 arg2) {
  pthis->particleFramerateRange_8C_min = arg1;
  pthis->particleFramerateRange_8C_max = arg2;
}

void particleEmitter_setUnknown(ParticleEmitter *pthis, s32 arg1, f32 arg2) {
  pthis->unk100 = arg1;
  pthis->unk104 = (s16)arg2;
}

//
void particleEmitter_setModel(ParticleEmitter *pthis, enum asset_e model_id) {
  if (pthis->assetId_0_15 != model_id) {
    pthis->assetId_0_15 = model_id;
    particleEmitter_freeResources(pthis);
    pthis->model_20 = assetcache_get(model_id);
  }
}

void particleEmitter_setSpawnPositionRange(ParticleEmitter *pthis, f32 min_x,
                                           f32 min_y, f32 min_z, f32 max_x,
                                           f32 max_y, f32 max_z) {
  pthis->particleSpawnPositionRange_94_min_x = min_x;
  pthis->particleSpawnPositionRange_94_min_y = min_y;
  pthis->particleSpawnPositionRange_94_min_z = min_z;
  pthis->particleSpawnPositionRange_94_max_x = max_x;
  pthis->particleSpawnPositionRange_94_max_y = max_y;
  pthis->particleSpawnPositionRange_94_max_z = max_z;
}

void particleEmitter_setPosition(ParticleEmitter *pthis, f32 position[3]) {
  pthis->postion_28[0] = position[0];
  pthis->postion_28[1] = position[1];
  pthis->postion_28[2] = position[2];
}

void particleEmitter_setStartingScaleRange(ParticleEmitter *pthis, f32 min,
                                           f32 max) {
  pthis->particleStartingScaleRange_AC_min = min;
  pthis->particleStartingScaleRange_AC_max = max;
}

void particleEmitter_setFinalScaleRange(ParticleEmitter *pthis, f32 min,
                                        f32 max) {
  pthis->particleFinalScaleRange_B4_min = min;
  pthis->particleFinalScaleRange_B4_max = max;
}

void particleEmitter_setScaleAndLifetimeRanges(
    ParticleEmitter *pthis, ParticleScaleAndLifetimeRanges *settings) {
  pthis->particleStartingScaleRange_AC_min = settings->unk0[0];
  pthis->particleStartingScaleRange_AC_max = settings->unk0[1];

  if (-1.0f != settings->unk8[0]) {
    pthis->particleFinalScaleRange_B4_min = settings->unk8[0];
    pthis->particleFinalScaleRange_B4_max = settings->unk8[1];
  }

  particleEmitter_setSpawnIntervalRange(pthis, settings->unk10[0],
                                        settings->unk10[1]);

  pthis->particleLifeTimeRange[0] = settings->unk18[0];
  pthis->particleLifeTimeRange[1] = settings->unk18[1];

  pthis->fade_in = settings->unk20;
  pthis->fade_out = settings->unk24;
}

void particleEmitter_applySettingsAndEmit(
    ParticleEmitter *pthis,
    ParticleSettingsScaleAndLifetimeDrawModeEmitCount *settings) {
  particleEmitter_setScaleAndLifetimeRanges(pthis,
                                            &settings->scale_and_lifetime);
  particleEmitter_setDrawMode(pthis, (s32)settings->drawmode);
  particleEmitter_emitN(pthis, (s32)settings->count);
}

void particleEmitter_setVelocityAndAccelerationRanges(
    ParticleEmitter *pthis, ParticleSettingsVelocityAcceleration *settings) {
  particleEmitter_setParticleVelocityRange(
      pthis, settings->velocity.min[0], settings->velocity.min[1],
      settings->velocity.min[2], settings->velocity.max[0],
      settings->velocity.max[1], settings->velocity.max[2]);

  particleEmitter_setAccelerationRange(
      pthis, settings->acceleration.min[0], settings->acceleration.min[1],
      settings->acceleration.min[2], settings->acceleration.max[0],
      settings->acceleration.max[1], settings->acceleration.max[2]);
}

void particleEmitter_setPositionAndVelocityRanges(
    ParticleEmitter *pthis, ParticleSettingsVelocityPosition *settings) {
  particleEmitter_setParticleVelocityRange(
      pthis, settings->velocity.min[0], settings->velocity.min[1],
      settings->velocity.min[2], settings->velocity.max[0],
      settings->velocity.max[1], settings->velocity.max[2]);

  particleEmitter_setSpawnPositionRange(
      pthis, settings->spawn_position.min[0], settings->spawn_position.min[1],
      settings->spawn_position.min[2], settings->spawn_position.max[0],
      settings->spawn_position.max[1], settings->spawn_position.max[2]);
}

void particleEmitter_setVelocityAccelerationAndPositionRanges(
    ParticleEmitter *pthis,
    ParticleSettingsVelocityAccelerationPosition *settings) {
  particleEmitter_setParticleVelocityRange(
      pthis, settings->velocity.min[0], settings->velocity.min[1],
      settings->velocity.min[2], settings->velocity.max[0],
      settings->velocity.max[1], settings->velocity.max[2]);

  particleEmitter_setAccelerationRange(
      pthis, settings->acceleration.min[0], settings->acceleration.min[1],
      settings->acceleration.min[2], settings->acceleration.max[0],
      settings->acceleration.max[1], settings->acceleration.max[2]);

  particleEmitter_setSpawnPositionRange(
      pthis, settings->spawn_position.min[0], settings->spawn_position.min[1],
      settings->spawn_position.min[2], settings->spawn_position.max[0],
      settings->spawn_position.max[1], settings->spawn_position.max[2]);
}

void particleEmitter_setAngularVelocityRange(ParticleEmitter *pthis, f32 arg1,
                                             f32 arg2, f32 arg3, f32 arg4,
                                             f32 arg5, f32 arg6) {
  pthis->unkBC[0] = arg1;
  pthis->unkBC[1] = arg2;
  pthis->unkBC[2] = arg3;
  pthis->unkC8[0] = arg4;
  pthis->unkC8[1] = arg5;
  pthis->unkC8[2] = arg6;
}

void particleEmitter_setSpawnIntervalRange(ParticleEmitter *pthis, f32 min,
                                           f32 max) {
  pthis->spawnIntervalRange_D4_min = min;
  pthis->spawnIntervalRange_D4_max = max;
  if (0.0f == pthis->particleSpawnTimer_24 || max < pthis->particleSpawnTimer_24)
    pthis->particleSpawnTimer_24 = randf2(pthis->spawnIntervalRange_D4_min,
                                         pthis->spawnIntervalRange_D4_max);
}

void particleEmitter_setParticleLifeTimeRange(ParticleEmitter *pthis, f32 min,
                                              f32 max) {
  pthis->particleLifeTimeRange[0] = min;
  pthis->particleLifeTimeRange[1] = max;
}

void particleEmitter_setParticleVelocityRange(ParticleEmitter *pthis, f32 x_min,
                                              f32 x_max, f32 y_min, f32 y_max,
                                              f32 z_min, f32 z_max) {
  pthis->sphericalParticleVelocity_48 = FALSE;
  pthis->particleVelocityRange_E4.cartisian_min_x = x_min;
  pthis->particleVelocityRange_E4.cartisian_min_y = x_max;
  pthis->particleVelocityRange_E4.cartisian_min_z = y_min;
  pthis->particleVelocityRange_E4.cartisian_max_x = y_max;
  pthis->particleVelocityRange_E4.cartisian_max_y = z_min;
  pthis->particleVelocityRange_E4.cartisian_max_z = z_max;
}

void particleEmitter_setSphericalParticleVelocityRange(
    ParticleEmitter *pthis, f32 pitch_min, f32 yaw_min, f32 radial_min,
    f32 pitch_max, f32 yaw_max, f32 radial_max) {
  pthis->sphericalParticleVelocity_48 = TRUE;
  pthis->particleVelocityRange_E4.spherical.yaw_min = yaw_min;
  pthis->particleVelocityRange_E4.spherical.yaw_max = yaw_max;
  pthis->particleVelocityRange_E4.spherical.pitch_min = pitch_min;
  pthis->particleVelocityRange_E4.spherical.pitch_max = pitch_max;
  pthis->particleVelocityRange_E4.spherical.radius_min = radial_min;
  pthis->particleVelocityRange_E4.spherical.radius_max = radial_max;
}

void particleEmitter_setCullDistance(ParticleEmitter *pthis, f32 arg1) {
  pthis->unkFC = arg1;
}

void particleEmitter_setWaveAmplitude(ParticleEmitter *pthis, f32 arg1, f32 arg2,
                                      f32 arg3) {
  pthis->unk118[0] = arg1;
  pthis->unk118[1] = arg2;
  pthis->unk118[2] = arg3;
}

void particleEmitter_setWaveFrequency(ParticleEmitter *pthis, f32 arg1, f32 arg2,
                                      f32 arg3) {
  pthis->unk10C[0] = arg1;
  pthis->unk10C[1] = arg2;
  pthis->unk10C[2] = arg3;
}

void particleEmitter_setWaveLength(ParticleEmitter *pthis, f32 arg1) {
  pthis->unk108 = arg1;
}

void particleEmitter_setRGB(ParticleEmitter *pthis, s32 arg1[3]) {
  pthis->rgb[0] = arg1[0];
  pthis->rgb[1] = arg1[1];
  pthis->rgb[2] = arg1[2];
}

void particleEmitter_setSpawnInterval(ParticleEmitter *pthis, f32 arg1) {
  pthis->doneSpawning_0_23 = FALSE;
  pthis->spawnIntervalTimer_38 = arg1;
}

void particleEmitter_noop(ParticleEmitter *pthis) { return; }

void particleEmitter_update(ParticleEmitter *pthis) {
  Particle *particle;
  f32 tick = time_getDelta();
  f32 temp_f0;
  f32 sp78[3];
  f32 sp6C[3];
  f32 sp68;

  if (partEmitMgrEnable) {
    for (particle = pthis->pList_start_124;
         particle < pthis->pList_end_128;) { // L802F005C
      particle->age_48 += tick;
      if (particle->lifetime_4C <= particle->age_48) {
        heap_memcpy(particle, --pthis->pList_end_128, sizeof(Particle));
      } else { // L802F00A0
        temp_f0 = particle->age_48 / particle->lifetime_4C;
        if (temp_f0 < pthis->fade_in)
          particle->fade = temp_f0 / pthis->fade_in;
        else if (temp_f0 <= pthis->fade_out)
          particle->fade = 1.0f;
        else {
          particle->fade =
              1.0f - ((temp_f0 - pthis->fade_out) / (1.0f - pthis->fade_out));
        } // L802F00F0
        particle->scale =
            particle->initialSize_34 + temp_f0 * particle->finalSizeDiff;

        if (pthis->sprite_1C) {
          particle->frame += particle->framerate * tick;
          if (!((s32)particle->frame < sprite_getFrameCount(pthis->sprite_1C))) {
            if (pthis->draw_mode & PART_EMIT_NO_LOOP) {
              particle->frame = sprite_getFrameCount(pthis->sprite_1C) - 1;
            } else {
              particle->frame = 0.0f;
            }
          }
        } // L802F0180

        particle->position[0] += particle->velocity_50[0] * tick;
        particle->position[1] += particle->velocity_50[1] * tick;
        particle->position[2] += particle->velocity_50[2] * tick;

        particle->rotation[0] += particle->angluar_velocity[0] * tick;
        particle->rotation[1] += particle->angluar_velocity[1] * tick;
        particle->rotation[2] += particle->angluar_velocity[2] * tick;

        particle->velocity_50[0] =
            particle->velocity_50[0] + particle->acceleration[0] * tick;
        particle->velocity_50[1] =
            particle->velocity_50[1] + particle->acceleration[1] * tick;
        particle->velocity_50[2] =
            particle->velocity_50[2] + particle->acceleration[2] * tick;

        if (pthis->unk100) {
          particle->position[1] = getCollisionHeightFromWaterSurface(pthis->unk100) + pthis->unk104;
        } // L802F0254

        if (0.0f != pthis->unkFC &&
            !viewport_isPointWithinDistance(&particle->position, pthis->unkFC)) {
          heap_memcpy(particle, --pthis->pList_end_128, sizeof(Particle));
        } else { // L802F029C
          if (particle->unk5C > 0) {
            sp6C[0] = particle->position[0];
            sp6C[1] = particle->position[1] + 50.0f;
            sp6C[2] = particle->position[2];
            if (particleEmitter_checkCollision(pthis, &sp6C, &particle->position,
                                               &sp78, 0)) {
              if (pthis->sfx_id) {
                sp68 = mlAbsF(particle->velocity_50[1]) / 10.0;
                if (1.0f < sp68) {
                  sp68 = 1.0f;
                } // L802F0324
                if (particleSfxTimer == 0.0f) {
                  func_8030E6A4(pthis->sfx_id, randf2(pthis->unk6C, pthis->unk70),
                                (s32)((f32)pthis->unk7C * sp68));
                  particleSfxTimer = 0.25f;
                }
              } // L802F0384
              particle->position[1] += 2.0f;
              particle->velocity_50[1] =
                  mlAbsF(particle->velocity_50[1]) * pthis->unk68;
              if ((pthis->draw_mode & 0x2) == 0) {
                particle->initialSize_34 *= pthis->unk68;
                particle->finalSizeDiff *= pthis->unk68;
              } // L802F03DC

              particle->angluar_velocity[0] *= pthis->unk68;
              particle->angluar_velocity[1] *= pthis->unk68;
              particle->angluar_velocity[2] *= pthis->unk68;
              if (--particle->unk5C == 0) {
                if (pthis->particleCallback_80)
                  pthis->particleCallback_80(pthis, particle->position);
                heap_memcpy(particle, --pthis->pList_end_128, sizeof(Particle));
                continue;
              }
            }
          }
          particle++;
        }
      } // L802F045C
    } // L802F0468
    if (0.0f < pthis->spawnIntervalTimer_38) { // if exactly 0.0f (no update)
      pthis->spawnIntervalTimer_38 -= tick;
      if (pthis->spawnIntervalTimer_38 <= 0.0f) // only can stop spawning
        pthis->doneSpawning_0_23 = TRUE;
    }

    if (pthis->doneSpawning_0_23 != TRUE) {
      pthis->particleSpawnTimer_24 -= tick;
      if (pthis->particleSpawnTimer_24 <= 0.0f) {
        pthis->particleSpawnTimer_24 = randf2(pthis->spawnIntervalRange_D4_min,
                                             pthis->spawnIntervalRange_D4_max);
        if (pthis->pList_end_128 < pthis->pList_capacity_12C)
          particleEmitter_initParticle(pthis, pthis->pList_end_128++);
      }
    }
  } // L802F0514
}

void particleEmitter_setupSmoke(ParticleEmitter *pthis, f32 arg1[3]) {
  static s32 D_803689B8[3] = {0xAF, 0x87, 0};
  particleEmitter_setSprite(pthis, ASSET_70E_SPRITE_SMOKE_2);
  particleEmitter_setRGB(pthis, D_803689B8);
  particleEmitter_setFade(pthis, 0.0f, 0.1f);
  particleEmitter_setStartingFrameRange(pthis, 0, 7);
  particleEmitter_setSpawnPositionRange(pthis, -80.0f, 0.0f, -80.0f, 80.0f,
                                        60.0f, 80.0f);
  particleEmitter_setPosition(pthis, arg1);
  particleEmitter_setStartingScaleRange(pthis, 1.0f, 1.0f);
  particleEmitter_setFinalScaleRange(pthis, 2.0f, 3.0f);
  particleEmitter_setSpawnIntervalRange(pthis, 0.0f, 0.01f);
  particleEmitter_setParticleLifeTimeRange(pthis, 3.0f, 4.0f);
  particleEmitter_setParticleVelocityRange(pthis, -200.0f, 0.0f, -200.0f, 200.0f,
                                           100.0f, 200.0f);
}

void particleEmitter_setupGoldRock(ParticleEmitter *pthis, f32 position[3]) {
  particleEmitter_setAccelerationRange(pthis, 0.0f, -800.0f, 0.0f, 0.0f, -800.0f,
                                       0.0f);
  particleEmitter_setBounceFactor(pthis, 0.6f);
  particleEmitter_setCollisionCount(pthis, 3);
  particleEmitter_setModel(pthis, ASSET_896_MODEL_GOLD_ROCK);
  particleEmitter_setSpawnPositionRange(pthis, -120.0f, -60.0f, -120.0f, 120.0f,
                                        60.0f, 120.0f);
  particleEmitter_setPosition(pthis, position);
  particleEmitter_setStartingScaleRange(pthis, 0.2f, 0.3f);
  particleEmitter_setAngularVelocityRange(pthis, -300.0f, -300.0f, -300.0f,
                                          300.0f, 300.0f, 300.0f);
  particleEmitter_setSpawnIntervalRange(pthis, 0.0f, 0.01f);
  particleEmitter_setParticleLifeTimeRange(pthis, 10.0f, 10.0f);
  particleEmitter_setParticleVelocityRange(pthis, -500.0f, 150.0f, -500.0f,
                                           500.0f, 400.0f, 500.0f);
}

void partEmitMgr_init(void) {
  partEmitMgr = (ParticleEmitter **)heap_malloc(0);
  partEmitMgrLength = 0;
}

void partEmitMgr_free(void) {
  int i;
  for (i = 0; i < partEmitMgrLength; i++) {
    particleEmitter_free(partEmitMgr[i]);
  }
  bk_free(partEmitMgr);
  partEmitMgr = NULL;
  partEmitMgrLength = 0;
}

void partEmitMgr_noop(void) { return; }

void partEmitMgr_update(void) {
  int i;
  ParticleEmitter *iPtr;

  particleSfxTimer = MAX(0.0, particleSfxTimer - time_getDelta());
  if (partEmitMgr) {
    for (i = 0; i < partEmitMgrLength; i++) {
      iPtr = partEmitMgr[i];
      particleEmitter_update(iPtr);
      if (iPtr->auto_free && iPtr->doneSpawning_0_23 == TRUE &&
          iPtr->pList_end_128 == iPtr->pList_start_124) {
        iPtr->dead = TRUE;
      } else {
        iPtr->dead = FALSE;
      }
    } // L802F09C0
    for (i = 0; i < partEmitMgrLength;) {
      if (partEmitMgr[i]->dead) {
        partEmitMgr_freeEmitter(partEmitMgr[i]);
      } else {
        i++;
      }
    }
  } // L802F0A14
}

void partEmitMgr_drawPass0(Gfx **gdl, Mtx **mptr, Vtx **vptr) {
  int i;
  for (i = 0; i < partEmitMgrLength; i++) {
    __particleEmitter_drawOnPass(partEmitMgr[i], gdl, mptr, vptr, 4);
  }
}

void partEmitMgr_drawPass1(Gfx **gdl, Mtx **mptr, Vtx **vptr) {
  int i;
  for (i = 0; i < partEmitMgrLength; i++) {
    __particleEmitter_drawOnPass(partEmitMgr[i], gdl, mptr, vptr, 0);
  }
}

void partEmitMgr_draw(Gfx **gdl, Mtx **mptr, Vtx **vptr) {
  partEmitMgr_drawPass0(gdl, mptr, vptr);
  partEmitMgr_drawPass1(gdl, mptr, vptr);
}

ParticleEmitter *partEmitMgr_newEmitter(u32 cnt) {
  partEmitMgr = bk_realloc(partEmitMgr, (++partEmitMgrLength) * 4);
  partEmitMgr[partEmitMgrLength - 1] = particleEmitter_new(cnt);
  partEmitMgr[partEmitMgrLength - 1]->auto_free = TRUE;
  return partEmitMgr[partEmitMgrLength - 1];
}

void partEmitMgr_freeEmitter(ParticleEmitter *pthis) {
  int i = 0;
  while (pthis != partEmitMgr[i] && i < partEmitMgrLength) {
    i++;
  }
  if (i == partEmitMgrLength)
    return;

  particleEmitter_free(pthis);
  partEmitMgr[i] = partEmitMgr[partEmitMgrLength - 1];
  partEmitMgrLength--;
  partEmitMgr =
      bk_realloc(partEmitMgr, partEmitMgrLength * sizeof(ParticleEmitter *));
}

void particleEmitter_manualFree(ParticleEmitter *pthis) {
  pthis->auto_free = FALSE;
}

void particleEmitter_autoFree(ParticleEmitter *pthis) { pthis->auto_free = TRUE; }

ParticleEmitter *partEmitMgr_defragEmitter(ParticleEmitter *pthis) {
  int i;
  s32 a3;

  if (pthis) {
    a3 = (s32)pthis;
    i = 0;
    while (partEmitMgr[i] != pthis && i < partEmitMgrLength) {
      i++;
    }
    pthis = (ParticleEmitter *)defrag(pthis);
    pthis->pList_start_124 =
        (Particle *)((s32)pthis + (u32)((s32)pthis->pList_start_124 - a3));
    pthis->pList_end_128 =
        (Particle *)((s32)pthis + (u32)((s32)pthis->pList_end_128 - a3));
    pthis->pList_capacity_12C =
        (Particle *)((s32)pthis + (u32)((s32)pthis->pList_capacity_12C - a3));
    if (i < partEmitMgrLength) {
      partEmitMgr[i] = pthis;
    }

  } // L802F0E44
  return pthis;
}

void partEmitMgr_defrag(void) {
  partEmitMgr = (ParticleEmitter **)defrag(partEmitMgr);
}

void partEmitMgr_enable(void *arg0, s32 arg1) {
  partEmitMgrEnable = BOOL(arg1 == 2);
}

void particleEmitter_setYPosition(ParticleEmitter *pthis, f32 arg1) {
  Particle *iPtr;
  pthis->postion_28[1] = arg1;
  for (iPtr = pthis->pList_start_124; iPtr < pthis->pList_end_128; iPtr++)
    iPtr->position[1] = arg1;
}
