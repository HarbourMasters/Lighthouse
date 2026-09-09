#ifndef BANJO_KAZOOIE_CORE1_BAMOTOR_H
#define BANJO_KAZOOIE_CORE1_BAMOTOR_H

#include <ultra64.h>

#define BAMOTOR_THREAD_ID 8
#define BAMOTOR_THREAD_PRI 25

void baMotor_startMotor(void);
void baMotor_stopMotor(void);
void baMotor_reinitialize(void);
void baMotor_entry(void *arg);
void baMotor_80250BA4(s32 arg0, s32 arg1, s32 arg2);
void baMotor_80250C08(void);
void baMotor_init(void);
void baMotor_stub(void);
void baMotor_80250D94(f32 arg0, f32 arg1, f32 arg2);
void baMotor_80250E6C(f32 arg0, f32 arg1);
void baMotor_80250E94(f32 arg0, f32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5);
void baMotor_forceStop(void);

OSMesgQueue *baMotor_getRetraceQueue(void); // [port] Watchdog diagnostics

#endif
