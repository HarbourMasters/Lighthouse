#ifndef BANJO_KAZOOIE_CORE2_SNACKERCTL_H
#define BANJO_KAZOOIE_CORE2_SNACKERCTL_H

typedef enum {
    SNACKER_CTL_STATE_0_INACTIVE,
    SNACKER_CTL_STATE_1_RBB,
    SNACKER_CTL_STATE_2_TTC
} SnackerCtlState;

void snackerctl_reset(void);
void snackerctl_update(void);
SnackerCtlState snackerctl_get_state(void);

#endif
