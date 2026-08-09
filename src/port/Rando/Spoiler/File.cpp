#include "Spoiler.h"
#include "port/UI/Notification.h"
#include "ship/Context.h"
#include <fstream>

extern std::string CollapsedJSONArray(const nlohmann::ordered_json& jsonFile);

namespace fs = std::filesystem;

namespace Rando {

namespace Spoiler {

void SaveToFile(const std::string& fileName, nlohmann::ordered_json spoiler) {
    std::string testDirectory = Ship::Context::GetPathRelativeToAppDirectory("randomizer/", "bk64");
    std::string filePath = Ship::Context::GetPathRelativeToAppDirectory("randomizer/" + fileName, "bk64");

    if (!fs::exists(testDirectory)) {
        fs::create_directories(testDirectory);
    }

    std::ofstream fileStream(filePath);
    if (!fileStream.is_open()) {
        Notification::Emit(
            { .message = "Error: Failed to open spoiler file.", .messageColor = ImVec4(0.85f, 0.3f, 0, 1) });
        return;
    }

    std::string collapsedString = CollapsedJSONArray(spoiler);
    if (fileStream.is_open()) {
        fileStream << collapsedString;
        fileStream.close();
    } else {
        Notification::Emit(
            { .message = "Error: Failed to open save file.", .messageColor = ImVec4(0.85f, 0.3f, 0, 1) });
    }

    fileStream << spoiler.dump(4);
    RefreshSpoilerLogs();
}

nlohmann::json LoadFromFile(const std::string& fileName) {
    nlohmann::json spoiler;
    std::string spoilerFilePath = Ship::Context::GetPathRelativeToAppDirectory("randomizer/" + fileName, "bk64");
    std::ifstream fileStream(spoilerFilePath);

    if (!fs::exists(spoilerFilePath)) {
        return spoiler;
    }

    if (!fileStream.is_open()) {
        Notification::Emit(
            { .message = "Error: Failed to open spoiler file.", .messageColor = ImVec4(0.85f, 0.3f, 0, 1) });
        return spoiler;
    }

    try {
        fileStream >> spoiler;
    } catch (nlohmann::json::exception& e) {
        throw std::runtime_error("Failed to parse spoiler file: " + std::string(e.what()));
    }

    if (!spoiler.contains("type") || spoiler["type"] != "LIGHTHOUSE_RANDO_SPOILER") {
        throw std::runtime_error("Spoiler file is not a valid spoiler file");
    }

    return spoiler;
}

} // namespace Spoiler

} // namespace Rando
