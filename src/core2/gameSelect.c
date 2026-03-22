#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#include "core2/modelRender.h"

#include "core2/gc/zoombox.h"
#include "core2/quiz_storage.h"

#include "port/GameConfig.h"

s32 gSelectedGameNum = -1;

#ifndef ABS
#define	ABS(d)		((d) >= 0) ? (d) : -(d)
#endif

void debugScoreStates(void);
void clearScoreStates(void);

Actor *gameSelect_draw(ActorMarker *, Gfx **, Mtx **, Vtx **);
Actor *gameSelect_zoomboxDraw(ActorMarker *, Gfx **, Mtx **, Vtx **);
void gameSelect_update(Actor *this);
void gameSelect_initAndUpdate(Actor *this);

extern void func_802C71F0(Actor *);
extern void func_802C74F4(Actor *, s32, f32 );
extern void warp_lairEnterLairFromSMLevel(s32, s32);
extern void warp_smExitBanjosHouse(s32, s32);
extern void func_80335110(s32);
extern void controller_getJoystick(s32, f32*);

extern char *gcpausemenu_TimeToA(int);
extern struct5Bs *func_803097A0(void);

extern void port_setViBlack(int active);

/* .data */
f32 INITIAL_CAMERA_POSITIONS[3][3] = {
    {-320.0f, 340.0f, 350.0f},
    {110.0f, 340.0f, 110.0f},
    {-413.333313f, 353.333313f, -234.305511f}
};
u8 *CONTROL_STICK_INSTRUCTIONS = "USE THE CONTROL STICK TO SELECT A GAME.";   
u8 *ERASE_INSTRUCTIONS = "PRESS A TO PLAY THE GAME OR Z TO ERASE IT!";
u8 *ERASE_CONFIRMATION = "ARE YOU SURE? PRESS A TO CONFIRM, OR B TO CANCEL";
s32 gameNumber = -1;
f32 INITIAL_CAMERA_TARGETS[3][3] = {
    {-435.0f,      278.0f,  -159.0f},
    { 444.635437f, 216.0f,  -356.591675f},
    {  55.0f,      191.822906f, -905.96875f}
};

ActorAnimationInfo banjoSleepingAnimations[] = {
    {0x000, 0.0f},
    {0x24D, 9e+09f},
    {0x24D, 2.0f},  
    {0x24E, 1.0f},
    {0x24F, 0.6f},  
    {0x24D, 2.0f}
};
ActorInfo gameSelect_banjoSleeping = { 0xE4, 0x195, 0x532, 0x1, banjoSleepingAnimations, gameSelect_initAndUpdate, actor_update_func_80326224, gameSelect_zoomboxDraw, 0, 0, 0.0f, 0};

ActorAnimationInfo banjoGameboyAnimations[] = {
    {0x000, 0.0f}, 
    {0x250, 9e+09f},
    {0x250, 4.5f}, 
    {0x251, 1.0f},
    {0x252, 0.67f}, 
    {0x250, 4.5f},
};
ActorInfo gameSelect_banjoGameboy = { 0xE5, 0x196, 0x532, 0x1, banjoGameboyAnimations, gameSelect_update, actor_update_func_80326224, gameSelect_draw, 0, 0, 0.0f, 0};

ActorAnimationInfo banjoCookingAnimations[] = {
    {0x000, 0.0f},
    {0x24A, 9e+09f},  
    {0x24A, 1.0f},
    {0x24B, 1.0f},  
    {0x24C, 1.0f},
    {0x24A, 1.0f}
};
ActorInfo gameSelect_banjoCooking = { 0xE6, 0x197, 0x532, 0x1, banjoCookingAnimations, gameSelect_update, actor_update_func_80326224, gameSelect_draw, 0, 0, 0.0f, 0};


/* .bss */
s32 mm_hut_smash_count;
u32 CH_TREASUREHUNT_PUZZLE_CURRENT_STEP;
struct FF_StorageStruct* D_8037DCB8;
s32 D_8037DCBC;
u8 gCompletedBottlesBonusGames[7]; // bottle bonus puzzle?
u8 D_8037DCC7;
u8 D_8037DCC8;
u8 D_8037DCC9;
u8 D_8037DCCA;
u8 D_8037DCCB;
u8 D_8037DCCC;
u8 D_8037DCCD;
u8 D_8037DCCE[3];
s32 pad_8037DCD4;
s32 pad_8037DCD8;

struct {
    u8 *unk0;
    u8 *unk4;
} selectInstructions;
s32 previousGameNumber;
s32 isFileMoving;
GcZoombox *chGameSelectTopZoombox;
GcZoombox *chGameSelectBottomZoombox;
f32 cameraPositions[2][3];
f32 cameraDelta[2][3];
s32 cookingSoundEffectIndex;
s32 isTopTextNotFinishedDisplaying;
f32 gameSelectCameraDelta;
f32 cycleInstructionsTimer;



/* .code */
Actor *gameSelect_draw(ActorMarker *marker, Gfx **gfx, Mtx **mtx, Vtx **vtx){
    s32 sp1C = marker->id - 0xe4;
    func_8033A45C(3, sp1C);
    func_8033A45C(1, 1);
    func_8033A45C(4, 1);
    func_8033A45C(9, 1);
    func_8033A45C(5, 0);
    func_8033A45C(8, 0);
    func_8033A45C(6, 0);
    func_8033A45C(7, 0);
    func_8033A45C(0xC, 1);
    func_8033A45C(0xF, 1);
    if(sp1C == gameNumber){
        modelRender_setEnvColor(0xFF, 0xFF, 0xFF, 0xFF);
    }
    else{
        modelRender_setEnvColor(0x64, 0x64, 0x64, 0xFF);
    }
    return actor_draw(marker, gfx, mtx, vtx);
}

Actor *gameSelect_zoomboxDraw(ActorMarker *marker, Gfx **gfx, Mtx **mtx, Vtx **vtx){
    Actor *ret_val = gameSelect_draw(marker, gfx, mtx, vtx);
    if(chGameSelectBottomZoombox)
        gczoombox_draw(chGameSelectBottomZoombox, gfx, mtx, vtx);
    if(chGameSelectTopZoombox)
        gczoombox_draw(chGameSelectTopZoombox, gfx, mtx, vtx);
    return ret_val;
    
}

void topZoomboxCallback(s32 arg0, s32 arg1){
    if(arg1 == 3)
        isTopTextNotFinishedDisplaying = 0;
}

void *calculateGameSelectCameraPosition(f32 arg0[3], f32 arg1[3], f32 arg2) {
    f32 phi_f12;
    f32 sp40[3];
    s32 i;
    static bool dummy_index;
    static f32 bounciness;
    static f32 sin_bounciness_half_pi;

    arg2 = (arg2 > 0.75) ? 0.75 : arg2;
    sp40[0] = arg1[0] - arg0[0];
    sp40[1] = arg1[1] - arg0[1];
    sp40[2] = arg1[2] - arg0[2];
    dummy_index = dummy_index^1;
    phi_f12 = gu_sqrtf(sp40[0]*sp40[0] + sp40[1]*sp40[1] + sp40[2]*sp40[2]);
    if (phi_f12 < 10.0f) {
        phi_f12 = 500.0f;
    }
    bounciness = 1.0 + (9.0f / gu_sqrtf(phi_f12));
    sin_bounciness_half_pi = sinf(bounciness*1.5707963267948966);
    for(i = 0; i < 3; i++){
        cameraDelta[dummy_index][i] = arg0[i] + ((arg1[i] - arg0[i])*sinf((((arg2 / 0.75) * 3.1415926535897931) / 2) * bounciness)) / sin_bounciness_half_pi;
        cameraPositions[dummy_index][i] += (cameraDelta[dummy_index][i] - cameraPositions[dummy_index][i]) / 5.0;

    }
    return &cameraPositions[dummy_index];
}

void setGameInformationZoombox(s32 gamenum){
    u8 * sp20[2];
    static u8 upperTextLine[0x20];
    static u8 lowerTextLine[0x20];

    debugScoreStates();
    gameNumber = gamenum;
    clearScoreStates();
    if(gameFile_isNotEmpty(gamenum)){
        gameFile_load(gamenum);
        D_8037DCCE[gamenum] = (itemscore_timeScores_get(LEVEL_6_LAIR)) ? 1 : 0;
    
        strcpy(upperTextLine, "");
        strcat(upperTextLine, "GAME ");
        switch(gamenum){
            case 0: //L802C4820
                strIToA(upperTextLine, 1);
                break;
            case 1: //L802C4838
                strIToA(upperTextLine, 3);
                break;
            case 2: //L802C484C
                strIToA(upperTextLine, 2);
                break;
        }//L802C4858
        strcat(upperTextLine, ": TIME ");
        strcat(upperTextLine, gcpausemenu_TimeToA(itemscore_timeScores_getTotal()));
        strcat(upperTextLine, ",");
        strcat(upperTextLine, "");

        strcpy(lowerTextLine, "");
        strIToA(lowerTextLine, jiggyscore_total());
        strcat(lowerTextLine, " JIGSAW");
        if(jiggyscore_total() != 1){
            strcat(lowerTextLine, "S");
        }
        strcat(lowerTextLine, ", ");
        strIToA(lowerTextLine, itemscore_noteScores_getTotal());
        strcat(lowerTextLine, " NOTE");
        if(itemscore_noteScores_getTotal() != 1){
            strcat(lowerTextLine, "S");
        }
        strcat(lowerTextLine, ".");
        strcat(lowerTextLine, "");
    }//L802C49AC
    else{
        D_8037DCCE[gamenum] = 0;
        strcpy(upperTextLine, "");
        strcat(upperTextLine, "GAME ");
        switch (gamenum){
            case 0:
                strIToA(upperTextLine, 1);
                break;
            case 1:
                strIToA(upperTextLine, 3);
                break;
            case 2:
                strIToA(upperTextLine, 2);
                break;
        }//L802C4A40
        strcat(upperTextLine, ": EMPTY");
        strcpy(lowerTextLine, "");
    }//L802C4A68
    sp20[0] = upperTextLine;\
    sp20[1] = lowerTextLine;
    func_8031877C(chGameSelectBottomZoombox);
    gczoombox_setStrings(chGameSelectBottomZoombox, 2, (char **)sp20); // [port]
    gczoombox_maximize(chGameSelectBottomZoombox);
    gczoombox_resolve_minimize(chGameSelectBottomZoombox);
}

void eraseGame(s32 arg0){
    gameFile_clear(arg0);
    setGameInformationZoombox(arg0);
}

void gameSelect_free(Actor * this){
    int i;

    if(chGameSelectTopZoombox){
        gczoombox_free(chGameSelectTopZoombox);
        chGameSelectTopZoombox = NULL;
    }

    if(chGameSelectBottomZoombox){
        gczoombox_free(chGameSelectBottomZoombox);
        chGameSelectBottomZoombox = NULL;
    }

    for(i = 0; i < 3; i++){
        gameFile_8033CFD4(i);
    }

    if(cookingSoundEffectIndex){
        func_802F9D38(cookingSoundEffectIndex);
        cookingSoundEffectIndex = 0; // [port] was NULL, use 0 for s32
    }

    comusic_8025AB44(COMUSIC_73_GAMEBOY, 0, 4000);
    func_8025AABC(COMUSIC_73_GAMEBOY);
    func_8025AB00();
}

void spawnGameSelectProps(ActorMarker *marker){
    Actor *this;
    s32 sp20;
    Actor *other;
    f32 sp18;
    sp20 = marker->id - 0xe4;
    this = marker_getActor(marker);
    sp18 = this->scale;
    other = actor_spawnWithYaw_f32(sp20 + 0x198, this->position, (s32)this->yaw);
    other->scale = sp18;
}

void gameSelect_update(Actor *this){
    int sp84;
    int sp80;
    s32 sp74[3];
    s32 *tmp_a2; //pad70
    s32 pad_6C;
    s32 pad_68;
    s32 sp5C[6]; // [port] was [3], but controller_copyFaceButtons writes 6 entries
    f32 sp54[2]; // [port] was scalar, but controller_getJoystick writes f32[2]
    f32 sp50;
    int i; //sp4C
    struct5Bs *sp48;
    f32 sp44;
    s32 tmp_a2_2;
    f32 sp34[3];

    sp84 = this->marker->id - 0xe4;
    gSelectedGameNum = sp84;
    sp80 = (sp84 == gameNumber);
    sp50 = time_getDelta();
    if(chGameSelectBottomZoombox == NULL)
        return;

    if(!this->initialized){
        __spawnQueue_add_1((GenFunction_1)spawnGameSelectProps, (uintptr_t)this->marker);
        func_802C7318(this);
        this->unk130 = func_802C71F0;
        if(sp84 == 0){
            func_802C75A0(this, 1);
            func_802C74F4(this, 0, 1.0f);
            func_802C74F4(this, 1, 1.0f);
        }//L802C4CD8
        this->initialized = true;
    }//L802C4CE4
    func_802C7478(this);
    if(!sp80){
        if(this->state != 1){
            subaddie_set_state(this, 1);
        }
    }
    else{//L802C4D24
        controller_copySideButtons(0, sp74);
        controller_copyFaceButtons(0, sp5C);
        controller_getJoystick(0, sp54); // [port] was &sp54, array decays to f32*
        switch(this->state){
            case 2:
            case 5:
            switch(sp84){
                case 0://L802C4D8C
                    if(actor_animationIsAt(this, 0.1f))
                        sfxsource_play(SFX_5D_BANJO_RAAOWW, 8000);

                    if(actor_animationIsAt(this, 0.7f))
                        sfxsource_play(SFX_5E_BANJO_PHEWWW, 8000);
                    break;
                case 1://L802C4DD0
                    if(randf() < 0.1){
                    // if(randf() < D_80376118){
                        gcsfx_playWithPitch(MIN(2.0f, randf() *3.0f) + 311.0f, 1.0f, 12000);
                    }
                    break;
                case 2://L802C4E74
                    if(randf() < 0.03){
                        gcsfx_playWithPitch(0x3ed, randf()*0.3 + 0.7, 15000);
                    }
                    break;
            }//L802C4ED4
            break;
        }//L802C4ED4
        if(!func_8038AAB0()){
            switch(this->state){
                case 1://L802C4F10
                    if(sp84 == 1){
                        sfxsource_play(SFX_136_GAMEBOY_STARTUP, 15000);
                        timedFunc_set_3(0.25f, (GenFunction_3)comusic_8025AB44, COMUSIC_73_GAMEBOY, -1, 2000);
                        func_8025A58C(0, 2000);
                    }
                    else{
                        comusic_8025AB44(COMUSIC_73_GAMEBOY, 0, 4000);
                        func_8025A58C(-1, 2000);
                    }

                    if(sp84 == 2){
                        cookingSoundEffectIndex = func_802F9AA8(SFX_12B_BOILING_AND_BUBBLING);
                        func_802F9F80(cookingSoundEffectIndex, 0.5f, 9000000000.0f, 0.5f);
                        func_802F9DB8(cookingSoundEffectIndex, 0.9f, 0.9f, 0.0f);
                        func_802FA060(cookingSoundEffectIndex, 15000, 15000, 0.0f);
                    }
                    else{
                        if(cookingSoundEffectIndex){
                            func_802F9D38(cookingSoundEffectIndex);
                            cookingSoundEffectIndex = 0; // [port] was NULL, use 0 for s32
                        }
                    }
                    setGameInformationZoombox(sp84);
                    subaddie_set_state(this, 2);
                    break;
                case 5://L802C5040
                    if(isTopTextNotFinishedDisplaying == 0 && 
                        (sp5C[FACE_BUTTON(BUTTON_A)] == 1 || sp5C[FACE_BUTTON(BUTTON_B)] == 1)
                    ){
                        if(sp5C[FACE_BUTTON(BUTTON_A)] == 1){
                            eraseGame(sp84);
                            coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 22000);
                        }
                        subaddie_set_state(this, 2);
                        func_8031877C(chGameSelectTopZoombox);
                        gczoombox_setStrings(chGameSelectTopZoombox, 2, (char **)&selectInstructions);
                        cycleInstructionsTimer = 0.0f;
                    }
                    break;
                case 3://L802C50C8
                case 4://L802C50C8
                    if(anctrl_isStopped(this->anctrl)){
                        chBottlesBonus_func_802DEB80();
                        // [port] Restore bottles bonus after vanilla reset.
                        // gameFile_load handles the slot lookup and restores both
                        // lives and bottles bonus via port_restoreFileEnhancementData.
                        {
                            extern u8 gCompletedBottlesBonusGames[7];
                            extern s32 chBottlesBonusPuzzleIndex;
                            s32 _i;
                            gameFile_load(gSelectedGameNum);
                            for (_i = 0; _i < 7; _i++) {
                                if (gCompletedBottlesBonusGames[_i]) {
                                    chBottlesBonusPuzzleIndex = _i + 1;
                                }
                            }
                        }
                        if(!gameFile_isNotEmpty(sp84)){
                            // [port] BB romhacks can override the new-game boot map
                            s32 newGameMap = port_getRomhackNewGameMap();
                            if (newGameMap < 0) {
                                newGameMap = MAP_85_CS_SPIRAL_MOUNTAIN_3;
                            }
                            timedFunc_set_3(0.0f, (GenFunction_3)transitionToMap, newGameMap, 0, 1);
                            {
                                s32 knowAll = port_getRomhackKnowAllMoves();
                                if (knowAll >= 0) {
                                    ability_setAllLearned(knowAll);
                                }
                            }
                        }
                        else{//L802C511C
                            sp44 = 0.0f;
                            if(this->state == 4 &&  (sp84 == 0 || sp84 == 1))
                                sp44 = 0.25f;
                            if(chmole_learnedAllSpiralMountainAbilities() && fileProgressFlag_get(FILEPROG_BD_ENTER_LAIR_CUTSCENE)){
                                timedFunc_set_2(sp44, (GenFunction_2)warp_lairEnterLairFromSMLevel, 0, 0);
                            }
                            else{//L802C5188
                                timedFunc_set_2(sp44, (GenFunction_2)warp_smExitBanjosHouse, 0, 0);
                            }//L802C51A0
                            timedFunc_set_1(sp44, (GenFunction_1)func_80335110, 1);
                        }//L802C51B8
                        this->state = 6;
                    }
                    break;
                case 2://L802C51CC
                    if(sp74[0] == 1){
                        if(gameFile_isNotEmpty(sp84)){
                            func_8031877C(chGameSelectTopZoombox);
                            func_803183A4(chGameSelectTopZoombox, (&ERASE_CONFIRMATION)[code94620_func_8031B5B0()]);
                            isTopTextNotFinishedDisplaying = 1;
                            subaddie_set_state(this, 5);
                        }
                        else{//L802C5240
                            coMusicPlayer_playMusic(COMUSIC_2C_BUZZER, 22000);
                        }
                    }
                    else if(sp5C[FACE_BUTTON(BUTTON_A)] == 1){//L802C5250
                        if(gameFile_isNotEmpty(sp84)){
                            if(randf() < 0.1){
                                switch(sp84){
                                    case 0://L802C52B8
                                        sfxsource_play(SFX_31_BANJO_OHHWAAOOO, 28000);
                                        gcsfx_play(SFX_135_CARTOONY_SPRING);
                                        timedFunc_set_2(0.4f, (GenFunction_2)sfxsource_play, SFX_13A_GLASS_BREAKING_7, 0x7fff);
                                        timedFunc_set_2(0.9f, (GenFunction_2)sfxsource_play, SFX_150_PORCELAIN_CRASH, 0x7fff);
                                        timedFunc_set_2(1.0f, (GenFunction_2)sfxsource_play, SFX_151_CAT_MEOW, 0x7fff);
                                        break;
                                    case 1://L802C5320
                                        timedFunc_set_2(0.4f, (GenFunction_2)sfxsource_play, SFX_31_BANJO_OHHWAAOOO, 28000);
                                        timedFunc_set_2(0.2f, (GenFunction_2)sfxsource_play, SFX_E_SHOCKSPRING_BOING, 28000);
                                        gcsfx_play(SFX_2D_KABOING);
                                        break;
                                    case 2://L802C5364
                                        timedFunc_set_2(0.15f, (GenFunction_2)sfxsource_play, SFX_32_BANJO_EGHEE, 28000);
                                        sfxsource_play(SFX_3F6_RUBBING, 28000);
                                        gcsfx_play(SFX_8F_SNOWBALL_FLYING);
                                        break;
                                }//L802C5394
                                subaddie_set_state(this, 4);
                                levelSpecificFlags_set(sp84 + 0x35, 1);
                            }
                            else{//L802C53B4
                                sfxsource_playHighPriority(SFX_3EA_UNKNOWN);
                                subaddie_set_state(this, 3);
                            }
                        }else{//L802C53D0
                            sfxsource_play(SFX_4F_BANJO_WAHOO, 28000);
                            subaddie_set_state(this, 3);
                        }//L802C53E8
                        if(sp84 == 0)
                            func_802C75A0(this, 2);

                        if(sp84 == 1)
                            comusic_8025AB44(COMUSIC_73_GAMEBOY, 0, 4000);
                        
                        func_8025A58C(0, 0x1f4);
                        actor_playAnimationOnce(this);
                    }
                    else{//L802C5434
                        if((0.7 < ((0.0f <= sp54[0]) ? sp54[0] : -sp54[0])) && isFileMoving == 0
                        ){
                            tmp_a2_2 = gameNumber;
                            if(sp54[0] < 0.0f){
                                isFileMoving = 1;
                                switch(gameNumber){
                                    case 0:
                                        isFileMoving = 0;
                                        break;
                                    case 1:
                                        gameNumber = 2;
                                        break;
                                    case 2:
                                        gameNumber = 0;
                                        break;
                                }
                            }
                            else{//L802C54D4
                                isFileMoving = 1;
                                switch(gameNumber){
                                    case 0:
                                        gameNumber = 2;
                                        break;
                                    case 1:
                                        isFileMoving = 0;
                                        break;
                                    case 2:
                                        gameNumber = 1;
                                        break;
                                }
                            }//L802C550C
                            if(isFileMoving){
                                previousGameNumber = tmp_a2_2;
                                gameSelectCameraDelta = 0.0f;
                            }
                        }else{//L802C5530
                            if(((0.0f <= sp54[0]) ? sp54[0] : -sp54[0]) < 0.3){
                                isFileMoving = 0;
                            }
                        }
                    }//L802C556C
                    if(isTopTextNotFinishedDisplaying == 0){
                        cycleInstructionsTimer += sp50;
                        if(20.0 < cycleInstructionsTimer){
                            func_8031877C(chGameSelectTopZoombox);
                            gczoombox_setStrings(chGameSelectTopZoombox, 2, (char **)&selectInstructions);
                            cycleInstructionsTimer = 0.0f;
                        }
                    }
                    break;
                case 6://L802C55E8
                    break;
            }
        }//L802C55E8
        gameSelectCameraDelta += sp50;
        sp48 = func_803097A0();
        if(this->marker->unk14_21){
            for(i = 0; i < 3; i++){
                func_8034A174(sp48, i+5, sp34);
                ml_vec3f_copy(INITIAL_CAMERA_POSITIONS[i], sp34);
            }
        }
        ncStaticCamera_setPositionAndTarget(
            calculateGameSelectCameraPosition(INITIAL_CAMERA_POSITIONS[previousGameNumber], INITIAL_CAMERA_POSITIONS[gameNumber], gameSelectCameraDelta), 
            calculateGameSelectCameraPosition(INITIAL_CAMERA_TARGETS[previousGameNumber], INITIAL_CAMERA_TARGETS[gameNumber], gameSelectCameraDelta)
        );
        if(this->marker->unk14_21) {
            osViBlack(0);
            port_setViBlack(0); // [port]
        }
    }//L802C5734
}

void gameSelect_initAndUpdate(Actor * this){
    int i = code94620_func_8031B5B0();
    selectInstructions.unk0 = (&CONTROL_STICK_INSTRUCTIONS)[i];
    selectInstructions.unk4 = (&ERASE_INSTRUCTIONS)[i];

    if(!this->initialized){
        gameFile_8033CE40();
         if(chGameSelectBottomZoombox == NULL){
            chGameSelectBottomZoombox = gczoombox_new(0xA0, ZOOMBOX_SPRITE_C_BANJO_2, 2, 0, NULL);
            gczoombox_open(chGameSelectBottomZoombox);
            gczoombox_func_803184C8(chGameSelectBottomZoombox, 30.0f, 5, 2, 0.4f, 0, 0);
        }//L802C57FC

        if(chGameSelectTopZoombox == NULL){
            chGameSelectTopZoombox = gczoombox_new(0xA, ZOOMBOX_SPRITE_D_KAZOOIE_1, 2, 1, topZoomboxCallback);
            gczoombox_setStrings(chGameSelectTopZoombox, 2, (char **)&selectInstructions);
            gczoombox_open(chGameSelectTopZoombox);
            gczoombox_maximize(chGameSelectTopZoombox);
        }//L802C5860

        marker_setFreeMethod(this->marker, gameSelect_free);
        isFileMoving = 0;
        debugScoreStates();
        clearScoreStates();
        previousGameNumber = 0;
        gameNumber = 0;
        cameraPositions[1][0] = INITIAL_CAMERA_POSITIONS[0][0];
        cameraPositions[1][1] = INITIAL_CAMERA_POSITIONS[0][1];
        cameraPositions[1][2] = INITIAL_CAMERA_POSITIONS[0][2];

        cameraPositions[0][0] = INITIAL_CAMERA_TARGETS[0][0];
        cameraPositions[0][1] = INITIAL_CAMERA_TARGETS[0][1];
        cameraPositions[0][2] = INITIAL_CAMERA_TARGETS[0][2];
        gameSelectCameraDelta = 0.75f;
        cycleInstructionsTimer = func_8038AAB0(&INITIAL_CAMERA_TARGETS[0], &previousGameNumber) ? 20.0 : 0.0;
        actor_collisionOff(this);
        coMusicPlayer_playMusic(COMUSIC_73_GAMEBOY, 0);
    }//L802C5940
    if(!func_8038AAB0()){
        if(chGameSelectBottomZoombox)
            gczoombox_update(chGameSelectBottomZoombox);
        if(chGameSelectTopZoombox)
            gczoombox_update(chGameSelectTopZoombox);
    }
    gameSelect_update(this);
}

void gameSelect_saveAndExit(void){
    s32 sp1C = level_get();
    s32 t6 = map_get() == MAP_83_CS_GAME_OVER_MACHINE_ROOM;
    s32 a1 = (0 < sp1C && sp1C < 0xd);
    if( a1 || t6)
    {
        if(gameNumber != -1 && !func_802E4A08() && map_get() != MAP_91_FILE_SELECT){
            gameFile_save(gameNumber);
            gameFile_8033CFD4(gameNumber);
        }
    }
}

s32 gameSelect_getGameNumber(void){
    return gameNumber;
}

void gameSelect_setGameNumber(s32 arg0){
    gameNumber = arg0;
}

void gameSelect_resetGameNumber(void){
    gameNumber = -1;
}
