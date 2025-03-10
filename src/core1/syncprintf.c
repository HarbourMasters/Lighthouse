#ifdef LIGHTHOUSE_P
#include "compat.h"
#include <stdarg.h>
#else
#include <rmon.h>
#include <bk_stdarg.h>
#include <os.h>
#endif

void osSyncPrintf(const char* fmt, ...) {
    int ans;
    va_list ap;
    // these functions intentionally left blank.  ifdeffed out in rom release
}
void rmonPrintf(const char* fmt, ...) {
    int ans;
    va_list ap;
}
