#ifndef RANDO_H
#define RANDO_H

// #include "port/Rando/CustomObject/CustomObject.h"

#include "StaticData/StaticData.h"
#include <libultraship/libultra/gbi.h>
#include "save.h"

extern "C" {
extern SaveData gameFile_saveData[4];
enum map_e map_get();
}


#define IS_RANDO (gameFile_saveData[selectedFileNum].shipSaveData.fileType == FILE_TYPE_SAVE_RANDO)
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
