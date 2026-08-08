#ifndef RANDO_SPOILER_H
#define RANDO_SPOILER_H

#include <vector>
#include <string>
#include "port/Rando/Logic/Logic.h"
#include <filesystem>

namespace Rando {

namespace Spoiler {
extern std::vector<std::string> spoilerLogs;

nlohmann::ordered_json GenerateFromPoolGeneration();

void GenerateFromSpoiler(nlohmann::json spoiler);
void SaveToFile(const std::string& fileName, nlohmann::ordered_json spoiler);
nlohmann::json LoadFromFile(const std::string& filePath);

void RefreshSpoilerLogs();

} // namespace Spoiler

} // namespace Rando

#endif