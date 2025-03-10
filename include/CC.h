#ifndef CC_H
#define CC_H

#include "functions.h"
#include "variables.h"
#include <ultra64.h>

#ifdef __cplusplus
extern "C" {
#endif

s32 getClankerState(void);
void updateClankerState(s32 arg0);
int isClankerInState3(void);
void playClankerCutscene(void);
void func_8031CD20(s32 arg0, s32 arg1, s32 arg2);
void getClankerBonePosition(f32 arg0[3], f32 arg1[3]);
void getClankerBoneRotation(f32 arg0[3], f32 arg1[3]);
void CC_func_80387D4C(void);
void setClankerBoneTransform(s32 arg0);
void activateClanker(void);

#ifdef __cplusplus
}
#endif


#endif // CC_H
