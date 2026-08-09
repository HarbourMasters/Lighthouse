#ifndef RANDO_H
#define RANDO_H

#include "StaticData/StaticData.h"
#include <libultraship/libultra/gbi.h>
#include "save.h"

extern "C" {
extern SaveData gameFile_saveData[4];
}

extern int16_t selectedFileNum;

#define DEFAULT_FILE_NUM -1

#define IS_RANDO                         \
    (selectedFileNum == DEFAULT_FILE_NUM \
         ? false                         \
         : gameFile_saveData[selectedFileNum].shipSaveData.fileType == FILE_TYPE_SAVE_RANDO)

#define RANDO_SAVE_CHECKS gameFile_saveData[selectedFileNum].shipSaveData.randoSaveData.randoSaveCheck
#define RANDO_SAVE_OPTIONS gameFile_saveData[selectedFileNum].shipSaveData.randoSaveData.randoSaveOption
#define RANDO_SAVE_FLAGS gameFile_saveData[selectedFileNum].shipSaveData.randoSaveData.randoSaveFlag

// Loaded rando file's seed id, or 0 if none.
#define RANDO_SEED \
    (selectedFileNum == DEFAULT_FILE_NUM ? 0 : gameFile_saveData[selectedFileNum].shipSaveData.randoSaveData.seedId)

// #define RANDO_SAVE_ENTRANCES(fileNum) gSaveBuffer.files[fileNum]->shipSaveData.randoSaveData.randoSaveEntrances
// #define RANDO_EVENTS gSaveContext.save.shipSaveInfo.rando.randoEvents
// #define RANDO_STARTING_ITEMS gSaveContext.save.shipSaveInfo.rando.randoStartingItems

namespace Rando {

void Init();

} // namespace Rando

#endif // RANDO_H
