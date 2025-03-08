#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

//sfxManager.c

s16 *sfxSourceIndices = NULL;

void sfx_alloc(void) {
  int i;

  if (sfxSourceIndices)
    return;
  sfxSourceIndices = (s16 *)heap_malloc(10 * sizeof(s16));
  for (i = 0; i < 10; i++) {
    sfxSourceIndices[i] = 0;
  }
}

void sfx_release(void) {
  int i;

  if (!sfxSourceIndices)
    return;
  for (i = 0; i < 10; i++) {
    if (sfxSourceIndices[i])
      func_8030E394(sfxSourceIndices[i]);
  }
  bk_free(sfxSourceIndices);
  sfxSourceIndices = NULL;
}

void sfx_initSfxSource(s32 idx, s32 lookup_idx, s32 sample_rate, f32 volume) {
  u8 sfx_source_index;

  if (func_8030ED70(lookup_getSfxId(lookup_idx))) {
    sfx_source_index = sfxsource_createSfxsourceAndReturnIndex();
    if (sfx_source_index) {
      sfxsource_setSfxId(sfx_source_index, lookup_getSfxId(lookup_idx));
      sfxsource_playSfxAtVolume(sfx_source_index, volume);
      sfxsource_setSampleRate(sfx_source_index, sample_rate);
      func_8030E2C4(sfx_source_index);
      sfx_freeSfxSource(idx);
      sfxSourceIndices[idx] = sfx_source_index;
    }
  } else {
    func_8030E6A4(lookup_getSfxId(lookup_idx), volume, sample_rate);
  }
}

void sfx_freeSfxSource(int idx) {
  if (sfxSourceIndices[idx]) {
    sfxsource_freeSfxsourceByIndex(sfxSourceIndices[idx]);
  }
  sfxSourceIndices[idx] = 0;
}
