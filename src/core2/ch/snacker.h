#ifndef __BANJO_KAZOOIE_CORE2_CH_SNACKER_H__
#define __BANJO_KAZOOIE_CORE2_CH_SNACKER_H__

#include "../snackerctl.h" //for SnackerCtlState
#include "prop.h"          // for ActorInfo

extern ActorInfo gChSnacker;

void chSnacker_initialize(void);
void chsnacker_setControlState(SnackerCtlState nextState);

#endif // __BANJO_KAZOOIE_CORE2_CH_SNACKER_H__
