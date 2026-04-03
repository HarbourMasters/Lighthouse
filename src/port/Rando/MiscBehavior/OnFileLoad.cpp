#include "MiscBehavior.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/enhancements/events/hooks/Events.h"
#include "port/ui/Notification.h"

// #include "port/Rando/Logic/Logic.h"
// #include "port/Rando/Spoiler/Spoiler.h"

extern "C" {
bool gameFile_isNotEmpty(s32 gamenum);
}

void Rando::MiscBehavior::OnFileLoad() {
    REGISTER_LISTENER(OnGameFileLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnGameFileLoad* ev = (OnGameFileLoad*)event;
        selectedFileNum = ev->fileNum;

        if (gameFile_isNotEmpty(selectedFileNum)) {
            return;
        }

        if (!IS_RANDO) {
            if (!CVarGetInteger("gRandoSettings.Enabled", 0)) {
                gameFile_saveData[selectedFileNum].shipSaveData.randoSaveData.isRando = true;
            }
        }
    });
}
