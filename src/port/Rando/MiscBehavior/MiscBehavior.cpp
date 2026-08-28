#include "MiscBehavior.h"

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/Spoiler/Spoiler.h"

#include "save.h"

extern "C" {
#include "core2/gc/zoombox.h"

extern s8 gameFile_GameIdToFileIdMap[4];
void func_803152C4(GcZoombox* self);
}

static void FinishPortraitCrossfade(GcZoombox* zoombox) {
    func_803152C4(zoombox);
    zoombox->unk1A4_14 = 0;
    zoombox->unk1A4_13 = 0;
    zoombox->unk17C = 0.0f;
}

void RegisterMiscBehaviour() {
    REGISTER_LISTENER(OnFileSelectPortrait, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnFileSelectPortrait* ev = (OnFileSelectPortrait*)event;

        if (ev->zoombox == nullptr || ev->gamenum < 0 || ev->gamenum >= 3) {
            return;
        }

        int32_t fileNum = gameFile_GameIdToFileIdMap[ev->gamenum];
        bool isRando = fileNum >= 0 && fileNum < 4 && gameFile_saveData[fileNum].magic != 0 &&
                       gameFile_saveData[fileNum].shipSaveData.fileType == FILE_TYPE_SAVE_RANDO;

        // Cheato zoombox sprite for Rando files, otherwise Vanilla
        GcZoomboxSprite wanted = isRando ? ZOOMBOX_SPRITE_5B_CHEATO : ZOOMBOX_SPRITE_C_BANJO_2;
        if (gczoombox_loadSprite((GcZoombox*)ev->zoombox, wanted)) {
            FinishPortraitCrossfade((GcZoombox*)ev->zoombox);
        }
    });

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

    REGISTER_LISTENER(OnLoadFileSelect, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnLoadFileSelect* ev = (OnLoadFileSelect*)event;

        selectedFileNum = DEFAULT_FILE_NUM;
    });
}

static RegisterShipInitFunc initFunc(RegisterMiscBehaviour, { "IS_RANDO" });
