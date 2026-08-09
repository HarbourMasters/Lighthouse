#include <libultraship/bridge/consolevariablebridge.h>
#include "ship/Context.h"
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"

#include <fstream>
#include <filesystem>
#include "port/Save/Types.h"

extern "C" {
#include "variables.h"
#include "core1/sns.h"

extern s32 D_80385F30[0x2C];

// Bottles bonus text flags
// Skip for persistence
extern u8 D_8037DCC7;
extern u8 D_8037DCC8;
extern u8 D_8037DCC9;
extern u8 D_8037DCCA;

extern u8 gCompletedBottlesBonusGames[7];

s32 jiggyscore_total(void);
bool fileProgressFlag_get(enum file_progress_e flag);
void sns_set_item_state(s32 item, s32 set, bool value);
void sns_update_global_save_data_checksum(void);
}

using nlohmann::json;
namespace fs = std::filesystem;

#define CVAR_NAME_EXTRA_LIVES CVAR_ENHANCEMENT("Saving.PersistExtraLives")
#define CVAR_NAME_BOTTLES_BONUS CVAR_ENHANCEMENT("Saving.PersistBottlesBonus")
#define CVAR_NAME_STOPNSWOP CVAR_ENHANCEMENT("Gameplay.StopNSwop100")

#define CVAR_EXTRA_LIVES CVarGetInteger(CVAR_NAME_EXTRA_LIVES, 0)
#define CVAR_BOTTLES_BONUS CVarGetInteger(CVAR_NAME_BOTTLES_BONUS, 0)
#define CVAR_STOPNSWOP CVarGetInteger(CVAR_NAME_STOPNSWOP, 0)

// Called from gameSelect.c after gameFile_load() when starting a game.
extern "C" void port_syncBottlesBonusIndex(void) {
    extern s32 chBottlesBonusPuzzleIndex;

    for (int i = 0; i < 7; i++) {
        if (gCompletedBottlesBonusGames[i]) {
            chBottlesBonusPuzzleIndex = i + 1;
        }
    }
}

void RegisterStopNSwop100_Init() {
    COND_HOOK(OnGameStart, EVENT_PRIORITY_NORMAL, CVAR_STOPNSWOP, [](IEvent* event) {
        if (jiggyscore_total() == 100 && fileProgressFlag_get(FILEPROG_FC_DEFEAT_GRUNTY)) {
            for (int i = 1; i < SNS_ITEM_length; i++) {
                sns_set_item_state(i, SNS_UNLOCKED, true);
            }
            sns_update_global_save_data_checksum();
        }
    });
}

void RegisterRestoreExtraLives_Init() {
    COND_HOOK(OnGameLoad, EVENT_PRIORITY_NORMAL, CVAR_EXTRA_LIVES, [](IEvent* event) {
        OnGameLoad* ev = (OnGameLoad*)event;

        if (ev->fileNum < 0 || ev->fileNum >= SAVE_SLOT_COUNT) {
            return;
        }

        json j = Ship_RetrieveSaveFile(ev->fileNum);

        if (j.contains("enhancements")) {
            if (j["enhancements"].contains("life")) {
                D_80385F30[ITEM_16_LIFE] = j["enhancements"]["life"].get<int>();
            }
        }
    })
}

void RegisterRestoreBottlesBonus_Init() {
    COND_HOOK(OnGameLoad, EVENT_PRIORITY_NORMAL, CVAR_BOTTLES_BONUS, [](IEvent* event) {
        // Bottles bonus data is loaded from global.json at init.
        // On game load, just set the skip-text flags if any are completed.
        bool anyCompleted = false;
        for (int k = 0; k < 7; k++) {
            if (gCompletedBottlesBonusGames[k]) {
                anyCompleted = true;
                break;
            }
        }
        if (anyCompleted) {
            D_8037DCC7 = 1;
            D_8037DCC8 = 1;
            D_8037DCC9 = 1;
            D_8037DCCA = 1;
        }
    });
}

static RegisterShipInitFunc initExtraLivesFunc(RegisterRestoreExtraLives_Init, { CVAR_NAME_EXTRA_LIVES });
static RegisterShipInitFunc initBottleBonusFunc(RegisterRestoreBottlesBonus_Init, { CVAR_NAME_BOTTLES_BONUS });
static RegisterShipInitFunc initStopNSwopFunc(RegisterStopNSwop100_Init, { CVAR_NAME_STOPNSWOP });