#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
#include "functions.h"
}

static void RegisterRomhackHooks_Init() {
    const bool romhack = port_isRomhack();

    COND_VB_SHOULD(VB_ABILITY_CLEAR_ALL, EVENT_PRIORITY_NORMAL, romhack, { *should = false; });

    COND_VB_SHOULD(VB_SPAWNQUEUE_GLOBALIZE_OVERLAYS, EVENT_PRIORITY_NORMAL, romhack, { *should = true; });

    COND_VB_SHOULD(VB_ITEM_MAX, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 item = va_arg(args, s32);
        s32* max = va_arg(args, s32*);
        int override = -1;
        switch (item) {
            case ITEM_D_EGGS:
                override = fileProgressFlag_get(FILEPROG_BE_CHEATO_BLUEEGGS) ? port_getRomhackMaxEggsCheato()
                                                                             : port_getRomhackMaxEggs();
                break;
            case ITEM_F_RED_FEATHER:
                override = fileProgressFlag_get(FILEPROG_BF_CHEATO_REDFEATHERS) ? port_getRomhackMaxRedFeathersCheato()
                                                                                : port_getRomhackMaxRedFeathers();
                break;
            case ITEM_10_GOLD_FEATHER:
                override = fileProgressFlag_get(FILEPROG_C0_CHEATO_GOLDFEATHERS)
                               ? port_getRomhackMaxGoldFeathersCheato()
                               : port_getRomhackMaxGoldFeathers();
                break;
            default:
                break;
        }
        if (override >= 0) {
            *max = override;
        }
    });

    COND_VB_SHOULD(VB_LEVEL_NOTE_MAX, EVENT_PRIORITY_NORMAL, romhack, {
        s32* max = va_arg(args, s32*);
        const int override = port_getRomhackNotesMax();
        if (override >= 0) {
            *max = override;
        }
    });

    COND_VB_SHOULD(VB_LEVEL_JIGGY_MAX, EVENT_PRIORITY_NORMAL, romhack, {
        s32* max = va_arg(args, s32*);
        const int override = port_getRomhackJiggiesPerWorld();
        if (override >= 0) {
            *max = override;
        }
    });

    COND_VB_SHOULD(VB_LEVEL_HONEYCOMB_MAX, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 level = va_arg(args, s32);
        s32* max = va_arg(args, s32*);
        int specialLevel = port_getRomhackSpecialLevel();
        int hcMax = port_getRomhackHoneycombsPerWorld();
        int hcSpecial = port_getRomhackExtraHcStart();
        if (specialLevel < 0) {
            specialLevel = LEVEL_B_SPIRAL_MOUNTAIN;
        }
        if (hcMax < 0) {
            hcMax = 2;
        }
        if (hcSpecial < 0) {
            hcSpecial = 6;
        }
        *max = (level == specialLevel) ? hcSpecial : hcMax;
    });

    COND_VB_SHOULD(VB_PAUSEMENU_HIDDEN_ROW_LEVELS, EVENT_PRIORITY_NORMAL, romhack, {
        s32* hideCollectiblesLevel = va_arg(args, s32*);
        s32* hideJiggiesLevel = va_arg(args, s32*);
        const int collectibles = port_getRomhackHideCollectiblesLevel();
        const int jiggies = port_getRomhackHideJiggiesLevel();
        if (collectibles >= 0) {
            *hideCollectiblesLevel = collectibles;
        }
        if (jiggies >= 0) {
            *hideJiggiesLevel = jiggies;
        }
    });

    COND_VB_SHOULD(VB_PAUSEMENU_LEVEL_NAME, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 page = va_arg(args, s32);
        const char** name = va_arg(args, const char**);
        *name = port_getRomhackLevelName(page);
    });

    COND_VB_SHOULD(VB_JIGSAW_PUZZLE_COST, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 index = va_arg(args, s32);
        s32* cost = va_arg(args, s32*);
        const int override = port_getRomhackJiggyPuzzleCost(index);
        if (override >= 0) {
            *cost = override;
        }
    });

    COND_VB_SHOULD(VB_JIGSAW_PUZZLE_SIZE, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 index = va_arg(args, s32);
        s32* size = va_arg(args, s32*);
        const int override = port_getRomhackJiggyPuzzleSize(index);
        if (override >= 0) {
            *size = override;
        }
    });

    COND_VB_SHOULD(VB_JIGSAW_PUZZLE_FLAG, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 index = va_arg(args, s32);
        s32* flag = va_arg(args, s32*);
        const int override = port_getRomhackJiggyPuzzleFlag(index);
        if (override >= 0) {
            *flag = override;
        }
    });

    COND_VB_SHOULD(VB_MAP_LEVEL, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 map = va_arg(args, s32);
        int* level = va_arg(args, int*);
        const int remap = port_getRomhackSceneRemap(map);
        if (remap >= 0) {
            *level = remap;
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_MAP_SCENE_DEF, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 map = va_arg(args, s32);
        int* opa = va_arg(args, int*);
        int* xlu = va_arg(args, int*);
        short* min = va_arg(args, short*);
        short* max = va_arg(args, short*);
        float* scale = va_arg(args, float*);
        if (port_getRomhackSceneDefFull(map, opa, xlu, min, max, scale)) {
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_MAP_SKYBOX, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 map = va_arg(args, s32);
        int* models = va_arg(args, int*);
        float* scales = va_arg(args, float*);
        float* rotations = va_arg(args, float*);
        if (port_getRomhackSkyboxFull(map, models, scales, rotations)) {
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_MAP_MUSIC, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 map = va_arg(args, s32);
        int* track1 = va_arg(args, int*);
        int* track2 = va_arg(args, int*);
        if (port_getRomhackMusic(map, track1, track2)) {
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_WARP_DEST, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 warpIdx = va_arg(args, s32);
        s32* dest = va_arg(args, s32*);
        *dest = port_getRomhackWarpDest(warpIdx);
    });

    COND_HOOK(OnWarpResolveDest, EVENT_PRIORITY_HIGH, romhack, [](IEvent* event) {
        auto* ev = (OnWarpResolveDest*)event;
        if (ev->warpId == WARP_ID_SM_EXIT_BANJOS_HOUSE) {
            const int bkcf = port_getRomhackWarpExitBanjosHouse();
            if (bkcf >= 0) {
                *ev->dest = bkcf;
            }
            const int map = port_getRomhackStartLevel1();
            if (map >= 0) {
                *ev->dest = (map << 8) | (*ev->dest & 0xFF);
            }
        } else if (ev->warpId == WARP_ID_LAIR_ENTER_LAIR_FROM_SM_LEVEL) {
            const int bkcf = port_getRomhackWarpEnterLair();
            if (bkcf >= 0) {
                *ev->dest = bkcf;
            }
        }
    });

    COND_VB_SHOULD(VB_GAMESELECT_NEW_GAME_MAP, EVENT_PRIORITY_NORMAL, romhack, {
        s32* map = va_arg(args, s32*);
        const int override = port_getRomhackNewGameMap();
        if (override >= 0) {
            *map = override;
        }
    });

    COND_HOOK(OnNewGame, EVENT_PRIORITY_LOW, romhack, [](IEvent* event) {
        if (*((OnNewGame*)event)->skipIntro == 0 && port_getRomhackKnowAllMoves() >= 0) {
            ability_setAllLearned(-1);
        }
    });

    COND_VB_SHOULD(VB_MUMBO_TRANSFORM_COST, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 transformId = va_arg(args, s32);
        s32* cost = va_arg(args, s32*);
        const int override = port_getRomhackMumboCost(transformId);
        if (override >= 0) {
            *cost = override;
            *should = false;
        }
    });
}

static RegisterShipInitFunc initRomhackHooks(RegisterRomhackHooks_Init, { "BOOT" });
