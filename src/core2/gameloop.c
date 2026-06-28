#include <ultra64.h>
#include "core1/core1.h"
#include "functions.h"
#include "variables.h"

#include "gc/gctransition.h"
#include "bk_time.h"
#include "port/Patches/Patches.h"

extern void print_updateBoldLetterFontDelayedFreeing(void);
extern void func_802FA0F8(void);
extern void timedFuncQueue_update(void);
extern void gsworld_setEnableDraw(s32);
extern void func_8025A2B0(void);
extern void func_8025A430(s32, s32, s32);
extern void gsworld_setEnableUpdate(s32);
extern void func_8034BB90(void);
extern void func_8030C27C(void);
extern void func_80321C34(void);
extern void func_8030ED0C(void);
extern void comusicPlayer_update(void);

enum transition_e {
    TRANSITION_0_NONE
};

void func_802E3BD0(s32 frame_buffer_indx);
void func_802E40A8(s32 map, s32 exit);
void func_802E40C4(s32 arg0);
void func_802E40D0(s32 map, s32 exit);
void func_802E40E8(s32 transition);
bool func_802E4A08(void);

f32 func_8033DC20(void);
extern void func_80324C58(void);

/* .data */
s16 D_803687F0[] = {
    0x159A, 0x02BC, 0x21C8, 0x00C8, 0x14ED, 0x03A2, 0x1951, 0x0003,
    '**', ' B', 'AN', 'JO', ' K', 'AZ', 'OO', 'IE',
    ' (', 'c)', ' R', 'AR', 'E ', 'Lt', 'd ', '19',
    '98', ' *', '*\0',   0,    0,    0,    0,    0,
 
};

/* .bss */
struct{
    s32 unk0;
    s32 game_mode; //game_mode
    f32 unk8; 
    s32 unkC; //freeze_scene_flag (used for pause menu)
    f32 unk10;
    u8 transition;
    u8 map;
    u8 exit;
    u8 unk17; //reset_on_map_load
    u8 unk18;
    u8 unk19;
    u8 unk1A;
    u8 unk1B;
    u8 unk1C;
} D_8037E8E0;

u8 GetCurrentMap() {
    return D_8037E8E0.map;
}

void func_802E3800(void){
    viewport_setPosition_f3(0.0f, 0.0f, 0.0f);
    viewport_setRotation_f3(-30.0f, 30.0f, 0.0f);
    viewport_moveAlongZAxis(3000.0f);
    viewport_update();
}

void func_802E3854(void){
    int i;

    func_8033B61C();
    dummy_func_80254464();
    for(i = 0; i < 0xF; i++){
        func_802E6820(5);
        modelRender_defrag();
        mapSavestate_defrag();
        gctransition_defrag();
        printbuffer_defrag();
        code_C9E70_defrag();
        func_802FA4E0();
        func_8033B5FC();
        timedFuncQueue_defrag();
        comusic_defrag();
    }
}

void func_802E38E8(enum map_e map, s32 exit, s32 reset_on_load){
    if(reset_on_load || level_get() != map_getLevel(map)){
        func_8030AFD8(1);
        func_80321854();
        func_803216D0(map); //load_map_asm
        func_8030AFA0(map);
    }
    else{
        func_8030AFD8(1);
        func_8030AFA0(map);
    }
    func_802FA508();
    gsworld_set(map, exit, 0);
    func_802E3800();
    func_8033DC10();
}

void func_802E398C(s32 arg0) {
    gsworld_free();
    func_8030ED0C();
    comusicPlayer_update();
    if (arg0 != 0) {
        func_802E3854();
    }
}

void func_802E39D0(Gfx **gdl, Mtx **mptr, Vtx **vptr, s32 framebuffer_idx, s32 arg4){
    Mtx* m_start = *mptr; 
    Vtx* v_start = *vptr;

    scissorBox_SetForGameMode(gdl, framebuffer_idx);
    D_8037E8E0.unkC = false;
    port_mirror_beginScene();
    gsworld_draw(gdl, mptr, vptr);
    CALL_EVENT(OnWorldDraw, gdl, mptr, vptr);
    port_mirror_endScene();
    port_mirror_undoProjection(gdl, mptr);
    if (port_shouldCaptureTransition()) {
        port_captureTransitionFb(gdl);
    }
    if(!arg4){
        func_802E67AC();
        func_802E3BD0(getActiveFramebuffer());
        func_802E67C4();
        func_802E5F10(gdl);
    }
    if( D_8037E8E0.game_mode == GAME_MODE_A_SNS_PICTURE
        && D_8037E8E0.unk19 != 6
        && D_8037E8E0.unk19 != 5
    ){
        gctransition_draw(gdl, mptr, vptr);
    }

    // [port] Return rendering to main FB after scene draw + transitions for
    // SNS/Bottles modes. Must come AFTER gctransition_draw so the transition
    // fade renders into the aux FB (visible on the picture), not the main FB.
    if (D_8037E8E0.game_mode == GAME_MODE_8_BOTTLES_BONUS || D_8037E8E0.game_mode == GAME_MODE_A_SNS_PICTURE) {
        gsSPResetFB((*gdl)++);
    }
    
    if( D_8037E8E0.game_mode == GAME_MODE_8_BOTTLES_BONUS
        || D_8037E8E0.game_mode == GAME_MODE_A_SNS_PICTURE
    ){
        func_8030C2D4(gdl, mptr, vptr);
    }

    // [port] Skip all HUD/overlay draws while pause menu is capturing the
    // background snapshot, otherwise they get baked into the pause background.
    extern bool gcpausemenu_isCapturing(void);
    bool capturing = gcpausemenu_isCapturing();

    if(!game_is_frozen() && gsworld_getEnableDraw() && !capturing){
        func_8032D474(gdl, mptr, vptr);
    }

    gcpausemenu_draw(gdl, mptr, vptr);
    if(!game_is_frozen() && !capturing){
        dummy_func_8025AFC0(gdl, mptr, vptr);
    }

    if (!capturing) {
        gcdialog_draw(gdl, mptr, vptr);
    }
    if(!game_is_frozen() && !capturing){
        itemPrint_draw(gdl, mptr, vptr);
    }

    if (!capturing) {
        printbuffer_draw(gdl, mptr, vptr);
    }

    if( D_8037E8E0.game_mode != GAME_MODE_A_SNS_PICTURE
        || D_8037E8E0.unk19 == 6
        || D_8037E8E0.unk19 == 5
    ){
        gctransition_draw(gdl, mptr, vptr);
    }
    // [port] Populate gFramebuffers from the GPU via gDPReadFB at native resolution.
    // Transitions and particles read from gFramebuffers during game logic.
    // On N64, gFramebuffers was the render target directly; on PC the GPU renders
    // to its own buffer, so we read it back here when requested.
    if (port_consumeReadbackRequest()) {
        gDPReadFB((*gdl)++, 0, (u16 *)gFramebuffers[getActiveFramebuffer()],
                  0, 0, gFramebufferWidth, gFramebufferHeight, 1);
    }
    core1_15B30_finishDList(gdl);
    osWritebackDCache(m_start, sizeof(Mtx)*( *mptr - m_start));
    osWritebackDCache(v_start, sizeof(Vtx)*( *vptr - v_start));
}

void func_802E3BD0(s32 frame_buffer_indx){
    framebufferdraw_setBufferIndex(frame_buffer_indx);
}

void func_802E3BF0(void){
    return;
}

//_set_game_mode
void game_setMode(enum game_mode_e next_mode, s32 arg1){
    s32 prev_mode = D_8037E8E0.game_mode;
    s32 sp20;
    s32 sp1C;

    if( ( ( D_8037E8E0.game_mode == GAME_MODE_3_NORMAL || func_802E4A08())
            && next_mode != GAME_MODE_4_PAUSED
          )
          || ( D_8037E8E0.game_mode == GAME_MODE_4_PAUSED && next_mode != GAME_MODE_3_NORMAL )
        ){
        func_80324C58();
    }

    if(D_8037E8E0.game_mode == GAME_MODE_4_PAUSED && next_mode != GAME_MODE_4_PAUSED ){
        gcpausemenu_free();
    }

    //L802E3C84
    if(next_mode == GAME_MODE_8_BOTTLES_BONUS || next_mode == GAME_MODE_A_SNS_PICTURE){
        func_8030C1A0();
    }
    else{
        func_8030C204();
    }//L802E3CB4

    D_8037E8E0.game_mode = next_mode;

    if(next_mode == 2){
        gsworld_setUnk0(3);
    }
    else if(next_mode == GAME_MODE_3_NORMAL || func_802E4A08()){
        if(prev_mode != GAME_MODE_4_PAUSED) {
            gsworld_setUnk0(2);
        }//L802E3D18
        if(arg1){
            sp20 = false;
            if(next_mode == GAME_MODE_3_NORMAL){
                if(volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE)){
                    sp20 = true;
                    sp1C = 7;
                }
                else if(func_8032190C()
                    && level_get() != LEVEL_C_BOSS
                    && level_get() != LEVEL_B_SPIRAL_MOUNTAIN
                    && level_get() != LEVEL_6_LAIR
                    && level_get() != LEVEL_D_CUTSCENE
                ){
                    sp20 = true;
                    sp1C = 1;
                }
            }
            else if(func_802E4A08()){//L802E3DBC
                sp20 = true;
                sp1C = func_8034BDA4(D_8037E8E0.map, D_8037E8E0.exit);
            }

            if(sp20)
                gctransition_8030BEA4(sp1C);
            else
                gctransition_8030BD4C();
        }
        func_80346CA8();
        D_8037E8E0.unk10 = 0.0f; 
    }
    else if(next_mode == GAME_MODE_4_PAUSED){//L802E3E24
        gsworld_setEnableUpdate(0);
        FUNC_8030E624(SFX_C9_PAUSEMENU_ENTER, 1.1f, 32750);
        pfsManager_update();
        func_8025A430(0, 2000, 3);
        func_8025A23C(COMUSIC_6F_PAUSE_SCREEN);
        gcpausemenu_init();
    }//L802E3E6C
}

void func_802E3E7C(enum game_mode_e mode){
    s32 sp34;
    s32 sp30;
    s32 map;
    s32 sp28;
    s32 prev_mode;
    s32 prev_map;

    core1_15B30_sendMesg3ToRenderThread();
    sp34 = D_8037E8E0.unk18;
    sp30 = D_8037E8E0.unk17;
    map = D_8037E8E0.map;
    sp28 = D_8037E8E0.exit;
    prev_mode = D_8037E8E0.unk0;
    game_setMode(GAME_MODE_2_UNKNOWN, 0);
    if(!volatileFlag_getAndSet(VOLATILE_FLAG_21, 0) || map_getLevel(gsworld_getMap()) == map_getLevel(D_8037E8E0.map)){
        if(!volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE))
            mapSavestate_save(gsworld_getMap());
    }
    func_802E398C(1);
    prev_map = gsworld_getMap();
    func_802E38E8(map, sp28, sp34);
    mapSavestate_apply(map);
    D_8037E8E0.unk0 = prev_mode;
    game_setMode(mode, sp30);
    jiggylist_map_actors();
    func_80346CA8();
    CALL_EVENT(OnMapLoad, prev_map, map, sp28);
}

s32 func_802E3F80(void){
    return D_8037E8E0.unk0;
}

extern void Graphics_PushFrame(Gfx* data);
extern void Framebuffer_ReadbackGPU(int bufferIndex);

void game_draw(s32 arg0){
    Gfx *gfx;
    Gfx *gfx_start;
    Gfx *sp2C;
    Mtx *mtx;
    Mtx *mtx_start;
    Vtx *vtx;
    Vtx *vtx_start;

    if(arg0) {
        scissorBox_setDefault();
    }

    getGraphicsStacks(&gfx, &mtx, &vtx);

    if(D_8037E8E0.unkC == 1){
        getGraphicsStacks(&gfx, &mtx, &vtx);
    }

    gfx_start = gfx;
    mtx_start = mtx;
    vtx_start = vtx;

    func_802E39D0(&gfx, &mtx, &vtx, getActiveFramebuffer(), arg0);

    graphicsCache_checkFrame(gfx_start, gfx, mtx_start, mtx, vtx_start, vtx);

    // Lighthouse [Port] not sure if this should be here or after the following block
    Graphics_PushFrame(gfx_start);

    // (particles, bottles bonus, screen captures) see actual rendered content.
    Framebuffer_ReadbackGPU(getActiveFramebuffer());

    if(D_8037E8E0.unkC == 0){
        sp2C = gfx;
        viMgr_func_8024C1DC();
        core1_15B30_addF3DEXTaskData_40000000(gfx_start, sp2C);

        if(arg0) {
            scissorBox_setDefault();
        }
    }
}

void func_802E4048(s32 map, s32 exit, s32 transition){
    func_802E40A8(map, exit);
    func_802E40E8(transition);
    func_802E40C4(1);
}

//take me there
void transitionToMap(enum map_e map, s32 exit, s32 transition){
    func_802E40D0(map, exit);
    func_802E40E8(transition);
    func_802E40C4(1);
}

void func_802E40A8(s32 map, s32 exit){
    D_8037E8E0.unk18 = 1;
    D_8037E8E0.map = map;
    D_8037E8E0.exit = exit;
}

void func_802E40C4( s32 arg0){
    D_8037E8E0.transition = arg0;   
}

void func_802E40D0(s32 map, s32 exit){
    D_8037E8E0.unk18 = 0;
    D_8037E8E0.map = map;
    D_8037E8E0.exit = exit;
}

void func_802E40E8(s32 transition){
    D_8037E8E0.unk17 = transition;
    D_8037E8E0.unk19 = 0;
    if(transition && !gctransition_8030BDC0()){
        gctransition_8030BE60();
    }
    
}

void func_802E412C(s32 arg0, s32 arg1){
    D_8037E8E0.unk17 = arg0;
    D_8037E8E0.unk19 = arg1;
    if(arg0 && !gctransition_8030BDC0()){
        gctransition_8030BEA4(arg1);
    }
}

void func_802E4170(void){
    game_setMode(GAME_MODE_2_UNKNOWN,0);
    defragManager_free();
    func_802E5F68();
    if(!func_802E4A08())
        print_free();
    timedFuncQueue_free();
    func_802F9C48();
    modelRender_free();
    depthBuffer_stub();
    func_802E398C(0);
    func_8030AFD8(0);
    func_80321854();
    debugScoreStates();
    animCache_free();
    comusicPlayer_free();
    func_8030D8DC();
}

void func_802E4214(enum map_e map_id){
    D_8037E8E0.transition = TRANSITION_0_NONE;
    D_8037E8E0.unk19 = D_8037E8E0.unk18 = 0;
    D_8037E8E0.map = D_8037E8E0.exit = D_8037E8E0.unk17 = 0;
    D_8037E8E0.unk1B = D_8037E8E0.unk1A = 0;
    D_8037E8E0.unkC = 0;
    D_8037E8E0.unk1C = 0;
    savedata_init();
    sns_save_and_update_global_data();
    func_8030D86C();
    comusicPlayer_init();
    func_80322764();
    timedFuncQueue_init();
    func_802F9CD8();
    if (EventSystem_Should(VB_RESET_DIALOG_LANGUAGE, true)) {
        func_8031B62C();
    }
    if(!func_802E4A08())
        print_init();
    func_802E5F38();
    defragManager_init();
    modelRender_init();
    func_80253428(1);
    animCache_init();
    viewport_reset();
    viewport_setNearAndFar(1.0f, 10000.0f);
    rand_reset();
    scissorBox_setDefault();
    func_80253FE8();
    time_reset();
    func_8033DC04();
    clearScoreStates();
    D_8037E8E0.game_mode = GAME_MODE_2_UNKNOWN;
    D_8037E8E0.unk8 = 0.0f;
    time_setDeltaReal_sec(0.0f);
    time_setDeltaReal_frames(0);
    func_803216D0(map_id);
    func_8030AFA0(map_id);
    func_802E3854();
    func_802E38E8(map_id, 0, 0);
    D_8037E8E0.unk0 = 0;
    game_setMode(GAME_MODE_3_NORMAL,1);
}

void func_802E4384(void){
    if(D_8037E8E0.unk8 == 0.0f){
        time_setDeltaReal_sec(0.0f);
    }
    else{
        func_8033DC18();
        // [port] Use a fixed 2-VI timestep for normal gameplay.
        s32 viDivisor = viMgr_func_8024BFA0();
        func_8033DC20(); // always consume wall-clock to keep last_ticks fresh
        if (viDivisor > 2) {
            time_setDeltaReal_frames(viDivisor);
        } else {
            time_setDeltaReal_frames(2);
        }
    }

    func_8033DC10();

    D_8037E8E0.unk8 += time_getDelta();
}

// [port] After an SNS/demo map reload, the first render frame has no aux FBO set
// up yet, so the scene renders directly to the primary FBO (visible as a black flash).
// On N64 this was hidden by viBlack. Skip one extra draw frame to let the aux FBO
// initialize before presenting.
static s32 sSkipDrawFrames = 0;

bool func_802E4424(void) {
    s32 sp1C;
    u8 temp_v0;

    viewport_debug();
    rand_shuffle();
    if (!gctransition_8030BDC0()) {
        temp_v0 = D_8037E8E0.transition;
        D_8037E8E0.transition = TRANSITION_0_NONE;
        switch (temp_v0) {                          /* switch 1 */
            case 9:                                     /* switch 1 */
                if( (D_8037E8E0.game_mode == GAME_MODE_7_ATTRACT_DEMO)
                    || (D_8037E8E0.game_mode == GAME_MODE_8_BOTTLES_BONUS)
                    || (D_8037E8E0.game_mode == GAME_MODE_A_SNS_PICTURE)
                    || (D_8037E8E0.game_mode == GAME_MODE_9_BANJO_AND_KAZOOIE)
                ) {
                    func_8034B940();
                }
                gcparade_8031ABF8();
                func_802E3E7C(GAME_MODE_3_NORMAL);
                return false;

            case 10:                                    /* switch 1 */
                if( (D_8037E8E0.game_mode == GAME_MODE_7_ATTRACT_DEMO)
                    || (D_8037E8E0.game_mode == GAME_MODE_8_BOTTLES_BONUS)
                    || (D_8037E8E0.game_mode == GAME_MODE_A_SNS_PICTURE)
                    || (D_8037E8E0.game_mode == GAME_MODE_9_BANJO_AND_KAZOOIE)
                ) {
                    func_8034B940();
                }
                gcparade_8031ABA0();
                func_802E3E7C(GAME_MODE_3_NORMAL);
                return false;

            case 1:                                     /* switch 1 */
                if( (D_8037E8E0.game_mode == GAME_MODE_7_ATTRACT_DEMO)
                    || (D_8037E8E0.game_mode == GAME_MODE_8_BOTTLES_BONUS)
                    || (D_8037E8E0.game_mode == GAME_MODE_A_SNS_PICTURE)
                    || (D_8037E8E0.game_mode == GAME_MODE_9_BANJO_AND_KAZOOIE)
                ) {
                    func_8034B940();
                }
                func_802E3E7C(GAME_MODE_3_NORMAL);
                return false;

            case 6:                                     /* switch 1 */
                // [port] Hold audio across the attract-demo load so its jingle starts fresh once
                // the demo is on screen instead of playing (and drifting) through the cold freeze.
                port_beginDemoAudioHold();
                func_8034B8C0(D_8037E8E0.map, D_8037E8E0.exit);
                func_802E3E7C(GAME_MODE_7_ATTRACT_DEMO);
                return false;

            case 12:                                    /* switch 1 */
                func_8034B8C0(D_8037E8E0.map, D_8037E8E0.exit);
                func_802E3E7C(GAME_MODE_A_SNS_PICTURE);
                sSkipDrawFrames = 2; // [port] skip next draws so aux FBO initializes first
                return false;

            case 7:                                     /* switch 1 */
                func_8034B8C0(D_8037E8E0.map, D_8037E8E0.exit);
                func_802E3E7C(GAME_MODE_8_BOTTLES_BONUS);
                return false;

            case 8:                                     /* switch 1 */
                func_8034B8C0(D_8037E8E0.map, D_8037E8E0.exit);
                func_802E3E7C(GAME_MODE_9_BANJO_AND_KAZOOIE);
                return false;

            case 11:                                    /* switch 1 */
                func_802E3E7C(D_8037E8E0.game_mode);
                return false;

            case 2:                                     /* switch 1 */
                func_8023DFF0(1);
                return false;

            case 3:                                     /* switch 1 */
                func_8023DFF0(4);
                return false;
            case 0:
                break;
        }
    }
    if (D_8037E8E0.unk1A != 0) {
        game_setMode(D_8037E8E0.unk1A - 1, D_8037E8E0.unk1B);
        D_8037E8E0.unk1A = 0;
    }
    sp1C = gsworld_update();
    func_80321C34();
    func_8030ED0C();
    comusicPlayer_update();
    switch (D_8037E8E0.game_mode) {
        case GAME_MODE_8_BOTTLES_BONUS:
        case GAME_MODE_A_SNS_PICTURE:
            func_8030C27C();
            /* fallthrough */
        case GAME_MODE_7_ATTRACT_DEMO:
            /* fallthrough */
        case GAME_MODE_9_BANJO_AND_KAZOOIE:
            func_8034BB90();
            if ((controller_getStartButton(0) == 1) && (D_8037E8E0.unk0 != 0)) {
                game_setMode(GAME_MODE_1_UNKNOWN, 0U);
            }
            break;
        case GAME_MODE_3_NORMAL:                                     /* switch 2 */
            D_8037E8E0.unk10 += time_getDelta();
            if( (func_8024E698(0) == 1)
                && func_8028F070()
                && (func_8028EC04() == 0)
                && !gctransition_8030BDC0()
                && gctransition_done()
                && (level_get() != 0)
                && (0.6 < D_8037E8E0.unk10)
                && gcpausemenu_80314B00()
                && !player_isDead()
                && func_8032056C()
                && levelSpecificFlags_validateCRC1()
                && dummy_func_80320248()
            ) {
                game_setMode(GAME_MODE_4_PAUSED, 0U);
            } else if ((controller_getStartButton(0) == 1) && (D_8037E8E0.unk0 != 0)) {
                game_setMode(GAME_MODE_1_UNKNOWN, 0U);
            } else if (sp1C == 0) {
                game_setMode(GAME_MODE_3_NORMAL, 1U);
            }
            break;

        case GAME_MODE_4_PAUSED:                                     /* switch 2 */
            if (gcPauseMenu_update() || cutscenetrigger_update()) {
                FUNC_8030E624(SFX_C9_PAUSEMENU_ENTER, 0.899316, 32736);
                gsworld_setEnableUpdate(1);
                func_8025A430(-1, 2000, 3);
                func_8025A2B0();
                gsworld_setEnableDraw(1);
                game_setMode(GAME_MODE_3_NORMAL, 0U);
            }
            break;
    }
    if ((D_8037E8E0.game_mode == GAME_MODE_3_NORMAL) || (func_802E4A08() != 0)) {
        timedFuncQueue_update();
        func_802FA0F8();
    }
    gctransition_update();
    if (func_802E4A08() == 0) {
        print_updateBoldLetterFontDelayedFreeing();
    }
    // [port] After SNS/demo map reload, skip the first draw frame so the aux FBO
    // can initialize before we present. Without this, the scene renders to the
    // primary FBO for one frame, causing a visible black flash.
    if (sSkipDrawFrames > 0) {
        sSkipDrawFrames--;
        return false;
    }
    return true;
}

void func_802E48B8(enum game_mode_e mode, s32 arg1){
    game_setMode(mode, arg1);
}

s32 game_defrag(void){
    func_802555C4(); //reset defragged flag in memory.c
    if( !level_get() )
        return 0;
    
    glspline_defrag();
    animCache_defrag();
    pem_defragAll();
    ncCameraNodeList_defrag();
    modelRender_defrag();
    func_8028FB68();
    partEmitMgr_defrag();
    mapModel_defrag();
    cubeList_defrag();
    actorArray_defrag();
    spawnQueue_defrag();
    func_802F3300();
    printbuffer_defrag();
    gcdialog_defrag();
    if(D_8037E8E0.game_mode == GAME_MODE_4_PAUSED)
        gcpausemenu_defrag();
    switch(overlayManager_getLoadedID()){
        case OVERLAY_2_WHALE:
            func_803894A0();
            break;
        case OVERLAY_D_WITCH:
            code_C9E70_defrag();
            break;
    }
    return func_802555D0(); //returns defrag flag in memory.c
}

void func_802E49E0(void){
    D_8037E8E0.unkC = true;
}

int game_is_frozen(void){
    return D_8037E8E0.unkC;
}

s32 getGameMode(void){
    return D_8037E8E0.game_mode;
}

bool func_802E4A08(void){
    return (D_8037E8E0.game_mode == GAME_MODE_6_FILE_PLAYBACK) 
        || (D_8037E8E0.game_mode == GAME_MODE_5_UNKNOWN)
        || (D_8037E8E0.game_mode == GAME_MODE_7_ATTRACT_DEMO)
        || (D_8037E8E0.game_mode == GAME_MODE_8_BOTTLES_BONUS)
        || (D_8037E8E0.game_mode == GAME_MODE_9_BANJO_AND_KAZOOIE)
        || (D_8037E8E0.game_mode == GAME_MODE_A_SNS_PICTURE);
}

void func_802E4A70(void){
    D_8037E8E0.unk1C = 1;
}

void func_802E4A80(void){
    D_8037E8E0.unk1C = 0;
}

u8 func_802E4A8C(void){
    return D_8037E8E0.unk1C;
}

s32 func_802E4A98(s32 arg0){
    return D_803687F0[8*(arg0 - 0x80) + 0];
}

s32 func_802E4AAC(s32 arg0){
    return D_803687F0[8*(arg0 - 0x80) + 1];
}

s32 func_802E4AC0(s32 arg0){
    return D_803687F0[8*(arg0 - 0x80) + 2];
}

s32 func_802E4AD4(s32 arg0){
    return D_803687F0[8*(arg0 - 0x80) + 3];
}

s32 func_802E4AE8(s32 arg0){
    return D_803687F0[8*(arg0 - 0x80) + 4];
}

s32 func_802E4AFC(s32 arg0){
    return D_803687F0[8*(arg0 - 0x80) + 5];
}

s32 func_802E4B10(s32 arg0){
    return D_803687F0[8*(arg0 - 0x80) + 6];
}

s32 func_802E4B24(s32 arg0){
    return D_803687F0[8*(arg0 - 0x80) + 7];
}

f32 func_802E4B38(void){
    return D_8037E8E0.unk8;
}
