#include <libultraship/bridge/consolevariablebridge.h>
#include "ship/Context.h"
#include "port/ui/cvar_prefixes.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"

#include <fstream>
#include <filesystem>
#include "port/save/Types.h"

extern "C" {
extern s32 D_80385F30[0x2C];
extern u8 D_8037DCC7;
extern u8 D_8037DCC8;
extern u8 D_8037DCC9;
extern u8 D_8037DCCA;

extern u8 gCompletedBottlesBonusGames[7];
}

using nlohmann::json;
namespace fs = std::filesystem;

#define CVAR_NAME_EXTRA_LIVES CVAR_ENHANCEMENT("Saving.PersistExtraLives")
#define CVAR_NAME_BOTTLES_BONUS CVAR_ENHANCEMENT("Saving.PersistExtraLives")

#define CVAR_EXTRA_LIVES CVarGetInteger(CVAR_NAME_EXTRA_LIVES, 0)
#define CVAR_BOTTLES_BONUS CVarGetInteger(CVAR_NAME_BOTTLES_BONUS, 0)

void RegisterRestoreExtraLives_Init() {
    REGISTER_LISTENER(OnGameLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnGameLoad* ev = (OnGameLoad*)event;

        if (!CVAR_EXTRA_LIVES) {
            D_80385F30[ITEM_16_LIFE] = 3;
            return;
        }

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
    REGISTER_LISTENER(OnGameLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnGameLoad* ev = (OnGameLoad*)event;

        if (!CVAR_BOTTLES_BONUS) {
            return;
        }

        if (ev->fileNum < 0 || ev->fileNum >= SAVE_SLOT_COUNT) {
            return;
        }

        json j = Ship_RetrieveSaveFile(ev->fileNum);

        if (j.contains("enhancements")) {
            if (j["enhancements"].contains("bottlesBonusCompleted")) {
                const auto& bb = j["enhancements"]["bottlesBonusCompleted"];
                bool anyCompleted = false;
                for (int k = 0; k < 7 && k < (int)bb.size(); k++) {
                    int state = gCompletedBottlesBonusGames[k] |= bb[k].get<int>() ? 1 : 0;
                    if (state == 1) {
                        anyCompleted = true;
                    }
                }
                if (anyCompleted) {
                    D_8037DCC7 = 1; // skip instructions text
                    D_8037DCC8 = 1; // skip quit lose text
                    D_8037DCC9 = 1; // skip timeout lose text
                    D_8037DCCA = 1; // skip "secret game discovered" text
                }
            }
        }
    })
}

static RegisterShipInitFunc initExtraLivesFunc(RegisterRestoreExtraLives_Init, { CVAR_NAME_EXTRA_LIVES });
static RegisterShipInitFunc initBottleBonusFunc(RegisterRestoreBottlesBonus_Init, { CVAR_NAME_BOTTLES_BONUS });