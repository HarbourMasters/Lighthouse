// Save Patches
//
// Port-specific hooks for save/file-select. StopNSwop 100% unlock
// listens on OnGameLoad; Bottles Bonus index sync is called directly
// from gameSelect.c after gameFile_load().

#include <libultraship/libultraship.h>
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/ui/cvar_prefixes.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "core1/sns.h"

s32 jiggyscore_total(void);
bool fileProgressFlag_get(enum file_progress_e flag);
void sns_set_item_state(s32 item, s32 set, bool value);
void sns_update_global_save_data_checksum(void);
}

// Called from gameSelect.c after gameFile_load() when starting a game.
extern "C" void port_syncBottlesBonusIndex(void) {
    extern u8 gCompletedBottlesBonusGames[7];
    extern s32 chBottlesBonusPuzzleIndex;

    for (int i = 0; i < 7; i++) {
        if (gCompletedBottlesBonusGames[i]) {
            chBottlesBonusPuzzleIndex = i + 1;
        }
    }
}

void RegisterSavePatches_Init() {
    REGISTER_LISTENER(OnGameLoad, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        if (CVarGetInteger(CVAR_ENHANCEMENT("Gameplay.StopNSwop100"), 0)) {
            if (jiggyscore_total() == 100 && fileProgressFlag_get(FILEPROG_FC_DEFEAT_GRUNTY)) {
                for (int i = 1; i < SNS_ITEM_length; i++) {
                    sns_set_item_state(i, SNS_UNLOCKED, true);
                }
                sns_update_global_save_data_checksum();
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterSavePatches_Init, { CVAR_ENHANCEMENT("Gameplay.StopNSwop100") });
