#include "functions.h"
#include "variables.h"
#include <ultra64.h>

/* Freelist
  This is a data container that contains arbitrary element types.
  Every element in the array starts with an i16, that represents
  either:
    - it's own array index in the array if not free, OR
    - the next free element's array index if this element is free.

  The 0th element in the array is never used, and contains the first
  index in the freelist.
*/

/* .code */
void freelist_clear(FLA_s *this) {
  int i;
  s32 ptr = (s32)(this + 1);
  for (i = 1; i < this->elem_cnt; i++) {
    *(s16 *)ptr = i;
    ptr += this->elem_size;
  }
  *(s16 *)ptr = 0;
}

void *freelist_at(FLA_s *this, s32 indx) {
  return (void *)((s32)(this + 1) + indx * this->elem_size);
}

s32 freelist_size(FLA_s *this) { return this->elem_cnt; }

void *freelist_freeListPtr(FLA_s *this) { return (void *)(this + 1); }

// returns next free element in the freelist
void *freelist_next(FLA_s **this_ptr, s32 *arg1) {
  s32 new_cnt;
  FLA_s *this;
  s16 *first_ptr;
  s16 *next_ptr;
  s32 prev_cnt;
  s32 i;

  this = *this_ptr;
  first_ptr = (s16 *)(this + 1);
  if (*first_ptr == 0) {
    prev_cnt = this->elem_cnt;
    new_cnt = prev_cnt + 10;
    this = bk_realloc(this, (this->elem_size * new_cnt) + sizeof(FLA_s));
    first_ptr = (s16 *)(this + 1);
    this->elem_cnt = new_cnt;
    *this_ptr = this;
    for (i = new_cnt - 1,
        next_ptr = (s16 *)((s32)(this + 1) + i * this->elem_size);
         i >= prev_cnt; i--) {
      *next_ptr = *first_ptr;
      *first_ptr = i;
      next_ptr = (s16 *)((s32)next_ptr - this->elem_size);
    }
  }
  i = *first_ptr;
  next_ptr = (s16 *)((s32)first_ptr + (i * this->elem_size));
  *arg1 = i;
  *first_ptr = *next_ptr;
  return (void *)next_ptr;
}

bool freelist_elementIsAlive(FLA_s *this, s32 index) {
  u8 *tmp_v0 = (this + 1);
  s16 *i_ptr;
  u8 *new_var;

  new_var = (this + 1);
  for (i_ptr = (s16 *)(this + 1); *i_ptr;
       i_ptr = (s16 *)(((*i_ptr) * this->elem_size) + (new_var = tmp_v0))) {
    if ((*i_ptr) == index) {
      return FALSE;
    }
  }

  return TRUE;
}
void freelist_free(FLA_s *this) { bk_free(this); }

FLA_s *freelist_new(s32 size, s32 cnt) {
  FLA_s *this;
  cnt++;
  this = (FLA_s *)heap_malloc(size * cnt + sizeof(FLA_s));
  this->elem_size = size;
  this->elem_cnt = cnt;
  freelist_clear(this);
  return this;
}

// removes first free element
void freelist_freeElement(FLA_s *this, s32 indx) {
  void **ptr_n = (void *)((s32)(this + 1) + indx * this->elem_size);
  *(s16 *)ptr_n = *(s16 *)(this + 1);
  *(s16 *)(this + 1) = indx;
}

FLA_s *freelist_defrag(FLA_s *this) { return (FLA_s *)defrag(this); }
