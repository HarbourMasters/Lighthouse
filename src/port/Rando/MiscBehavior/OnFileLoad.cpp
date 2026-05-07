#include "MiscBehavior.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/enhancements/events/hooks/Events.h"
#include "port/ui/Notification.h"

#include "port/save/Types.h"

#include "port/Rando/Logic/Logic.h"
// #include "port/Rando/Spoiler/Spoiler.h"

void Rando::MiscBehavior::OnFileLoad() {
    REGISTER_LISTENER(OnGameLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnGameLoad* ev = (OnGameLoad*)event;
        selectedFileNum = ev->fileNum;
    });

    REGISTER_LISTENER(OnSaveLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnSaveLoad* ev = (OnSaveLoad*)event;
        SaveData* saveData = (SaveData*)ev->saveData;

        if (saveData->magic != 0) {
            return;
        }

        if (CVarGetInteger("gRandoSettings.Enable", 0)) {
            saveData->shipSaveData.fileType = FILE_TYPE_SAVE_RANDO;
            Rando::Logic::GenerateShufflePool();
        }
    });
}
