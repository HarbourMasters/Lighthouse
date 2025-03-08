
#ifndef _OS_LIBC_H_
#define _OS_LIBC_H_

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

#include <ultratypes.h>

/* byte string operations */

extern void bcopy(const void *, void *, int);
extern int bcmp(const void *, const void *, int);
extern void bzero(void *, int);

/* Printf */

extern int sprintf(char *s, const char *fmt, ...);
extern void osSyncPrintf(const char *fmt, ...);

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif

#endif /* !_OS_LIBC_H_ */
