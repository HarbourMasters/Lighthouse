#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H
#include "ship/Context.h"
#include <nlohmann/json.hpp>
#include <string>

#include "port/UI/cvar_prefixes.h"
#include <libultraship/libultra/types.h>
#include "save.h"

#define CVAR_NAME_BOTTLES_BONUS CVAR_ENHANCEMENT("Saving.PersistBottlesBonus")

void SaveManager_Init();
std::string SaveManager_GetSavePath(const std::string& filename);
int32_t SaveManager_GetActiveSlot();
ShipSaveData* SaveManager_GetActiveShipData();
std::string CollapsedJSONArray(const nlohmann::ordered_json& jsonFile);
nlohmann::ordered_json Convert_SaveDataToJSON(SaveData* saveData, int32_t fileNum);
SaveData* Convert_JSONToSaveData(int32_t fileNum);

#endif // SAVE_MANAGER_H