#pragma once

#include "libultraship/libultra/types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t GetUnixTimestamp();
bool Ship_IsCStringEmpty(const char* str);
int port_checkHeap(const char* label);

// [port] SPDLOG level wrappers callable from C
void BK_LOG_INFO(const char* fmt, ...);
void BK_LOG_WARN(const char* fmt, ...);
void BK_LOG_ERROR(const char* fmt, ...);

// [port] Start the audio processing thread (called from audioManager_startThread after soundfont patching)
void port_audioStartThread(void);

const char* port_mapName(int map_id);

// [port] Get the boot sequence setting (0=Default, 1=Authentic, 2=FileSelect)
int port_getBootSequence(void);

// [port] Currently selected game number (0-2), set at file pick. -1 if none.
extern s32 gSelectedGameNum;

// stick axis via LUS controller mappings. Used to enable the stick
// diagonal/rebound filter only when relevant.
bool port_CButtonIsAxis(void);

#ifdef __cplusplus
}
#endif
