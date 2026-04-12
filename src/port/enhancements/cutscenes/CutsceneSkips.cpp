// Cutscene Skip Enhancements
//
// Moves CVar checks for cutscene skipping out of decomp files and into
// port-side event listeners.

#include <libultraship/bridge.h>
#include "port/ui/cvar_prefixes.h"
#include "port/enhancements/events/hooks/Events.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
}

#define CVAR_SKIP_BOOT_LOGOS CVAR_ENHANCEMENT("Cutscenes.SkipBootLogos")
#define CVAR_SKIP_INTRO CVAR_ENHANCEMENT("Cutscenes.StartSkipIntro")
#define CVAR_SKIP_JIGGY_DANCE CVAR_ENHANCEMENT("Cutscenes.SkipJiggyDance")

void RegisterSkipBootLogos_Init() {
    COND_HOOK(OnBootLogosCheck, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_SKIP_BOOT_LOGOS, 0), [](IEvent* event) {
        auto* ev = reinterpret_cast<OnBootLogosCheck*>(event);
        *ev->skipLogos = true;
    });
}

void RegisterSkipIntroCutscene_Init() {
    COND_HOOK(OnIntroCutsceneCheck, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_SKIP_INTRO, 0), [](IEvent* event) {
        auto* ev = reinterpret_cast<OnIntroCutsceneCheck*>(event);
        *ev->skipIntro = true;
    });
}

void RegisterSkipJiggyDance_Init() {
    COND_VB_SHOULD(VB_PLAY_JIGGY_DANCE, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_SKIP_JIGGY_DANCE, 0),
                   { *should = false; });
}

static RegisterShipInitFunc initBootLogosFunc(RegisterSkipBootLogos_Init, { CVAR_SKIP_BOOT_LOGOS });
static RegisterShipInitFunc initSkipIntroFunc(RegisterSkipIntroCutscene_Init, { CVAR_SKIP_INTRO });
static RegisterShipInitFunc initSkipJiggyDanceFunc(RegisterSkipJiggyDance_Init, { CVAR_SKIP_JIGGY_DANCE });
