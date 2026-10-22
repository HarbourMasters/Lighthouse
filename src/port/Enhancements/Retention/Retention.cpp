// Helpers shared by the note, jinjo and consumable retention systems.
//
// libultraship must precede the decomp headers: core2/timedfunc.h (via functions.h) defines a
// C-compat `reinterpret_cast` macro that breaks the MSVC C++ standard library.
#include <libultraship/libultraship.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Enhancements/Retention/Retention.h"
#include "port/Rando/Rando.h" // gameFile_saveData, FILE_TYPE_SAVE_RANDO
#include "port/Save/SaveManager.h"
#include "port/ShipInit.hpp"

#include "functions.h"
extern "C" {
#include "enums.h"
}

namespace retention {

bool systemActive() {
    int32_t slot = SaveManager_GetActiveSlot();
    if (slot < 0 || gameFile_saveData[slot].shipSaveData.fileType == FILE_TYPE_SAVE_RANDO) {
        return false;
    }
    return !func_802E4A08();
}

} // namespace retention

static void RegisterRetentionSaving_Init() {
    // bcopy's scratch-slot save buffer doesn't carry our bits (or Brentilda's heard hints); sync them in.
    COND_HOOK(OnSaveFileSave, EVENT_PRIORITY_HIGH, true, [](IEvent* event) {
        OnSaveFileSave* ev = (OnSaveFileSave*)event;
        SaveData* buf = (SaveData*)ev->saveBuffer;
        ShipSaveData* live = SaveManager_GetActiveShipData();
        if (buf != nullptr && live != nullptr && &buf->shipSaveData != live) {
            buf->shipSaveData.noteRetention = live->noteRetention;
            buf->shipSaveData.jinjoRetention = live->jinjoRetention;
            buf->shipSaveData.consumableRetention = live->consumableRetention;
            buf->shipSaveData.heardBrentildaHints = live->heardBrentildaHints;
        }
    });
}

static RegisterShipInitFunc initRetentionSaving(RegisterRetentionSaving_Init);
