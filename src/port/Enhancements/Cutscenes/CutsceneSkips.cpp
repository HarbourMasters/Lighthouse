// Cutscene Skip Enhancements
//
// Moves CVar checks for cutscene skipping out of decomp files and into
// port-side event listeners.

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
int getGameMode(void);
enum level_e level_get(void);
int volatileFlag_get(enum volatile_flags_e index);
int func_8028F070(void);
void gcparade_beginFFParade(void);
}

#define CVAR_SKIP_BOOT_LOGOS CVAR_ENHANCEMENT("Cutscenes.SkipBootLogos")
#define CVAR_SKIP_INTRO CVAR_ENHANCEMENT("Cutscenes.StartSkipIntro")
#define CVAR_SKIP_MISC_CUTSCENES CVAR_ENHANCEMENT("Cutscenes.SkipMiscCutscenes")
#define CVAR_SKIP_CLUCKER_CUTSCENE CVAR_ENHANCEMENT("Cutscenes.SkipCluckerCutscene")
#define CVAR_TRIGGER_FF_PARADE CVAR_DEVELOPER_TOOLS("TriggerFFParade")

void RegisterSkipBootLogos_Init() {
    COND_VB_SHOULD(VB_PLAY_BOOT_LOGOS, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_SKIP_BOOT_LOGOS, 0),
                   { *should = false; });
}

void RegisterSkipIntroCutscene_Init() {
    COND_VB_SHOULD(VB_PLAY_INTRO_CUTSCENE, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_SKIP_INTRO, 0),
                   { *should = false; });
}

void RegisterSkipMiscCutscenes_Init() {
    COND_HOOK(OnMiscCutscenesCheck, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_SKIP_MISC_CUTSCENES, 0),
              [](IEvent* event) {
                  auto* ev = reinterpret_cast<OnMiscCutscenesCheck*>(event);
                  *ev->skipMiscCutscenes = true;
              });
}

void RegisterSkipCluckerCutscene_Init() {
    COND_HOOK(OnGetLevelSpecificFlag, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_SKIP_CLUCKER_CUTSCENE, 0),
              [](IEvent* event) {
                  auto* ev = reinterpret_cast<OnGetLevelSpecificFlag*>(event);
                  if (ev->flagId == LEVEL_FLAG_14_TTC_UNKNOWN) {
                      ev->result = 1;
                      event->Cancelled = true;
                  }
              });
}

void RegisterTriggerFFParade_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_TRIGGER_FF_PARADE, 0), [](IEvent* event) {
        if (getGameMode() != GAME_MODE_3_NORMAL || level_get() <= 0 || !func_8028F070()) {
            return;
        }
        if (volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE) ||
            volatileFlag_get(VOLATILE_FLAG_20_BEGIN_CHARACTER_PARADE)) {
            return;
        }
        gcparade_beginFFParade();
    });
}

static RegisterShipInitFunc initBootLogosFunc(RegisterSkipBootLogos_Init, { CVAR_SKIP_BOOT_LOGOS });
static RegisterShipInitFunc initSkipIntroFunc(RegisterSkipIntroCutscene_Init, { CVAR_SKIP_INTRO });
static RegisterShipInitFunc initSkipMiscCutscenesFunc(RegisterSkipMiscCutscenes_Init, { CVAR_SKIP_MISC_CUTSCENES });
static RegisterShipInitFunc initSkipCluckerCutsceneFunc(RegisterSkipCluckerCutscene_Init,
                                                        { CVAR_SKIP_CLUCKER_CUTSCENE });
static RegisterShipInitFunc initTriggerFFParadeFunc(RegisterTriggerFFParade_Init, { CVAR_TRIGGER_FF_PARADE });
