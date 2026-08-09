#include "MiscBehavior.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "src/port/ShipInit.hpp"
#include "ship/Context.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/UI/Notification.h"

#include "port/Save/Types.h"
#include "port/UI/cvar_prefixes.h"

#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CheckTracker/CheckTracker.h"
#include "port/Rando/Spoiler/Spoiler.h"
#include "port/Rando/CheckTracker/CheckTracker.h"

extern "C" {
enum map_e gsworld_getMap(void);
}

extern "C" {
enum map_e gsworld_getMap(void);
}

void Rando::MiscBehavior::OnFileLoad() {
    REGISTER_LISTENER(OnGameLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnGameLoad* ev = (OnGameLoad*)event;
        selectedFileNum = ev->fileNum;
        Rando::Logic::shuffledPool.clear();
    });

    REGISTER_LISTENER(OnGameStart, EVENT_PRIORITY_NORMAL, [](IEvent* event) { ShipInit::Init("IS_RANDO"); });

    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSaveLoad* ev = (OnSaveLoad*)event;
        SaveData* saveData = (SaveData*)ev->saveData;

        Rando::Logic::shuffledPool.clear();

        if (saveData->magic != 0) {
            if (saveData->shipSaveData.fileType == FILE_TYPE_SAVE_RANDO) {
                Rando::Logic::GeneratePoolFromSaveData(saveData);
            }
            return;
        }

        if (CVarGetInteger(CVAR_RANDOMIZER_SETTING("Enable"), 0)) {
            Rando::Logic::InitializeSaveData(saveData);
            std::string spoilerPath = CVarGetString(CVAR_RANDOMIZER_SETTING("SpoilerFile"), "");
            if (CVarGetInteger(CVAR_RANDOMIZER_SETTING("UseExistingLog"), 0) && !spoilerPath.empty()) {
                // std::string spoilerPath = CVarGetString(CVAR_RANDOMIZER_SETTING("SpoilerFile"), "");
                Rando::Spoiler::GenerateFromSpoiler(Rando::Spoiler::LoadFromFile(spoilerPath.c_str()));
            } else {
                Rando::Logic::GenerateShufflePool(saveData);
                Rando::Logic::GrantStartingLoadout();
                Rando::Logic::GrantFileProgressFlags();
                std::string spoilerName = std::to_string(saveData->shipSaveData.randoSaveData.seedId).c_str();
                std::erase(spoilerName, '-');
                spoilerName += ".json";
                Rando::Spoiler::SaveToFile(spoilerName, Rando::Spoiler::GenerateFromPoolGeneration());
            }

            saveData->shipSaveData.fileType = FILE_TYPE_SAVE_RANDO;
            saveData->shipSaveData.fileCreatedAt = GetUnixTimestamp();
        }
    });

    REGISTER_LISTENER(OnGameStart, EVENT_PRIORITY_NORMAL, [](IEvent* event) { CALL_EVENT(InitRandoEvents); });

    REGISTER_LISTENER(OnLoadFileSelect, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnLoadFileSelect* ev = (OnLoadFileSelect*)event;

        selectedFileNum = DEFAULT_FILE_NUM;
    });
}
