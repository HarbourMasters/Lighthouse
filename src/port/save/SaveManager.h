#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H
#include "ship/Context.h"
#include <string>

void SaveManager_Init();
std::string SaveManager_GetSavePath(const std::string& filename);

#endif // SAVE_MANAGER_H