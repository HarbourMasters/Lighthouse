#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#include "animation.h"
#include "assets.h"

extern f32 glspline_catmull_rom_interpolate(f32, s32, f32 *);
extern BKSpriteDisplayData *func_80344A1C(BKSprite_s *arg0);
f32 animationInterpolationData[] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
s32 assetCacheCurrentSize = 0;
u8 assetCacheLength = 0; // assetCache_size;
u8 assetCacheCurrentIndex = 0;
u8 assetCacheFlag = 0;

/* .bss */
s32 assetCacheTempBuffer;
u8 pad_80383CB8[0x8];
AssetROMHead *assetRomHeader;
AssetFileMeta *assetRomMetaList;
u32 assetRomStartOffset;
s32 assetRomDataOffset;    // asset_data_rom_offset
void **assetCachePointers; // assetCache_ptrs;
BKSpriteDisplayData **assetCacheDisplayData;
u8 *assetCacheDependencies; // assetCache_dependencies;
s16 *assetCacheIds;         // assetCache_indexs
bk_vector(struct21s) * assetCacheDependencyLists[2];

/* .public */
extern s32 assetcache_release(void *arg0);

f32 animationFile_getTime(AnimationFile *anim_file, f32 arg1);
f32 animationFile_interpolate(AnimationFile *anim_file,
                              AnimationFileElement *arg1, f32 arg2);
s32 animationFile_getEndFrame(AnimationFile *this);
void boneTransformList_applyTransform(BoneTransformList *arg0, s32 arg1,
                                      f32 arg2[3][3]);
void assetCache_read(enum asset_e asset_id, s32 offset, s32 size,
                     void *dst_ptr);

/* .core2 */
f32 animationFile_getProgress(AnimationFile *this, s32 arg1) {
  if (arg1 == this->unk2)
    return 0.999999f;
  return (f32)(arg1 - this->unk0) / (f32)(this->unk2 - this->unk0);
}

void animationFile_getBoneTransformList(
    AnimationFile *anim_file, f32 progress,
    BoneTransformList *bone_transform_list) {
  s32 bone_id;
  int i;
  f32 tmp_f22;
  AnimationFileElement *tmp_s0;
  f32 sp54[3][3];

  tmp_f22 = animationFile_getTime(anim_file, progress);
  tmp_s0 = (AnimationFileElement *)((s32)anim_file + sizeof(AnimationFile));
  bone_id = 0;
  for (i = 0; i < anim_file->elem_cnt; i++) { // L8033AAB8
    if (tmp_s0->unk0_15 != bone_id) {
      if (bone_id != 0)
        boneTransformList_applyTransform(bone_transform_list, bone_id, sp54);
      bone_id = tmp_s0->unk0_15;
      sp54[0][0] = sp54[0][1] = sp54[0][2] = 0.0f;
      sp54[1][0] = sp54[1][1] = sp54[1][2] = 1.0f;
      sp54[2][0] = sp54[2][1] = sp54[2][2] = 0.0f;
    }
    sp54[0][tmp_s0->unk0_3] =
        animationFile_interpolate(anim_file, tmp_s0, tmp_f22);
    tmp_s0 += tmp_s0->data_cnt;
    tmp_s0++;
  } // L8033AB60
  boneTransformList_applyTransform(bone_transform_list, bone_id, sp54);
}

f32 animationFile_getTime(AnimationFile *this, f32 arg1) {
  return this->unk0 + arg1 * (this->unk2 - this->unk0);
}

f32 animationFile_getNormalizedTime(AnimationFile *this) {
  f32 tmp = animationFile_getEndFrame(this);
  return (tmp - 1.0) / tmp;
}

s32 animationFile_getEndFrame(AnimationFile *this) { return this->unk2; }

s32 animationFile_getStartFrame(AnimationFile *this) { return this->unk0; }

s32 animationFile_getFrameCount(AnimationFile *this) {
  return this->unk2 - this->unk0 + 1;
}

s32 animationFile_count(AnimationFile *this) { return this->elem_cnt; }

f32 animationFile_interpolate(AnimationFile *this, AnimationFileElement *elem,
                              f32 time) {
  AnimationFileData *end_anim;
  AnimationFileData *start_anim;
  AnimationFileData *var_v0;
  f32 temp_f12;
  f32 knot_list[4];
  u32 temp_t2;

  start_anim = &elem->data[0];
  if ((s32)time < start_anim->unk0_13) {
    knot_list[0] = knot_list[1] = animationInterpolationData[elem->unk0_3];
    knot_list[2] = (f32)start_anim->unk2 / 64;
    knot_list[3] = (start_anim->unk0_15 == 1 && elem->data_cnt >= 2)
                       ? (f32)(start_anim + 1)->unk2 / 64
                       : knot_list[2];
    return glspline_catmull_rom_interpolate(
        (time - this->unk0) / (start_anim->unk0_13 - this->unk0), 4, knot_list);
  }
  end_anim = start_anim + elem->data_cnt;
  end_anim--;
  if ((s32)time >= (end_anim->unk0_13)) {
    knot_list[1] = (f32)end_anim->unk2 / 64;
    knot_list[0] = ((end_anim->unk0_14 == 1) && (elem->data_cnt >= 2))
                       ? (f32)(end_anim - 1)->unk2 / 64
                       : knot_list[1];
    knot_list[2] = knot_list[3] = knot_list[1];

    return glspline_catmull_rom_interpolate(time - end_anim->unk0_13, 4,
                                            knot_list);
  }

  var_v0 = start_anim + 1;
  while (var_v0 < end_anim) {
    var_v0 = &start_anim[(end_anim - start_anim) / 2];
    if (var_v0->unk0_13 <= (s32)time) {
      start_anim = var_v0;
      if (!start_anim)
        ;
    } else {
      end_anim = var_v0;
    }
    var_v0 = start_anim + 1;
  }

  knot_list[1] = (f32)start_anim->unk2 / 64;
  knot_list[2] = (f32)end_anim->unk2 / 64;
  temp_f12 =
      (time - start_anim->unk0_13) / (end_anim->unk0_13 - start_anim->unk0_13);
  if ((start_anim->unk0_14 == 0) && (end_anim->unk0_15 == 0)) {
    return knot_list[1] + ((knot_list[2] - knot_list[1]) * temp_f12);
  }

  knot_list[0] =
      (start_anim->unk0_14 == 1 && (start_anim - 1) >= &elem->data[0])
          ? (f32)(start_anim - 1)->unk2 / 64
          : knot_list[1];
  knot_list[3] =
      (end_anim->unk0_15 == 1 && (end_anim + 1) < &elem->data[elem->data_cnt])
          ? (f32)(end_anim + 1)->unk2 / 64
          : knot_list[2];
  return glspline_catmull_rom_interpolate(temp_f12, 4, knot_list);
}

void boneTransformList_applyTransform(BoneTransformList *bone_transform_list,
                                      s32 bone_id, f32 arg2[3][3]) {
  f32 sp18[4];
  func_80345CD4(sp18, arg2[0]);
  func_8033A8F0(bone_transform_list, bone_id, sp18);
  boneTransformList_setBoneScale(bone_transform_list, bone_id, arg2[1]);
  func_8033A968(bone_transform_list, bone_id, arg2[2]);
}

void assetCache_incrementDependency(void *ptr) {
  struct21s *start_ptr;
  struct21s *end_ptr;
  struct21s *iPtr;

  end_ptr = (struct21s *)vector_getEnd(assetCacheDependencyLists[0]);
  start_ptr = (struct21s *)vector_getBegin(assetCacheDependencyLists[0]);

  for (iPtr = start_ptr; iPtr < end_ptr && ptr != iPtr->unk1; iPtr++)
    ;

  if (iPtr < end_ptr) {
    iPtr->unk0++;
  } else {
    iPtr = (struct21s *)vector_pushBackNew(&assetCacheDependencyLists[0]);
    iPtr->unk0 = 1;
    iPtr->unk1 = ptr;
  }
}

bool assetCache_hasDependency(void *arg0) {
  struct21s *start_ptr;
  struct21s *end_ptr;
  struct21s *iPtr;
  s32 j;

  for (j = 0; j < 2; j++) {
    end_ptr = (struct21s *)vector_getEnd(assetCacheDependencyLists[j]);
    start_ptr = (struct21s *)vector_getBegin(assetCacheDependencyLists[j]);
    for (iPtr = start_ptr; iPtr < end_ptr && arg0 != iPtr->unk1; iPtr++) {
    }
    if (iPtr < end_ptr) {
      return TRUE;
    }
  }
  return FALSE;
}

void assetCache_initDependencyLists(void) {
  assetCacheDependencyLists[0] = vector_new(sizeof(struct21s), 0x10);
  assetCacheDependencyLists[1] = vector_new(sizeof(struct21s), 0x10);
}

void assetCache_clearDependencies(void) {
  bk_vector(struct21s) * tmp_a0;
  struct21s *iPtr;
  struct21s *start_ptr;
  struct21s *endPtr;
  int i;

  tmp_a0 = assetCacheDependencyLists[0];
  assetCacheDependencyLists[0] = assetCacheDependencyLists[1];
  assetCacheDependencyLists[1] = tmp_a0;

  endPtr = (struct21s *)vector_getEnd(assetCacheDependencyLists[0]);
  start_ptr = (struct21s *)vector_getBegin(assetCacheDependencyLists[0]);
  for (iPtr = start_ptr; iPtr < endPtr; iPtr++) {
    for (i = 0; i < iPtr->unk0; i++)
      assetcache_release(iPtr->unk1);
  }

  vector_clear(assetCacheDependencyLists[0]);
}

void assetCache_defragDependencyLists(void) {
  assetCacheDependencyLists[0] =
      (bk_vector(struct21s) *)defrag(assetCacheDependencyLists[0]);
  assetCacheDependencyLists[1] =
      (bk_vector(struct21s) *)defrag(assetCacheDependencyLists[1]);
}

void assetCache_allocate(s32 arg0) {
  assetCachePointers[assetCacheLength] = heap_malloc(arg0);
  assetCacheDisplayData[assetCacheLength] = NULL;
  assetCacheDependencies[assetCacheLength] = 1;
  assetCacheIds[assetCacheLength] = -1;
  assetCacheLength += 1;
}

bool assetCache_releaseSprite(void **sprite_ptr, BKSpriteDisplayData **arg1) {
  void *new_var;
  if ((*sprite_ptr) == NULL)
    return 0;

  new_var = *sprite_ptr;
  assetcache_release(new_var);
  *sprite_ptr = 0;
  *arg1 = 0;
  return 1;
}

bool assetCache_releaseBKSprite(BKSprite_s **sprite_ptr,
                                BKSpriteDisplayData **arg1) {
  if (*sprite_ptr == NULL)
    return FALSE;

  assetCache_incrementDependency(*sprite_ptr);
  *sprite_ptr = NULL;
  *arg1 = NULL;

  if (sprite_ptr)
    ;

  return TRUE;
}

s32 assetcache_release(void *arg0) {
  s32 i;
  if (arg0) {
    for (i = 0; i < assetCacheLength && arg0 != assetCachePointers[i]; i++)
      ;

    if (i == assetCacheLength)
      return 2;

    assetCacheCurrentIndex = i;
    if (assetCacheDependencies[i] == 1) {
      if (assetCacheDisplayData[i])
        func_803449DC(assetCacheDisplayData[i]);
      bk_free(arg0);
      assetCacheLength--;
      assetCacheDependencies[i] = assetCacheDependencies[assetCacheLength];
      assetCachePointers[i] = assetCachePointers[assetCacheLength];
      assetCacheDisplayData[i] = assetCacheDisplayData[assetCacheLength];
      assetCacheIds[i] = assetCacheIds[assetCacheLength];
      return 0;
    } else {
      assetCacheDependencies[i]--;
      return 1;
    }
  } else {
    return 3;
  }
}

void assetcache_update_ptr(void *arg0, void *arg1) {
  s32 i;

  if ((arg0 == NULL) || (arg1 == NULL) || (arg0 == arg1))
    return;

  for (i = 0; i < assetCacheLength && arg0 != assetCachePointers[i]; i++)
    ;

  if (i != assetCacheLength && arg1 != assetCachePointers[i])
    assetCachePointers[i] = arg1;
}

void assetCache_defrag(void) { assetCache_defragDependencyLists(); }

void assetCache_clearAllDependencies(void) {
  sendGfxTaskMessage();
  assetCache_clearDependencies();
  assetCache_clearDependencies();
}

s32 asset_getFlag(enum asset_e arg0) { return assetRomMetaList[arg0].unk6; }

s32 assetSection_getCount(void) { return assetRomHeader->count - 1; }

s32 assetCache_releaseTexture(void) { return assetCacheCurrentSize; }

s32 asset_getSize(s32 arg0) {
  return assetRomMetaList[arg0 + 1].offset - assetRomMetaList[arg0].offset;
}

bool asset_isCompressed(enum asset_e arg0) { // asset_compressed?
  return (assetRomMetaList[arg0].compFlag & 1) != 0;
}

// returns raw sprite(as saved in ROM) and points arg1 to a parsed sprite(?)
BKSprite_s *assetCache_releaseSound(enum asset_e sprite_id,
                                  BKSpriteDisplayData **arg1) {
  BKSprite_s *s0;
  s0 = assetcache_get(sprite_id);
  if (assetCacheDisplayData[assetCacheCurrentIndex] == NULL) {
    func_803382E4(-1);
    func_80338308(sprite_getUnk8(s0), sprite_getUnkA(s0));
    assetCacheDisplayData[assetCacheCurrentIndex] = func_80344A1C(s0);
  }
  *arg1 = assetCacheDisplayData[assetCacheCurrentIndex];
  return s0;
}

void assetCache_releaseAnimation(void) { assetCacheFlag = 1; }

void *assetcache_get(enum asset_e assetId) {
  s32 comp_size; // sp_44
  s32 i;
  volatile s32 sp3C;       // sp3C
  s32 uncomp_size;         // sp38
  void *uncompressed_file; // sp34
  u8 sp33;                 // sp33
  void *compressed_file;   // sp2C
  s32 sp28;                // sp28

  sp28 = (s32)assetCacheFlag;
  assetCacheFlag = (u8)0U;
  for (i = 0; i < assetCacheLength && assetId != assetCacheIds[i]; i++)
    ;
  assetCacheCurrentIndex = i;
  if (i == 0x96)
    return NULL;

  if (i < assetCacheLength) { // asset exists in array;
    assetCacheDependencies[i]++;
    return assetCachePointers[i];
  }
  comp_size =
      assetRomMetaList[assetId + 1].offset - assetRomMetaList[assetId].offset;
  if (comp_size & 1)
    comp_size++;
  sp3C = comp_size;

  if (assetRomMetaList[assetId].compFlag & 0x0001) { // compressed
    assetCache_read(assetId, 0, 0x10, &assetCacheTempBuffer);
    assetCacheCurrentSize =
        rarezip_get_uncompressed_size(&assetCacheTempBuffer);
    uncomp_size = assetCacheCurrentSize;
    if (uncomp_size & 0xF) {
      uncomp_size -= uncomp_size & 0xF;
      uncomp_size += 0x10;
    }

    if (heap_hasFreeBlock((u32)comp_size + uncomp_size) && !sp28) {
      sp33 = 1;
      uncompressed_file = heap_malloc((u32)comp_size + uncomp_size);
      compressed_file = (void *)((s32)uncompressed_file + uncomp_size);
    } else {
      sp33 = 2;
      if (sp28 != 0) {
        heap_setFlagTrue();
      }
      uncompressed_file = heap_malloc(uncomp_size);
      compressed_file = heap_malloc(comp_size);
    }
  } else { // uncompressed
    uncompressed_file = heap_malloc(comp_size);
    compressed_file = uncompressed_file;
  }
  piMgr_read(compressed_file,
             assetRomMetaList[assetId].offset + assetRomDataOffset, sp3C);
  if (assetRomMetaList[assetId].compFlag & 0x0001) { // decompress
    rarezip_inflate(compressed_file, uncompressed_file);
    bk_realloc(uncompressed_file, assetCacheCurrentSize);
    osWritebackDCache(uncompressed_file, assetCacheCurrentSize);
    if (sp33 == 2) {
      bk_free(compressed_file);
    }
  }
  assetCacheCurrentIndex = assetCacheLength;
  assetCacheDependencies[assetCacheLength] = 1;
  assetCachePointers[assetCacheLength] = uncompressed_file;
  assetCacheDisplayData[assetCacheLength] = 0;
  assetCacheIds[assetCacheLength] = assetId;
  assetCacheLength++;
  return uncompressed_file;
}

void assetCache_read(enum asset_e asset_id, s32 offset, s32 size,
                     void *dst_ptr) {
  piMgr_read(dst_ptr,
             assetRomMetaList[asset_id].offset + assetRomDataOffset + offset,
             size);
}

void assetCache_resizeAsset(void *assetPtr, s32 size) {
  s32 tmp;
  s32 i;

  for (i = 0; i < assetCacheLength && assetPtr != assetCachePointers[i]; i++)
    ;
  assetCachePointers[i] = bk_realloc(assetPtr, size);
}

void assetCache_init(void) {
  #ifndef LIGHTHOUSE_P
  assetCacheFlag = 0;
  assetCache_initDependencyLists();
  assetCachePointers = (void **)heap_malloc(150 * sizeof(void *));
  assetCacheDisplayData = heap_malloc(600);
  assetCacheDependencies = (u8 *)heap_malloc(150 * sizeof(u8));
  assetCacheIds = (s16 *)heap_malloc(150 * sizeof(s16));
  assetCacheLength = 0;
  assetRomHeader = (AssetROMHead *)heap_malloc(sizeof(AssetROMHead));
  assetRomStartOffset = (u32)assets_ROM_START;
  piMgr_read(assetRomHeader, assetRomStartOffset, sizeof(AssetROMHead));
  assetRomMetaList =
      (AssetFileMeta *)heap_malloc(assetRomHeader->count * sizeof(AssetFileMeta));
  piMgr_read(assetRomMetaList, assetRomStartOffset + sizeof(AssetROMHead),
             assetRomHeader->count * sizeof(AssetFileMeta));
  assetRomDataOffset = assetRomStartOffset + sizeof(AssetROMHead) +
                       assetRomHeader->count * sizeof(AssetFileMeta);
  #endif
}

s32 asset_getCompressedSize(enum asset_e arg0) {
  return assetRomMetaList[arg0 + 1].offset - assetRomMetaList[arg0].offset;
}

s32 assetCache_getDependencyCount(enum asset_e arg0) {
  s32 i;

  for (i = 0; i < assetCacheLength && arg0 != assetCacheIds[i]; i++)
    ;
  if (i < assetCacheLength) {
    return assetCacheDependencies[i];
  }
  return 0;
}

void assetCache_releaseBKModelBin(BKModelBin **arg0) {
  assetCache_incrementDependency(*arg0);
  *arg0 = NULL;
}

void assetCache_free(void *arg0) { assetCache_incrementDependency(arg0); }

void assetCache_clear(void) { assetCache_clearDependencies(); }

void assetCache_checkDependency(void *arg0) { assetCache_hasDependency(arg0); }

s32 assetCache_loadAsset(enum asset_e id, void *dst, s32 size) {
  s32 comp_size;
  s32 var_s0;
  s32 sp34;
  s32 phi_v0;
  s32 comp_ptr;
  u8 sp2B;
  s32 sp20;

  // find asset in cache
  for (phi_v0 = 0; phi_v0 < assetCacheLength && id != assetCacheIds[phi_v0];
       phi_v0++)
    ;
  assetCacheCurrentIndex = phi_v0;
  if (phi_v0 == 150) { // asset not in cache
    return 0;
  }
  comp_ptr = assetRomMetaList[id + 1].offset - assetRomMetaList[id].offset;
  if (comp_ptr & 1) {
    comp_ptr++;
  }
  sp34 = comp_ptr;

  if (assetRomMetaList[id].compFlag & 1) {
    assetCache_read(id, 0, 0x10, &assetCacheTempBuffer);
    assetCacheCurrentSize =
        rarezip_get_uncompressed_size(&assetCacheTempBuffer);

    // get aligned uncompressed size
    var_s0 = assetCacheCurrentSize;
    if (var_s0 & 0xF) {
      var_s0 = (var_s0 - (var_s0 & 0xF)) + 0x10;
    }

    if (size >= (comp_ptr + var_s0)) {
      sp2B = 1;
      comp_ptr = (s32)dst + var_s0;
    } else if (size >= var_s0) {
      sp2B = 2;
      comp_ptr = (s32)heap_malloc(comp_ptr);
    } else {
      return 0;
    }
  } else {
    var_s0 = comp_ptr;
    if (comp_ptr & (0x10 - 1))
      var_s0 = (comp_ptr - (comp_ptr & (0x10 - 1))) + 0x10;

    if (size >= comp_ptr) {
      comp_ptr = (s32)dst;
    } else {
      return 0;
    }
  }
  comp_size = assetRomMetaList[id].offset + assetRomDataOffset;
  piMgr_read((void *)comp_ptr, comp_size, sp34);
  if (assetRomMetaList[id].compFlag & 1) {
    rarezip_inflate(comp_ptr, dst);
    osWritebackDCache(dst, assetCacheCurrentSize);
    if (sp2B == 2) {
      bk_free((void *)comp_ptr);
    }
  }
  return var_s0;
}
