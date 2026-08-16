// BanjoDecomp: core2/code_956B0.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "core1/core1.h"

extern void player_walkToPosition(f32 *, f32,  void(*)(ActorMarker *), ActorMarker *);
extern void func_8028F760(s32, f32, f32);
extern void func_8031CE70(f32 *arg0, enum map_e arg1, s32 arg2);
extern void func_8031FFAC(void);
extern NodeProp *func_80304ED0(void*, f32 *);
extern void func_8031CD44(enum map_e, s32, f32, f32, s32);
extern void mapSpecificFlags_set(s32, s32);

bool cutscene_skipEnterLairCutsceneCheck(void);
bool cutscene_skipGameOverCutsceneCheck(void);
bool cutscene_skipIntroCutsceneCheck(void);
bool cutscene_skipBeachCutsceneCheck(void);

#include "port/Romhack/RomhackConfig.h"

extern u8 D_8037DCCE[];

/* .data */
enum actor_e D_8036DDD0[] = {0, 0x184, 0x185, 0x186, -1};

/* .bss */
u8 D_80383190;

/* .code */
// cutscene_skipIntroCutsceneCheck
bool cutscene_skipIntroCutsceneCheck(void) {
    // [port] Skip intro cutscene
    if (!EventSystem_Should(VB_PLAY_INTRO_CUTSCENE, true) && func_8024E698(0) == 1) { return true; }

    // [port] Romhack gate: hacks that force the skip drop the "has a save file" term.
    if ((func_8024E698(0) == 1) &&
        (!EventSystem_Should(VB_CUTSCENE_SKIP_REQUIRE_PROGRESS, true) || gameFile_anyNonEmpty() != 0)) {
        return true;
    }
    return false;
}

// cutscene_skipEnterLairCutsceneCheck
bool cutscene_skipEnterLairCutsceneCheck(void) {
    // [port] Skip lair cutscene
    bool skipMiscCutscenes = false;
    CALL_EVENT(OnMiscCutscenesCheck, &skipMiscCutscenes);
    if (skipMiscCutscenes && func_8024E698(0) == 1) { return true; }

    if ((func_8024E698(0) == 1)
        && (!EventSystem_Should(VB_CUTSCENE_SKIP_REQUIRE_PROGRESS, true)
            || (D_8037DCCE[0] != 0)
            || (D_8037DCCE[1] != 0)
            || (D_8037DCCE[2] != 0))) {
        return true;
    }
    return false;
}


bool cutscene_skipGameOverCutsceneCheck(void) {
    s32 sp24;

    // [port] Romhack gate: hacks that always run the game-over return drop every
    // progress precondition below and pick their own destination map.
    bool requireProgress = EventSystem_Should(VB_CUTSCENE_SKIP_REQUIRE_PROGRESS, true);

    sp24 = func_8024E698(0);
    if (!requireProgress || mapSpecificFlags_get(0) != 0) {
        fileProgressFlag_set(FILEPROG_E1_UNKNOWN, 1);
    }
    bool skipMiscCutscenes = false;
    CALL_EVENT(OnMiscCutscenesCheck, &skipMiscCutscenes);
    if (skipMiscCutscenes && func_8024E698(0) == 1) { return true; }

    if ((sp24 == 1) && (!requireProgress || fileProgressFlag_get(FILEPROG_E1_UNKNOWN)) &&
        !gctransition_8030BDC0()) {
        if (!requireProgress || !mapSpecificFlags_get(0xC)) {
            mapSpecificFlags_set(0xC, true);
            func_802DC528(0, 0);
            timedFunc_set_2(11.0f, (GenFunction_2)func_802DC560, 0, 0);
            // [port] Honor BootSequence so Save & Quit lands at the same place as a fresh boot.
            s32 returnMap = getDefaultBootMap();
            EventSystem_Should(VB_GAME_OVER_RETURN_MAP, true, &returnMap);
            timedFunc_set_3(12.0f, (GenFunction_3)transitionToMap, returnMap, 0, 1);
        } else {
            timedFuncQueue_flush();
        }
    }
    return false;
}

bool cutscene_skipBeachCutsceneCheck(void){
    func_803219F4(1);
    return false;
}

//checks is a cutscene can be inturrupted and performs take me there
void cutscenetrigger_check(s32 cs_map, s32 arg1, s32 return_map, s32 return_exit, bool (* condFunc)(void)){
    if(gsworld_getMap() != cs_map)
        return;

    if((condFunc && condFunc()) || mapSpecificFlags_get(arg1)){
        mapSpecificFlags_set(arg1, 0);
        transitionToMap(return_map, (return_exit == -1)? 0: return_exit, 1);
    }
}

//check cutscene interrupts
s32 cutscenetrigger_update(void){
    cutscenetrigger_check(MAP_86_CS_SPIRAL_MOUNTAIN_4,        0, MAP_89_CS_INTRO_BANJOS_HOUSE_2,  -1, NULL);
    cutscenetrigger_check(MAP_7D_CS_SPIRAL_MOUNTAIN_1,        0, MAP_7C_CS_INTRO_BANJOS_HOUSE_1,  -1, NULL);
    cutscenetrigger_check(MAP_7C_CS_INTRO_BANJOS_HOUSE_1,     0, MAP_86_CS_SPIRAL_MOUNTAIN_4,     -1, NULL);
    cutscenetrigger_check(MAP_89_CS_INTRO_BANJOS_HOUSE_2,     0, MAP_1_SM_SPIRAL_MOUNTAIN,      0x12, NULL);
    cutscenetrigger_check(MAP_85_CS_SPIRAL_MOUNTAIN_3,        0, MAP_7B_CS_INTRO_GL_DINGPOT_1,    -1, NULL);
    cutscenetrigger_check(MAP_7B_CS_INTRO_GL_DINGPOT_1,       1, MAP_81_CS_INTRO_GL_DINGPOT_2,    -1, NULL);
    cutscenetrigger_check(MAP_81_CS_INTRO_GL_DINGPOT_2,       0, MAP_7D_CS_SPIRAL_MOUNTAIN_1,     -1, NULL);
    cutscenetrigger_check(MAP_82_CS_ENTERING_GL_MACHINE_ROOM, 0, MAP_69_GL_MM_LOBBY,            0x12, cutscene_skipEnterLairCutsceneCheck);
    // [port] Honor BootSequence so post-cutscene transitions match a fresh boot.
    cutscenetrigger_check(MAP_83_CS_GAME_OVER_MACHINE_ROOM,   0, getDefaultBootMap(),             -1, cutscene_skipGameOverCutsceneCheck);
    cutscenetrigger_check(MAP_87_CS_SPIRAL_MOUNTAIN_5,        0, MAP_88_CS_SPIRAL_MOUNTAIN_6,     -1, NULL);
    cutscenetrigger_check(MAP_94_CS_INTRO_SPIRAL_7,           0, MAP_8E_GL_FURNACE_FUN,            4, NULL);
    cutscenetrigger_check(MAP_88_CS_SPIRAL_MOUNTAIN_6,        1, MAP_96_CS_END_BEACH_1,           -1, NULL);
    cutscenetrigger_check(MAP_98_CS_END_SPIRAL_MOUNTAIN_1,    0, getDefaultBootMap(),             -1, NULL);
    cutscenetrigger_check(MAP_99_CS_END_SPIRAL_MOUNTAIN_2,    0, getDefaultBootMap(),             -1, NULL);
    cutscenetrigger_check(MAP_20_CS_END_NOT_100,              0, MAP_98_CS_END_SPIRAL_MOUNTAIN_1, -1, NULL);
    cutscenetrigger_check(MAP_95_CS_END_ALL_100,              0, MAP_99_CS_END_SPIRAL_MOUNTAIN_2, -1, NULL);
    cutscenetrigger_check(MAP_97_CS_END_BEACH_2,              0, MAP_99_CS_END_SPIRAL_MOUNTAIN_2, -1, cutscene_skipBeachCutsceneCheck);
    cutscenetrigger_check(MAP_85_CS_SPIRAL_MOUNTAIN_3,      0xC, MAP_1_SM_SPIRAL_MOUNTAIN,      0x12, cutscene_skipIntroCutsceneCheck);
    cutscenetrigger_check(MAP_7B_CS_INTRO_GL_DINGPOT_1,     0xC, MAP_1_SM_SPIRAL_MOUNTAIN,      0x12, cutscene_skipIntroCutsceneCheck);
    cutscenetrigger_check(MAP_81_CS_INTRO_GL_DINGPOT_2,     0xC, MAP_1_SM_SPIRAL_MOUNTAIN,      0x12, cutscene_skipIntroCutsceneCheck);
    cutscenetrigger_check(MAP_7D_CS_SPIRAL_MOUNTAIN_1,      0xC, MAP_1_SM_SPIRAL_MOUNTAIN,      0x12, cutscene_skipIntroCutsceneCheck);
    cutscenetrigger_check(MAP_7C_CS_INTRO_BANJOS_HOUSE_1,   0xC, MAP_1_SM_SPIRAL_MOUNTAIN,      0x12, cutscene_skipIntroCutsceneCheck);
    cutscenetrigger_check(MAP_86_CS_SPIRAL_MOUNTAIN_4,      0xC, MAP_1_SM_SPIRAL_MOUNTAIN,      0x12, cutscene_skipIntroCutsceneCheck);
    cutscenetrigger_check(MAP_89_CS_INTRO_BANJOS_HOUSE_2,   0xC, MAP_1_SM_SPIRAL_MOUNTAIN,      0x12, cutscene_skipIntroCutsceneCheck);
    if(gsworld_getMap() == MAP_95_CS_END_ALL_100 && mapSpecificFlags_get(1)){
        func_8034B9E4();
        mapSpecificFlags_set(1, 0);
    }
    return 0;
}

void func_8031CB50(enum map_e map_id, s32 exit_id, s32 arg2) {
    s32 sp1C;

    if ((D_80383190 == 0) && (getGameMode() != GAME_MODE_8_BOTTLES_BONUS) && (getGameMode() != GAME_MODE_7_ATTRACT_DEMO)) {
        // [port] Romhack gate: a listener may call musicKeepsPlaying() here to
        // carry a special-music state across this warp.
        EventSystem_Should(VB_WARP_KEEPS_MUSIC, true, map_id);
        sp1C = core2_9B650_getMusicTrackFromMap(gsworld_getMap());
        if ((core2_9B650_getMusicTrackFromMap(map_id) != sp1C) && (func_80322914() == 0)) {
            func_8025A388(0, 0x4E2);
            func_8025AB00();
            core1_ce60_incOrDecCounter(false);
        }
        if (func_802E4A08()) {
            func_802E40D0(map_id, exit_id);
            func_802E40E8(1);
            func_802E40C4(0xB);
        } else {
            transitionToMap(map_id, exit_id, 1);
        }
        gsworld_setEnableUpdate(arg2);
    }
}

void func_8031CC40(enum map_e map_id, s32 arg1) {
    func_8031CB50(map_id, arg1, 0);
}

void func_8031CC60(s32 arg0) {
    func_8031CB50(arg0 >> 8, arg0 & 0xFF, 1);
}

void func_8031CC8C(NodeProp *arg0, s32 arg1) {
    // arg1 = MAP_ID + ENTRY_ID
    f32 vec[3];
    f32 unused[3];

    if ((D_80383190 == 0) && (getGameMode() != GAME_MODE_8_BOTTLES_BONUS)) {
        if (getGameMode() != GAME_MODE_7_ATTRACT_DEMO) {
            if (arg0 != NULL) {
                ml_vec3h_to_vec3f(vec, (s16 *)arg0);
                func_8031CE70(vec, arg1 >> 8, arg1 & 0xFF);
            } else {
                func_8031CE70(NULL, arg1 >> 8, arg1 & 0xFF);
            }
        }
    }
}

void func_8031CD20(NodeProp *arg0, s32 arg1, s32 arg2) {
    func_8031CC8C(arg0, (arg1 << 8) + arg2);
}

void func_8031CD44(enum map_e arg0, s32 arg1, f32 arg2, f32 yaw, s32 arg4) {
    f32 sp3C[3];
    f32 sp30[3];
    f32 sp24[3];

    player_getPosition((f32 *) &sp3C);
    func_80256E24(sp24, 0.0f, yaw, 0.0f, 0.0f, ml_map_f((f32) arg4, 0.0f, 200.0f, 10.0f, 800.0f));
    sp24[0] = sp3C[0] + sp24[0];
    sp24[1] = sp3C[1] + sp24[1];
    sp24[2] = sp3C[2] + sp24[2];
    sp30[0] = sp24[0];
    sp30[1] = arg2;
    sp30[2] = sp24[2];
    ncDynamicCamera_setUpdateEnabled(0);
    func_8031CB50(arg0, arg1, 1);
    player_walkToPosition(sp30, 1.0f, NULL, NULL);
}

void func_8031CE28(s32 arg0, s32 arg1, f32 arg2) {
    f32 vec[3];

    player_getPosition(vec);
    func_8031CD44(arg0, arg1, vec[1], arg2, 0x25);
}

void func_8031CE70(f32 *arg0, enum map_e arg1, s32 arg2) {
    f32 playerPos[3];
    f32 sp38[3];
    NodeProp *phi_s0;
    f32 phi_f2;

    if ((D_80383190 == 0) && (getGameMode() != GAME_MODE_8_BOTTLES_BONUS) && (getGameMode() != GAME_MODE_7_ATTRACT_DEMO)) {
        if (arg0 != 0) {
            phi_s0 = func_80304ED0(&D_8036DDD0[1], arg0);
        } else {
            phi_s0 = NULL;
        }
        player_getPosition(playerPos);
        if (phi_s0 != NULL) {
            nodeprop_getPosition(phi_s0, sp38);
            phi_f2 = 500.0f;
            if (phi_s0->unk8 == 0x186) {
                phi_f2 = 1000.0f;
            }
            if (ml_vec3f_distance(arg0, sp38) < phi_f2) {
                if (phi_s0->unk8 == 0x184) {
                    ncDynamicCamera_setUpdateEnabled(0);
                    func_8031CB50(arg1, arg2, 1);
                    player_walkToPosition(sp38, 1.0f, NULL, NULL);
                } else if (phi_s0->unk8 == 0x185) {
                    func_8031CD44(arg1, arg2, sp38[1], (f32) phi_s0->yaw, phi_s0->scale);
                } else {
                    func_8031CD44(arg1, arg2, playerPos[1], (f32) phi_s0->yaw, phi_s0->scale);
                }
                return;
            }
        }
        func_8031CB50(arg1, arg2, 0);
    }
}

// set map and exit id?
void func_8031D04C(enum map_e arg0, s32 exit_id) {
    func_8031CB50(arg0, exit_id, 0);
}

void func_8031D06C(enum map_e arg0, s32 arg1) {
    func_8031CB50(MAP_20_CS_END_NOT_100, 0, 0);
}

void func_8031D09C(NodeProp *arg0, ActorMarker *arg1) {
    func_8034B968();
}

void func_8031D0C0(NodeProp *arg0, ActorMarker *arg1) {
    if (func_8024E698(0) == 1) {
        func_802E412C(1, 2);
        func_8025A2FC(0, 0x320);
        func_8025AB00();
        func_8031D09C(arg0, arg1);
    }
}

void warp_mmEnterMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0E01);
}

void warp_mmExitMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0201);
}

void warp_mmEnterTickersTowerLower(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0C02);
}

void warp_mmExitTickersTowerLower(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0202);
}

void warp_mmEnterTickersTowerUpper(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0C01);
}

void warp_mmExitTickersTowerUpper(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0203);
}

void warp_csNintendoLogo(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1E00);
}

void warp_gvEnterWaterPyramidLower(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1506);
}

void warp_gvEnterRubeePyramid(NodeProp *arg0, ActorMarker *arg1) {
    item_set(ITEM_6_HOURGLASS, 0);
    core1_7090_freeSfxSource(1);
    func_8031CC8C(arg0, 0x1607);
}

void warp_gvEnterMatchingPyramid(NodeProp *arg0, ActorMarker *arg1) {
    item_set(ITEM_6_HOURGLASS, 0);
    func_8031CC8C(arg0, 0x1301);
}

void warp_gvEnterWaterPyramidUpper(NodeProp *arg0, ActorMarker *arg1) {
    core1_7090_freeSfxSource(0);
    volatileFlag_set(VOLATILE_FLAG_AC_GV_TRAPDOOR_MISSED, 1);
    func_8031CC8C(arg0, 0x1502);
}

void warp_gvEnterMazePyramid(NodeProp *arg0, ActorMarker *arg1) {
    if (fileProgressFlag_getN(FILEPROG_F8_KING_SANDYBUTT_PYRAMID_STATE, 2) == 3) {
        func_8031CC8C(arg0, 0x1401);
    }
}

void warp_gvExitMatchingPyramid(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1203);
}

void warp_gvExitMazePyramid(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1204);
}

void warp_gvExitWaterPyramidLower(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1205);
}

void warp_gvExitRubeePyramid(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1206);
}

void warp_bgsEnterTanktup(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1101);
}

void warp_bgsExitTanktup(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0D03);
}

void warp_bgsExitVileRightNostril(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0D04);
}

void warp_bgsExitVileLeftNostril(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0D05);
}

void warp_ttcEnterSandcastle(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0A01);
}

void warp_ttcStairAlcoveDown(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x070F);
}

void warp_ttcStairAlcoveUp(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x070E);
}

void warp_ttcEnterBlubbersShipUpper(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0505);
}

void warp_ttcEnterBlubbersShipSide(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0506);
}

void warp_ttcExitLighthouseTop(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0708);
}

void warp_ttcExitSandcastle(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0703);
}

// Unused
void func_8031D550(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0704);
}

// Unused
void func_8031D574(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0705);
}

void warp_ttcExitBlubbersShipUpper(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0706);
}

void warp_ttcExitBlubbersShipSide(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0707);
}

// Unused
void warp_ttcExitLighthouseTopUnused(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0708);
}

// Unused
void func_8031D604(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0709);
}

void func_8031D628(NodeProp *arg0, ActorMarker *arg1) {
    Actor *actor;

    actor = actorArray_findActorFromActorId(0x13E);
    if (actor) {
        marker_despawn(actor->marker);
    }
    func_8031CB50(MAP_7_TTC_TREASURE_TROVE_COVE, WARP_TTC_C_LIGHTHOUSE_BOTTOM, 0);
}

void warp_mmmEnterDiningRoomDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2601);
}

void warp_mmmEnterDiningRoomChimney(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2602);
}

void warp_mmmEnterWellTop(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2501);
}

void warp_mmmEnterTumblarShed(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2401);
}

void warp_mmmEnterCellar(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1D01);
}

void warp_mmmEnterRedFeatherRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2A01);
}

void warp_mmmEnterBlueEggRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2801);
}

void warp_mmmEnterNoteRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2901);
}

void warp_mmmEnterBrokenFloorboardRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2E01);
}

void warp_mmmEnterBedroom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2D01);
}

void warp_mmmEnterBathroomWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2C01);
}

void warp_mmmExitDiningRoomDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B01);
}

// Unsure
void func_8031D820(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B02);
}

void warp_mmmExitWellTop(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B03);
}

void warp_mmmExitTumblarShed(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B04);
}

void warp_mmmExitChurchFrontDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B05);
}

void warp_mmmExitChurchSecretRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B06);
}

// Unsure
void func_8031D8D4(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B07);
}

void warp_mmmExitRainBarrelBottom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B08);
}

void warp_mmmExitCellar(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B09);
}

void warp_mmmExitRedFeatherRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B0A);
}

void warp_mmmExitBlueEggRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B0B);
}

void warp_mmmExitBathroomWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B0C);
}

void warp_mmmExitBrokenFloorboardRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B0D);
}

void warp_mmmExitBedroom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B0E);
}

void warp_mmmExitNoteRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B0F);
}

void warp_mmmChurchTowerUp(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B10);
}

void warp_mmmChurchTowerDown(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B11);
}

void warp_mmmExitMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B12);
}

void warp_mmmEnterChurchSecretRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2B01);
}

void func_8031DAA8(enum map_e arg0, s32 arg1) {
    func_8028F918(0);
    func_8031CB50(arg0, arg1, 0);
}

void func_8031DAE0(NodeProp *arg0, ActorMarker *arg1) {
    if (mapSpecificFlags_get(2) == 0) {
        volatileFlag_set(VOLATILE_FLAG_AD_MMM_CHURCH_DOOR_MISSED, 1);
        core1_7090_freeSfxSource(0);
        mapSpecificFlags_set(2, 1);
        coMusicPlayer_playMusic(COMUSIC_3B_MINIGAME_VICTORY, 0x6D60);
        func_8028F918(1);
        timedFunc_set_2(1.8f, (GenFunction_2)&func_8031DAA8, 0x1C, 1);
        func_802D6924();
    }
}

// Unused
void warp_mmmEnterMumbosHutUnused(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3001);
}

void warp_mmmEnterMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3001);
}

void warp_mmmEnterRainBarrel(NodeProp *arg0, ActorMarker *arg1) {
    if (player_getTransformation() == TRANSFORM_3_PUMPKIN) {
        func_8031CC8C(arg0, (s32)(uintptr_t)arg1 + 0x2F00);
    }
}

void func_8031DBE8(void) {
    func_8031CB50(MAP_2F_MMM_WATERDRAIN_BARREL, WARP_MMM_DRAINPIPE_1_TOP_ENTRANCE, 1);
}

void func_8031DC10(NodeProp *arg0, ActorMarker *arg1) {
    f32 vec[3];

    if (player_getTransformation() == TRANSFORM_3_PUMPKIN) {
        ml_vec3h_to_vec3f(vec, (s16*)arg0);
        func_8028F6E4(BS_INTR_2F_LOGGO, vec);
        timedFunc_set_0(0.8f, &func_8031DBE8);
    }
}

void warp_mmmEnterRainBarrelBottom(NodeProp *arg0, ActorMarker *arg1) {
    warp_mmmEnterRainBarrel(arg0, (ActorMarker *)(uintptr_t)2);
}

void warp_bgsEnterMrVileNostril(NodeProp *arg0, ActorMarker *arg1) {
    if (player_getTransformation() == TRANSFORM_5_CROC) {
        func_8031CC8C(arg0, (s32)(uintptr_t)arg1 + 0x1000);
    }
}

void warp_bgsEnterMrVileRightNostril(NodeProp *arg0, ActorMarker *arg1) {
    warp_bgsEnterMrVileNostril(arg0, (ActorMarker *)(uintptr_t)3);
}

void warp_bgsEnterMrVileLeftNostril(NodeProp *arg0, ActorMarker *arg1) {
    warp_bgsEnterMrVileNostril(arg0, (ActorMarker *)(uintptr_t)4);
}

void warp_bgsEnterTanktupConditional(NodeProp *arg0, ActorMarker *arg1) {
    s16 pos[3];

    pos[0] = arg0->x;
    pos[1] = arg0->y;
    pos[2] = arg0->z;
    if (func_8038F570(pos) != 0) {
        func_8031CC8C(arg0, 0x1101);
    }
}

void warp_ttcEnterNippersShell(NodeProp *arg0, ActorMarker *arg1) {
    s16 pos[3];

    pos[0] = arg0->x;
    pos[1] = arg0->y;
    pos[2] = arg0->z;
    if (chNipper_isInState7(pos) != 0) {
        func_8031CC8C(arg0, 0x601);
    }
}

void warp_ccExitWonderwingRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2203);
}

void warp_ccEnterClankerBelly(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2201);
}

void warp_ccEnterClankerMouth(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2202);
}

void warp_ccEnterWonderwingRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2301);
}

void warp_mmEnterLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0205);
}

void warp_gvEnterJinxy(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1A02);
}

void warp_gvExitJinxy(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1202);
}

void warp_rbbEnterCaptainsRoomWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3F01);
}

void warp_rbbEnterCabinRoomWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3901);
}

void warp_rbbEnterEngineRoomPipe(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3404);
}

void warp_rbbEnterEngineRoomDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3401);
}

void warp_rbbEnterKitchenPipe(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3C01);
}

void warp_rbbEnterNavigationRoomWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3D01);
}

void warp_rbbEnterBoomBoxPipe(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3B01);
}

void warp_rbbEnterChumpWarehouseWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3502);
}

void warp_rbbEnterBoatRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3601);
}

void warp_rbbEnterChompaContainer(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3701);
}

void warp_rbbEnterSeamanGrublinContainer(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3E01);
}

void warp_rbbEnterBoomBoxContainer(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3801);
}

void warp_rbbExitCaptainsRoomWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3101);
}

void warp_rbbExitCabinRoomWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3102);
}

void warp_rbbExitEngineRoomPipe(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3103);
}

void warp_rbbExitKitchenPipe(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3104);
}

void warp_rbbExitNavigationRoomWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3105);
}

void warp_rbbExitBoomBoxPipe(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3106);
}

void warp_rbbExitEngineRoomDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3107);
}

void warp_rbbExitBoatRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3108);
}

void warp_rbbExitChompaContainer(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3109);
}

void warp_rbbExitSeamanGrublinContainer(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x310A);
}

void warp_rbbExitBoomBoxContainer(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x310B);
}

void warp_rbbExitBossBoomBoxRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x310C);
}

void warp_rbbEnterBossBoomBoxRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CB50(MAP_3A_RBB_BOSS_BOOM_BOX, WARP_RBB_BOSS_1_ENTRANCE, 0);
}

void func_8031E204(NodeProp *node, s32 arg1, s32 arg2){
    f32 sp34[3];
    f32 sp28[3];
    f32 sp1C[3];

    nodeprop_getPosition(nodeprop_findByActorIdAndPosition_s16(ACTOR_154_UNKNOWN, &node->x), sp34);
    nodeprop_getPosition(nodeprop_findByActorIdAndPosition_s16(ACTOR_155_UNKNOWN, &node->x), sp28);
    player_getPosition(sp1C);
    if(sp28[1] < sp1C[1]){
        sp1C[1] = sp28[1];
    }
    func_8028F760(2, (sp1C[1] - sp34[1])/(sp28[1] - sp34[1]), 0.0f);
    func_8031CC8C(node, (arg1 <<8) + arg2);
}

void warp_rbbEnterChumpWarehouseDoor(NodeProp *node, ActorMarker *marker){
    func_8031E204(node, MAP_35_RBB_WAREHOUSE, 0x1);
}

void warp_rbbExitChumpWarehouseDoor(NodeProp *node, ActorMarker *marker){
    func_8031E204(node, MAP_31_RBB_RUSTY_BUCKET_BAY, 0xd);
}

// Unknown
void func_8031E308(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3206);
}

// Unknown
void func_8031E32C(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x5200);
}

// Unknown
void func_8031E350(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x5100);
}

// Unknown
void func_8031E374(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x5101);
}

// Unknown
void func_8031E398(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4200);
}

// Unknown
void func_8031E3BC(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3201);
}

// Unknown
void func_8031E3E0(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3202);
}

// Unknown
void func_8031E404(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3203);
}

// Unknown
void func_8031E428(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3204);
}

// Unknown
void func_8031E44C(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3205);
}

// Unknown
void func_8031E470(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3207);
}

// Unknown
void func_8031E494(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3208);
}

// Unknown
void func_8031E4B8(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3209);
}

// Unknown
void func_8031E4DC(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x320A);
}

// Unknown
void func_8031E500(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x320B);
}

// Unknown
void func_8031E524(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x320C);
}

// Unknown
void func_8031E548(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x320D);
}

// Unknown
void func_8031E56C(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x320E);
}

// Unknown
void func_8031E590(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x320F);
}

// Unknown
void func_8031E5B4(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3210);
}

// Unknown
void func_8031E5D8(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3211);
}

// Unknown
void func_8031E5FC(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3212);
}

// Unknown
void func_8031E620(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3213);
}

// Unknown
void func_8031E644(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3214);
}

// Unknown
void func_8031E668(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3215);
}

// Unknown
void func_8031E68C(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3216);
}

// Unknown
void func_8031E6B0(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3217);
}

void warp_ccwExitWinterToHub(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4001);
}

void warp_ccwExitSpringToHub(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4002);
}

void warp_ccwExitSummerToHub(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4003);
}

void warp_ccwExitAutumnToHub(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4004);
}

void warp_ccwEnterWinter(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4601);
}

void warp_ccwEnterSpring(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4301);
}

void warp_ccwEnterSummer(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4401);
}

void warp_ccwEnterAutumn(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4501);
}

void warp_gvExitSandybuttMazeBack(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1207);
}

void warp_bgsExitMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0D06);
}

void warp_bgsEnterMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4701);
}

void warp_ttcEnterLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0704);
}

void warp_ccEnterLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0B05);
}

void warp_bgsEnterLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x0D02);
}

void warp_gvEnterLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1208);
}

void warp_mmmEnterLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x1B14);
}

void warp_rbbEnterLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3110);
}

void warp_ttcExitNipper(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x070A);
}

void warp_ttcEnterSpringMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4A01);
}

void warp_ttcEnterSummerMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4B01);
}

void warp_ttcEnterAutumnMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4C01);
}

void warp_ttcEnterWinterMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4D01);
}

void warp_ttcExitSpringMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4309);
}

void warp_ttcExitSummerMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4409);
}

void warp_ttcExitAutumnMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4509);
}

void warp_ttcExitWinterMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4609);
}

void warp_ccwEnterSpringWhipcrackRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6501);
}

void warp_ccwEnterSummerWhipcrackRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6601);
}

void warp_ccwEnterAutumnWhipcrackRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6701);
}

void warp_ccwEnterWinterWhipcrackRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6801);
}

void warp_ccwExitSpringWhipcrackRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4308);
}

void warp_ccwExitSummerWhipcrackRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4408);
}

void warp_ccwExitAutumnWhipcrackRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4508);
}

void warp_ccwExitWinterWhipcrackRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4608);
}

void warp_ccwEnterSpringNabnutDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x5E01);
}

void warp_ccwEnterSummerNabnutDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x5F01);
}

void warp_ccwEnterAutumnNabnutDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6001);
}

void warp_ccwExitSpringNabnutDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4307);
}

void warp_ccwExitSummerNabnutDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4407);
}

void warp_ccwExitAutumnNabnutDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4507);
}

void warp_ccwExitWinterNabnutDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4607);
}

void warp_ccwEnterWinterAcornStorage(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6201);
}

void warp_ccwEnterAutumnFloodedAttic(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6301);
}

void warp_ccwEnterWinterFloodedAttic(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6401);
}

void warp_ccwExitWinterAcornStorage(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4606);
}

void warp_ccwExitAutumnFloodedAttic(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4506);
}

void warp_ccwExitWinterFloodedAttic(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4605);
}

void warp_fpEnterMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4801);
}

void warp_fpEnterBoggyIgloo(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4101);
}

void warp_fpEnterXmasTree(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x5301);
}

void warp_fpExitMumbosHut(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2707);
}

void warp_fpExitBoggyIgloo(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2708);
}

void warp_fpExitXmasTree(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2709);
}

void warp_lairEnterGVLobbyFromPointingStatueRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6E01);
}

void warp_lairEnterMMLobbyFromPuzzlesRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6901);
}

void warp_lairEnterFPLobbyFromGVLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6F01);
}

void warp_lairEnterGVLobbyFromFPLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6E02);
}

void warp_lairEnterGVLobbyFromGVLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6E03);
}

void warp_lairEnterMMLobbyFromMMLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6902);
}

// Unknown
void func_8031EF20(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6903);
}

void warp_lairEnterPuzzlesRoomFromMMLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6A01);
}

void warp_lairEnterPointingStatueRoomFromGVLobbyNoteDoor(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7101);
}

void warp_lairEnterPuzzlesRoomFromCCWPuzzleRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6A02);
}

void warp_lairEnterCCWPuzzleRoomFromPuzzlesRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6B01);
}

void warp_lairEnterCCWPuzzleRoomFromRedCauldronRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6B02);
}

void warp_lairEnterRedCauldronRoomFromCCWPuzzleRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6C01);
}

void warp_lairEnterCCWPuzzleRoomFromTTCLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6B03);
}

void warp_lairEnterTTCLobbyFromCCWPuzzleRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6D01);
}

void warp_lairEnterCCWPuzzleRoomFromCCLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6B04);
}

void warp_lairEnterCCLobbyFromCCWPuzzleRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7001);
}

void warp_lairEnterCCLobbyFromCCLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7002);
}

void warp_fpEnterLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2701);
}

// Unknown
void func_8031F0F4(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6F03);
}

// Unknown
void func_8031F118(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6F04);
}

void warp_lairEnterCCWPuzzleFromPointingGruntyStatueRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6B05);
}

void warp_lairEnterPointingGruntyStatueFromCCWPuzzleRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7102);
}

void warp_lairEnterGVPuzzleRoomFromMMMLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7401);
}

void warp_lairEnterMMMLobbyFromGVPuzzleRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7501);
}

void warp_lairEnterGVPuzzleRoomFromFPLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7402);
}

void warp_lairFPLobbyFromGVPuzzleRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6F05);
}

void warp_lairEnterPointingGruntyStatueFromBGSLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7103);
}

void warp_lairEnterBGSLobbyFromPointingGruntyStatueRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7201);
}

void warp_lairEnterCryptFromMMMLobby(NodeProp *arg0, ActorMarker *arg1) {
    if (player_getTransformation() == TRANSFORM_3_PUMPKIN) {
        func_8031CC8C(arg0, 0x7A01);
    }
}

void warp_lairEnterMMMLobbyFromCrypt(NodeProp *arg0, ActorMarker *arg1) {
    if (player_getTransformation() == TRANSFORM_3_PUMPKIN) {
        func_8031CC8C(arg0, 0x7503);
    }
}

void warp_fpExitWozzasCave(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x2706);
}

void warp_fpEnterWozzasCave(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7F01);
}

void warp_lairEnterBGSLobbyFromBGSLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7202);
}

void warp_lairEnter640NoteDoorRoomFromFPLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7601);
}

void warp_lairEnter640NoteDoorRoomFromCCWLobbyTokenEntrance(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7602);
}

void warp_lairEnter640NoteDoorRoomFromCCWLobbyDoorEntrance(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7603);
}

void warp_lairEnter640NoteDoorRoomFromRBBLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7604);
}

void warp_lairRBBLobbyFrom640NoteDoorRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7701);
}

void warp_lairCCWLobbyFrom640NoteDoorRoomDoorEntrance(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7901);
}

void warp_lairCCWLobbyFrom640NoteDoorRoomTokenEntrance(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7902);
}

void warp_ccwEnterSummerZubbaHive(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x5A02);
}

void warp_ccwEnterAutumnZubbaHive(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x5C02);
}

void warp_ccwExitSpringZubbaHive(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4306);
}

void warp_ccwExitSummerZubbaHive(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4406);
}

void warp_ccwExitAutumnZubbaHive(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4505);
}

void warp_ccwEnterSpringZubbaHive(NodeProp *arg0, ActorMarker *arg1) {
    if (player_getTransformation() == TRANSFORM_6_BEE) {
        func_8031CC8C(arg0, 0x5B01);
    }
}

void warp_lairEnterFPLobbyFrom640NoteDoorRoom(NodeProp *arg0, ActorMarker *arg1) {
    item_set(ITEM_6_HOURGLASS, 0);
    func_8031CC8C(arg0, 0x6F02);
}

void warp_lairEnterRBBLobbyFromRBBLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7702);
}

void warp_lairEnterRBBLobbyFromRBBPuzzleRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7703);
}

void warp_lairEnterRBBLobbyFromMMMPuzzleRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7704);
}

void warp_lairEnterMMMPuzzleFromRBBLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7801);
}

void warp_lairEnterRBBPuzzleFromRBBLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7802);
}

void warp_lairEnterCCWLobbyFromFurnaceFunPath(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7903);
}

void warp_lairEnterFurnaceFunPathFromCCWLobby(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x8001);
}

void warp_smEnterBanjosHouse(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x8C01);
}

void warp_smExitBanjosHouse(NodeProp *arg0, ActorMarker *arg1) {
    // [port] BB romhacks can override this warp destination via BKCF; port
    // listeners can refine further by responding to OnWarpResolveDest.
    s32 dest = 0x112;
    s32 bkcf = port_getRomhackWarpExitBanjosHouse();
    if (bkcf >= 0) {
        dest = bkcf;
    }
    s32 mapOverride = port_getRomhackStartLevel1();
    if (mapOverride >= 0) {
        dest = (mapOverride << 8) | (dest & 0xFF);
    }
    CALL_EVENT(OnWarpResolveDest, WARP_ID_SM_EXIT_BANJOS_HOUSE, 0x112, bkcf, &dest);
    func_8031CC8C(arg0, dest);
}

void warp_lairEnterMMLobbyFromSMLevel(NodeProp *arg0, ActorMarker *arg1) {
    s32 dest = 0x6912;
    CALL_EVENT(OnWarpResolveDest, WARP_ID_LAIR_ENTER_MM_LOBBY_FROM_SM_LEVEL, 0x6912, -1, &dest);
    func_8031CC8C(arg0, dest);
}

void warp_smExitLair(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x113);
}

void warp_rbbExitAnchorRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x3113);
}

void warp_rbbEnterAnchorRoom(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x8B04);
}

void warp_mmmEnterWellBottom(NodeProp *arg0, ActorMarker *arg1) {
    if (player_getTransformation() == TRANSFORM_3_PUMPKIN) {
        func_8031CC8C(arg0, 0x2504);
    }
}

void warp_mmmExitWellBottom(NodeProp *arg0, ActorMarker *arg1) {
    if (player_getTransformation() == TRANSFORM_3_PUMPKIN) {
        func_8031CC8C(arg0, 0x1B13);
    }
}

void warp_mmmEnterBathroomFromLoggo(NodeProp *arg0, ActorMarker *arg1) {
    if (player_getTransformation() == TRANSFORM_3_PUMPKIN) {
        func_8031CC8C(arg0, 0x2C04);
    }
}

void warp_mmmEnterLoggo(NodeProp *arg0, ActorMarker *arg1) {
    if (player_getTransformation() == TRANSFORM_3_PUMPKIN) {
        func_8031CC60(0x8D04);
    }
}

// Unused
void func_8031F80C(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7502);
}

void warp_lairTTCLobbyFromTTCLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6D04);
}

void warp_lairCCWLobbyFromCCWLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x7906);
}

void warp_ccwEnterLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4007);
}

void warp_lairFPLobbyFromFPLevel(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6F06);
}

void warp_ccwEnterSpringNabnutWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x5E02);
}

void warp_ccwEnterSummerNabnutWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x5F02);
}

void warp_ccwEnterAutumnNabnutWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6002);
}

void warp_ccwEnterWinterNabnutWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x6102);
}

void warp_ccwExitSpringNabnutWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4304);
}

void warp_ccwExitSummerNabnutWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4404);
}

void warp_ccwExitAutumnNabnutWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4504);
}

void warp_ccwExitWinterNabnutWindow(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x4604);
}

void func_8031F9E0(void){}

void func_8031F9E8(){
    D_80383190 = 0;
}

void func_8031F9F4(s32 arg0){
    D_80383190 = arg0;
}

void warp_ttcExitSharkfoodIsland(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x780);
}

void warp_gvEnterSNSChamper(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x9205);
}

void warp_gvExitSNSChamper(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x120A);
}

void warp_lairEnterDingpotRoomFromFurnaceFun(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x9305);
}

// warp_lairEnterFurnaceFunFrom?
void func_8031FA90(NodeProp *arg0, ActorMarker *arg1) {
    func_8031CC8C(arg0, 0x8E05);
}

void func_8031FAB4(NodeProp *arg0, ActorMarker *arg1) {
    if ((fileProgressFlag_get(FILEPROG_FC_DEFEAT_GRUNTY) != 0) && (jiggyscore_total() == 100)) {
        // Beach Cutscene
        func_8031CC8C(arg0, 0x9501);
    } else {
        // Final Battle
        func_8031CC8C(arg0, 0x9001);
    }
}

void warp_lairEnterLairFromSMLevel(NodeProp *arg0, ActorMarker *arg1) {
    if (fileProgressFlag_get(FILEPROG_BD_ENTER_LAIR_CUTSCENE) != 0) {
        // MM Lobby
        // [port] BB romhacks can override this warp destination via BKCF; port
        // listeners can refine further by responding to OnWarpResolveDest.
        s32 dest = 0x6912;
        s32 bkcf = port_getRomhackWarpEnterLair();
        if (bkcf >= 0) {
            dest = bkcf;
        }
        CALL_EVENT(OnWarpResolveDest, WARP_ID_LAIR_ENTER_LAIR_FROM_SM_LEVEL, 0x6912, bkcf, &dest);
        func_8031CC8C(arg0, dest);
    } else {
        fileProgressFlag_set(FILEPROG_BD_ENTER_LAIR_CUTSCENE, 1);
        // Enter Lair Cutscene
        func_8031CC8C(arg0, 0x8204);
    }
}

void func_8031FB6C(NodeProp *arg0, ActorMarker *arg1) {
    func_8030E6D4(SFX_7C_CHEBOOF);
    func_8031CC8C(arg0, 0x7104);
}

void clearScoreStates(void) {
    bsStoredState_clear();
    func_8031FFAC();
    item_setItemsStartCounts();
    jiggyscore_clearAll();
    honeycombscore_clear();
    mumboscore_clear();
    volatileFlag_clear();
    func_802D6344();
}

void debugScoreStates(void) {
    mumboscore_debug();
    honeycombscore_debug();
    jiggyscore_debug();
    func_803465DC();
    bsStoredState_debug();
    gameSelect_resetGameNumber();
}
