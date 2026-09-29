#ifndef BANJO_KAZOOIE_CORE2_CH_SNACKER_H
#define BANJO_KAZOOIE_CORE2_CH_SNACKER_H

#include "prop.h" // for ActorInfo
#include "core2/snackerctl.h" //for SnackerCtlState

extern ActorInfo gChSnacker;

void chSnacker_initialize(void);
void chsnacker_setControlState(SnackerCtlState nextState);

#endif
