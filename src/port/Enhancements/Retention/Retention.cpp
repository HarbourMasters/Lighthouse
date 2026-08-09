// Helpers shared by the note and jinjo retention systems.
//
// libultraship must precede the decomp headers: core2/timedfunc.h (via functions.h) defines a
// C-compat `reinterpret_cast` macro that breaks the MSVC C++ standard library.
#include <libultraship/libultraship.h>
#include "port/Enhancements/Retention/Retention.h"
#include "port/Rando/Rando.h" // selectedFileNum, gameFile_saveData, DEFAULT_FILE_NUM, FILE_TYPE_SAVE_RANDO

#include "functions.h"
extern "C" {
#include "enums.h"
}

namespace retention {

int32_t activeSlot() {
    if (selectedFileNum == DEFAULT_FILE_NUM || selectedFileNum < 0 || selectedFileNum >= 4) {
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

} // namespace retention
