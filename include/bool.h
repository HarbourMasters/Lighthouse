#ifndef BANJO_KAZOOIE_BOOL_H
#define BANJO_KAZOOIE_BOOL_H

#include <stdbool.h>

#ifndef __cplusplus
#define NOT(boolean) ((boolean) ^ 1)
#define BOOL(boolean) ((boolean) ? true : false)
#endif

// Lighthouse [port] Decomp code uses TRUE/FALSE (typedef int bool upstream).
// Windows headers may also define these; values are identical.
#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

#if 0
#include <ultra64.h>

typedef int bool;
#endif
#endif
