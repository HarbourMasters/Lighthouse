#ifndef BANJO_KAZOOIE_CORE1_MEMORY_H
#define BANJO_KAZOOIE_CORE1_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif

//copy memory area
void heap_memcpy(void * dst, void *src, int size);

//copy an array of wide_characters
void heap_copyWordMemory(void * dst, void *src, int size); 

//copy memory area
//never used?
void heap_moveMemory(u8* dst, u8* src, s32 n);

#ifdef __cplusplus
}
#endif

#endif
