#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void PortEnhancements_Register();
void PortEnhancements_Init();
void PortEnhancements_Exit();

void RegisterReturnToLair_Init();

#ifdef __cplusplus
};
#endif
