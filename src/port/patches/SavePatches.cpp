// Save Patches
//
// Port-specific hooks called from decomp save/file-select code.
// Keeps the original .c files intact by consolidating port logic here.

#include <libultraship/libultraship.h>
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/ui/cvar_prefixes.h"

extern "C" {
#include "functions.h"
#include "variables.h"
#include "core1/sns.h"
#include "port/patches/Patches.h"

extern s32 D_80386068; // lives backup — func_80347AA8 restores from this
extern s32 D_80385F30[];
void sns_set_item_state(s32 item, s32 set, bool value);
void sns_update_global_save_data_checksum(void);
}

// File Load Hook
// Called from gameFile_load() in fileselect.c after saveData_load().

extern "C" void port_onFileLoaded(int gamenum, int filenum) {
    // Restore persisted enhancement data (extra lives, bottles bonus).
    port_restoreFileEnhancementData(filenum);

    // Sync the lives backup so func_80347AA8 restores correctly after map transitions.
    D_80386068 = D_80385F30[ITEM_16_LIFE];

    // Unlock Stop N' Swop items as a reward for 100% completion.
    if (CVarGetInteger(CVAR_ENHANCEMENT("Gameplay.StopNSwop100"), 0)) {
        if (jiggyscore_total() == 100 && fileProgressFlag_get(FILEPROG_FC_DEFEAT_GRUNTY)) {
            for (int i = 1; i < SNS_ITEM_length; i++) {
                sns_set_item_state(i, SNS_UNLOCKED, true);
            }
            sns_update_global_save_data_checksum();
        }
    }
}

// Bottles Bonus Index Sync
// Called from gameSelect.c after gameFile_load() when starting a game.
// Scans the completion array so the bottles bonus minigame resumes at the
// correct puzzle index.

extern "C" void port_syncBottlesBonusIndex(void) {
    extern u8 gCompletedBottlesBonusGames[7];
    extern s32 chBottlesBonusPuzzleIndex;

    for (int i = 0; i < 7; i++) {
        if (gCompletedBottlesBonusGames[i]) {
            chBottlesBonusPuzzleIndex = i + 1;
        }
    }
}
