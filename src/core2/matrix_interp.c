// BanjoDecomp: core2/code_C3A40.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

/* .code */
/*
1.0|         ,----.
   |        /      \
   |       /        \
0.0|______/          \_____ arg1
       |   |   |   |
      [2] [3] [0] [1]
*/
f32 core2_C3A40_getIntensity(f32 arg0[4], f32 arg1) {
    if (arg0[1] <= arg1) {
        return 0.0f;
    }
    if (arg0[0] <= arg1) {
        return (arg0[1] - arg1) / (arg0[1] - arg0[0]);
    }
    if (arg0[3] <= arg1) {
        return 1.0f;
    }
    if (arg0[2] <= arg1) {
        return (arg1 - arg0[2]) / (arg0[3] - arg0[2]);
    }
    return 0.0f;
}


void core2_C3A40_getDefaultValues(f32 arg0[4]){
    arg0[1] = 10000.0f;
    arg0[0] = 4000.0f;
    arg0[3] = 2000.0f;
    arg0[2] = -500.0f;
}
