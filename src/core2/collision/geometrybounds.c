// BanjoDecomp: core2/code_B6640.c
#include <ultra64.h>

void core2_B6640_calculateLineBoundingBox(f32 start_point[3], f32 end_point[3], f32 margin, f32 min_bounds[3], f32 max_bounds[3]) {
    int i;

    for (i = 0; i < 3; i++) {
        if (start_point[i] < end_point[i]) {
            min_bounds[i] = start_point[i];
            max_bounds[i] = end_point[i];
        } else {
            min_bounds[i] = end_point[i];
            max_bounds[i] = start_point[i];
        }

        min_bounds[i] -= margin;
        max_bounds[i] += margin;
    }
}
