// BanjoDecomp: core2/gsworld.c
#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"

#include "core2/anim/sprite.h"
#include <core2/file.h>
#include "core2/particle.h"
#include "port/Interpolation/FrameInterpolation.h"
#include "port/Enhancements/Retention/Retention.h"

/* .data */
extern u8 D_80370250 = 0;

/* .bss */
struct gsworld_data_s {
    s32 unk0; // probably game_mode_e
    enum map_e map;
    s32 exit;
};
struct gsworld_data_s sGsWorldData;
s32 D_803835DC;
u32 sEnableDraw;

enum gsWorldStartIndicators {
    GS_WORLD_START_INDICATOR_0_END,
    GS_WORLD_START_INDICATOR_1_CUBES,
    GS_WORLD_START_INDICATOR_2_UNUSED,
    GS_WORLD_START_INDICATOR_3_CAMERAS,
    GS_WORLD_START_INDICATOR_4_LIGHTING
};

/* public */
void gsworld_setEnableUpdate(s32);
void gsworld_setEnableDraw(s32);
void gsworld_load(enum map_e);
void gsworld_stub3(s32);
extern Struct64s *func_8032994C(void);
extern void func_802F2ED0(Struct64s *, Gfx **, Mtx **, Vtx **);

/* .code */
void gsworld_draw(Gfx** gfx, Mtx **mtx, Vtx **vtx) {
    f32 sp44;
    f32 sp40;

    if (sEnableDraw == 0) {
        drawRectangle2D(gfx, 0, 0, gFramebufferWidth, gFramebufferHeight, 0, 0, 0);
        core2_34790_getClipDistances(&sp44, &sp40);
        viewport_setNearAndFar(sp44, sp40);
        viewport_setRenderViewportAndPerspectiveMatrix(gfx, mtx);
        return;
    }
    // Lighthouse [port] Crashing here, not sure what this is.
    // if (func_80320708() == 0) {
    //     eeprom_writeBlocks(0, 0, 0x80BC7230, EEPROM_MAXBLOCKS);
    // }
    spawnQueue_unlock();
    FrameInterpolation_RecordOpenChild("sky", 0);
    sky_draw(gfx, mtx, vtx);
    FrameInterpolation_RecordCloseChild();
    core2_34790_getClipDistances(&sp44, &sp40);
    viewport_setNearAndFar(sp44, sp40);
    viewport_setRenderViewportAndPerspectiveMatrix(gfx, mtx);
    if (mapModel_has_xlu_bin() != 0) {
        FrameInterpolation_RecordOpenChild("map_opa", 0);
        mapModel_opa_draw(gfx, mtx, vtx);
        FrameInterpolation_RecordCloseChild();
        if (game_is_frozen() == 0) {
            leveloverlay_drawCallback(gfx, mtx, vtx);
        }
        if (game_is_frozen() == 0) {
            FrameInterpolation_RecordOpenChild("player", 0);
            player_draw(gfx, mtx, vtx);
            FrameInterpolation_RecordCloseChild();
            CALL_EVENT(OnPlayerDraw, gfx, mtx, vtx);
        }
        if (game_is_frozen() == 0) {
            func_80302C94(gfx, mtx, vtx);
        }
        if (game_is_frozen() == 0) {
            FrameInterpolation_RecordOpenChild("jiggylist", 0);
            jiggylist_draw(gfx, mtx, vtx);
            FrameInterpolation_RecordCloseChild();
        }
        if (game_is_frozen() == 0) {
            func_803500D8(gfx, mtx, vtx);
        }
        if (game_is_frozen() == 0) {
            func_802F2ED0(func_8032994C(), gfx, mtx, vtx);
        }
        if (game_is_frozen() == 0) {
            FrameInterpolation_RecordOpenChild("part_pass0", 0);
            partEmitMgr_drawPass0(gfx, mtx, vtx);
            FrameInterpolation_RecordCloseChild();
        }
        if (game_is_frozen() == 0) {
            FrameInterpolation_RecordOpenChild("map_xlu", 0);
            mapModel_xlu_draw(gfx, mtx, vtx);
            FrameInterpolation_RecordCloseChild();
        }
        if (!game_is_frozen()) {
            core2_A5BC0_drawUnknownMarkers(gfx, mtx, vtx);
        }
        if (game_is_frozen() == 0) {
            FrameInterpolation_RecordOpenChild("part_pass1", 0);
            partEmitMgr_drawPass1(gfx, mtx, vtx);
            FrameInterpolation_RecordCloseChild();
        }
        if (game_is_frozen() == 0) {
            func_8034F6F0(gfx, mtx, (s32)(intptr_t)vtx);
        }
        func_802D520C(gfx, mtx, vtx);
    } else {
        FrameInterpolation_RecordOpenChild("map_opa", 0);
        mapModel_opa_draw(gfx, mtx, vtx);
        FrameInterpolation_RecordCloseChild();
        leveloverlay_drawCallback(gfx, mtx, vtx);
        func_8034F6F0(gfx, mtx, (s32)(intptr_t)vtx);
        FrameInterpolation_RecordOpenChild("player", 0);
        player_draw(gfx, mtx, vtx);
        FrameInterpolation_RecordCloseChild();
        CALL_EVENT(OnPlayerDraw, gfx, mtx, vtx);
        func_80302C94(gfx, mtx, vtx);
        core2_A5BC0_drawUnknownMarkers(gfx, mtx, vtx);
        FrameInterpolation_RecordOpenChild("jiggylist", 0);
        jiggylist_draw(gfx, mtx, vtx);
        FrameInterpolation_RecordCloseChild();
        func_803500D8(gfx, mtx, vtx);
        func_802F2ED0(func_8032994C(), gfx, mtx, vtx);
        func_802D520C(gfx, mtx, vtx);
        FrameInterpolation_RecordOpenChild("part_draw", 0);
        partEmitMgr_draw(gfx, mtx, vtx);
        FrameInterpolation_RecordCloseChild();
    }
    if (game_is_frozen() == 0) {
        func_80350818(gfx, mtx, vtx);
    }
    if (game_is_frozen() == 0) {
        func_802BBD0C(gfx, mtx, vtx);
    }
    spawnQueue_lock();
}

void gsworld_stub1(s32 arg0, s32 arg1, s32 arg2){
}

enum map_e gsworld_getMap(void){
    return sGsWorldData.map;
}

s32 gsworld_getExit(){
    return sGsWorldData.exit;
}

void gsworld_transitionToExit(s32 arg0) {
    transitionToMap(sGsWorldData.map, arg0, 1);
}

s32 gsworld_getUnk0(){
    return sGsWorldData.unk0;
}

void gsworld_setUnk0(s32);

void gsworld_free(void) {
    func_80255A14();
    gsworld_setUnk0(3);
    func_8034F734();
    func_803500E8();
    func_80350BC8();
    func_8030F1D0();
    gcparade_free();//null
    leveloverlay_releaseCallback_OnlyFP();
    func_803518E8();
    func_802D48F0();
    func_803224FC();
    func_8028E644();
    leveloverlay_releaseCallback_NotFP();
    func_80341A54();
    spawnQueue_free();
    print_freeBoldLetterFont();
    func_802FAC3C();
    bundle_free();
    commonParticle_freeAllParticles();
    func_8033FA24();
    func_80344C80();
    animsprite_terminate();
    animBinCache_free();
    func_802BC10C();
    ncCameraNodeList_free();
    pem_freeDependencies();
    pem_freeAll();
    partEmitMgr_free();
    func_802F7CE0();
    func_8031F9E0();
    func_80323100();
    cubeList_free();
    func_8031B710();
    mapModel_free();
    propModelList_free();
    lighting_free();
    sky_free();
    func_8034C8D8();
    func_80323238();
    func_803343AC();
    func_803308A0();
    func_8032AEB4();
    func_8033297C();
    func_803231E8();
    func_80320B7C();
    func_802BAF20();
    code7AF80_freeTotalCounts();
    func_80332A38();
    if (func_802E4A08() == 0) {
        itemPrint_free();
    }
    dialogBin_terminate();
    playerModel_free();
    if (!func_80322914()) {
        func_8024F7C4(func_803226E8(sGsWorldData.map));
    }
    core1_7090_release();
    AnimTextureListCache_free();
    leveloverlay_debug();
    func_8033BD6C();
    func_80255198();//heap_flush_free_queue
    animCache_flushAll();
}

void gsworld_set(enum map_e arg0, s32 arg1, s32 arg2) {
    sGsWorldData.unk0 = 3;
    CALL_EVENT(OnMapLoad, sGsWorldData.map, arg0, arg1);
    sGsWorldData.map = arg0;
    // [port] Drop the prev tree; the next sub-frame would otherwise lerp
    // the old map's geometry against the new one's.
    FrameInterpolation_DontInterpolateCamera();
    sGsWorldData.exit = arg1;
    leveloverlay_init();
    gsworld_setEnableUpdate(1);
    gsworld_setEnableDraw(1);
    func_802D2CB8();
    core1_7090_alloc();
    if (gsworld_getMap() == MAP_8E_GL_FURNACE_FUN) {
        func_8038E7C4();
    }
    if (func_80322914() == 0) {
        func_8024F764(func_803226E8(sGsWorldData.map));
    }
    func_80320B84();
    AnimTextureListCache_init();
    func_8034C97C();
    func_8030A078();
    func_8031B718();
    playerModel_set();
    if (!func_802E4A08()) {
        itemPrint_init();
    }
    dialogBin_initialize();
    spawnQueue_malloc();
    func_803329AC();
    func_80350BFC();
    func_80323190();
    func_80332894();
    func_803305AC();
    func_8031F9E8();
    func_80323230();
    commonParticleType_init();
    animBinCache_init();
    animsprite_init();
    func_80344C50();
    func_8033F9C0();
    ncCameraNodeList_init();
    nccamera_init();
    partEmitMgr_init();
    pem_setAllInactive();
    pem_initDependencies();
    func_802F7D30();
    propModelList_init();
    lighting_init();
    sky_reset();
    func_803343D0();
    cubeList_init();
    func_802FA69C();
    commonParticle_init();
    if (arg2 == 0) {
        gsworld_load(arg0);
    }
    func_80305990(0);
    func_8030C740();
    gcdialog_init();
    mapSpecificFlags_clearAll();
    func_803411B0();
    spawnQueue_reset();
    leveloverlay_initCallback_NotFP();
    func_8028E4B0();
    leveloverlay_initCallback_OnlyFP();
    func_80323120();
    func_803223AC();
    bundle_reset();
    func_8034F774();
    func_80350174();
    gcparade_init();
    func_80351998();
    func_802BC2CC(sGsWorldData.exit);
    func_802D63D4();
    func_80255A04();
    func_802D6948();
    if (func_802E4A08() == 0) {
        print_resetBoldFontTexture();
    }
    if (arg0 != MAP_1F_CS_START_RAREWARE) {
        func_8024F150();
    }
}

void gsworld_reload(void) {
    gsworld_free();
    gsworld_set(sGsWorldData.map, sGsWorldData.exit, 1);
}

void gsworld_stub2(void) {
    gsworld_stub3(sGsWorldData.map);
}

void gsworld_setUnk0(s32 arg0) {
    core1_15B30_sendMesg3ToRenderThread();
    func_802BC21C(sGsWorldData.unk0, arg0);
    func_8028F7F4(sGsWorldData.unk0, arg0);
    func_8030D8A8(sGsWorldData.unk0, arg0);
    func_803045CC(sGsWorldData.unk0, arg0);
    func_80323140(sGsWorldData.unk0, arg0);
    func_80351A1C(sGsWorldData.unk0, arg0);
    func_803225B0(sGsWorldData.unk0, arg0);
    leveloverlay_unk14Callback(sGsWorldData.unk0, arg0);
    func_802F0E80((void *)(intptr_t)sGsWorldData.unk0, arg0);
    commonParticle_setActive(sGsWorldData.unk0, arg0);
    sGsWorldData.unk0 = arg0;
}

s32 gsworld_update(void) {
    s32 phi_v1;
    s32 phi_v0;

    codeCF5F0_triggerAntiTamperMeasurement();
    func_802D5628();
    itemPrint_update();
    if (getGameMode() != GAME_MODE_4_PAUSED) {
        func_802F7E54();
    }
    if (D_803835DC == 0) {
        return 1;
    } else {
        func_802BAF40();
        func_8032AA9C();
        func_80323170();
        func_80351C48();
        func_80330FF4();
        func_8028E71C();
        // [port] anti-tamper: address-based cheat code check always fails on PC,
        // causing a 150M-iteration CPU stall every 16 frames in BGS. Stubbed.
#if 0
        phi_v0 = globalTimer_getTime();
        if (D_80370250) {
            phi_v1 = 0xF;
        } else {
            phi_v1 = 0x1F;
        }
        if (((phi_v1 & phi_v0) == 3) && (overlayManager_getLoadedID() == OVERLAY_5_BEACH)) {
            if ((maCastle_isSecretCheatCodeRelatedValueEqualToScrambledAddressValue() == false) || (D_80370250 != 0)) {
                D_80370250 = (u8)1;
                for (phi_v0 = 0; phi_v0 != 0x8F0D180; phi_v0++){
                }
            }
        }
#endif
        commonParticle_update();
        pem_updateAll();
        animCache_update();
        animBinCache_update();
        ncCamera_update();
        func_803045D8();
        func_80332E08();
        func_803465E4();
        func_8031B790();
        func_8034C9D4();
        propModelList_flush(1);
        if (EventSystem_Should(VB_SKY_UPDATE, true)) {
            sky_update();
        }
        partEmitMgr_update();
        func_8034F918();
        func_80350250();
        if (mapSpecificFlags_validateCRC1() == 0) {
            func_8028FCBC();
        }
        AnimTextureListCache_update();
        func_80350CA4();
        dialogBin_update();
        func_80310D2C();
        gcparade_update();
        leveloverlay_updateCallback();
        func_80321924();
        func_80334428();
        cutscenetrigger_update();
        func_802D2CDC();
        func_803306C8(1);
        func_8032AD7C(1);
        func_80322490();
        if (map_getLevel(sGsWorldData.map) == LEVEL_D_CUTSCENE) {
            func_802C79C4();
        }
        func_8032AABC();
        sns_stub();
        return 1;
    }
}

void gsworld_setEnableUpdate(s32 arg0){
    D_803835DC = arg0;
}

s32 gsworld_getEnableUpdate(){
    return D_803835DC;
}

void gsworld_setEnableDraw(s32 arg0){
    sEnableDraw = arg0;
}

s32 gsworld_getEnableDraw(){
    return sEnableDraw;
}

void gsworld_load(enum map_e map_id) {
    File *fp;

    port_noteRetention_beginMapLoad((int32_t)map_id);
    port_carriedSync_beginMapLoad((int32_t)map_id);

    core1_15B30_sendMesg3ToRenderThread();
    fp = file_openMap(map_id); //LevelSetupFile_Open
    if (fp == NULL) {
        return; // [port] safety: file_openMap can return NULL
    }
    while (file_isNextByteExpected(fp, GS_WORLD_START_INDICATOR_0_END) == 0) {
        if (file_isNextByteExpected(fp, GS_WORLD_START_INDICATOR_2_UNUSED)) {
            /* NO OP */
        } else if (file_isNextByteExpected(fp, GS_WORLD_START_INDICATOR_1_CUBES)) {
            cubeList_fromFile(fp);
        } else if (file_isNextByteExpected(fp, GS_WORLD_START_INDICATOR_3_CAMERAS)) {
            ncCameraNodeList_fromFile(fp);
        } else if (file_isNextByteExpected(fp, GS_WORLD_START_INDICATOR_4_LIGHTING)) {
            lightingVectorList_fromFile(fp);
        } else {
            break; // [port] unrecognized tag, avoid infinite loop
        }
    }
    file_close(fp); //file close
}

void gsworld_stub3(s32 arg0){
}
