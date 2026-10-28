#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <PR/libaudio.h>

ALBankFile* AudioSoundFont_LoadBankFile(const char* descriptorPath);
void AudioSoundFont_FreeAll(void);

#ifdef __cplusplus
}
#endif
