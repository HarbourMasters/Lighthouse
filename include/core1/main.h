#ifndef BANJO_KAZOOIE_CORE1_MAIN_H
#define BANJO_KAZOOIE_CORE1_MAIN_H

#include "enums.h"

#ifdef __cplusplus
extern "C" {
#endif 

void initialize_system(s32 arg0);
void system_flushHeapQueue(void);
void system_setGameMode(s32 arg0);
u32 globalTimer_getTimeMasked(u32 mask);
//s32 globalTimer_getTime(void);
void globalTimer_reset(void);
enum map_e getSpecialBootMap(void);
enum map_e getDefaultBootMap(void);
void system_setDefaultBootMap(void);
void system_setSpecialBootMap(void);
void core1_init(void);
void globalTimer_incTimer(void);
void globalTimer_decTimer(void);
void mainLoop(void);
void mainThread_entry(void *arg);
void system_scheduleModeChange(s32 arg0);
s32 system_getCurrentMode(void);
void setBootMap(enum map_e map_id);
void mainThread_create(void);
OSThread *mainThread_get(void);
void disableInput_set(void);

#ifdef __cplusplus
}   
#endif

#endif
