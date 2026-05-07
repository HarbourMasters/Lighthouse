#pragma once

#include "libultraship/libultra/types.h"

void LoadGuiTextures();

#ifdef __cplusplus
#include <nlohmann/json.hpp>
#define WIDGET_COLOR UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5))

using nlohmann::json;
json Ship_RetrieveSaveFile(int32_t filenum);
extern std::string Ship_ConvertEnumToReadableName(const std::string& input);

extern "C" {
#endif

uint64_t GetUnixTimestamp();
bool Ship_IsCStringEmpty(const char* str);
int port_checkHeap(const char* label);

// SPDLOG level wrappers callable from C
void BK_LOG_INFO(const char* fmt, ...);
void BK_LOG_WARN(const char* fmt, ...);
void BK_LOG_ERROR(const char* fmt, ...);

// Start the audio processing thread (called from audioManager_startThread after soundfont patching)
void port_audioStartThread(void);

// Flag: when true, audio spin-waits should force-stop immediately.
extern int gPortResetPending;

// Get the name of a map by its ID
const char* port_mapName(int map_id);

// Get the boot sequence setting (0=Default, 1=Authentic, 2=FileSelect)
int port_getBootSequence(void);

// Currently selected game number (0-2), set at file pick. -1 if none.
extern s32 gSelectedGameNum;

// Check if any C button is mapped to a stick axis via LUS controller mappings.
bool port_CButtonIsAxis(void);

#ifdef __cplusplus
}
#endif
