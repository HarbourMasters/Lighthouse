#ifndef BANJO_KAZOOIE_CORE1_MEMORY_H
#define BANJO_KAZOOIE_CORE1_MEMORY_H
#include "bk_string.h"

#include <ultra64.h>

/*
 * These functions are slightly modified from the Standard C Library.
 * To avoid naming collisions and make porting easier, they were prefixed.
 */

void bk_memcpy(void *dest, void *src, int count);
void bk_wmemcpy(void *dest, void *src, int count);
void bk_memmove(void *dest, void *src, int count);

//void *bk_malloc(int size);
void *bk_malloc(size_t size);
void bk_free(void *ptr);
//void *bk_realloc(void *ptr, int new_size);
void *bk_realloc(void *ptr, size_t new_size);

#endif
