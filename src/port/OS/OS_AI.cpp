// This file should eventually go back to LUS; these are unchanged from their
// libultra/os.cpp originals, and only live here because taking osSetTimer
// port-side means taking that whole object with it.

extern "C" {
#include "libultraship/libultra/types.h"
#include "libultraship/libultra/os.h"
}

extern "C" u32 osAiGetLength(void) {
    return 0;
}

extern "C" s32 osAiSetNextBuffer(void* buff, size_t len) {
    (void)buff;
    (void)len;
    return 0;
}
