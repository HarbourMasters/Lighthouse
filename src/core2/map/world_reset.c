// BanjoDecomp: core2/code_9A740.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

extern void rbb_propellorCtrl_stop(void); //rbb

typedef struct {
    u8 entered_from_lair;
    u8 level;
} struct_9A740;

/* .bss */
struct {
    u8 entered_from_lair; // set when the last map change went from the lair into a level
    u8 level;
}sLevelState;

/* .code */
/* Enters the level of `map`: called by game_loadMap when the new map is in a different level
 * (or a level reset was requested), and by game_init and the demo player. Loads the overlay for
 * the map's level, then resets the per-level state (item scores, spawned jiggies, level flags,
 * level-specific props) unless game_getKeepLevelState() asks to keep it (cleared here). */
void level_load(enum map_e map){
    s32 prev_lvl = sLevelState.level;
    sLevelState.level = map_getLevel(map);
    overlayManager_load(leveloverlay_getOverlayFromLevel(sLevelState.level));
    sLevelState.entered_from_lair = 0;
    if(game_getKeepLevelState()){
        game_clearKeepLevelState();
    }else{
        if( sLevelState.level != LEVEL_6_LAIR 
            && sLevelState.level != LEVEL_C_BOSS
            && prev_lvl == LEVEL_6_LAIR
        ){
            sLevelState.entered_from_lair = 1;
        }
    
        // [port] Romhack gate: hacks that keep per-level world state replace this
        // outright, snapshotting the outgoing level and restoring the incoming one.
        if (EventSystem_Should(VB_LEVEL_LOAD_SAVESTATE_INIT, true, prev_lvl)) {
            mapSavestate_init();
        }
        if (EventSystem_Should(VB_LEVEL_LOAD_RESET_SCORES, true, sLevelState.level)) {
            itemscore_levelReset(sLevelState.level);
            CALL_EVENT(OnLevelReset, sLevelState.level);
            jiggyscore_clearAllSpawned();
            levelSpecificFlags_clear();
        }
        bsStoredState_clearTimers();
        func_803219A8();
        if( volatileFlag_getAndSet(VOLATILE_FLAG_17, false) 
            && getGameMode() != 0
            && sLevelState.level != LEVEL_D_CUTSCENE
            && map != MAP_91_FILE_SELECT
        ){
            volatileFlag_set(VOLATILE_FLAG_18, true);
        }

        if (EventSystem_Should(VB_LEVEL_LOAD_RESET_MAP_SETPIECES, true, map)) {
            if(sLevelState.level == LEVEL_9_RUSTY_BUCKET_BAY){
                rbb_propellorCtrl_reset();
            }

            switch(map){
                case MAP_2_MM_MUMBOS_MOUNTAIN:
                    mm_resetHuts();
                    break;
                case MAP_7_TTC_TREASURE_TROVE_COVE:
                    chTreasurehunt_resetProgress();
                    break;
                case MAP_1B_MMM_MAD_MONSTER_MANSION:
                    chFlowerpot_reset();
                    break;
            }
        }
    }
}

/* Leaves the current level: called by game_loadMap before level_load, and by game_free and the
 * demo player */
void level_unload(void){
    if(!game_getKeepLevelState()){
        if( sLevelState.level == LEVEL_9_RUSTY_BUCKET_BAY){
            rbb_propellorCtrl_stop();
        }

        if( sLevelState.level == LEVEL_1_MUMBOS_MOUNTAIN
            && getGameMode() != 0
            && fileProgressFlag_get(FILEPROG_31_MM_OPEN)
            && !fileProgressFlag_get(FILEPROG_C1_BADDIES_ESCAPE_TEXT)
        ){
            volatileFlag_set(VOLATILE_FLAG_22, 1);
        }
        bsStoredState_8029A924(); //null
        func_803465BC(); //null
        // [port] Romhack gate: see gcparade_8031ABF8.
        if (EventSystem_Should(VB_MAP_SAVESTATE_CLEAR_ALL, true)) {
            mapSavestate_clearAll();
        }
        func_8032196C();
    }
}

enum level_e level_get(void){
    return sLevelState.level;
}

int level_enteredFromLair(void){
    return sLevelState.entered_from_lair;
}

void level_setEnteredFromLair(int arg0){
    sLevelState.entered_from_lair = arg0;
}

void level_update(void){
    if(sLevelState.level == LEVEL_9_RUSTY_BUCKET_BAY){
        rbb_propellorCtrl_update();
    }
}
