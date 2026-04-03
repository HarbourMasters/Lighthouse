#ifndef RANDO_H
#define RANDO_H

// #include "port/Rando/CustomObject/CustomObject.h"
// #include "include/types.h"

#include <libultraship/libultra/gbi.h>
#include "save.h"

extern "C" {
extern SaveData gameFile_saveData[4];
}

#define IS_RANDO (gameFile_saveData[selectedFileNum].shipSaveData.randoSaveData.isRando)
// #define IS_RANDO(fileNum) (gSaveBuffer.files[fileNum]->shipSaveData.features.rando)
// #define RANDO_SAVE_CHECKS(fileNum) gSaveBuffer.files[fileNum]->shipSaveData.randoSaveData.randoSaveChecks
// #define RANDO_SAVE_ENTRANCES(fileNum) gSaveBuffer.files[fileNum]->shipSaveData.randoSaveData.randoSaveEntrances
// #define RANDO_SAVE_OPTIONS(fileNum) gSaveBuffer.files[fileNum]->shipSaveData.randoSaveData.randoSaveOptions
// #define RANDO_EVENTS gSaveContext.save.shipSaveInfo.rando.randoEvents
// #define RANDO_STARTING_ITEMS gSaveContext.save.shipSaveInfo.rando.randoStartingItems

extern int16_t selectedFileNum;

namespace Rando {

void Init();
// bool IsItemObtainable(RandoItemId randoItemId, RandoCheckId randoCheckId = RC_UNKNOWN);
} // namespace Rando

#endif // RANDO_H
