#ifndef _STATIC_LENGTH_ARRAY_H_
#define _STATIC_LENGTH_ARRAY_H_

#include <ultra64.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct variable_length_array{
    s32 elem_size;
    void * begin;
    void * end;
    void * mem_end;
    u8  data[];
}VLA;

#define bk_vector(T) struct variable_length_array
//^defined to keep element type with vla

#ifdef __cplusplus
}
#endif

#endif
