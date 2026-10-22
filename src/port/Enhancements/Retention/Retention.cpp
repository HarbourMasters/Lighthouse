// Helpers shared by the note, jinjo and consumable retention systems.
//
// libultraship must precede the decomp headers: core2/timedfunc.h (via functions.h) defines a
// C-compat `reinterpret_cast` macro that breaks the MSVC C++ standard library.
#include <libultraship/libultraship.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"
#include "port/Rando/Rando.h" // selectedFileNum, gameFile_saveData, FILE_TYPE_SAVE_RANDO
#include "port/ShipInit.hpp"

#include "functions.h"
extern "C" {
#include "enums.h"
}

namespace retention {

int32_t activeSlot() {
    if (selectedFileNum < 0 || selectedFileNum >= 4 || gameSelect_getGameNumber() == -1) {
        return -1;
    }
    return (int32_t)selectedFileNum;
}

bool systemActive() {
    int32_t slot = activeSlot();
    if (slot < 0 || gameFile_saveData[slot].shipSaveData.fileType == FILE_TYPE_SAVE_RANDO) {
        return false;
    }
    return !func_802E4A08();
}

ShipSaveData* activeShipData() {
    int32_t slot = activeSlot();
    return slot >= 0 ? &gameFile_saveData[slot].shipSaveData : nullptr;
}

} // namespace retention

static void RegisterRetentionSaving_Init() {
    // bcopy's scratch-slot save buffer doesn't carry our bits; sync them in.
    COND_HOOK(OnSaveFileSave, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        OnSaveFileSave* ev = (OnSaveFileSave*)event;
        SaveData* buf = (SaveData*)ev->saveBuffer;
        ShipSaveData* live = retention::activeShipData();
        if (buf != nullptr && live != nullptr && &buf->shipSaveData != live) {
            buf->shipSaveData.noteRetention = live->noteRetention;
            buf->shipSaveData.jinjoRetention = live->jinjoRetention;
            buf->shipSaveData.consumableRetention = live->consumableRetention;
            buf->shipSaveData.heardBrentildaHints = live->heardBrentildaHints;
        }
    });
}

static RegisterShipInitFunc initRetentionSaving(RegisterRetentionSaving_Init);
