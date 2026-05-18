#include <ultra64.h>
#include "functions.h"
#include "variables.h"


bool cameraAreaList_searchForEntryInBounds(BKCameraAreaList *this, u8 *id, u32 count) {
    BKCameraArea *start_ptr;

    start_ptr = (BKCameraArea *)this->data;
    while(count != 0){
        if (start_ptr[*id].in_bounds != 0) {
            return true;
        }
        count--;
        id++;
    }
    return false;
}

void cameraAreaList_updateInBoundsFlag(BKCameraAreaList *this, f32 camera_position[3], f32 scale) {
    BKCameraArea *start_ptr;
    BKCameraArea *end_ptr;
    BKCameraArea *i_ptr;
    s32 i;
    s16 sp18[3];

    start_ptr = (BKCameraArea *)this->data;
    sp18[0] = (s16) (camera_position[0] * (1.0 / scale));
    sp18[1] = (s16) (camera_position[1] * (1.0 / scale));
    sp18[2] = (s16) (camera_position[2] * (1.0 / scale));
    end_ptr = start_ptr + this->count;
    for(i_ptr = start_ptr; i_ptr < end_ptr; i_ptr++){
        for(i = 0; i < 3; i++){
            if ((sp18[i] < i_ptr->min[i]) || (i_ptr->max[i] < sp18[i])) {
               break;
            }
        }
        i_ptr->in_bounds = (i == 3);
    }
}
