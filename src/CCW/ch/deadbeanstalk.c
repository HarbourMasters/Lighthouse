// BanjoDecomp: ch/deadbeanstalk.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

void chDeadBeanstalk_update(Actor *this);

/* .data */
ActorInfo D_8038ECA0 = { 
    0x1C2, 0x30B, 0x4E3,
    0x0, NULL,
    chDeadBeanstalk_update, NULL, actor_draw,
    0, 0, 1.0f, 0
};

/* .code */
void chDeadBeanstalk_setNextState(Actor *this, s32 next_state){
    if(next_state == 1){
        skeletalAnim_set(this->unk148, 0x219, 0.0f, 1.0f);
        skeletalAnim_setBehavior(this->unk148, SKELETAL_ANIM_4_STOPPED);
    }
    this->state = next_state;
}

void chDeadBeanstalk_update(Actor *this){
    if(!this->volatile_initialized){
        this->volatile_initialized = true;
        if(fileProgressFlag_get(FILEPROG_E5_CCW_FLOWER_AUTUMN)){
            chDeadBeanstalk_setNextState(this, 1);
        }
        else{
            marker_despawn(this->marker);
        }
    }
}
