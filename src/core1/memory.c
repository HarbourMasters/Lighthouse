#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include "version.h"
#include <ultra64.h>

#ifdef LIGHTHOUSE_P
#include <compat.h>
#endif

//heap.c?

/*  Every chunk of allocated memory is prefixed with a HeapHeader.
 *
 *  Chunks are 0x10 aligned, and cannot have a capacity < 1.
 *  This means the smallest chunksize with the header is 0x20.
 *
 *  If a chunk is empty, it's contains ptrs to the previous and
 *  next empty chunks. This forms a link list over all the empty
 *  chunks (EmptyHeapBlock) */

#define chunkSize(s) ((u32)(s)->next - (u32)(s) - sizeof(HeapHeader))
#if VERSION == VERSION_USA_1_0
#define HEAP_SIZE 0x210520
#elif VERSION == VERSION_PAL
#define HEAP_SIZE 0x211120
#endif
#define LAST_HEAP_BLOCK HEAP_SIZE / sizeof(EmptyHeapBlock) - 1

typedef enum {
  HEAP_BLOCK_EMPTY = 0,
  HEAP_BLOCK_USED = 1,
  HEAP_BLOCK_PERM = 2
} heap_block_type_e;

typedef struct heap_header {
  struct heap_header *prev;
  struct heap_header *next;
  u8 pad8[4];
  u32 unusedBytes_C_31 : 24; // size?
  u32 unkC_7 : 2;            // state?
  u32 padC_5 : 6;
} HeapHeader;

typedef struct empty_heap_block {
  HeapHeader hdr;
  struct empty_heap_block *prev_free;
  struct empty_heap_block *next_free;
  u8 pad18[0x8];
} EmptyHeapBlock;

extern EmptyHeapBlock heapBlocks[LAST_HEAP_BLOCK + 1];
extern EmptyHeapBlock lastHeapBlock;

/* .data */
u32 occupiedBytes = 0; // occupied heap size
u8 heapFlag = 0;
u8 memoryStatus = 0;
void *defragPointer = NULL;
void *tempBlockPointer = NULL;
s32 tempBlockCounter = 0;
void *tempBlockArg = NULL;
s32 heapCounter = 0;

struct {
  bool unk0;
} heapFlagStruct = {FALSE}; //this does need to be a struct

UNK_TYPE(void *) tempBlockMemory = NULL;

/* .bss */
s32 defragCounter;
void *heapPointer1;
void *heapPointer2;
s32 defragFlag;
u32 requested_size;
bool heapCondition; //HeapHeader *heapCondition;

struct {
  void *unk0[0x10];
  void **unk40;
} freeBlockQueue;

EmptyHeapBlock *heap_findFreeBlock(s32 size);
void heap_sortEmptyBlock(EmptyHeapBlock *arg0);
void heap_incrementCounter(void);

/* .code */
s32 heap_alignSize(s32 size) {
  s32 misalign = size & 0xf;
  return (misalign) ? (size - misalign + 0x10) : size;
}

int heap_dummyFunction(int arg0) { return FALSE; }

void heap_defragEmptyBlock(EmptyHeapBlock *arg0) {
  EmptyHeapBlock *defrag_ptr = NULL;
  if (arg0->hdr.next->unkC_7 == HEAP_BLOCK_EMPTY) { // absorb next block
    defrag_ptr = arg0;
    // remove next from empty block link list
    ((EmptyHeapBlock *)arg0->hdr.next)->next_free->prev_free =
        ((EmptyHeapBlock *)arg0->hdr.next)->prev_free;
    ((EmptyHeapBlock *)arg0->hdr.next)->prev_free->next_free =
        ((EmptyHeapBlock *)arg0->hdr.next)->next_free;
    // remove next from block link list
    arg0->hdr.next->next->prev = arg0;
    arg0->hdr.next = arg0->hdr.next->next;
  }
  if (arg0->hdr.prev->unkC_7 == HEAP_BLOCK_EMPTY) {
    defrag_ptr = arg0->hdr.prev;
    // remove self from empty block link list
    arg0->next_free->prev_free = arg0->prev_free;
    arg0->prev_free->next_free = arg0->next_free;
    // remove self from block link list
    arg0->hdr.prev->next = arg0->hdr.next;
    arg0->hdr.next->prev = arg0->hdr.prev;
  }
  if (defrag_ptr != NULL) {
    heap_sortEmptyBlock(defrag_ptr);
  } else {
    heap_sortEmptyBlock(arg0);
  }
}

void heap_markBlockAsEmpty(EmptyHeapBlock *arg0) {
  #ifndef LIGHTHOUSE_P
  arg0->hdr.unkC_7 = HEAP_BLOCK_EMPTY;
  arg0->hdr.unusedBytes_C_31 = 0;
  if ((u8 *)arg0->hdr.next - (u8 *)arg0 < 10000) {
    arg0->prev_free = &heapBlocks;
    arg0->next_free = heapBlocks->next_free;
    heapBlocks->next_free->prev_free = arg0;
    heapBlocks->next_free = arg0;
  } else {
    arg0->prev_free = heapBlocks[LAST_HEAP_BLOCK].prev_free;
    arg0->next_free = &heapBlocks[LAST_HEAP_BLOCK];

    heapBlocks[LAST_HEAP_BLOCK].prev_free->next_free = arg0;
    heapBlocks[LAST_HEAP_BLOCK].prev_free = arg0;
  }
  heap_defragEmptyBlock(arg0);
  #endif
}

void heap_memcpy(void *dst, void *src, int size) {
  while (size > 0) {
    *(u8 *)dst = *(u8 *)src;
    size--;
    dst = (u8 *)dst + 1;
    src = (u8 *)src + 1;
  }
}

void heap_copyWordMemory(void *dst, void *src, int size) {
  while (size > 0) {
    *(u32 *)dst = *(u32 *)src;
    size -= 4;
    dst = (u32 *)dst + 1;
    src = (u32 *)src + 1;
  }
}

void heap_moveMemory(u8 *dst, u8 *src, s32 n) {
  if (dst < src) { // copy
    while (n--) {
      *(dst++) = *(src++);
    }
  } else { // copy backwards to avoid data lose
    dst += n - 1;
    src += n - 1;
    while (n--) {
      *(dst--) = *(src--);
    }
  }
}

s32 heap_getTotalSize(void) { return HEAP_SIZE; }

s32 heap_stub_return_zero(void) { return 0; }

u32 heap_getBlockSize(void *arg0) {
  HeapHeader *sPtr = (HeapHeader *)arg0 - 1;
  return chunkSize(sPtr) - sPtr->unusedBytes_C_31;
}

void heap_clearPointers(void) {
  heapPointer1 = NULL;
  heapPointer2 = NULL;
}

void heap_init(void) {
  #ifndef LIGHTHOUSE_P
  bzero(heapBlocks, HEAP_SIZE);
  heap_clearPointers();
  freeBlockQueue.unk40 = &freeBlockQueue.unk0[0];
  occupiedBytes = 0;
  heapFlag = 0;
  tempBlockPointer = 0;
  tempBlockCounter = 0;
  tempBlockArg = 0;
  heapCounter = 0;
  heapFlagStruct.unk0 = FALSE;
  heapBlocks[0].hdr.prev = NULL;
  heapBlocks[0].hdr.next = &heapBlocks[1];
  heapBlocks[0].hdr.unkC_7 = 2;
  heapBlocks[0].hdr.unusedBytes_C_31 = 0;
  heapBlocks[0].prev_free = NULL;
  heapBlocks[0].next_free = &heapBlocks[1];

  heapBlocks[1].hdr.prev = &heapBlocks[0];
  heapBlocks[1].hdr.next = &heapBlocks[LAST_HEAP_BLOCK];
  heapBlocks[1].hdr.unkC_7 = 0;
  heapBlocks[1].hdr.unusedBytes_C_31 = 0;
  heapBlocks[1].prev_free = &heapBlocks[0];
  heapBlocks[1].next_free = &heapBlocks[LAST_HEAP_BLOCK];

  heapBlocks[LAST_HEAP_BLOCK].hdr.prev = &heapBlocks[1];
  heapBlocks[LAST_HEAP_BLOCK].hdr.next = &heapBlocks[LAST_HEAP_BLOCK + 1];
  heapBlocks[LAST_HEAP_BLOCK].hdr.unkC_7 = 2;
  heapBlocks[LAST_HEAP_BLOCK].hdr.unusedBytes_C_31 = 0;
  heapBlocks[LAST_HEAP_BLOCK].prev_free = &heapBlocks[1];
  heapBlocks[LAST_HEAP_BLOCK].next_free = NULL;
  sns_init_base_payloads();
  #endif
}

void *heap_allocate_temp_block(s32 size) {
  #ifndef LIGHTHOUSE_P
  tempBlockMemory = heap_malloc(ALIGN((u32)&heapBlocks[1] + 0x100, 0x100) -
                      (u32)&heapBlocks[1] - sizeof(EmptyHeapBlock));
  return heap_malloc(0x80);
  #else
  return NULL;
  #endif
}

void *heap_allocate_and_free_temp_block(s32 arg0) {
  #ifndef LIGHTHOUSE_P
  void *sp1C = heap_malloc(ALIGN(((u32)&heapBlocks[LAST_HEAP_BLOCK] -
                             (u32)heapBlocks[LAST_HEAP_BLOCK].prev_free) -
                                0x2FF,
                            0x100) +
                      -sizeof(EmptyHeapBlock));
  void *sp18 = heap_malloc(0x80);
  bk_free(sp1C);
  bk_free(tempBlockMemory);
  tempBlockMemory = NULL;
  return sp18;
  #else
  return 0;
  #endif
}

//unused?
void heap_free_temp_blocks(void) {
  if (tempBlockPointer) {
    bk_free(tempBlockPointer);
    tempBlockPointer = NULL;
  }

  if (tempBlockArg) {
    bk_free(tempBlockArg);
    tempBlockArg = NULL;
  }
}

static u32 _heap_get_occupied_size(void) { return occupiedBytes; }

u32 heap_get_occupied_size(void) { return _heap_get_occupied_size(); }

bool heap_hasFreeBlock(s32 size) {
  s32 v0 = heap_findFreeBlock(size);
  return BOOL(v0);
}

EmptyHeapBlock *heap_findFreeBlock(s32 size) {
  #ifndef LIGHTHOUSE_P
  EmptyHeapBlock *a1;
  s32 aligned_size;
  u32 block_size;

  a1 = heapBlocks->next_free;
  aligned_size = heap_alignSize(size > 0 ? size : 1);
  while (chunkSize(&a1->hdr) < aligned_size &&
         a1->next_free != &heapBlocks[LAST_HEAP_BLOCK]) {
    a1 = a1->next_free;
  }
  return (chunkSize(&a1->hdr) < aligned_size) ? 0 : a1;
  #else
  return 0;
  #endif
}

EmptyHeapBlock *heap_findBlock(bool arg0) {
  #ifndef LIGHTHOUSE_P
  EmptyHeapBlock *v1;
  EmptyHeapBlock *v0;
  if (!arg0) {
    // from start
    v1 = heapBlocks->next_free;
    while (chunkSize(&v1->hdr) < requested_size &&
           v1->next_free != &heapBlocks[LAST_HEAP_BLOCK]) {
      v1 = v1->next_free;
    }

    if (chunkSize(&v1->hdr) < requested_size)
      return NULL;
    return v1;
  } else {
    // from back
    v1 = NULL;
    v0 = heapBlocks->next_free;
    while (v0 != &heapBlocks[LAST_HEAP_BLOCK]) {
      if (chunkSize(&v0->hdr) >= requested_size && v1 < v0)
        v1 = v0;
      v0 = v0->next_free;
    }

    if (!v1)
      return NULL;

    if (chunkSize(&v1->hdr) < requested_size)
      return NULL;
    return v1;
  }
  #else
  return 0;
  #endif
}

EmptyHeapBlock *heap_findBlockByCondition(s32 arg0) {
  #ifndef LIGHTHOUSE_P
  if (heapCondition) {
    return heap_findBlock(1); // closest to back
  } else {
    return heap_findBlock(0); // closest to from
  }
  #else
  return 0;
  #endif
}

int heap_stub_return_false(int arg0) { return FALSE; }

// returns n'th free block and size
void *heap_getNthFreeBlock(s32 *size, u32 arg1) {
  #ifndef LIGHTHOUSE_P
  EmptyHeapBlock *var_v1;

  var_v1 = &lastHeapBlock;
  while (arg1 != 0) {
    var_v1 = var_v1->prev_free;
    if (var_v1 == &heapBlocks[0]) {
      // less than n blocks
      return NULL;
    }
    arg1--;
  }
  *size = ((s32)(var_v1->hdr.next) - (s32)var_v1) - sizeof(HeapHeader);
  return (s32)var_v1 + 0x10;
  #else
  return 0;
  #endif
}

void heap_setFlagTrue(void) { heapFlagStruct.unk0 = TRUE; }

void *heap_malloc(s32 size) {
  #ifndef LIGHTHOUSE_P
  u32 capacity;
  EmptyHeapBlock *v1;
  EmptyHeapBlock *a0;

  heapCondition = heapFlagStruct.unk0;
  heapFlagStruct.unk0 = FALSE;
  if (heapBlocks->next_free == &heapBlocks[LAST_HEAP_BLOCK])
    return NULL;

  requested_size = heap_alignSize((size > 0) ? size : 1);
  if (!(v1 = heap_findBlockByCondition(0))) { // remove stall cache ptrs
    heapCondition = FALSE;
    func_803306C8(2);
    if (!heap_findBlockByCondition(0))
      func_8030A850(2);

    if (!heap_findBlockByCondition(0))
      animCache_flushStale();

    if (!heap_findBlockByCondition(0))
      animBinCache_flushStale(0); // nonpersistent anim

    if (!heap_findBlockByCondition(0))
      func_8032AD7C(2);

    if (!(v1 = heap_findBlockByCondition(0))) {
      assetCache_clearAllDependencies();
      game_freeze();
      func_803306C8(3); // modelCache

      if (!heap_findBlockByCondition(0))
        func_8030A850(3); // propModelCache

      if (!heap_findBlockByCondition(0))
        func_8032AD7C(2); // actorArray

      if (!(v1 = heap_findBlockByCondition(0))) {
        if (!heap_findBlockByCondition(0))
          freeAllExceptCurrentEmitter(); // particleEmitters

        if (!heap_findBlockByCondition(0))
          animBinCache_flushStale(1); // persistent anim

        if (v1 = heap_findBlockByCondition(0)) {
        } else
          return NULL;
      }
    } // L80254E38

  } // L80254E38

  if (requested_size + sizeof(HeapHeader) < chunkSize(&v1->hdr)) {
    if (heapCondition) {
      // reverse split chunk => //split empty chunk: |prev| a0 |next| => |prev|
      // a0 | v1 |next|
      a0 = v1;
      v1 = (EmptyHeapBlock *)((u32)v1->hdr.next -
                              (requested_size + sizeof(HeapHeader)));
      v1->hdr.next = a0->hdr.next;
      a0->hdr.next->prev = v1;
      a0->hdr.next = v1;
      v1->hdr.prev = a0;
      heap_sortEmptyBlock(a0);
    } else { // L80254EA4
      // split chunk: |prev| v1 |next| => |prev| v1 | __a0__ |next|
      a0 = (HeapHeader *)((u32)v1 + (requested_size + sizeof(HeapHeader)));
      a0->next_free = v1->next_free;
      a0->prev_free = v1->prev_free;
      a0->next_free->prev_free = a0;
      a0->prev_free->next_free = a0;
      a0->hdr.prev = v1;
      a0->hdr.next = v1->hdr.next;
      a0->hdr.next->prev = a0;
      a0->hdr.unkC_7 = 0;
      a0->hdr.unusedBytes_C_31 = 0;
      v1->hdr.next = a0;
      heap_sortEmptyBlock(a0);
    }
  } else { // L80254F08
    // use full chunk/ remove chunk from empty chunk link list
    v1->next_free->prev_free = v1->prev_free;
    v1->prev_free->next_free = v1->next_free;
  } // L80254F20
  capacity = (u32)v1->hdr.next - (u32)v1;
  v1->hdr.unusedBytes_C_31 = capacity - size - 0x10;
  v1->hdr.unkC_7 = 1;
  occupiedBytes += capacity;
  return (u8 *)v1 + sizeof(HeapHeader);
  #else
  return 0;
  #endif
}

//unused?
void heap_incrementCounterMultipleTimes(void) {
  int i;
  for (i = 0; i < 50; i++) {
    heap_incrementCounter();
  }
}

void heap_sortEmptyBlock(EmptyHeapBlock *arg0) {
  #ifndef LIGHTHOUSE_P
  EmptyHeapBlock *v0 = arg0;
  EmptyHeapBlock *v1;
  EmptyHeapBlock *a2 = &heapBlocks[LAST_HEAP_BLOCK];

  // move arg0 back while larger than next
  while (arg0->next_free < &heapBlocks[LAST_HEAP_BLOCK] &&
         (s32)chunkSize(&v0->next_free->hdr) + 0x10 <
             (s32)chunkSize(&v0->hdr) + 0x10) {
    v1 = arg0->next_free;
    arg0->next_free = v1->next_free;
    v1->next_free->prev_free = arg0;
    v1->next_free = arg0;
    arg0->prev_free->next_free = v1;
    v1->prev_free = arg0->prev_free;
    arg0->prev_free = v1;
  }

  // move arg0 foward while smaller prev
  while (((v1 = arg0->prev_free) > &heapBlocks[0]) &&
         (s32)chunkSize(&v0->hdr) + 0x10 <
             (s32)chunkSize(&v0->prev_free->hdr) + 0x10) {
    a2 = arg0->prev_free;

    arg0->next_free->prev_free = a2;
    a2->next_free = arg0->next_free;
    arg0->next_free = a2;
    arg0->prev_free = a2->prev_free;
    a2->prev_free->next_free = arg0;
    a2->prev_free = arg0;
  }
  #else
  return 0;
  #endif
}

void bk_free(void *ptr) {
  HeapHeader *sPtr; // stack_ptr

  if (ptr) {
    sPtr = (HeapHeader *)ptr - 1;
    occupiedBytes = occupiedBytes -
                         (u32)((u8 *)sPtr->next - (u8 *)ptr) -
                         sizeof(HeapHeader);

    heap_markBlockAsEmpty(sPtr);

    if ((u32)ptr == (u32)tempBlockPointer)
      tempBlockPointer = NULL;

    if ((u32)ptr == (u32)tempBlockArg)
      tempBlockArg = NULL;
  }
}

void heap_queue_free_block(void **arg0) {
  *freeBlockQueue.unk40 = *arg0;
  freeBlockQueue.unk40++;
  *arg0 = NULL;
}

// heap_free_queue_flush
void heap_free_queue_flush(void) {
  while (freeBlockQueue.unk40 > &freeBlockQueue.unk0[0]) {
    freeBlockQueue.unk40--;
    bk_free(*freeBlockQueue.unk40);
  }
}

// resizes and fragments a block;
void heap_resizeAndFragmentBlock(HeapHeader *block, s32 size) {
  u32 remaining_bytes;
  EmptyHeapBlock *a0;
  u32 tmp, tmp2, tmp3;

  block->unusedBytes_C_31 = chunkSize(block) - size;
  tmp = ((u32)(block)->next) - ((u32)(block));
  if (size > 0) {
    // tmp2 = chunkSize(block) - sizeof(HeapHeader);
    tmp2 = heap_alignSize(size);
    remaining_bytes = chunkSize(block) - tmp2;
  } else {
    remaining_bytes = tmp - sizeof(EmptyHeapBlock);
  }

  if (remaining_bytes >= sizeof(EmptyHeapBlock)) {
    tmp = (chunkSize(block) + sizeof(HeapHeader)) - remaining_bytes;
    if (tmp)
      ;
    a0 = (s32)block + tmp;
    occupiedBytes -= remaining_bytes;

    a0->hdr.prev = block;
    a0->hdr.next = block->next;
    block->next = &a0->hdr;
    a0->hdr.next->prev = &a0->hdr;
    block->unusedBytes_C_31 = chunkSize(block) - size;
    heap_markBlockAsEmpty(a0);
  }
}

void heap_resizeAndSortBlock(HeapHeader *block, s32 size) {
  heap_resizeAndFragmentBlock(block, size);
  if (block->next->unkC_7 == HEAP_BLOCK_EMPTY) {
    heap_sortEmptyBlock(block->next);
  }
}
//unused?
void *heap_getPointer1(void) { return heapPointer1; }

//unused?
void *heap_getPointer2(void) { return heapPointer2; }

void *bk_realloc(void *ptr, s32 size) {

  HeapHeader *sPtr;
  void *newSeg;
  EmptyHeapBlock *emptySeg;

  heapPointer1 = ptr;
  heapPointer2 = ptr;
  sPtr = (HeapHeader *)ptr - 1;
  if (!((u32)((u8 *)sPtr->next - (u8 *)ptr) < size)) {
    // current pointer has enough free space to accomidate size change
    heap_resizeAndSortBlock(sPtr, size);
    return ptr;
  }

  defragPointer = ptr;
  emptySeg = (EmptyHeapBlock *)sPtr->next;
  if (emptySeg->hdr.unkC_7 == HEAP_BLOCK_EMPTY &&
      !((u32)((u8 *)emptySeg->hdr.next - (u8 *)sPtr) - 0x10 <
        size)) { // combine current heap segment with the next one (if next one
                 // is free).
    // remove empty segment from list
    emptySeg->next_free->prev_free = emptySeg->prev_free;
    emptySeg->prev_free->next_free = emptySeg->next_free;
    occupiedBytes += (u8 *)emptySeg->hdr.next - (u8 *)emptySeg;
    sPtr->next = emptySeg->hdr.next;
    emptySeg->hdr.next->prev = sPtr;
    heap_resizeAndSortBlock(sPtr, size);
    defragPointer = 0;
    return ptr;
  } // L80255430

  if (!(newSeg = heap_malloc(size))) {
    return 0;
  }

  func_80253010(newSeg, ptr, heap_alignSize(size));
  bk_free(ptr);
  ptr = newSeg;
  defragPointer = 0;
  heapPointer2 = newSeg;

  if (newSeg)
    ;

  return ptr;
}

u32 heap_get_free_size(void) { return HEAP_SIZE - heap_get_occupied_size(); }

s32 heap_findLargestEmptyBlock(s32 *size_ptr) {
  #ifndef LIGHTHOUSE_P
  EmptyHeapBlock *v0;
  s32 i;
  s32 size;

  v0 = heapBlocks->next_free;
  *size_ptr = 0;
  i = 0;
  while (v0 != &heapBlocks[LAST_HEAP_BLOCK]) {
    size = (s32)v0->hdr.next - (s32)v0;
    *size_ptr = (size < *size_ptr) ? *size_ptr : size;
    v0 = v0->next_free;
    i++;
  }
  return i;
  #else
  return 0;
  #endif
}

void heap_free_temp_blocks_if_needed(void) {
  defragCounter = (memoryStatus) ? -6000000 : 0;

  if (tempBlockPointer && tempBlockCounter + 1 < heapCounter) {
    bk_free(tempBlockPointer);
    tempBlockPointer = NULL;

    if (tempBlockArg) {
      bk_free(tempBlockArg);
      tempBlockArg = NULL;
    }
  }
}

void heap_resetDefragFlag(void) { defragFlag = FALSE; }

bool heap_isDefragFlagSet(void) { return defragFlag; }

void *defrag(void *this) {
  HeapHeader *new_block;
  HeapHeader *this_block;
  EmptyHeapBlock *new_empty;
  EmptyHeapBlock *prev_empty;
  EmptyHeapBlock *next_empty;
  HeapHeader *prev_block;
  s32 size;

  if (this == NULL || this == defragPointer) {
    return this;
  }

  size = (s32)((HeapHeader *)this)[-1].next - (s32)this + 0x10;

  this_block = &((HeapHeader *)this)[-1];
  if (defragCounter + size >= 1000000) {
    return this;
  }
  new_block = this_block->prev;

  if (new_block->unkC_7 != HEAP_BLOCK_EMPTY || chunkSize(new_block) < 0x10) {
    return this;
  }

  // previous block is empty, move contents of this block forward
  defragFlag = TRUE;
  defragCounter = defragCounter + size;

  next_empty = ((EmptyHeapBlock *)new_block)->next_free;
  prev_empty = ((EmptyHeapBlock *)new_block)->prev_free;
  prev_block = new_block->prev;
  func_80253010(new_block, this_block, size);
  // create new empty block at end of new_block;
  new_empty = (EmptyHeapBlock *)((s32)new_block + size);
  new_empty->hdr.prev = new_block;
  new_empty->hdr.next = new_block->next;
  new_block->next = &new_empty->hdr;
  new_block->prev = prev_block;
  new_empty->hdr.next->prev = &new_empty->hdr;
  prev_empty->next_free = new_empty;
  next_empty->prev_free = new_empty;
  new_empty->next_free = next_empty;
  new_empty->prev_free = prev_empty;
  new_empty->hdr.unkC_7 = HEAP_BLOCK_EMPTY;
  new_empty->hdr.unusedBytes_C_31 = 0;

  if (new_block)
    ;

  heap_defragEmptyBlock(
      new_empty); // combine new_empty with any surrounding empty blocks
  return (void *)((s32)new_block + sizeof(HeapHeader));
}

void *defrag_asset(void *arg0) {
  void *sp1C;
  if (arg0 == NULL || arg0 == defragPointer)
    return arg0;

  sp1C = defrag(arg0);
  assetcache_update_ptr(arg0, sp1C);
  return sp1C;
}

// recache??? defrag_cache???
void *heap_defrag_cache(void *this) {
  HeapHeader *this_block;
  HeapHeader *prev_block;
  s32 size;
  s32 pad;
  void *sp24;

  if (this == NULL || this == defragPointer || tempBlockPointer || assetCache_checkDependency(this)) {
    return this;
  }

  size = (s32)((HeapHeader *)this)[-1].next - (s32)this + 0x10;
  this_block = &((HeapHeader *)this)[-1];

  if (defragCounter + size >= 1000000)
    return this;

  prev_block = this_block->prev;
  if (prev_block->unkC_7 != HEAP_BLOCK_EMPTY) {
    return this;
  }

  sp24 = heap_malloc(size - sizeof(HeapHeader));
  func_80253010(sp24, this, size - sizeof(HeapHeader));
  osWritebackDCache(sp24, size - sizeof(HeapHeader));
  defragCounter += size - sizeof(HeapHeader);
  tempBlockPointer = this;
  tempBlockCounter = heapCounter;
  return sp24;
}

// recache asset?? defragment cached obj???
void *heap_defrag_and_update_cache(void *arg0) {
  void *sp1C;
  if (arg0 == NULL || arg0 == defragPointer) {
    return arg0;
  }

  sp1C = heap_defrag_cache(arg0);
  assetcache_update_ptr(arg0, sp1C);
  return sp1C;
}

//unused?
void *heap_defrag_and_update_cache_with_arg(void *arg0, void *arg1) {
  void *v1;
  v1 = heap_defrag_and_update_cache(arg0);
  if (v1 == arg0) {
    return arg0;
  } else {
    tempBlockArg = arg1;
    return v1;
  }
}

//unused?
bool heap_isPrevBlockEmpty(void *arg0) {
  HeapHeader *block;

  if ((arg0 == NULL) || (arg0 == defragPointer) || (tempBlockPointer != NULL)) {
    return FALSE;
  }

  block = &((HeapHeader *)arg0)[-1];
  return (block->prev->unkC_7 != HEAP_BLOCK_EMPTY) ? FALSE : TRUE;
}

//unused?
HeapHeader *heap_getPrevBlock(void *ptr) {
  return ((HeapHeader *)((s32)ptr - sizeof(HeapHeader)))->prev;
}

//unused?
void heap_setTempBlock(void *arg0, int arg1) {
  tempBlockPointer = arg0;
  tempBlockCounter = heapCounter;
}

bool heap_checkMemoryStatus(void) {
  return (memoryStatus == 0)
             ? (defragCounter >= 0xF4240) || ((heapFlag == 1) ? 0 : 1)
             : 0;
}

void heap_setFlag(void) { heapFlag = 1; }

void heap_clearFlag(void) { heapFlag = 0; }

//unused?
void heap_setMemoryStatusTrue(void) { memoryStatus = TRUE; }

//unused?
void heap_setMemoryStatusFalse(void) { memoryStatus = FALSE; }

//unused?
void heap_defrag_if_needed(void) {
  heap_free_temp_blocks_if_needed();
  if (heap_checkMemoryStatus() && memoryStatus != TRUE)
    return;

  if (!heap_checkMemoryStatus())
    defragTextRenderer(1);

  if (!heap_checkMemoryStatus())
    printbuffer_defrag();

  if (!heap_checkMemoryStatus()) {
    ml_defrag();
    dummy_func_80254464();
  }
}

void heap_incrementCounter(void) { heapCounter++; }

bool heap_isTempBlockSet(void) { return (tempBlockPointer) ? 1 : 0; }

int heap_getBlockType(int arg0) {
  if (arg0 == 2)
    return 0x3;
  return 0x12;
}
