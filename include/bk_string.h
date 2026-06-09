/**
 * BK custom string functions (strFToA, strcatc, strToUpper, etc.).
 *
 * [port] Named bk_string.h rather than core2/string.h to avoid shadowing the
 * system <string.h> on modern toolchains — Lighthouse's CMake adds include/
 * to -I, so a local string.h would be picked up ahead of the C standard
 * header (which we need for memcpy/memset/strcmp etc.). Upstream decomp calls
 * this file core2/string.h because N64/libultra has no system <string.h> to
 * collide with.
 */
#ifndef BK_STRING_H
#define BK_STRING_H

#if 0
#include <ultra64.h>
#else
#include <libultraship/libultra/types.h>
#endif

/* Custom string functions for Banjo-Kazooie */
void strcatc(char *dst, char src);
void strFToA(char *dst, f32 val);
void _strFToA(char *dst, f32 val, s32 decPlaces);
void strIToA(char *str, s32 num);
void _strIToA(char *str, s32 num, char prefix);
s32 strcmpToTok(char *str1, char* str2, char* str3);
// char *strtok(char *str, const char *delim);
void strcpyToTok(char *arg0, char *arg1, char *arg2);
void strToUpper(char *str);

#endif /* BK_STRING_H */
