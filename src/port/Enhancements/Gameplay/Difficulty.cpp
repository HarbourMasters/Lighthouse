#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Romhack/RomhackConfig.h"

extern "C" {
#include "enums.h"
#include "functions.h"
}

static bool sSaveErased = false;

void RegisterDamageScale_Init() {
    COND_HOOK(OnItemAdjust, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_ENHANCEMENT("Gameplay.DamageScale"), 1) > 1,
              [](IEvent* event) {
                  auto* ev = (OnItemAdjust*)event;
                  if (ev->item != ITEM_14_HEALTH || *ev->diff >= 0) {
                      return;
                  }
                  int mode = CVarGetInteger(CVAR_ENHANCEMENT("Gameplay.DamageScale"), 1);
                  *ev->diff = (mode >= 4) ? -9999 : *ev->diff * mode;
              });
}

static RegisterShipInitFunc initDamageScale(RegisterDamageScale_Init, { CVAR_ENHANCEMENT("Gameplay.DamageScale") });

void RegisterDifficulty_Init() {
    // Permadeath, ignore lives and game over with file erase
    COND_HOOK(OnPlayerDeath, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_ENHANCEMENT("Gameplay.Permadeath"), 0),
              [](IEvent* event) {
                  if (RomhackConfig_IsActive()) {
                      return;
                  }
                  item_adjustByDiffWithoutHud(ITEM_16_LIFE, -item_getCount(ITEM_16_LIFE));

                  s32 gamenum = gameSelect_getGameNumber();
                  if (gamenum == -1) {
                      return;
                  }

                  gameFile_clear(gamenum);
                  gameFile_8033CFD4(gamenum);
                  sSaveErased = true;
              });

    COND_HOOK(OnGameLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) { sSaveErased = false; });

    COND_VB_SHOULD(VB_SAVE_AND_EXIT, EVENT_PRIORITY_NORMAL, true, {
        if (sSaveErased) {
            *should = false;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterDifficulty_Init, { CVAR_ENHANCEMENT("Gameplay.Permadeath") });
