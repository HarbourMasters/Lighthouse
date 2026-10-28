// [port] beanstalk.c's typedefs and states, moved here for the Anchor catch-up (Puzzles/ClickClockWood.cpp).
#ifndef BANJO_KAZOOIE_CCW_CH_BEANSTALK_H
#define BANJO_KAZOOIE_CCW_CH_BEANSTALK_H

typedef struct{
    s16 waitingAnim;
    u8 waitingAnimBehavior;
    u8 isInit;
    s16 growingAnim;
    s16 grownAnim;
    s16 completeFileProgress;
    s16 completeSpawnsJiggy;
}Struct_CCW_1B20_0;

typedef struct{
    Struct_CCW_1B20_0 *unk0;
}ActorLocal_CCW_1B20;

enum chCCWBeanstalk_State_e {
    CH_CCW_BEANSTALK_STATE_0_NOT_INIT,
    CH_CCW_BEANSTALK_STATE_1_WAITING,
    CH_CCW_BEANSTALK_STATE_2_GROWING,
    CH_CCW_BEANSTALK_STATE_3_GROWN
};

#endif
