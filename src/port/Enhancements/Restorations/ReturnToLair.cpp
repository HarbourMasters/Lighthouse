#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Romhack/RomhackConfig.h"

#include "include/core2/anctrl.h"
extern "C" {
#include "core2/gc/pauseMenu.h"
#include "include/core2/gc/zoombox.h"
}

#include "functions.h"

extern "C" struct1Bs D_8036C560[]; // not declared in variables.h

struct1As* menuData;

#define CVAR_NAME CVAR_ENHANCEMENT("Gameplay.ReturnToLair")
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterReturnToLair_Init() {
    // Registered whatever the setting, so turning it off puts the vanilla layout back.
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
                menuData[1].portrait = ZOOMBOX_SPRITE_5_GRUNTILDA_PAUSE;
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
                menuData[1].portrait = ZOOMBOX_SPRITE_4_BANJO_PAUSE;
                menuData[1].str = (u8*)"EXIT TO WITCH'S LAIR";
            }
        } else {
            menuData[0].y = 55;
            menuData[1].y = -100;
            menuData[2].y = 90;
            menuData[3].y = 125;
            menuData[1].delay = 0.3f;
            menuData[2].delay = 0.1f;
            menuData[3].delay = 0.2f;
            menuData[1].portrait = ZOOMBOX_SPRITE_4_BANJO_PAUSE;
            menuData[1].str = (u8*)"EXIT TO WITCH'S LAIR";
        }
    });

    // Draw the main menu with the selected box on top, the others back to front for Return to Lair. In the totals
    // menu the selection is a page, not a box. Registered whatever the setting, since the selected box goes on top
    // either way.
    COND_VB_SHOULD(VB_PAUSEMENU_DRAW_ZOOMBOXES, EVENT_PRIORITY_NORMAL, true, {
        Gfx** gfx = va_arg(args, Gfx**);
        Mtx** mtx = va_arg(args, Mtx**);
        Vtx** vtx = va_arg(args, Vtx**);
        GcZoombox** zoombox = va_arg(args, GcZoombox**);
        s32 menu = va_arg(args, s32);
        s32 selection = va_arg(args, s32);
        if (menu == 0) { // PAUSE_MENU_0_MAIN
            const bool backToFront = CVAR && !port_isRomhack();
            for (s32 n = 0; n < 4; n++) {
                const s32 i = backToFront ? 3 - n : n;
                if (i != selection) {
                    gczoombox_draw(zoombox[i], gfx, mtx, vtx);
                }
            }
            gczoombox_draw(zoombox[selection], gfx, mtx, vtx);
            *should = false;
        }
    });

    // Shrink the pause-menu zoombox text so the longer string fits at the vanilla box scale.
    // Only sub-1.0 boxes (the pause menus) are touched.
    COND_HOOK(OnZoomboxTextDraw, EVENT_PRIORITY_NORMAL, CVAR && !port_isRomhack(), [](IEvent* event) {
        auto* ev = reinterpret_cast<OnZoomboxTextDraw*>(event);
        if (ev->boxScale < 1.0f) {
            *ev->scale = 0.85f;
            *ev->xOffset += 4;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterReturnToLair_Init, { CVAR_NAME });