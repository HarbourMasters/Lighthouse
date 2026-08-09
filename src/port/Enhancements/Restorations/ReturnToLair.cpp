#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Romhack/RomhackConfig.h"

#include "include/core2/anctrl.h"
#include "include/core2/gc/zoombox.h"

#include "functions.h"

extern "C" struct1Bs D_8036C560[]; // not declared in variables.h

typedef struct struct_1A_s {
    f32 delay;
    f32 unk4;
    u8* str;
    s16 y;
    u8 portrait;
    u8 unkF;
} struct1As;

struct1As* menuData;

#define CVAR_NAME CVAR_ENHANCEMENT("Restorations.ReturnToLair")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterReturnToLair_Init() {
    COND_VB_SHOULD(VB_INIT_RETURN_TO_LAIR, EVENT_PRIORITY_NORMAL, true, {
        menuData = va_arg(args, struct1As*);

        if (CVAR && !port_isRomhack()) {
            s32 level = level_get();
            bool isSpiralMountain = (level == LEVEL_B_SPIRAL_MOUNTAIN);
            // The enter-lair cutscene flag is what flips the save's start map to the Lair, so it
            // doubles as "the player has reached the Lair at least once".
            bool beenToLair = fileProgressFlag_get(FILEPROG_BD_ENTER_LAIR_CUTSCENE);
            bool validWorld =
                level > 0 && level < LEVEL_C_BOSS && level != LEVEL_6_LAIR && D_8036C560[level - 1].map != -1;
            // Spiral Mountain has a valid Lair warp entry, but only offer the option there once the
            // Lair has actually been visited.
            bool showOption = validWorld && (!isSpiralMountain || beenToLair);
            *should = !showOption;
            if (!*should) {
                menuData[0].y = 45;
                menuData[1].y = 75;
                menuData[2].y = 105;
                menuData[3].y = 135;
                menuData[1].delay = 0.1f;
                menuData[2].delay = 0.2f;
                menuData[3].delay = 0.3f;
                menuData[1].portrait = ZOOMBOX_SPRITE_5_GRUNTILDA_2;
                // From Spiral Mountain you head forward to the Lair rather than exiting back to it.
                menuData[1].str = isSpiralMountain ? (u8*)"GO TO GRUNTY'S LAIR" : (u8*)"EXIT TO WITCH'S LAIR";
            } else {
                // Option hidden — reset to vanilla layout
                menuData[0].y = 55;
                menuData[1].y = -100;
                menuData[2].y = 90;
                menuData[3].y = 125;
                menuData[1].delay = 0.3f;
                menuData[2].delay = 0.1f;
                menuData[3].delay = 0.2f;
                menuData[1].portrait = ZOOMBOX_SPRITE_4_BANJO_1;
                menuData[1].str = (u8*)"EXIT TO WITCH'S LAIR";
            }
        } else {
            *should = true;
            menuData[0].y = 55;
            menuData[1].y = -100;
            menuData[2].y = 90;
            menuData[3].y = 125;
            menuData[1].delay = 0.3f;
            menuData[2].delay = 0.1f;
            menuData[3].delay = 0.2f;
            menuData[1].portrait = ZOOMBOX_SPRITE_4_BANJO_1;
            menuData[1].str = (u8*)"EXIT TO WITCH'S LAIR";
        }
    });

    COND_VB_SHOULD(VB_PAUSE_MENU_PORTRAIT_DEPTH, EVENT_PRIORITY_NORMAL, CVAR, { *should = false; });

    // Shrink the pause-menu zoombox text so the longer string fits at the vanilla box scale.
    // Only sub-1.0 boxes (the pause menus) are touched; the box scale arrives as the first vararg.
    COND_VB_SHOULD(VB_ZOOMBOX_TEXT_ADJUST, EVENT_PRIORITY_NORMAL, CVAR, {
        double boxScale = va_arg(args, double);
        f32* outScale = va_arg(args, f32*);
        int* xOfs = va_arg(args, int*);
        if (boxScale < 1.0) {
            if (outScale != nullptr) {
                *outScale = 0.85f;
            }
            if (xOfs != nullptr) {
                *xOfs += 4;
            }
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterReturnToLair_Init, { CVAR_NAME });