#include "MiscBehavior.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/UI/Notification.h"

#include "port/Save/Types.h"
#include "port/UI/cvar_prefixes.h"

#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CheckTracker/CheckTracker.h"
// #include "port/Rando/Spoiler/Spoiler.h"

extern "C" {
enum map_e gsworld_getMap(void);
}

void Rando::MiscBehavior::OnFileLoad() {
    REGISTER_LISTENER(OnGameLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnGameLoad* ev = (OnGameLoad*)event;
        selectedFileNum = ev->fileNum;
        Rando::Logic::shuffledPool.clear();
    });

    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSaveLoad* ev = (OnSaveLoad*)event;
        SaveData* saveData = (SaveData*)ev->saveData;

        Rando::Logic::shuffledPool.clear();

        if (saveData->magic != 0) {
            if (saveData->shipSaveData.fileType == FILE_TYPE_SAVE_RANDO) {
                Rando::Logic::GeneratePoolFromSaveData(saveData);
                CALL_EVENT(InitRandoEvents);
            }
            return;
        }

        if (CVarGetInteger(CVAR_RANDOMIZER_SETTING("Enable"), 0)) {
            Rando::Logic::InitializeSaveData(saveData);
            Rando::Logic::GenerateShufflePool(saveData);
            Rando::Logic::GrantStartingLoadout();
            saveData->shipSaveData.fileType = FILE_TYPE_SAVE_RANDO;
            saveData->shipSaveData.fileCreatedAt = GetUnixTimestamp();
            CALL_EVENT(InitRandoEvents);
        }
    });

    REGISTER_LISTENER(OnLoadFileSelect, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnLoadFileSelect* ev = (OnLoadFileSelect*)event;

        isCheckTrackerInitialized = false;
        selectedFileNum = DEFAULT_FILE_NUM;
    });
}
