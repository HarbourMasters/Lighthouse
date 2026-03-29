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

// [port] Get the widescreen logical width (320 at 4:3, wider for widescreen)
int port_getViewportWidth(void);

// [port] Demo frame pacing: returns the N64 VI count for the current demo tick
// (typically 2 for 30fps, higher during original frame drops). Returns 0 outside demos.
int port_getDemoViCount(void);

// stick axis via LUS controller mappings. Used to enable the stick
// diagonal/rebound filter only when relevant.
bool port_CButtonIsAxis(void);

#ifdef __cplusplus
}
#endif
