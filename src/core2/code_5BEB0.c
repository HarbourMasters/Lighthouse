#include "functions.h"
#include "variables.h"
#include <ultra64.h>

//map_savestate.c

typedef struct map_savestate_s {
  u32 flags;
} MapSavestate;

/* .bss */
s32 mapSavestateArray[0x9A];

/* public functions */
void game_setFramebufferIndex(s32 frame_buffer_indx);
int game_isFrozen(void);

/* .code */
void mapSavestate_init(void) {
  int i;
  for (i = 0; i < 0x9A; i++) {
    mapSavestateArray[i] = 0;
  }
}

void mapSavestate_free_all(void) {
  int i;
  for (i = 0; i < 0x9A; i++) {
    if ((u32 *)mapSavestateArray[i] != NULL) {
      bk_free((void *)mapSavestateArray[i]);
      mapSavestateArray[i] = NULL;
    }
  }
}

void mapSavestate_defrag_all(void) {
  int i;
  for (i = 0; i < 0x9A; i++) {
    if (mapSavestateArray[i]) {
      mapSavestateArray[i] = defrag(mapSavestateArray[i]);
    }
  }
}

void mapSavestate_save(enum map_e map) {
  u32 wSize;
  s32 iBit;
  s32 reg_s4;
  u32 *valPtr;
  int new_var;
  wSize = 4;
  if (mapSavestateArray[map] != 0) {
    bk_free(mapSavestateArray[map]);
  }
  mapSavestateArray[map] = (MapSavestate *)heap_malloc(4 * (sizeof(u32)));
  valPtr = (u32 *)mapSavestateArray[map];
  *valPtr = mapSpecificFlags_getAll();
  iBit = 0x20;
  sortCubeProps(1);
  iterateOverAllCubeProps(-1);
  for (reg_s4 = iterateOverAllCubeProps(-2); reg_s4 != (-1); reg_s4 = iterateOverAllCubeProps(-2)) {
    new_var = sizeof(u32);
    if (!(iBit < ((wSize * (sizeof(u32))) * 8))) {
      wSize += 4;
      mapSavestateArray[map] =
          (MapSavestate *)bk_realloc(mapSavestateArray[map], wSize * new_var);
      valPtr = ((s32)mapSavestateArray[map]) + (wSize * new_var);
      valPtr[-1] = 0;
      new_var = 1;
      valPtr[-2] = 0;
      valPtr[-3] = 0;
      if (1)
        if (1)
          if (1)
            if (1)
              if (1)
                if (1)
                  if (1)
                    ;
      valPtr[-4] = 0;
    }
    valPtr = mapSavestateArray[map];
    valPtr[iBit >> 5] =
        (reg_s4) ? (valPtr[iBit >> 5] | (1 << (iBit & 0x1f)))
                 : (valPtr[iBit >> 5] & (~((1 ^ 0) << (iBit & 0x1f))));
    iBit++;
    wSize = wSize;
  }

  mapSavestateArray[map] = actors_appendToSavestate(
      mapSavestateArray[map], ((u32 *)mapSavestateArray[map]) + (4 * ((iBit + 0x7F) >> 7)));
}

void mapSavestate_apply(enum map_e map_id) {
  s32 iBit = 0;
  u32 *flag_ptr;
  u32 *word_ptr;
  ActorListSaveState *actor_list_ptr;
  u32 bit_value;

  if (mapSavestateArray[map_id] == NULL)
    return;

  flag_ptr = reinterpret_cast(u32 *, mapSavestateArray[map_id]);
  mapSpecificFlags_setAll(*flag_ptr);
  iBit += 8 * sizeof(u32);
  sortCubeProps(1);
  iterateOverAllCubeProps(-1);

  while (bit_value = BOOL(((u32 *)mapSavestateArray[map_id])[iBit >> 5] &
                          (1 << (iBit & 0x1f))),
         iterateOverAllCubeProps(bit_value) != -1) {
    iBit++;
  }
  sortCubeProps(0);

  actor_list_ptr = (ActorListSaveState *)mapSavestateArray[map_id] +
                   (((iBit + (0x80 - 1)) >> 7) * 4);
  func_8032A09C(mapSavestateArray[map_id], actor_list_ptr);
  bk_free((void *)mapSavestateArray[map_id]);
  mapSavestateArray[map_id] = NULL;
}
