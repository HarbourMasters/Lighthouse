#include "core2/vla.h"
#include "functions.h"
#include <ultra64.h>

/* VARIABLE LENGTH ARRAY */

void vector_clear(VLA *vthis){
    vthis->end = vthis->begin;
}

void *vector_getBegin(VLA *vthis){
    return vthis->begin;
}

void *vector_at(VLA *vthis, u32 n){
    return (void *)((u32) vthis->begin + n*vthis->elem_size);
}

s32 vector_getIndex(VLA *vthis, void *elemPtr){
    return ((s32)elemPtr - (s32)vthis->begin)/(s32)vthis->elem_size;
}

s32 vector_size(VLA *vthis){
    return ((s32)vthis->end - (s32)vthis->begin)/vthis->elem_size;
}

void *vector_getEnd(VLA *vthis){
    return vthis->end;
}

void *vector_pushBackNew(VLA **thisPtr){
    void *retVal;
    VLA* vthis;
    s32 size;
    s32 mem_size;

    vthis = *thisPtr;
    if(vthis->end == vthis->mem_end){
        size = ((s32)vthis->end - (s32)vthis->begin)/vthis->elem_size;
        mem_size = size + 5;
        vthis = bk_realloc(vthis,  mem_size*vthis->elem_size + sizeof(VLA));
        vthis->begin = &vthis->data;
        vthis->end = (u8 *)vthis->begin + size* vthis->elem_size;
        vthis->mem_end = (u8 *)vthis->begin + mem_size* vthis->elem_size;
        *thisPtr = vthis; 
    }
    retVal = vthis->end;
    vthis->end = (void *)((s32)vthis->end + vthis->elem_size);
    return retVal;
}

void *vector_insertNew(VLA **thisPtr, s32 indx){
    VLA *vthis;
    s32 i;

    vector_pushBackNew(thisPtr);
    vthis = *thisPtr;
    i = ((s32)vthis->end - (s32)vthis->begin)/vthis->elem_size;
    while(indx < --i){
        heap_memcpy((void *)((s32)vthis->begin + (i)*vthis->elem_size), (void *)((s32)vthis->begin + (i -1)*vthis->elem_size), vthis->elem_size);
    }
    return (void *)((s32)vthis->begin +  indx*vthis->elem_size);
}

void vector_free(VLA *vthis){
    bk_free(vthis);
}

VLA *vector_new(u32 elemSize, u32 cnt){
    VLA *vthis = heap_malloc(cnt*elemSize + sizeof(VLA));
    
    vthis->elem_size = elemSize;
    vthis->begin = &vthis->data;
    vthis->end = &vthis->data;
    vthis->mem_end = (u8*)vthis->end + cnt*elemSize;
    return vthis;
}

void vector_remove(VLA *vthis, u32 indx){
    u32 elemOffset = (u32)vthis->begin + indx * vthis->elem_size;\
    u32 nextOffset = (u32)vthis->begin + (indx + 1) * vthis->elem_size;\
    u32 size = (u32)vthis->end - (u32)vthis->begin;

    heap_memcpy((void *)elemOffset, (void *)nextOffset, size - (indx + 1) * vthis->elem_size);
    vthis->end = (void *)((u32)vthis->end - vthis->elem_size);
}


void vector_popBack_n(VLA *vthis, u32 n){
    vthis->end = (void *)((u32)vthis->end - n * vthis->elem_size);
}

void vector_assign(VLA *vthis, s32 indx, void* value){
    heap_memcpy((void*)((s32)vthis->begin + indx * vthis->elem_size), value, vthis->elem_size);
}

VLA * vector_defrag(VLA *vthis){
   s32 oldSize;
   s32 oldMemSize;

   oldSize = (s32) vthis->end - (s32)vthis->begin;
   oldMemSize = (s32) vthis->mem_end - (s32)vthis->begin;
   vthis = (VLA *)defrag(vthis);
   vthis->begin = &vthis->data;
   vthis->end = (void *)((s32)vthis->begin + oldSize);
   vthis->mem_end = (void *)((s32)vthis->begin + oldMemSize);
   return vthis;
}
