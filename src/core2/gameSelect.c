// BanjoDecomp: core2/ch/gameSelect.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "core2/ch/gameSelect.h"

#include "core2/modelRender.h"

#include "core2/gc/zoombox.h"
#include "core2/quiz/storage.h"

#include "port/Romhack/RomhackConfig.h"
#include "port/Patches/Patches.h"


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
extern void controller_copyJoystick(s32, f32*);

extern char *gcpausemenu_TimeToA(int);
extern Vec3fArray *func_803097A0(void);

/* .data */
f32 INITIAL_CAMERA_POSITIONS[3][3] = {
    {-320.0f, 340.0f, 350.0f},
    {110.0f, 340.0f, 110.0f},
    {-413.333313f, 353.333313f, -234.305511f}
};
u8 *D_80365DF4[] = {
    "USE THE CONTROL STICK TO SELECT A GAME.",
    NULL,
    NULL,
};
u8 *D_80365DF8[] = {
    "PRESS A TO PLAY THE GAME OR Z TO ERASE IT!",
    NULL,
    NULL,
};
u8 *D_80365DFC[] = {
    "ARE YOU SURE? PRESS A TO CONFIRM, OR B TO CANCEL.",
    NULL,
    NULL,
};
s32 gameNumber = -1;
f32 INITIAL_CAMERA_TARGETS[3][3] = {
    {-435.0f,      278.0f,  -159.0f},
    { 444.635437f, 216.0f,  -356.591675f},
    {  55.0f,      191.822906f, -905.96875f}
};

ActorAnimationInfo banjoGameboyAnimations[] = {
    {0x000, 0.0f},
    {ASSET_24D_ANIM_FSS_BANJO_SLEEPING_unk, 9e+09f},
    {ASSET_24D_ANIM_FSS_BANJO_SLEEPING_unk, 2.0f},  
    {ASSET_24E_ANIM_FSS_BANJO_SLEEPING_unk, 1.0f},
    {ASSET_24F_ANIM_FSS_BANJO_SLEEPING_unk, 0.6f},  
    {ASSET_24D_ANIM_FSS_BANJO_SLEEPING_unk, 2.0f}
};
ActorInfo gameSelect_banjoSleeping = { 0xE4, 0x195, 0x532, 0x1, banjoGameboyAnimations, gameSelect_initAndUpdate, actor_update_func_80326224, gameSelect_zoomboxDraw, 0, 0, 0.0f, 0};

ActorAnimationInfo banjoSleepingAnimations[] = {
    {0x000, 0.0f}, 
    {ASSET_250_ANIM_FSS_BANJO_GAMEBOY_unk, 9e+09f},
    {ASSET_250_ANIM_FSS_BANJO_GAMEBOY_unk, 4.5f}, 
    {ASSET_251_ANIM_FSS_BANJO_GAMEBOY_unk, 1.0f},
    {ASSET_252_ANIM_FSS_BANJO_GAMEBOY_unk, 0.67f}, 
    {ASSET_250_ANIM_FSS_BANJO_GAMEBOY_unk, 4.5f},
};
ActorInfo gameSelect_banjoGameboy = { 0xE5, 0x196, 0x532, 0x1, banjoSleepingAnimations, gameSelect_update, actor_update_func_80326224, gameSelect_draw, 0, 0, 0.0f, 0};

ActorAnimationInfo banjoCookingAnimations[] = {
    {0x000, 0.0f},
    {ASSET_24A_ANIM_FSS_BANJO_COOKING_unk, 9e+09f},  
    {ASSET_24A_ANIM_FSS_BANJO_COOKING_unk, 1.0f},
    {ASSET_24B_ANIM_FSS_BANJO_COOKING_unk, 1.0f},  
    {ASSET_24C_ANIM_FSS_BANJO_COOKING_unk, 1.0f},
    {ASSET_24A_ANIM_FSS_BANJO_COOKING_unk, 1.0f}
};
ActorInfo gameSelect_banjoCooking = { 0xE6, 0x197, 0x532, 0x1, banjoCookingAnimations, gameSelect_update, actor_update_func_80326224, gameSelect_draw, 0, 0, 0.0f, 0};


// Yes, Gaming Chair is before Kitchen
enum chgameselect_savefile_e {
    CH_GAME_SELECT_SAVEFILE_0_BED,
    CH_GAME_SELECT_SAVEFILE_1_GAMING_CHAIR,
    CH_GAME_SELECT_SAVEFILE_2_KITCHEN
};

/* .bss */
s32 mm_hut_smash_count;
u32 chtreasureHunt_puzzleCurrentStep;
struct FF_StorageStruct* ffStorage;
s32 mmhut_smashCount;
u8 gCompletedBottlesBonusGames[7]; // bottle bonus puzzle?
u8 D_8037DCC7;
u8 D_8037DCC8;
u8 D_8037DCC9;
u8 D_8037DCCA;
u8 D_8037DCCB;
u8 chBottleBonusPuzzleIndex;
u8 D_8037DCCD;
u8 D_8037DCCE[3];
s32 pad_8037DCD4;
s32 pad_8037DCD8;

struct {
    u8 *controlInstruction;
    u8 *eraseInstruction;
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
    modelRender_setAppendageVisibility(3, sp1C);
    modelRender_setAppendageVisibility(1, 1);
    modelRender_setAppendageVisibility(4, 1);
    modelRender_setAppendageVisibility(9, 1);
    modelRender_setAppendageVisibility(5, 0);
    modelRender_setAppendageVisibility(8, 0);
    modelRender_setAppendageVisibility(6, 0);
    modelRender_setAppendageVisibility(7, 0);
    modelRender_setAppendageVisibility(0xC, 1);
    modelRender_setAppendageVisibility(0xF, 1);
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

void *calculateGameSelectCameraPosition(f32 arg0[3], f32 arg1[3], f32 deltaTime) {
    f32 sqrt_totals;
    f32 delta[3];
    s32 i;
    static bool dummy_index;
    static f32 bounciness;
    static f32 sin_bounciness_half_pi;

    deltaTime = (deltaTime > 0.75) ? 0.75 : deltaTime;
    delta[0] = arg1[0] - arg0[0];
    delta[1] = arg1[1] - arg0[1];
    delta[2] = arg1[2] - arg0[2];
    dummy_index = dummy_index^1;
    sqrt_totals = gu_sqrtf(delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2]);
    if (sqrt_totals < 10.0f) {
        sqrt_totals = 500.0f;
    }
    bounciness = 1.0 + (9.0f / gu_sqrtf(sqrt_totals));
    sin_bounciness_half_pi = sinf(bounciness*1.5707963267948966);
    for(i = 0; i < 3; i++){
        cameraDelta[dummy_index][i] = arg0[i] + ((arg1[i] - arg0[i])*sinf((((deltaTime / 0.75) * 3.1415926535897931) / 2) * bounciness)) / sin_bounciness_half_pi;
        cameraPositions[dummy_index][i] += (cameraDelta[dummy_index][i] - cameraPositions[dummy_index][i]) / 5.0;

    }
    return &cameraPositions[dummy_index];
}

void setGameInformationZoombox(s32 gamenum){
    u8 * sp20[2];
    static u8 upperTextLine[0x40];
    static u8 lowerTextLine[0x40];
    static u8 *sGamePrefix[]  = { "GAME ",    "FICHIER ", "SPIEL " };
    static u8 *sTimeLabel[]   = { ": TIME ",  ": TEMPS ", ": ZEIT " };
    static u8 *sJigsawLabel[] = { " JIGSAW",  " PI" "\x63" "CE",  " PUZZLETEIL" };
    static u8 *sJigsawPlural[] = { "S", "S", "E" };
    static u8 *sNoteLabel[]   = { " NOTE",    " NOTE",    " NOTE" };
    static u8 *sNotePlural[]  = { "S", "S", "N" };
    static u8 *sEmptyLabel[]  = { ": EMPTY",  ": VIDE",   ": LEER" };
    s32 lang = code94620_func_8031B5B0();

    debugScoreStates();
    gameNumber = gamenum;
    clearScoreStates();
    CALL_EVENT(OnLoadFileSelect);
    if(gameFile_isNotEmpty(gamenum)){
        gameFile_load(gamenum);
        D_8037DCCE[gamenum] = (itemscore_timeScores_get(LEVEL_6_LAIR)) ? 1 : 0;

        bk_strcpy(upperTextLine, "");
        bk_strcat(upperTextLine, sGamePrefix[lang]);
        switch(gamenum){
            case CH_GAME_SELECT_SAVEFILE_0_BED: //L802C4820
                bk_strIToA(upperTextLine, 1);
                break;
            case CH_GAME_SELECT_SAVEFILE_1_GAMING_CHAIR: //L802C4838
                bk_strIToA(upperTextLine, 3);
                break;
            case CH_GAME_SELECT_SAVEFILE_2_KITCHEN: //L802C484C
                bk_strIToA(upperTextLine, 2);
                break;
        }//L802C4858
        bk_strcat(upperTextLine, sTimeLabel[lang]);
        bk_strcat(upperTextLine, gcpausemenu_TimeToA(itemscore_timeScores_getTotal()));
        bk_strcat(upperTextLine, ",");
        bk_strcat(upperTextLine, "");

        bk_strcpy(lowerTextLine, "");
        bk_strIToA(lowerTextLine, jiggyscore_total());
        bk_strcat(lowerTextLine, sJigsawLabel[lang]);
        if(jiggyscore_total() != 1){
            bk_strcat(lowerTextLine, sJigsawPlural[lang]);
        }
        bk_strcat(lowerTextLine, ", ");
        bk_strIToA(lowerTextLine, itemscore_noteScores_getTotal());
        bk_strcat(lowerTextLine, sNoteLabel[lang]);
        if(itemscore_noteScores_getTotal() != 1){
//          bk_strcat(lowerTextLine, "S");
            bk_strcat(lowerTextLine, sNotePlural[lang]);
        }
        bk_strcat(lowerTextLine, ".");
        bk_strcat(lowerTextLine, "");
    }//L802C49AC
    else{
        D_8037DCCE[gamenum] = 0;
        bk_strcpy(upperTextLine, "");
        bk_strcat(upperTextLine, sGamePrefix[lang]);
        switch (gamenum){
            case CH_GAME_SELECT_SAVEFILE_0_BED:
                bk_strIToA(upperTextLine, 1);
                break;
            case CH_GAME_SELECT_SAVEFILE_1_GAMING_CHAIR:
                bk_strIToA(upperTextLine, 3);
                break;
            case CH_GAME_SELECT_SAVEFILE_2_KITCHEN:
                bk_strIToA(upperTextLine, 2);
                break;
        }//L802C4A40
        bk_strcat(upperTextLine, sEmptyLabel[lang]);
        bk_strcpy(lowerTextLine, "");
    }//L802C4A68

    // [port] JP rebuilds these lines in its own layout
    CALL_EVENT(OnFileSelectInfoBuild, gamenum, (char *) upperTextLine, (char *) lowerTextLine);
    sp20[0] = upperTextLine;\
    sp20[1] = lowerTextLine;
    func_8031877C(chGameSelectBottomZoombox);
    gczoombox_setStrings(chGameSelectBottomZoombox, 2, (char **)sp20);
    gczoombox_maximize(chGameSelectBottomZoombox);
    gczoombox_resolve_minimize(chGameSelectBottomZoombox);
    CALL_EVENT(OnFileSelectPortrait, gamenum, chGameSelectBottomZoombox);
}

void eraseGame(s32 arg0){
    gameFile_clear(arg0);
    // [port] The clear only zeroes the slot in RAM; also delete the file.
    CALL_EVENT(OnGameFileErase, arg0);
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
        // gameFile_8033CFD4(i); Not needed with new Save System
    }

    if(cookingSoundEffectIndex){
        func_802F9D38(cookingSoundEffectIndex);
        cookingSoundEffectIndex = 0;
    }

    comusic_8025AB44(COMUSIC_73_GAMEBOY, 0, 4000);
    func_8025AABC(COMUSIC_73_GAMEBOY);
    func_8025AB00();
}

void spawnGameSelectProps(ActorMarker *marker){
    Actor *this;
    s32 sp20;
    Actor *prop;
    f32 sp18;
    sp20 = marker->id - 0xe4;
    this = marker_getActor(marker);
    sp18 = this->scale;
    prop = actor_spawnWithYaw_f32(sp20 + 0x198, this->position, (s32)this->yaw);
    prop->scale = sp18;
}

void gameSelect_update(Actor *this){
    int game_number;
    int game_numbers_match;
    s32 side_buttons[3];
    s32 *unused_padding; //pad70
    s32 unused_padding_2;
    s32 unused_padding_3;
    s32 face_buttons[6];
    f32 sp54[2];
    f32 delta_time;
    int i; //sp4C
    Vec3fArray *sp48;
    f32 function_time;
    s32 previous_game_number;
    f32 sp34[3];

    game_number = this->marker->id - 0xe4;
    gSelectedGameNum = game_number;
    game_numbers_match = (game_number == gameNumber);
    delta_time = time_getDelta();
    if(chGameSelectBottomZoombox == NULL)
        return;

    // [port] Let the localization layer rebuild the info zoombox live when the
    // language changed (it gates on the language generation + the selected slot).
    CALL_EVENT(OnFileSelectLanguageRefresh, game_number, game_numbers_match);

    // [port] Backstop for the swap in setGameInformationZoombox.
    if(game_numbers_match){
        CALL_EVENT(OnFileSelectPortrait, game_number, chGameSelectBottomZoombox);
    }

    if(!this->initialized){
        __spawnQueue_add_1((GenFunction_1)spawnGameSelectProps, (uintptr_t)this->marker);
        func_802C7318(this);
        this->unk130 = func_802C71F0;
        if(game_number == CH_GAME_SELECT_SAVEFILE_0_BED){
            func_802C75A0(this, 1);
            func_802C74F4(this, 0, 1.0f);
            func_802C74F4(this, 1, 1.0f);
        }//L802C4CD8
        this->initialized = true;
    }//L802C4CE4
    func_802C7478(this);
    if(!game_numbers_match){
        if(this->state != 1){
            subaddie_set_state(this, 1);
        }
    }
    else{//L802C4D24
        controller_copySideButtons(0, side_buttons);
        controller_copyFaceButtons(0, face_buttons);
        controller_copyJoystick(0, sp54);
        switch(this->state){
            case 2:
            case 5:
            switch(game_number){
                case CH_GAME_SELECT_SAVEFILE_0_BED://L802C4D8C
                    if(actor_animationIsAt(this, 0.1f))
                        sfxsource_play(SFX_5D_BANJO_RAAOWW, 8000);

                    if(actor_animationIsAt(this, 0.7f))
                        sfxsource_play(SFX_5E_BANJO_PHEWWW, 8000);
                    break;
                case CH_GAME_SELECT_SAVEFILE_1_GAMING_CHAIR://L802C4DD0
                    if(randf() < 0.1){
                    // if(randf() < D_80376118){
                        gcsfx_playWithPitch(MIN(2.0f, randf() *3.0f) + 311.0f, 1.0f, 12000);
                    }
                    break;
                case CH_GAME_SELECT_SAVEFILE_2_KITCHEN://L802C4E74
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
                    if(game_number == CH_GAME_SELECT_SAVEFILE_1_GAMING_CHAIR){
                        sfxsource_play(SFX_136_GAMEBOY_STARTUP, 15000);
                        timedFunc_set_3(0.25f, (GenFunction_3)comusic_8025AB44, COMUSIC_73_GAMEBOY, -1, 2000);
                        func_8025A58C(0, 2000);
                    }
                    else{
                        comusic_8025AB44(COMUSIC_73_GAMEBOY, 0, 4000);
                        func_8025A58C(-1, 2000);
                    }

                    if(game_number == CH_GAME_SELECT_SAVEFILE_2_KITCHEN){
                        cookingSoundEffectIndex = func_802F9AA8(SFX_12B_BOILING_AND_BUBBLING);
                        func_802F9F80(cookingSoundEffectIndex, 0.5f, 9000000000.0f, 0.5f);
                        func_802F9DB8(cookingSoundEffectIndex, 0.9f, 0.9f, 0.0f);
                        func_802FA060(cookingSoundEffectIndex, 15000, 15000, 0.0f);
                    }
                    else{
                        if(cookingSoundEffectIndex){
                            func_802F9D38(cookingSoundEffectIndex);
                            cookingSoundEffectIndex = 0;
                        }
                    }
                    setGameInformationZoombox(game_number);
                    subaddie_set_state(this, 2);
                    break;
                case 5://L802C5040
                    if(isTopTextNotFinishedDisplaying == 0 && 
                        (face_buttons[FACE_BUTTON(BUTTON_A)] == 1 || face_buttons[FACE_BUTTON(BUTTON_B)] == 1)
                    ){
                        if(face_buttons[FACE_BUTTON(BUTTON_A)] == 1){
                            eraseGame(game_number);
                            coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 22000);
                        }
                        subaddie_set_state(this, 2);
                        func_8031877C(chGameSelectTopZoombox);
                        CALL_CANCELLABLE_EVENT(LocalizeFileSelectPrompt, 0, chGameSelectTopZoombox) {
                            gczoombox_setStrings(chGameSelectTopZoombox, 2, (char **)&selectInstructions);
                        }
                        cycleInstructionsTimer = 0.0f;
                    }
                    break;
                case 3://L802C50C8
                case 4://L802C50C8
                    if(anctrl_isStopped(this->anctrl)){
                        chBottlesBonus_resetCompleted();
                        gameFile_load(gSelectedGameNum);
                        // [port] Rando refuses a new file whose seed failed to generate
                        if(!EventSystem_Should(VB_GAMESELECT_START_GAME, true, game_number)){
                            coMusicPlayer_playMusic(COMUSIC_2C_BUZZER, 22000);
                            if(game_number == CH_GAME_SELECT_SAVEFILE_0_BED)
                                func_802C75A0(this, 1);
                            subaddie_set_state(this, 1);
                            actor_loopAnimation(this);
                            break;
                        }
                        port_syncBottlesBonusIndex();
                        CALL_EVENT(OnGameStart);
                        if(EventSystem_Should(VB_GAMESELECT_START_NEW_GAME, !gameFile_isNotEmpty(game_number), game_number)){
                            s32 skipIntro = 0;
                            CALL_EVENT(OnNewGame, &skipIntro);
                            if (skipIntro) {
                                timedFunc_set_2(0.0f, (GenFunction_2)warp_lairEnterLairFromSMLevel, 0, 0);
                                timedFunc_set_1(0.0f, (GenFunction_1)gsworld_setEnableUpdate, 1);
                            } else {
                                // [port] Romhacks can override the new-game boot map
                                s32 newGameMap = port_getRomhackNewGameMap();
                                if (newGameMap < 0) {
                                    newGameMap = MAP_85_CS_SPIRAL_MOUNTAIN_3;
                                }
                                timedFunc_set_3(0.0f, (GenFunction_3)transitionToMap, newGameMap, 0, 1);
                                if (port_getRomhackKnowAllMoves() >= 0) {
                                    ability_setAllLearned(-1);
                                }
                            }
                        }
                        else{//L802C511C
                            function_time = 0.0f;
                            if(this->state == 4 &&  (game_number == CH_GAME_SELECT_SAVEFILE_0_BED || game_number == CH_GAME_SELECT_SAVEFILE_1_GAMING_CHAIR))
                                function_time = 0.25f;
                            // [port] Romhacks can replace the resume transition outright
                            if(port_getRomhackResumeWarpFunc() != NULL){
                                timedFunc_set_2(function_time, (GenFunction_2)port_getRomhackResumeWarpFunc(), 0, 0);
                            }
                            else if(chmole_learnedAllSpiralMountainAbilities() && fileProgressFlag_get(FILEPROG_BD_ENTER_LAIR_CUTSCENE)){
                                timedFunc_set_2(function_time, (GenFunction_2)warp_lairEnterLairFromSMLevel, 0, 0);
                            }
                            else{//L802C5188
                                timedFunc_set_2(function_time, (GenFunction_2)warp_smExitBanjosHouse, 0, 0);
                            }//L802C51A0
                            timedFunc_set_1(function_time, (GenFunction_1)gsworld_setEnableUpdate, 1);
                        }//L802C51B8
                        this->state = 6;
                    }
                    break;
                case 2://L802C51CC
                    if(side_buttons[0] == 1){
                        if(gameFile_isNotEmpty(game_number)){
                            func_8031877C(chGameSelectTopZoombox);
                            CALL_CANCELLABLE_EVENT(LocalizeFileSelectPrompt, 1, chGameSelectTopZoombox) {
                                func_803183A4(chGameSelectTopZoombox, D_80365DFC[code94620_func_8031B5B0()]);
                            }
                            isTopTextNotFinishedDisplaying = 1;
                            subaddie_set_state(this, 5);
                        }
                        else{//L802C5240
                            coMusicPlayer_playMusic(COMUSIC_2C_BUZZER, 22000);
                        }
                    }
                    else if(face_buttons[FACE_BUTTON(BUTTON_A)] == 1){//L802C5250
                        if(gameFile_isNotEmpty(game_number)){
                            if(randf() < 0.1){
                                switch(game_number){
                                    case CH_GAME_SELECT_SAVEFILE_0_BED://L802C52B8
                                        sfxsource_play(SFX_31_BANJO_OHHWAAOOO, 28000);
                                        gcsfx_play(SFX_135_CARTOONY_SPRING);
                                        timedFunc_set_2(0.4f, (GenFunction_2)sfxsource_play, SFX_13A_GLASS_BREAKING_7, 0x7fff);
                                        timedFunc_set_2(0.9f, (GenFunction_2)sfxsource_play, SFX_150_PORCELAIN_CRASH, 0x7fff);
                                        timedFunc_set_2(1.0f, (GenFunction_2)sfxsource_play, SFX_151_CAT_MEOW, 0x7fff);
                                        break;
                                    case CH_GAME_SELECT_SAVEFILE_1_GAMING_CHAIR://L802C5320
                                        timedFunc_set_2(0.4f, (GenFunction_2)sfxsource_play, SFX_31_BANJO_OHHWAAOOO, 28000);
                                        timedFunc_set_2(0.2f, (GenFunction_2)sfxsource_play, SFX_E_SHOCKSPRING_BOING, 28000);
                                        gcsfx_play(SFX_2D_KABOING);
                                        break;
                                    case CH_GAME_SELECT_SAVEFILE_2_KITCHEN://L802C5364
                                        timedFunc_set_2(0.15f, (GenFunction_2)sfxsource_play, SFX_32_BANJO_EGHEE, 28000);
                                        sfxsource_play(SFX_3F6_RUBBING, 28000);
                                        gcsfx_play(SFX_8F_SNOWBALL_FLYING);
                                        break;
                                }//L802C5394
                                subaddie_set_state(this, 4);
                                levelSpecificFlags_set(game_number + 0x35, 1);
                            }
                            else{//L802C53B4
                                sfxsource_playHighPriority(SFX_3EA_BANJO_GUH_HUH);
                                subaddie_set_state(this, 3);
                            }
                        }else{//L802C53D0
                            sfxsource_play(SFX_4F_BANJO_WAHOO, 28000);
                            subaddie_set_state(this, 3);
                        }//L802C53E8
                        if(game_number == CH_GAME_SELECT_SAVEFILE_0_BED)
                            func_802C75A0(this, 2);

                        if(game_number == CH_GAME_SELECT_SAVEFILE_1_GAMING_CHAIR)
                            comusic_8025AB44(COMUSIC_73_GAMEBOY, 0, 4000);
                        
                        func_8025A58C(0, 0x1f4);
                        actor_playAnimationOnce(this);
                    }
                    else{//L802C5434
                        if((0.7 < ((0.0f <= sp54[0]) ? sp54[0] : -sp54[0])) && isFileMoving == 0
                        ){
                            previous_game_number = gameNumber;
                            if(sp54[0] < 0.0f){
                                isFileMoving = 1;
                                switch(gameNumber){
                                    case CH_GAME_SELECT_SAVEFILE_0_BED:
                                        isFileMoving = 0;
                                        break;
                                    case CH_GAME_SELECT_SAVEFILE_1_GAMING_CHAIR:
                                        gameNumber = CH_GAME_SELECT_SAVEFILE_2_KITCHEN;
                                        break;
                                    case CH_GAME_SELECT_SAVEFILE_2_KITCHEN:
                                        gameNumber = CH_GAME_SELECT_SAVEFILE_0_BED;
                                        break;
                                }
                            }
                            else{//L802C54D4
                                isFileMoving = 1;
                                switch(gameNumber){
                                    case CH_GAME_SELECT_SAVEFILE_0_BED:
                                        gameNumber = CH_GAME_SELECT_SAVEFILE_2_KITCHEN;
                                        break;
                                    case CH_GAME_SELECT_SAVEFILE_1_GAMING_CHAIR:
                                        isFileMoving = 0;
                                        break;
                                    case CH_GAME_SELECT_SAVEFILE_2_KITCHEN:
                                        gameNumber = CH_GAME_SELECT_SAVEFILE_1_GAMING_CHAIR;
                                        break;
                                }
                            }//L802C550C
                            if(isFileMoving){
                                previousGameNumber = previous_game_number;
                                gameSelectCameraDelta = 0.0f;
                            }
                        }else{//L802C5530
                            if(((0.0f <= sp54[0]) ? sp54[0] : -sp54[0]) < 0.3){
                                isFileMoving = 0;
                            }
                        }
                    }//L802C556C
                    if(isTopTextNotFinishedDisplaying == 0){
                        cycleInstructionsTimer += delta_time;
                        if(20.0 < cycleInstructionsTimer){
                            func_8031877C(chGameSelectTopZoombox);
                            CALL_CANCELLABLE_EVENT(LocalizeFileSelectPrompt, 0, chGameSelectTopZoombox) {
                                gczoombox_setStrings(chGameSelectTopZoombox, 2, (char **)&selectInstructions);
                            }
                            cycleInstructionsTimer = 0.0f;
                        }
                    }
                    break;
                case 6://L802C55E8
                    break;
            }
        }//L802C55E8
        gameSelectCameraDelta += delta_time;
        sp48 = func_803097A0();
        if(this->marker->unk14_21){
            for(i = 0; i < 3; i++){
                vec3fArray_get_vec3f(sp48, i+5, sp34);
                ml_vec3f_copy(INITIAL_CAMERA_POSITIONS[i], sp34);
            }
        }
        ncStaticCamera_setPositionAndTarget(
            calculateGameSelectCameraPosition(INITIAL_CAMERA_POSITIONS[previousGameNumber], INITIAL_CAMERA_POSITIONS[gameNumber], gameSelectCameraDelta), 
            calculateGameSelectCameraPosition(INITIAL_CAMERA_TARGETS[previousGameNumber], INITIAL_CAMERA_TARGETS[gameNumber], gameSelectCameraDelta)
        );
        if(this->marker->unk14_21) {
            osViBlack(0);
        }
    }//L802C5734
}

void gameSelect_initAndUpdate(Actor * this){
    int i = code94620_func_8031B5B0();
    selectInstructions.controlInstruction = D_80365DF4[i];
    selectInstructions.eraseInstruction = D_80365DF8[i];

    if(!this->initialized){
        gameFile_8033CE40();
         if(chGameSelectBottomZoombox == NULL){
            chGameSelectBottomZoombox = gczoombox_new(0xA0, ZOOMBOX_SPRITE_C_BANJO_2, 2, 0, NULL);
            gczoombox_open(chGameSelectBottomZoombox);
            gczoombox_func_803184C8(chGameSelectBottomZoombox, 30.0f, 5, 2, 0.4f, 0, 0);
        }//L802C57FC

        if(chGameSelectTopZoombox == NULL){
            chGameSelectTopZoombox = gczoombox_new(0xA, ZOOMBOX_SPRITE_D_KAZOOIE_1, 2, 1, topZoomboxCallback);
            CALL_CANCELLABLE_EVENT(LocalizeFileSelectPrompt, 0, chGameSelectTopZoombox) {
                gczoombox_setStrings(chGameSelectTopZoombox, 2, (char **)&selectInstructions);
            }
            gczoombox_open(chGameSelectTopZoombox);
            gczoombox_maximize(chGameSelectTopZoombox);
        }//L802C5860

        marker_setFreeMethod(this->marker, gameSelect_free);
        isFileMoving = 0;
        debugScoreStates();
        clearScoreStates();
        previousGameNumber = CH_GAME_SELECT_SAVEFILE_0_BED;
        gameNumber = CH_GAME_SELECT_SAVEFILE_0_BED;
        cameraPositions[1][0] = INITIAL_CAMERA_POSITIONS[0][0];
        cameraPositions[1][1] = INITIAL_CAMERA_POSITIONS[0][1];
        cameraPositions[1][2] = INITIAL_CAMERA_POSITIONS[0][2];

        cameraPositions[0][0] = INITIAL_CAMERA_TARGETS[0][0];
        cameraPositions[0][1] = INITIAL_CAMERA_TARGETS[0][1];
        cameraPositions[0][2] = INITIAL_CAMERA_TARGETS[0][2];
        gameSelectCameraDelta = 0.75f;
        cycleInstructionsTimer = func_8038AAB0() ? 20.0 : 0.0;
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
    s32 is_map_game_over = gsworld_getMap() == MAP_83_CS_GAME_OVER_MACHINE_ROOM;
    s32 is_level_id_valid = (0 < sp1C && sp1C < 0xd);
    // [port] Permadeath erases the save on death; the game over path routes back through
    // here, and saving would write the still-live game state over the erased slot.
    if(!EventSystem_Should(VB_SAVE_AND_EXIT, true)){
        return;
    }
    if( is_level_id_valid || is_map_game_over)
    {
        if(gameNumber != -1 && !func_802E4A08() && gsworld_getMap() != MAP_91_FILE_SELECT){
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
