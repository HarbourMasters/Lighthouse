#ifndef __BA_FLAP_H__
#define __BA_FLAP_H__

#include <ultra64.h>

#ifdef __cplusplus
extern "C" {
#endif


s32 baflap_getCount(void);
bool baflap_add(f32 duration);
void baflap_reset(void);
void baflap_activate(bool active);
void baflap_update(void);

#ifdef __cplusplus
}
#endif


#endif // __BA_FLAP_H__