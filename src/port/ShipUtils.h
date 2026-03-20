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

// [port] Map enum to human-readable name
const char* port_mapName(int map_id);

// [port] Update window title with current map ID for debugging
void port_setMapDebugTitle(int map_id);

// [port] Get the boot sequence setting (0=Default, 1=Authentic, 2=FileSelect)
int port_getBootSequence(void);

// [port] Get the widescreen logical width (320 at 4:3, wider for widescreen)
int port_getViewportWidth(void);

#ifdef __cplusplus
}
#endif
