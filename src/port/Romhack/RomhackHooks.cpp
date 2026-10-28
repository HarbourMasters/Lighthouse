#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/ShipInit.hpp"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "libultraship/libultra/gbi.h"
}

static void RegisterRomhackHooks_Init() {
    const bool romhack = RomhackConfig_IsActive();

    COND_VB_SHOULD(VB_ABILITY_CLEAR_ALL, EVENT_PRIORITY_NORMAL, romhack, { *should = false; });

    COND_VB_SHOULD(VB_SPAWNQUEUE_GLOBALIZE_OVERLAYS, EVENT_PRIORITY_NORMAL, romhack, { *should = true; });

    COND_VB_SHOULD(VB_ITEM_MAX, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 item = va_arg(args, s32);
        s32* max = va_arg(args, s32*);
        int override = -1;
        switch (item) {
            case ITEM_D_EGGS:
                override = fileProgressFlag_get(FILEPROG_BE_CHEATO_BLUEEGGS) ? RomhackConfig_GetMaxEggsCheato()
                                                                             : RomhackConfig_GetMaxEggs();
                break;
            case ITEM_F_RED_FEATHER:
                override = fileProgressFlag_get(FILEPROG_BF_CHEATO_REDFEATHERS)
                               ? RomhackConfig_GetMaxRedFeathersCheato()
                               : RomhackConfig_GetMaxRedFeathers();
                break;
            case ITEM_10_GOLD_FEATHER:
                override = fileProgressFlag_get(FILEPROG_C0_CHEATO_GOLDFEATHERS)
                               ? RomhackConfig_GetMaxGoldFeathersCheato()
                               : RomhackConfig_GetMaxGoldFeathers();
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
        const int override = RomhackConfig_GetNotesMax();
        if (override >= 0) {
            *max = override;
        }
    });

    COND_VB_SHOULD(VB_LEVEL_JIGGY_MAX, EVENT_PRIORITY_NORMAL, romhack, {
        s32* max = va_arg(args, s32*);
        const int override = RomhackConfig_GetJiggiesPerWorld();
        if (override >= 0) {
            *max = override;
        }
    });

    COND_VB_SHOULD(VB_LEVEL_HONEYCOMB_MAX, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 level = va_arg(args, s32);
        s32* max = va_arg(args, s32*);
        int specialLevel = RomhackConfig_GetSpecialLevel();
        int hcMax = RomhackConfig_GetHoneycombsPerWorld();
        int hcSpecial = RomhackConfig_GetExtraHcStart();
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
        const int collectibles = RomhackConfig_GetHideCollectiblesLevel();
        const int jiggies = RomhackConfig_GetHideJiggiesLevel();
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
        *name = RomhackConfig_GetLevelName(page);
    });

    COND_VB_SHOULD(VB_JIGSAW_PUZZLE_COST, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 index = va_arg(args, s32);
        s32* cost = va_arg(args, s32*);
        const int override = RomhackConfig_GetJiggyPuzzleCost(index);
        if (override >= 0) {
            *cost = override;
        }
    });

    COND_VB_SHOULD(VB_JIGSAW_PUZZLE_SIZE, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 index = va_arg(args, s32);
        s32* size = va_arg(args, s32*);
        const int override = RomhackConfig_GetJiggyPuzzleSize(index);
        if (override >= 0) {
            *size = override;
        }
    });

    COND_VB_SHOULD(VB_JIGSAW_PUZZLE_FLAG, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 index = va_arg(args, s32);
        s32* flag = va_arg(args, s32*);
        const int override = RomhackConfig_GetJiggyPuzzleFlag(index);
        if (override >= 0) {
            *flag = override;
        }
    });

    COND_VB_SHOULD(VB_MAP_LEVEL, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 map = va_arg(args, s32);
        int* level = va_arg(args, int*);
        const int remap = RomhackConfig_GetSceneRemap(map);
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
        if (RomhackConfig_GetSceneDefFull(map, opa, xlu, min, max, scale)) {
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_MAP_SKYBOX, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 map = va_arg(args, s32);
        int* models = va_arg(args, int*);
        float* scales = va_arg(args, float*);
        float* rotations = va_arg(args, float*);
        if (RomhackConfig_GetSkyboxFull(map, models, scales, rotations)) {
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_MAP_MUSIC, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 map = va_arg(args, s32);
        int* track1 = va_arg(args, int*);
        int* track2 = va_arg(args, int*);
        if (RomhackConfig_GetMusic(map, track1, track2)) {
            *should = false;
        }
    });

    COND_VB_SHOULD(VB_WARP_DEST, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 warpIdx = va_arg(args, s32);
        s32* dest = va_arg(args, s32*);
        *dest = RomhackConfig_GetWarpDest(warpIdx);
    });

    COND_HOOK(OnWarpResolveDest, EVENT_PRIORITY_HIGH, romhack, [](IEvent* event) {
        auto* ev = (OnWarpResolveDest*)event;
        if (ev->warpId == WARP_ID_SM_EXIT_BANJOS_HOUSE) {
            const int bkcf = RomhackConfig_GetWarpExitBanjosHouse();
            if (bkcf >= 0) {
                *ev->dest = bkcf;
            }
            const int map = RomhackConfig_GetStartLevel1();
            if (map >= 0) {
                *ev->dest = (map << 8) | (*ev->dest & 0xFF);
            }
        } else if (ev->warpId == WARP_ID_LAIR_ENTER_LAIR_FROM_SM_LEVEL) {
            const int bkcf = RomhackConfig_GetWarpEnterLair();
            if (bkcf >= 0) {
                *ev->dest = bkcf;
            }
        }
    });

    COND_VB_SHOULD(VB_GAMESELECT_NEW_GAME_MAP, EVENT_PRIORITY_NORMAL, romhack, {
        s32* map = va_arg(args, s32*);
        const int override = RomhackConfig_GetNewGameMap();
        if (override >= 0) {
            *map = override;
        }
    });

    COND_HOOK(OnNewGame, EVENT_PRIORITY_LOW, romhack, [](IEvent* event) {
        if (*((OnNewGame*)event)->skipIntro == 0 && RomhackConfig_GetKnowAllMoves() >= 0) {
            ability_setAllLearned(-1);
        }
    });

    COND_VB_SHOULD(VB_MUMBO_TRANSFORM_COST, EVENT_PRIORITY_NORMAL, romhack, {
        const s32 transformId = va_arg(args, s32);
        s32* cost = va_arg(args, s32*);
        const int override = RomhackConfig_GetMumboCost(transformId);
        if (override >= 0) {
            *cost = override;
            *should = false;
        }
    });

    // Hack model DLs can leave a palette (TLUT) mode enabled and leak it into the next model's draw.
    COND_HOOK(OnModelDrawEnd, EVENT_PRIORITY_NORMAL, romhack, [](IEvent* event) {
        Gfx** gfx = ((OnModelDrawEnd*)event)->gfx;
        gDPSetTextureLUT((*gfx)++, G_TT_NONE);
    });
}

static RegisterShipInitFunc initRomhackHooks(RegisterRomhackHooks_Init, { "BOOT" });
