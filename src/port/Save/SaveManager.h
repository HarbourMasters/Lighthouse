#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H
#include "ship/Context.h"
#include <string>

#include "port/UI/cvar_prefixes.h"

#define CVAR_NAME_BOTTLES_BONUS CVAR_ENHANCEMENT("Saving.PersistBottlesBonus")

void SaveManager_Init();
std::string SaveManager_GetSavePath(const std::string& filename);

#endif // SAVE_MANAGER_H