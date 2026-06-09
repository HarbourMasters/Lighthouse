#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/Romhack/RomhackConfig.h"

#include "include/core2/anctrl.h"
#include "include/core2/gc/zoombox.h"

extern "C" struct1Bs D_8036C560[];
extern "C" enum level_e level_get(void);

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
            *should = !(level > 0 && level < LEVEL_C_BOSS && level != LEVEL_6_LAIR &&
                        level != LEVEL_B_SPIRAL_MOUNTAIN && D_8036C560[level - 1].map != -1);
            if (!*should) {
                menuData[0].y = 45;
                menuData[1].y = 75;
                menuData[2].y = 105;
                menuData[3].y = 135;
                menuData[1].delay = 0.1f;
                menuData[2].delay = 0.2f;
                menuData[3].delay = 0.3f;
                menuData[1].portrait = ZOOMBOX_SPRITE_5_GRUNTILDA_2;
            } else {
                // Not in a world level — reset to vanilla layout
                menuData[0].y = 55;
                menuData[1].y = -100;
                menuData[2].y = 90;
                menuData[3].y = 125;
                menuData[1].delay = 0.3f;
                menuData[2].delay = 0.1f;
                menuData[3].delay = 0.2f;
                menuData[1].portrait = ZOOMBOX_SPRITE_4_BANJO_1;
            }
        } else {
            // Not in a world level — reset to vanilla layout
            menuData[0].y = 55;
            menuData[1].y = -100;
            menuData[2].y = 90;
            menuData[3].y = 125;
            menuData[1].delay = 0.3f;
            menuData[2].delay = 0.1f;
            menuData[3].delay = 0.2f;
            menuData[1].portrait = ZOOMBOX_SPRITE_4_BANJO_1;
        }
    });
}

static RegisterShipInitFunc initFunc(RegisterReturnToLair_Init, { CVAR_NAME });