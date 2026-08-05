#pragma once

#include "libultraship/libultra/types.h"
#include "enums.h"

void LoadGuiTextures();

#ifdef __cplusplus
#include <nlohmann/json.hpp>
#define WIDGET_COLOR UIWidgets::Colors(CVarGetInteger("gSettings.Menu.Theme", 5))
extern int32_t gSelectedFileNum;

using nlohmann::json;
json Ship_RetrieveSaveFile(int32_t filenum);
extern std::string Ship_ConvertEnumToReadableName(const std::string& input);
extern std::vector<file_progress_e> worldOpenFlags;

extern std::vector<std::string> worldNameList;
extern std::vector<std::string> abilityNameList;

void TableCellCenteredText(const char* text);
uint32_t Ship_Hash(std::string str);
std::string port_FormatTimeDisplay(uint64_t value);

extern "C" {
#endif

uint64_t GetUnixTimestamp();
bool Ship_IsCStringEmpty(const char* str);
int port_checkHeap(const char* label);

// Raw reads of the jiggy/honeycomb score bitfields, bypassing
// OnIsJiggyScoreCollected and OnIsHoneycombScoreCollected.
u32 port_jiggyscore_isCollectedRaw(enum jiggy_e jiggy_id);
bool port_honeycombscore_getRaw(enum honeycomb_e indx);

// SPDLOG level wrappers callable from C
void BK_LOG_DEBUG(const char* fmt, ...);
void BK_LOG_INFO(const char* fmt, ...);
void BK_LOG_WARN(const char* fmt, ...);
void BK_LOG_ERROR(const char* fmt, ...);

// Flag: when true, audio spin-waits should force-stop immediately.
extern int gPortResetPending;

// Get the name of a map by its ID
const char* port_mapName(int map_id);

// Get the boot sequence setting (0=Default, 1=Authentic, 2=FileSelect)
int port_getBootSequence(void);

// Currently selected game number (0-2), set at file pick. -1 if none.
extern s32 gSelectedGameNum;

// Per-frame controller input shaping
void port_shapeControllerInput(void* contPad);

// True in the non-interactive demo modes
bool IsDemoMode(void);

#ifdef __cplusplus
}
#endif
