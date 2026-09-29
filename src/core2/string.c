#include <ultra64.h>
#include "bk_string.h"

void bk_strcat(char *dest, const char *src) {
    while (*dest) {
        dest++;
    }

    while (*src) {
        *dest++ = *src++;
    }

    *dest = '\0';
}

void bk_strcatc(char *dest, char src) {
    while (*dest) {
        dest++;
    }

    *dest++ = src;
    *dest = '\0';
}

void bk_strFToA(char *dest, f32 val){
    s32 decimal;
    if (val < (f32) 0.0){
        bk_strcat(dest, "-");
        val = -val;
    }
    bk_strIToA(dest, (s32)val);
    bk_strcat(dest, ".");
    decimal = (s32)((val - (f32)((s32)val))*(f32)100.0);
    if(decimal < 10){
        bk_strcat(dest, "0");
    }
    bk_strIToA(dest, decimal);
}

void bk__strFToA(char *dest, f32 val, s32 decPlaces) {
    u32 i;
    if (val < 0.0f){
        bk_strcat(dest, "-");
        val = -val;
    }
    bk_strIToA(dest, val);
    if (decPlaces != 0){
        bk_strcat(dest, ".");
        for(i = decPlaces--; i > 0; i = decPlaces--) {
            val -= (s32)val;
            val *= 10;
            bk_strIToA(dest, val);
        }
    }
}

void bk_strIToA(char *str, s32 num) {
    bk__strIToA(str, num, 0);
}

void bk__strIToA(char *str, s32 num, char prefix) {
    s32 i;

    //Find end of string to concatinate onto
    while(*str != '\0'){
        str++;
    }

    // Check for a negative number, and if is prepend the '-' and make it positive
    if (num < 0) {
        *str = '-';
        str++;
        num = -num;
    } else if (prefix != 0){
        *str = prefix;
        str++;
    }
    for (i = 1000000000; num < i; i/=10) {}
    if (i == 0){
        *str = '0';
        str++;
    } else{
        while (i > 0){
            *str = '0' + (num / i);
            num %= i;
            str++;
            i /= 10;
        }
    }
    //Terminate the string with NULL
    *str = '\0';
}

int bk_strcmp(const char *lhs, const char *rhs) {
    while (*lhs && *rhs && *lhs == *rhs) {
        lhs++;
        rhs++;
    }

    if (*lhs == *rhs) {
        return 0;
    } else if ((*lhs == '\0') || (*lhs < *rhs)) {
        return -1;
    } else {
        return 1;
    }
}

void bk_strcpy(char *dest, const char *src) {
    while (*src) {
        *dest++ = *src++;
    }

    *dest = '\0';
}


int bk_strlen(const char *str) {
    int len = 0;

    while (*str++) {
        len++;
    }

    return len;
}

int bk_strcmptotok(char *str1, char *str2, char *str3) {
    while (*str2 == *str3) {
        str2++;
        str3++;
        if (((*str2 == '\0') || (*str2 == *str1)) && ((*str3 == '\0') || (*str3 == *str1))) {
            return 1;
        }
    }
    return 0;
}

char *bk_strtok(char *str, const char *delim) {
    while (*delim && *delim != *str ) {
        delim++;
    }

    if (*delim == *str) {
        delim++;
    }

    return (char *) delim;
}

void bk_strcpytotok(char *str1, char *str2, char *str3){
    while ((*str3 != '\0') && (*str3 != *str1)){
        *str2 = *str3;
        str3++;
        str2++;
    }
    *str2 = '\0';
}

void bk_strupr(char *str) {
    char *ch = str;

    while (*ch) {
        if ((*ch >= 'a') && (*ch <= 'z')) {
            *ch -= 'a' - 'A';
        }

        ch++;
    }
}
