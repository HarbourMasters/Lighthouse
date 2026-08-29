#include "Rando.h"
#include "port/Enhancements/Events/Hooks/Events.h"
// #include "port/Rando/EntranceTracker/EntranceTracker.h"
#include "port/Rando/Spoiler/Spoiler.h"
#include "port/ShipInit.hpp"

#include <filesystem>
namespace fs = std::filesystem;

int16_t selectedFileNum = DEFAULT_FILE_NUM;
const fs::path randomizerFolderPath(Ship::Context::GetPathRelativeToAppDirectory("randomizer", "bk64"));

// Entry point for the module, run once on game boot
void Rando::Init() {
    if (!fs::exists(randomizerFolderPath)) {
        fs::create_directory(randomizerFolderPath);
    }

    Rando::Spoiler::RefreshSpoilerLogs();
    // Rando::EntranceTracker::Init();
    // Ship::Context::GetInstance()->GetFileDropMgr()->RegisterDropHandler(Rando::Spoiler::HandleFileDropped);
}
