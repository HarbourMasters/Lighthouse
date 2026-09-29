#ifndef BANJO_KAZOOIE_CORE2_VLA_H
#define BANJO_KAZOOIE_CORE2_VLA_H

#include <ultra64.h>

typedef struct variable_length_array{
    s32 elem_size;
    void * begin;
    void * end;
    void * mem_end;
    u8  data[];
}VLA;

#define bk_vector(T) struct variable_length_array
//^defined to keep element type with vla

/* vla - variable length array*/
void    bk_vector_clear(VLA *self);
void *  bk_vector_getBegin(VLA *self);
void *  bk_vector_at(VLA *self, u32 n);
s32     bk_vector_getIndex(VLA *self, void *element);
s32     bk_vector_size(VLA *self);
void *  bk_vector_getEnd(VLA *self);
void *  bk_vector_pushBackNew(VLA **selfPtr);
void *  bk_vector_insertNew(VLA **selfPtr, s32 indx);
void    bk_vector_free(VLA *self);
VLA *   bk_vector_new(u32 elemSize, u32 cnt);
void    bk_vector_remove(VLA *self, u32 indx);
void    bk_vector_popBack_n(VLA *self, u32 n);
void    bk_vector_assign(VLA *self, s32 indx, void* value);
VLA *   bk_vector_defrag(VLA *self);

#endif
