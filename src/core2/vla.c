// BanjoDecomp: core2/vla.c
#include <ultra64.h>
#include "functions.h"
#include "core2/vla.h"

/* VARIABLE LENGTH ARRAY */

void bk_vector_clear(VLA *this){
    this->end = this->begin;
}

void *bk_vector_getBegin(VLA *this){
    return this->begin;
}

void *bk_vector_at(VLA *this, u32 n){
    return (void *)((uintptr_t) this->begin + n*this->elem_size);
}

s32 bk_vector_getIndex(VLA *this, void *elemPtr){
    return ((intptr_t)elemPtr - (intptr_t)this->begin)/(intptr_t)this->elem_size;
}

s32 bk_vector_size(VLA *this){
    return ((intptr_t)this->end - (intptr_t)this->begin)/this->elem_size;
}

void *bk_vector_getEnd(VLA *this){
    return this->end;
}

void *bk_vector_pushBackNew(VLA **thisPtr){
    void *retVal;
    VLA* this;
    s32 size;
    s32 mem_size;

    this = *thisPtr;
    if(this->end == this->mem_end){
        size = ((intptr_t)this->end - (intptr_t)this->begin)/this->elem_size;
        mem_size = size + 5;
        this = bk_realloc(this,  mem_size*this->elem_size + sizeof(VLA));
        this->begin = &this->data;
        this->end = (u8 *)this->begin + size* this->elem_size;
        this->mem_end = (u8 *)this->begin + mem_size* this->elem_size;
        *thisPtr = this; 
    }
    retVal = this->end;
    this->end = (void *)((uintptr_t)this->end + this->elem_size);
    return retVal;
}

void *bk_vector_insertNew(VLA **thisPtr, s32 indx){
    VLA *this;
    s32 i;

    bk_vector_pushBackNew(thisPtr);
    this = *thisPtr;
    i = ((intptr_t)this->end - (intptr_t)this->begin)/this->elem_size;
    while(indx < --i){
        memcpy((void *)((uintptr_t)this->begin + (i)*this->elem_size), (void *)((uintptr_t)this->begin + (i -1)*this->elem_size), this->elem_size);
    }
    return (void *)((uintptr_t)this->begin +  indx*this->elem_size);
}

void bk_vector_free(VLA *this){
    bk_free(this);
}

VLA *bk_vector_new(u32 elemSize, u32 cnt){
    VLA *this = bk_malloc(cnt*elemSize + sizeof(VLA));
    this->elem_size = elemSize;
    this->begin = &this->data;
    this->end = &this->data;
    this->mem_end = (u8*)this->end + cnt*elemSize;
    return this;
}

void bk_vector_remove(VLA *this, u32 indx){
    uintptr_t elemOffset = (uintptr_t)this->begin + indx * this->elem_size;\
    uintptr_t nextOffset = (uintptr_t)this->begin + (indx + 1) * this->elem_size;\
    uintptr_t size = (uintptr_t)this->end - (uintptr_t)this->begin;
    
    // [port] source and destination overlap when removing anywhere but the tail;
    // memcpy on overlapping buffers is UB on PC toolchains.
    memmove((void *)elemOffset, (void *)nextOffset, size - (indx + 1) * this->elem_size);
    this->end = (void *)((uintptr_t)this->end - this->elem_size);
}


void bk_vector_popBack_n(VLA *this, u32 n){
    this->end = (void *)((uintptr_t)this->end - n * this->elem_size);
}

void bk_vector_assign(VLA *this, s32 indx, void* value){
    memcpy((void*)((uintptr_t)this->begin + indx * this->elem_size), value, this->elem_size);
}

VLA * bk_vector_defrag(VLA *this){
    /*
    [port] Lighthouse - No N64 heap to compact on PC
    return NULL;

    intptr_t oldSize;
    intptr_t oldMemSize;

    oldSize = (intptr_t) this->end - (intptr_t)this->begin;
    oldMemSize = (intptr_t) this->mem_end - (intptr_t)this->begin;
    this = (VLA *)defrag(this);
    this->begin = &this->data;
    this->end = (void *)((uintptr_t)this->begin + oldSize);
    this->mem_end = (void *)((uintptr_t)this->begin + oldMemSize);
    */
    return this;
}
