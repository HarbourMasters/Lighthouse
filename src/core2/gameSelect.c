#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#include "core2/modelRender.h"

#include "core2/gc/zoombox.h"
#include "core2/quiz_storage.h"

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
extern void gsworld_setEnableUpdate(s32);
extern void controller_getJoystick(s32, f32*);

extern char *gcpausemenu_TimeToA(int);
extern Vec3fArray *func_803097A0(void);

/* .data */
f32 D_80365DD0[3][3] = {
    {-320.0f, 340.0f, 350.0f},
    {110.0f, 340.0f, 110.0f},
    {-413.333313f, 353.333313f, -234.305511f}
};
u8 *D_80365DF4[] = {
    "USE THE CONTROL STICK TO SELECT A GAME.",
    "S" "\x62" "LECTIONNEZ UN FICHIER _ L'AIDE DU STICK.",
    "W[HLE MIT DEM 3D-STICK",
};
u8 *D_80365DF8[] = {
    "PRESS A TO PLAY THE GAME OR Z TO ERASE IT!",
    "APPUYEZ SUR A POUR JOUER OU SUR Z POUR EFFACER!",
    "EIN SPIEL AUS. DR]CKE A, UM ZU SPIELEN, ODER DEN Z-TRIGGER, UM DEN SPIELSTAND ZU L\\SCHEN!",
};
u8 *D_80365DFC[] = {
    "ARE YOU SURE? PRESS A TO CONFIRM, OR B TO CANCEL.",
    "dTES-VOUS SiR? APPUYEZ SUR A POUR CONFIRMER OU SUR B POUR ANNULER.",
    "SICHER? DR]CKE A, UM ZU BEST[TIGEN, ODER B, UM ZU WIDERRUFEN.",
};
s32 D_80365E00 = -1;
f32 D_80365E04[3][3] = {
    {-435.0f,      278.0f,  -159.0f},
    { 444.635437f, 216.0f,  -356.591675f},
    {  55.0f,      191.822906f, -905.96875f}
};

ActorAnimationInfo banjoGameboyAnimations[] = {
    {0x000, 0.0f},
    {0x24D, 9e+09f},
    {0x24D, 2.0f},  
    {0x24E, 1.0f},
    {0x24F, 0.6f},  
    {0x24D, 2.0f}
};
ActorInfo D_80365E58 = { 0xE4, 0x195, 0x532, 0x1, banjoGameboyAnimations, gameSelect_initAndUpdate, actor_update_func_80326224, gameSelect_zoomboxDraw, 0, 0, 0.0f, 0};

ActorAnimationInfo D_80365E7C[] = {
    {0x000, 0.0f}, 
    {0x250, 9e+09f},
    {0x250, 4.5f}, 
    {0x251, 1.0f},
    {0x252, 0.67f}, 
    {0x250, 4.5f},
};
ActorInfo D_80365EAC = { 0xE5, 0x196, 0x532, 0x1, D_80365E7C, gameSelect_update, actor_update_func_80326224, gameSelect_draw, 0, 0, 0.0f, 0};

ActorAnimationInfo D_80365ED0[] = {
    {0x000, 0.0f},
    {0x24A, 9e+09f},  
    {0x24A, 1.0f},
    {0x24B, 1.0f},  
    {0x24C, 1.0f},
    {0x24A, 1.0f}
};
ActorInfo D_80365F00 = { 0xE6, 0x197, 0x532, 0x1, D_80365ED0, gameSelect_update, actor_update_func_80326224, gameSelect_draw, 0, 0, 0.0f, 0};


/* .bss */
s32 mm_hut_smash_count;
u32 CH_TREASUREHUNT_PUZZLE_CURRENT_STEP;
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
    u8 *unk0;
    u8 *unk4;
} selectInstructions;
s32 D_8037DCE8;
s32 D_8037DCEC;
GcZoombox *chGameSelectTopZoombox;
GcZoombox *chGameSelectBottomZoombox;
f32 cameraPositions[2][3];
f32 D_8037DD10[2][3];
s32 D_8037DD28;
s32 D_8037DD2C;
f32 D_8037DD30;
f32 D_8037DD34;



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
    if(sp1C == D_80365E00){
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
        D_8037DD2C = 0;
}

void *calculateGameSelectCameraPosition(f32 arg0[3], f32 arg1[3], f32 arg2) {
    f32 phi_f12;
    f32 sp40[3];
    s32 i;
    static bool dummy_index;
    static f32 D_8037DD3C;
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
    D_8037DD3C = 1.0 + (9.0f / gu_sqrtf(phi_f12));
    sin_bounciness_half_pi = sinf(D_8037DD3C*1.5707963267948966);
    for(i = 0; i < 3; i++){
        D_8037DD10[dummy_index][i] = arg0[i] + ((arg1[i] - arg0[i])*sinf((((arg2 / 0.75) * 3.1415926535897931) / 2) * D_8037DD3C)) / sin_bounciness_half_pi;
        cameraPositions[dummy_index][i] += (D_8037DD10[dummy_index][i] - cameraPositions[dummy_index][i]) / 5.0;

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
    static u8 *sEmptyLabel[]  = { ": EMPTY",  ": VIDE",   ": LEER" };
    s32 lang = code94620_func_8031B5B0();

    debugScoreStates();
    D_80365E00 = gamenum;
    clearScoreStates();
    if(gameFile_isNotEmpty(gamenum)){
        gameFile_load(gamenum);
        D_8037DCCE[gamenum] = (itemscore_timeScores_get(LEVEL_6_LAIR)) ? 1 : 0;

        strcpy(upperTextLine, "");
        strcat(upperTextLine, sGamePrefix[lang]);
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
        strcat(upperTextLine, sTimeLabel[lang]);
        strcat(upperTextLine, gcpausemenu_TimeToA(itemscore_timeScores_getTotal()));
        strcat(upperTextLine, ",");
        strcat(upperTextLine, "");

        strcpy(lowerTextLine, "");
        strIToA(lowerTextLine, jiggyscore_total());
        strcat(lowerTextLine, sJigsawLabel[lang]);
        if(jiggyscore_total() != 1){
            strcat(lowerTextLine, sJigsawPlural[lang]);
        }
        strcat(lowerTextLine, ", ");
        strIToA(lowerTextLine, itemscore_noteScores_getTotal());
        strcat(lowerTextLine, sNoteLabel[lang]);
        if(itemscore_noteScores_getTotal() != 1){
            strcat(lowerTextLine, "S");
        }
        strcat(lowerTextLine, ".");
        strcat(lowerTextLine, "");
    }//L802C49AC
    else{
        D_8037DCCE[gamenum] = 0;
        strcpy(upperTextLine, "");
        strcat(upperTextLine, sGamePrefix[lang]);
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
        strcat(upperTextLine, sEmptyLabel[lang]);
        strcpy(lowerTextLine, "");
    }//L802C4A68

    // [port] JP rebuilds these lines in its own layout
    CALL_EVENT(OnFileSelectInfoBuild, gamenum, (char *) upperTextLine, (char *) lowerTextLine);
    sp20[0] = upperTextLine;\
    sp20[1] = lowerTextLine;
    func_8031877C(chGameSelectBottomZoombox);
    gczoombox_setStrings(chGameSelectBottomZoombox, 2, (char **)sp20);
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
        // gameFile_8033CFD4(i); Not needed with new Save System
    }

    if(D_8037DD28){
        func_802F9D38(D_8037DD28);
        D_8037DD28 = 0;
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
    s32 sp5C[6];
    f32 sp54[2];
    f32 sp50;
    int i; //sp4C
    Vec3fArray *sp48;
    f32 sp44;
    s32 tmp_a2_2;
    f32 sp34[3];

    sp84 = this->marker->id - 0xe4;
    gSelectedGameNum = sp84;
    sp80 = (sp84 == D_80365E00);
    sp50 = time_getDelta();
    if(chGameSelectBottomZoombox == NULL)
        return;

    // [port] Let the localization layer rebuild the info zoombox live when the
    // language changed (it gates on the language generation + the selected slot).
    CALL_EVENT(OnFileSelectLanguageRefresh, sp84, sp80);

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
        controller_getJoystick(0, sp54);
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
                        D_8037DD28 = func_802F9AA8(SFX_12B_BOILING_AND_BUBBLING);
                        func_802F9F80(D_8037DD28, 0.5f, 9000000000.0f, 0.5f);
                        func_802F9DB8(D_8037DD28, 0.9f, 0.9f, 0.0f);
                        func_802FA060(D_8037DD28, 15000, 15000, 0.0f);
                    }
                    else{
                        if(D_8037DD28){
                            func_802F9D38(D_8037DD28);
                            D_8037DD28 = 0;
                        }
                    }
                    setGameInformationZoombox(sp84);
                    subaddie_set_state(this, 2);
                    break;
                case 5://L802C5040
                    if(D_8037DD2C == 0 && 
                        (sp5C[FACE_BUTTON(BUTTON_A)] == 1 || sp5C[FACE_BUTTON(BUTTON_B)] == 1)
                    ){
                        if(sp5C[FACE_BUTTON(BUTTON_A)] == 1){
                            eraseGame(sp84);
                            coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 22000);
                        }
                        subaddie_set_state(this, 2);
                        func_8031877C(chGameSelectTopZoombox);
                        CALL_CANCELLABLE_EVENT(LocalizeFileSelectPrompt, 0, chGameSelectTopZoombox) {
                            gczoombox_setStrings(chGameSelectTopZoombox, 2, (char **)&selectInstructions);
                        }
                        D_8037DD34 = 0.0f;
                    }
                    break;
                case 3://L802C50C8
                case 4://L802C50C8
                    if(anctrl_isStopped(this->anctrl)){
                        chBottlesBonus_resetCompleted();
                        gameFile_load(gSelectedGameNum);
                        port_syncBottlesBonusIndex();
                        if(!gameFile_isNotEmpty(sp84)){
                            // [port] BB romhacks can override the new-game boot map
                            s32 newGameMap = port_getRomhackNewGameMap();
                            if (newGameMap < 0) {
                                newGameMap = MAP_85_CS_SPIRAL_MOUNTAIN_3;
                            }
                            timedFunc_set_3(0.0f, (GenFunction_3)transitionToMap, newGameMap, 0, 1);
                            {
                                if (port_getRomhackKnowAllMoves() >= 0) {
                                    ability_setAllLearned(-1);
                                    ability_setAllUsed(-1);
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
                            timedFunc_set_1(sp44, (GenFunction_1)gsworld_setEnableUpdate, 1);
                        }//L802C51B8
                        this->state = 6;
                    }
                    break;
                case 2://L802C51CC
                    if(sp74[0] == 1){
                        if(gameFile_isNotEmpty(sp84)){
                            func_8031877C(chGameSelectTopZoombox);
                            CALL_CANCELLABLE_EVENT(LocalizeFileSelectPrompt, 1, chGameSelectTopZoombox) {
                                func_803183A4(chGameSelectTopZoombox, D_80365DFC[code94620_func_8031B5B0()]);
                            }
                            D_8037DD2C = 1;
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
                                        gcsfx_playAtSampleRate(SFX_135_CARTOONY_SPRING);
                                        timedFunc_set_2(0.4f, (GenFunction_2)sfxsource_play, SFX_13A_GLASS_BREAKING_7, 0x7fff);
                                        timedFunc_set_2(0.9f, (GenFunction_2)sfxsource_play, SFX_150_PORCELAIN_CRASH, 0x7fff);
                                        timedFunc_set_2(1.0f, (GenFunction_2)sfxsource_play, SFX_151_CAT_MEOW, 0x7fff);
                                        break;
                                    case 1://L802C5320
                                        timedFunc_set_2(0.4f, (GenFunction_2)sfxsource_play, SFX_31_BANJO_OHHWAAOOO, 28000);
                                        timedFunc_set_2(0.2f, (GenFunction_2)sfxsource_play, SFX_E_SHOCKSPRING_BOING, 28000);
                                        gcsfx_playAtSampleRate(SFX_2D_KABOING);
                                        break;
                                    case 2://L802C5364
                                        timedFunc_set_2(0.15f, (GenFunction_2)sfxsource_play, SFX_32_BANJO_EGHEE, 28000);
                                        sfxsource_play(SFX_3F6_RUBBING, 28000);
                                        gcsfx_playAtSampleRate(SFX_8F_SNOWBALL_FLYING);
                                        break;
                                }//L802C5394
                                subaddie_set_state(this, 4);
                                levelSpecificFlags_set(sp84 + 0x35, 1);
                            }
                            else{//L802C53B4
                                sfxsource_playHighPriority(SFX_3EA_BANJO_GUH_HUH);
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
                        if((0.7 < ((0.0f <= sp54[0]) ? sp54[0] : -sp54[0])) && D_8037DCEC == 0
                        ){
                            tmp_a2_2 = D_80365E00;
                            if(sp54[0] < 0.0f){
                                D_8037DCEC = 1;
                                switch(D_80365E00){
                                    case 0:
                                        D_8037DCEC = 0;
                                        break;
                                    case 1:
                                        D_80365E00 = 2;
                                        break;
                                    case 2:
                                        D_80365E00 = 0;
                                        break;
                                }
                            }
                            else{//L802C54D4
                                D_8037DCEC = 1;
                                switch(D_80365E00){
                                    case 0:
                                        D_80365E00 = 2;
                                        break;
                                    case 1:
                                        D_8037DCEC = 0;
                                        break;
                                    case 2:
                                        D_80365E00 = 1;
                                        break;
                                }
                            }//L802C550C
                            if(D_8037DCEC){
                                D_8037DCE8 = tmp_a2_2;
                                D_8037DD30 = 0.0f;
                            }
                        }else{//L802C5530
                            if(((0.0f <= sp54[0]) ? sp54[0] : -sp54[0]) < 0.3){
                                D_8037DCEC = 0;
                            }
                        }
                    }//L802C556C
                    if(D_8037DD2C == 0){
                        D_8037DD34 += sp50;
                        if(20.0 < D_8037DD34){
                            func_8031877C(chGameSelectTopZoombox);
                            CALL_CANCELLABLE_EVENT(LocalizeFileSelectPrompt, 0, chGameSelectTopZoombox) {
                                gczoombox_setStrings(chGameSelectTopZoombox, 2, (char **)&selectInstructions);
                            }
                            D_8037DD34 = 0.0f;
                        }
                    }
                    break;
                case 6://L802C55E8
                    break;
            }
        }//L802C55E8
        D_8037DD30 += sp50;
        sp48 = func_803097A0();
        if(this->marker->unk14_21){
            for(i = 0; i < 3; i++){
                vec3fArray_get_vec3f(sp48, i+5, sp34);
                ml_vec3f_copy(D_80365DD0[i], sp34);
            }
        }
        ncStaticCamera_setPositionAndTarget(
            calculateGameSelectCameraPosition(D_80365DD0[D_8037DCE8], D_80365DD0[D_80365E00], D_8037DD30), 
            calculateGameSelectCameraPosition(D_80365E04[D_8037DCE8], D_80365E04[D_80365E00], D_8037DD30)
        );
        if(this->marker->unk14_21) {
            osViBlack(0);
            port_setViBlack(0);
        }
    }//L802C5734
}

void gameSelect_initAndUpdate(Actor * this){
    int i = code94620_func_8031B5B0();
    selectInstructions.unk0 = D_80365DF4[i];
    selectInstructions.unk4 = D_80365DF8[i];

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
        D_8037DCEC = 0;
        debugScoreStates();
        clearScoreStates();
        D_8037DCE8 = 0;
        D_80365E00 = 0;
        cameraPositions[1][0] = D_80365DD0[0][0];
        cameraPositions[1][1] = D_80365DD0[0][1];
        cameraPositions[1][2] = D_80365DD0[0][2];

        cameraPositions[0][0] = D_80365E04[0][0];
        cameraPositions[0][1] = D_80365E04[0][1];
        cameraPositions[0][2] = D_80365E04[0][2];
        D_8037DD30 = 0.75f;
        D_8037DD34 = func_8038AAB0() ? 20.0 : 0.0;
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
    s32 t6 = gsworld_getMap() == MAP_83_CS_GAME_OVER_MACHINE_ROOM;
    s32 a1 = (0 < sp1C && sp1C < 0xd);
    if( a1 || t6)
    {
        if(D_80365E00 != -1 && !func_802E4A08() && gsworld_getMap() != MAP_91_FILE_SELECT){
            gameFile_save(D_80365E00);
            gameFile_8033CFD4(D_80365E00);
        }
    }
}

s32 gameSelect_getGameNumber(void){
    return D_80365E00;
}

void gameSelect_setGameNumber(s32 arg0){
    D_80365E00 = arg0;
}

void gameSelect_resetGameNumber(void){
    D_80365E00 = -1;
}
