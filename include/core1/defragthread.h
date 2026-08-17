#ifndef BANJO_KAZOOIE_CORE1_DEFRAGTHREAD_H
#define BANJO_KAZOOIE_CORE1_DEFRAGTHREAD_H

#include <ultra64.h>

#define DEFRAGMANAGER_THREAD_ID 2
#define DEFRAGMANAGER_THREAD_PRI 10
#define DEFRAGMANAGER_THREAD_PRI_HIGH 30

void defragthread_init(void);
void defragthread_free(void);
void defragthread_resume(void);
void defragthread_pause(void);
void defragthread_setPriority(OSPri pri);
void defragthread_entry(void *arg);

#endif
