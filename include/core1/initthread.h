#ifndef BANJO_KAZOOIE_CORE1_INITTHREAD_H
#define BANJO_KAZOOIE_CORE1_INITTHREAD_H

#include <ultra64.h>

#define INITTHREAD_ID 1
#define INITTHREAD_PRI OS_PRIORITY_IDLE

void initThread_create(void);
void initThread_entry(void *arg);

#endif
