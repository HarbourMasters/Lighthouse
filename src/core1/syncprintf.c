#ifdef LIGHTHOUSE_P
#include "pc_oscompat.h"
#else
#include <os.h>
#include <rmon.h>
#endif

#include <bk_stdarg.h>

void osSyncPrintf(const char *fmt, ...) {
  int ans;
  va_list ap;
  // these functions intentionally left blank.  ifdeffed out in rom release
}
void rmonPrintf(const char *fmt, ...) {
  int ans;
  va_list ap;
}
