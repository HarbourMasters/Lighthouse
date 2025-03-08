#ifndef BK_STRING_H
#define BK_STRING_H

#include "structs.h"

#ifdef __cplusplus
extern "C" {
#endif

void bk_strcat(char *dst, char *src);
void strcatc(char *dst, char src);
void floatToString(char *dst, f32 val);
void floatToStringWithPrecision(char *dst, f32 val, s32 decPlaces);
void strIToA(char *str, s32 num);
void _strIToA(char *str, s32 num, char base);
void bk_strcpy(char *dst, char *src);
s32 bk_strlen(char *str);
void strToUpper(char *str);
#ifdef __cplusplus
}
#endif

#endif
