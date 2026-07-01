#include <string.h>
#include "libultraship/libultra/types.h"

void bkmemcpy64(void* dest, void* src, s32 size) {
    memcpy(dest, src, size);
}

void bkmemset64(void* dest, s32 value, s32 size) {
    memset(dest, value, size);
}
