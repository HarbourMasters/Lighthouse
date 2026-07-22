#include <libultraship/bridge.h>
#include <spdlog/spdlog.h>
#include "port/Enhancements/Events/Hooks/Events.h"

extern "C" {
#include "enums.h"
#include "functions.h"

extern struct1Cs_1 D_8036C58C[0xD];

// Acorn replaced with collectible doubloon
extern ActorInfo D_8038F460;
void func_8038C7A8(Actor* thisx);

typedef struct {
    u8 map_id;
    u8 rgb[3];
    u8 alpha;
} CameraFogEntry;
extern CameraFogEntry D_80365D60[];

typedef struct {
    u8 uid;
    u8 state;
    u8 next_state;
    f32 duration;
    s32 model_index;
    s32 anim_index;
    f32 scale;
} TransitionInfoEntry;
extern TransitionInfoEntry D_8036C150[0x16];
}

namespace {

// Custom transition pair
constexpr s32 kPairMapA = MAP_2_MM_MUMBOS_MOUNTAIN;
constexpr s32 kPairMapB = MAP_31_RBB_RUSTY_BUCKET_BAY;
constexpr s32 kPairInIndex = 0xD;

GameMap sPrevMap = (GameMap)0;
GameMap sCurMap = (GameMap)0;

struct PauseRemap {
    int src;
    s16 level_id;
    s16 x;
};
constexpr PauseRemap kRemap[0xD] = {
    { 0, 0, 80 },
    { 1, 0, 64 },
    { 3, 0, 37 },
    { 4, 0, 27 },
    { 5, 0, 52 },
    { 6, 0, 35 },
    { 7, 0, 72 },
    { 8, 0, 78 },
    { 9, 0xA, 43 }, // CUT-THROAT COAST
    { 10, 0, 50 },
    { 11, 0, 48 },
    { 2, 0, 60 },
    { 12, 0, 72 },
};

void RebuildPauseMenuTable() {
    u8* names[0xD];
    for (int i = 0; i < 0xD; i++) {
        names[i] = D_8036C58C[i].string;
    }
    for (int page = 0; page < 0xD; page++) {
        D_8036C58C[page].string = names[kRemap[page].src];
        D_8036C58C[page].level_id = kRemap[page].level_id;
        D_8036C58C[page].x = kRemap[page].x;
    }
    D_8036C58C[8].string = (u8*)"CUT-THROAT COAST";
}

void ApplyDataPatches() {
    D_80365D60[3] = { 0, { 0x34, 0x6E, 0xEF }, 0x5A };
    D_8036C150[3].state = 7;      // TRANSITION_STATE_7_WHITE_IN
    D_8036C150[3].next_state = 0; // TRANSITION_STATE_0_NONE
    D_8036C150[3].duration = 0.7f;
    D_8036C150[3].model_index = 0;
    D_8036C150[3].anim_index = 0;
    D_8036C150[12].duration = 0.4f;
    D_8036C150[13].state = 3;      // TRANSITION_STATE_3_BLACK_OUT
    D_8036C150[13].next_state = 1; // TRANSITION_STATE_1_LOADING
    D_8036C150[13].duration = 0.4f;
    D_8036C150[13].model_index = 0;
    D_8036C150[13].scale = 0.0f;
}

} // namespace

extern "C" void CutThroatCoast_DoubloonUpdate(Actor* thisx) {
    thisx->yaw = mlNormalizeAngle(thisx->yaw + 12.0f);
    func_8038C7A8(thisx);
}

void RegisterCutThroatCoastPatches() {
    RebuildPauseMenuTable();
    ApplyDataPatches();
    D_8038F460.update_func = CutThroatCoast_DoubloonUpdate;

    // CTC sets the ability flags every frame, but is this needed for Lighthouse?
    REGISTER_LISTENER(OnMapLoad, EVENT_PRIORITY_HIGH, [](IEvent* event) {
        auto* ev = reinterpret_cast<OnMapLoad*>(event);
        sPrevMap = ev->prevMap;
        sCurMap = ev->nextMap;
        if (ev->nextMap != MAP_91_FILE_SELECT) {
            ability_setAllUsed(-1);
        }
    });

    // Pause menu pins below
    REGISTER_VB_SHOULD(VB_JIGGYSCORE_LEVEL_TOTAL, EVENT_PRIORITY_NORMAL, {
        s32 lvl = va_arg(args, s32);
        s32* result = va_arg(args, s32*);
        s32 cnt = 0;
        if (lvl == 0xA) {
            for (s32 i = 1; i < 0x65; i++) {
                if (jiggyscore_isCollected((enum jiggy_e)i)) {
                    cnt++;
                }
            }
            if (cnt > 0xA) {
                cnt = 0xA;
            }
        }
        *result = cnt;
        *should = false;
    });

    REGISTER_VB_SHOULD(VB_PAUSEMENU_LEVEL_TO_PAGE, EVENT_PRIORITY_NORMAL, {
        va_arg(args, s32);
        s32* page = va_arg(args, s32*);
        *page = 8;
        *should = false;
    });

    REGISTER_VB_SHOULD(VB_PAUSEMENU_SET_NEXT_PAGE, EVENT_PRIORITY_NORMAL, {
        s8* page = va_arg(args, s8*);
        *page = 8;
        *should = false;
    });

    REGISTER_VB_SHOULD(VB_PAUSEMENU_DRAW_JOYSTICKS, EVENT_PRIORITY_NORMAL, {
        (void)args;
        *should = false;
    });

    REGISTER_VB_SHOULD(VB_PAUSEMENU_BOLD_FONT_TEXTURE, EVENT_PRIORITY_NORMAL, {
        s32* fontId = va_arg(args, s32*);
        *fontId = 0x6E4;
        (void)should;
    });

    // Exiting via the exit pad should go back to the CTC lobby map
    REGISTER_VB_SHOULD(VB_MAP_CHANGE_REQUEST, EVENT_PRIORITY_NORMAL, {
        s32* map = va_arg(args, s32*);
        s32* exit = va_arg(args, s32*);
        if (*map == MAP_75_GL_MMM_LOBBY) {
            *map = MAP_1_SM_SPIRAL_MOUNTAIN;
            *exit = 2;
        }
        (void)should;
    });

    REGISTER_VB_SHOULD(VB_VOID_OUT_RESPAWN_TRANSITION, EVENT_PRIORITY_NORMAL, {
        s32 map = va_arg(args, s32);
        s32 exit = va_arg(args, s32);
        transitionToMap((enum map_e)map, exit, 1);
        *should = false;
    });

    REGISTER_VB_SHOULD(VB_MAP_TRANSITION_IN_INDEX, EVENT_PRIORITY_NORMAL, {
        s32 map = va_arg(args, s32);
        s32* inIndex = va_arg(args, s32*);
        s32 other = (map == sCurMap) ? sPrevMap : sCurMap;
        if ((map == kPairMapA && other == kPairMapB) || (map == kPairMapB && other == kPairMapA)) {
            *inIndex = kPairInIndex;
        }
        (void)should;
    });

    // The repurposed season switches are hidden below the ground when hit
    REGISTER_VB_SHOULD(VB_CCW_SEASON_SWITCH_PRESSED_INIT, EVENT_PRIORITY_NORMAL, {
        Actor* switchActor = va_arg(args, Actor*);
        switchActor->position[1] = -420.0f;
        subaddie_set_state(switchActor, 4);
        *should = false;
    });

    // Suppress the jiggy tutorial dialog
    REGISTER_VB_SHOULD(VB_JIGGY_COLLECT_TUTORIAL, EVENT_PRIORITY_NORMAL, {
        (void)args;
        *should = false;
    });

    // This MMM empty honeycomb doesn't require the pumpkin
    REGISTER_VB_SHOULD(VB_HONEYCOMB_PUMPKIN_REQUIREMENT, EVENT_PRIORITY_NORMAL, {
        (void)args;
        *should = false;
    });

    // TODO: the ROM also nops the vanilla sky_update call and the fullscreen
    // clear behind the skybox.

    // TODO: CTC's lighthouse beam is part of its XLU and rotates,
    // need to figure out how.

    // Suppress Brentilda's heal dialog
    REGISTER_VB_SHOULD(VB_BRENTILDA_HEAL_DIALOG, EVENT_PRIORITY_NORMAL, {
        (void)args;
        *should = false;
    });
}
