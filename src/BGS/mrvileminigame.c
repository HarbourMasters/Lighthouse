// BanjoDecomp: BGS/mrvileminigame.c
#include <ultra64.h>
#include "functions.h"
#include "bk_math.h"
#include "variables.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Network/Anchor/VileSync.h"

extern f32 *chMrVile_getPostion(ActorMarker *);
extern void bundle_setRandomVelocity(f32);
extern void func_802FDCB8(s32);

struct vilegame_piece{
    enum chvilegame_piece_type_e type;
    f32 position[3];
    ActorMarker *marker; //yumblie ptr;
};

typedef struct {
    u8 current_type;
    // u8 pad1[3];
    bk_vector(struct vilegame_piece) *game_pieces;
    BKModelBin *grumblie_model_bin;
    u8 dialogIndex;
    u8 mode;
    u8 player_score;
    u8 vile_score;
    f32 type_change_timer;
    ActorMarker *vile_marker;
}chMrVileMinigameActor;

void chMrVileMinigame_setState(Actor *this, s32 next_state);
void chMrVileMinigame_update(Actor *this);

/* .data */
ActorInfo D_80390960 = {
    MARKER_C6_VILE_GAME_CTRL, ACTOR_138_VILE_GAME_CTRL, 0,
    0, NULL,
    chMrVileMinigame_update, NULL, func_80325340,
    0, 0, 0.0f, 0
};

// Vile Wins
enum asset_e chMrVileMinigamePlayerLosesDialog[] = {
    0,
    VER_SELECT(ASSET_C66_DIALOG_MR_VILE_WINS_ROUND_1, 0x9A9, 0, 0),
    VER_SELECT(ASSET_C68_DIALOG_MR_VILE_WINS_ROUND_2, 0x9AB, 0, 0),
    VER_SELECT(ASSET_C6A_DIALOG_MR_VILE_WINS_ROUND_3, 0x9AD, 0, 0),
    VER_SELECT(ASSET_C92_DIALOG_MR_VILE_WINS_EXTRA_CHALLENGE_2, 0x9D5, 0, 0),
    VER_SELECT(ASSET_C93_DIALOG_MR_VILE_WINS_EXTRA_CHALLENGE_3, 0x9D6, 0, 0),
    VER_SELECT(ASSET_C94_DIALOG_MR_VILE_WINS_EXTRA_CHALLENGE_4, 0x9D7, 0, 0),
    0
};

// Player Wins
enum asset_e chMrVileMinigamePlayerWinsDialog[] = {
    0,
    VER_SELECT(ASSET_C67_DIALOG_MR_VILE_ROUND_2_START, 0x9AA, 0, 0),
    VER_SELECT(ASSET_C69_DIALOG_MR_VILE_ROUND_3_START, 0x9AC, 0, 0),
    0,
    VER_SELECT(ASSET_C95_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_1, 0x9D8, 0, 0),
    VER_SELECT(ASSET_C96_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_2, 0x9D9, 0, 0),
    VER_SELECT(ASSET_C97_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_3, 0x9DA, 0, 0)
};

// Player Wins Rematch
enum asset_e chMrVileMinigamePlayerWinsRematchDialog[] = {
    0,
    VER_SELECT(ASSET_C6E_DIALOG_MR_VILE_LOSE_ROUND_2_REMATCH, 0x9B1, 0, 0),
    VER_SELECT(ASSET_C6F_DIALOG_MR_VILE_LOSE_ROUND_3_REMATCH, 0x9B2, 0, 0),
    0,
    VER_SELECT(ASSET_C95_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_1, 0x9D8, 0, 0),
    VER_SELECT(ASSET_C96_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_2, 0x9D9, 0, 0),
    VER_SELECT(ASSET_C97_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_3, 0x9DA, 0, 0)
};

enum asset_e chMrVileMinigamePlayerDeclinesDialog[] = {
    VER_SELECT(ASSET_C65_DIALOG_MR_VILE_PLAYER_DECLINES, 0x9A8, 0, 0),
    VER_SELECT(ASSET_C65_DIALOG_MR_VILE_PLAYER_DECLINES, 0x9A8, 0, 0),
    VER_SELECT(ASSET_C65_DIALOG_MR_VILE_PLAYER_DECLINES, 0x9A8, 0, 0),
    VER_SELECT(ASSET_C8F_DIALOG_MR_VILE_PLAYER_DECLINES_EXTRA_CHALLENGE, 0x9D2, 0, 0),
    0,
    0,
    0
};

enum asset_e chMrVileMinigameRound1AndChallengeDialog[] = {
    VER_SELECT(ASSET_C64_DIALOG_MR_VILE_ROUND_1_START, 0x9A7, 0, 0),
    0,
    0,
    VER_SELECT(ASSET_C8E_DIALOG_MR_VILE_WINS_EXTRA_CHALLENGE_1, 0x9D1, 0, 0),
    0,
    0,
    0,
    0
};

enum asset_e chMrVileMinigameRematchDialog[] = {
    VER_SELECT(ASSET_C6D_DIALOG_MR_VILE_LOSE_ROUND_1_REMATCH, 0x9B0, 0, 0),
    VER_SELECT(ASSET_C70_DIALOG_MR_VILE_WIN_ROUND_2_REMATCH, 0x9B3, 0, 0),
    VER_SELECT(ASSET_C71_DIALOG_MR_VILE_WIN_ROUND_3_REMATCH, 0x9B4, 0, 0),
    VER_SELECT(ASSET_C8E_DIALOG_MR_VILE_WINS_EXTRA_CHALLENGE_1, 0x9D1, 0, 0),
    0,
    0,
    0,
    0
};

enum MrVileMinigameState {
    MR_VILE_MINIGAME_STATE_0_NO_INIT = 0,
    MR_VILE_MINIGAME_STATE_1_IDLE,
    MR_VILE_MINIGAME_STATE_2_YES_OR_NO,
    MR_VILE_MINIGAME_STATE_3_DECLINE_MINIGAME,
    MR_VILE_MINIGAME_STATE_4_ROUND_INTRO_EXPLANATION,
    MR_VILE_MINIGAME_STATE_5_PLAYING_MINIGAME,
    MR_VILE_MINIGAME_STATE_6_VILE_WINS,
    MR_VILE_MINIGAME_STATE_7_ATTACK_PLAYER,
    MR_VILE_MINIGAME_STATE_8_PLAYER_WINS,
    MR_VILE_MINIGAME_STATE_9_DROP_JIGGY,
    MR_VILE_MINIGAME_STATE_A_DROP_EXTRA_LIVES
};

enum MrVileMinigameMode {
    MR_VILE_MINIGAME_MODE_0_ROUND_1 = 0,
    MR_VILE_MINIGAME_MODE_1_ROUND_2,
    MR_VILE_MINIGAME_MODE_2_ROUND_3,
    MR_VILE_MINIGAME_MODE_3_CHALLENGE_1_AND_FF,
    MR_VILE_MINIGAME_MODE_4_CHALLENGE_2,
    MR_VILE_MINIGAME_MODE_4_CHALLENGE_3
};

#define MR_VILE_MINIGAME_TIMER      VER_SELECT(3600, 3000, 0, 0)
#define TYPE_CHANGE_TIMER_REGULAR  10.0f
#define TYPE_CHANGE_TIMER_FF        5.0f
#define EXTRA_LIFE_SCORE              35
#define MAX_DISTANCE_FROM_PIECE    65.25

/* .code */
bool chMrVileMinigame_didPlayerConsumePiece(f32 arg0[3]) {
    if (player_movementGroup() != BSGROUP_7_CROC_ATTACK) {
        return false;
    }
    func_8028E9C4(2, arg0);
    return true;
}

void chMrVileMinigame_spawnJiggy(Actor *this, s32 arg1) {
    chMrVileMinigameActor *local;
    Actor *vile;
    f32 sp94[3];
    f32 sp88[3];
    f32 sp7C[3];
    s32 var_s0;
    s32 var_v0;

    local = (chMrVileMinigameActor *)&this->local;
    vile = marker_getActor(local->vile_marker);
    if (arg1 != 0) {
        if (LENGTH_VEC3F(this->position) < 800.0f) {
            TUPLE_ASSIGN(sp7C, 0.0f, 150.0f, 300.0f)
            ml_vec3f_yaw_rotate_copy(sp7C, sp7C, vile->yaw);
            TUPLE_ADD_COPY(sp94, vile->position, sp7C)
            bundle_setYaw(vile->yaw);
            jiggy_spawn(JIGGY_28_BGS_MR_VILE, sp94);
        } else {
            sp7C[0] = 0.0f - vile->position[0];
            sp7C[1] = 0.0f;
            sp7C[2] = 0.0f - vile->position[2];
            ml_vec3f_set_length(sp7C, 150.0f);
            sp7C[1] = 75.0f;
            sp94[0] = vile->position[0] + sp7C[0];
            sp94[1] = vile->position[1] + sp7C[1];
            sp94[2] = vile->position[2] + sp7C[2];
            bundle_setYaw(func_8025715C(sp7C[0], sp7C[2]));
            jiggy_spawn(JIGGY_28_BGS_MR_VILE, sp94);
        }
    }
    for(var_s0 = 2; var_s0 != 0x3C; var_s0++){
        var_v0 = ((var_s0 & 1)) ?  -(var_s0 / 2) * 0xA : (var_s0 / 2) * 0xA;
        sp7C[0] = 0.0f;
        sp7C[1] = 600.0f;
        sp7C[2] = 1200.0f;
        ml_vec3f_yaw_rotate_copy(sp7C, sp7C, vile->yaw + var_v0);

        sp94[0] = vile->position[0] + sp7C[0];
        sp94[1] = vile->position[1] + sp7C[1];
        sp94[2] = vile->position[2] + sp7C[2];


        sp88[0] = -30.0f;
        sp88[1] = vile->yaw + var_v0;
        sp88[2] = 0.0f;

        sp7C[2] = 0.0f;
        sp7C[1] = sp94[1];
        sp7C[0] = 0.0f;
        if(ml_vec3f_distance(sp94, sp7C) <= 1000.0f){
            break;
        }
    }
    ncStaticCamera_setPositionAndRotation(sp94, sp88);
}

void chMrVileMinigame_vileOffersMinigame(ActorMarker *marker, enum asset_e text_id, s32 arg2){
    Actor *this;

    this = marker_getActor(marker);
    if(arg2 == 1){
        chMrVileMinigame_setState(this, 4);
    }
    else{
        chMrVileMinigame_setState(this, 3);
    }
}

void chMrVileMinigame_setStateIdleAfterDeclined(ActorMarker *marker, enum asset_e text_id, s32 arg2){
    Actor *this;

    this = marker_getActor(marker);
    chMrVileMinigame_setState(this, 1);
}

void chMrVileMinigame_setStatePlayingMinigame(ActorMarker *marker, enum asset_e text_id, s32 arg2){
    Actor *this;

    this = marker_getActor(marker);
    chMrVileMinigame_setState(this, 5);
}

void chMrVileMinigame_vileAttacksPlayer(ActorMarker *marker, enum asset_e text_id, s32 arg2) {
    Actor *this;
    chMrVileMinigameActor *local;

    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;
    timed_exitStaticCamera(0.0f);
    func_80324E38(0.0f, 0);
    local->dialogIndex--;
    chMrVileMinigame_setState(this, 7);
}

void chMrVileMinigame_vileWinsRound(ActorMarker *marker) {
    Actor *this;
    chMrVileMinigameActor *local;

    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;
    chMrVileMinigame_spawnJiggy(this, 0);
    gcdialog_showDialog(chMrVileMinigamePlayerLosesDialog[local->dialogIndex], 0xF, chMrVile_getPostion(local->vile_marker), this->marker, chMrVileMinigame_vileAttacksPlayer, NULL);
}

void chMrVileMinigame_startNextRound(ActorMarker *marker, enum asset_e text_id, s32 arg2){
    Actor *this;

    this = marker_getActor(marker);
    timed_exitStaticCamera(0.0f);
    func_80324E38(0.0f, 0);
    chMrVileMinigame_setState(this, 5);
}

void chMrVileMinigame_playerWinsRound(ActorMarker *marker) {
    Actor *this;
    chMrVileMinigameActor *local;

    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;
    chMrVileMinigame_spawnJiggy(this, 0);
    if (local->dialogIndex == local->mode) {
        gcdialog_showDialog(chMrVileMinigamePlayerWinsDialog[local->dialogIndex], 0xF, chMrVile_getPostion(local->vile_marker), this->marker, chMrVileMinigame_startNextRound, NULL);
    } else {
        gcdialog_showDialog(chMrVileMinigamePlayerWinsRematchDialog[local->dialogIndex], 0xF, chMrVile_getPostion(local->vile_marker), this->marker, chMrVileMinigame_startNextRound, NULL);
    }
    func_80347A14(0);
}

void chMrVileMinigame_setStateIdleAfterDeclinedAfterJiggy(ActorMarker *marker, enum asset_e text_id, s32 arg2){
    Actor *this;

    this = marker_getActor(marker);
    timed_exitStaticCamera(0.0f);
    func_80324E38(0.0f, 0);
    chMrVileMinigame_setState(this, 1);
}

void chMrVileMinigame_vileDropsJiggy(ActorMarker *marker) {
    Actor *this;
    chMrVileMinigameActor *local;

    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;

    chMrVileMinigame_spawnJiggy(this, 1);
    gcdialog_showDialog(
        VER_SELECT(ASSET_C6B_DIALOG_MR_VILE_PLAYER_WINS, 0x9AE, 0, 0),
        0xF,
        chMrVile_getPostion(local->vile_marker),
        this->marker,
        chMrVileMinigame_setStateIdleAfterDeclinedAfterJiggy,
        NULL
    );
}

void chMrVileMinigame_setStateIdleAfterExtraLives(ActorMarker *marker, enum asset_e text_id, s32 arg2){
    Actor *this;
    chMrVileMinigameActor *local;

    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;
    timed_exitStaticCamera(0.0f);
    func_80324E38(0.0f, 0);
    local->dialogIndex = 3;
    chMrVileMinigame_setState(this, 1);
}

void chMrVileMinigame_vileDropsExtraLives(ActorMarker *marker) {
    Actor *vile;
    Actor *this;
    chMrVileMinigameActor *local;
    s32 i;
    s32 var_s2;

    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;
    chMrVileMinigame_spawnJiggy(this, 0);
    var_s2 = actorArray_actorCount(ACTOR_49_EXTRA_LIFE);
    vile = marker_getActor(local->vile_marker);
    if (var_s2 > 0) {
        coMusicPlayer_playMusic(COMUSIC_15_EXTRA_LIFE_COLLECTED, 0x7FF8);
    }
    for(i = 0; i < 3; i++){
        if (var_s2 < 3) {
            bundle_setYaw(vile->yaw + (f32) (i * 30));
            bundle_setRandomVelocity(2.0f);
            bundle_spawn_f32(BUNDLE_6_MM_HUT_EXTRA_LIFE, vile->position);
            var_s2 += 1;
        } else {
            item_inc(ITEM_16_LIFE);
        }
    }
    gcdialog_showDialog(
        VER_SELECT(ASSET_C98_DIALOG_MR_VILE_GIVES_PRIZE, 0x9DB, 0, 0),
        0xF,
        chMrVile_getPostion(local->vile_marker),
        this->marker,
        chMrVileMinigame_setStateIdleAfterExtraLives,
        NULL
    );
}


void func_8038A044(void){
    func_8025A58C(-1, 400);
}

void chMrVileMinigame_setState(Actor *this, s32 next_state) {
    chMrVileMinigameActor *local;

    local = (chMrVileMinigameActor *)&this->local;
    mapSpecificFlags_set(6, false);
    if (next_state == 1) {
        if (local->vile_marker != NULL) {
            chMrVile_setStateIdleWalking(local->vile_marker);
        }
    }
    if (next_state == MR_VILE_MINIGAME_STATE_2_YES_OR_NO) {
        chMrVile_setStateTalkToPlayer(local->vile_marker);
        if (local->dialogIndex == 3) {
            if (local->mode >= MR_VILE_MINIGAME_MODE_4_CHALLENGE_2) {
                gcdialog_showDialog(
                    VER_SELECT(ASSET_C91_DIALOG_MR_VILE_EXTRA_CHALLENGE_REMATCH_2, 0x9D4, 0, 0),
                    0xE,
                    chMrVile_getPostion(local->vile_marker),
                    this->marker,
                    chMrVileMinigame_vileOffersMinigame,
                    NULL
                );
            } else {
                gcdialog_showDialog(
                    (local->dialogIndex == local->mode) ?
                        VER_SELECT(ASSET_C8D_DIALOG_MR_VILE_EXTRA_CHALLENGE_INTRO, 0x9D0, 0, 0) :
                        VER_SELECT(ASSET_C90_DIALOG_MR_VILE_EXTRA_CHALLENGE_REMATCH_1, 0x9D3, 0, 0),
                    0xE,
                    chMrVile_getPostion(local->vile_marker),
                    this->marker,
                    chMrVileMinigame_vileOffersMinigame,
                    NULL
                );
            }
        } else {
            gcdialog_showDialog(
                (local->dialogIndex == local->mode) ?
                    VER_SELECT(ASSET_C63_DIALOG_MR_VILE_INTRO, 0x9A6, 0, 0) :
                    VER_SELECT(ASSET_C6C_DIALOG_MR_VILE_TRY_AGAIN, 0x9AF, 0, 0),
                0xE,
                chMrVile_getPostion(local->vile_marker),
                this->marker,
                chMrVileMinigame_vileOffersMinigame,
                NULL
            );
        }
    }
    if (next_state == MR_VILE_MINIGAME_STATE_3_DECLINE_MINIGAME) {
        gcdialog_showDialog(
            chMrVileMinigamePlayerDeclinesDialog[local->dialogIndex],
            4,
            chMrVile_getPostion(local->vile_marker),
            this->marker,
            chMrVileMinigame_setStateIdleAfterDeclined,
            NULL
        );
    }
    if (next_state == MR_VILE_MINIGAME_STATE_4_ROUND_INTRO_EXPLANATION) {
        if (local->dialogIndex == local->mode) {
            gcdialog_showDialog(
                chMrVileMinigameRound1AndChallengeDialog[local->dialogIndex],
                0xE | ((chMrVileMinigameRound1AndChallengeDialog[local->dialogIndex] == VER_SELECT(ASSET_C8E_DIALOG_MR_VILE_WINS_EXTRA_CHALLENGE_1, 0x9D1, 0, 0)) ? 1 : 0) | 0xE,
                chMrVile_getPostion(local->vile_marker),
                this->marker,
                chMrVileMinigame_setStatePlayingMinigame,
                NULL
            );
        } else {
            gcdialog_showDialog(chMrVileMinigameRematchDialog[local->dialogIndex], 0xF , chMrVile_getPostion(local->vile_marker), this->marker, chMrVileMinigame_setStatePlayingMinigame, NULL);
        }
        func_80347A14(0);
    }
    if (next_state == 5) {
        local->dialogIndex++;
        if (local->mode < local->dialogIndex) {
            local->mode = local->dialogIndex;
        }
        if (local->dialogIndex == 7) {
            chMrVile_setStateRunFromPlayer(local->vile_marker);
        } else {
            local->current_type = YUMBLIE;
            local->player_score = 0;
            local->vile_score = 0;
            if (volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME) != 0) {
                local->type_change_timer = 5.0f;
            } else {
                local->type_change_timer = 10.0f;
            }
            item_set(ITEM_0_HOURGLASS_TIMER, 3600-1);
            item_set(ITEM_6_HOURGLASS, true);
            mapSpecificFlags_set(6, true);
            chMrVile_setStatePlayMinigame(local->vile_marker);
            func_8025A58C(0, 4000);
            timedFunc_set_2(1.0f, (GenFunction_2)coMusicPlayer_playMusic, COMUSIC_55_BGS_MR_VILE, 28000);
        }
    }
    if (this->state == 5) {
        if (local->dialogIndex != 7) {
            item_set(ITEM_6_HOURGLASS, false);
            if ((next_state != 6) && (next_state != 8) && (next_state != 9)) {
                func_8038A044();
            }
        }
        chMrVile_setInitialIdleStill(local->vile_marker);
        func_80347A14(1);
    }
    if (next_state == 6) {
        chMrVile_setStateTalkToPlayer(local->vile_marker);
        func_80324E38(0.0f, 3);
        timedFunc_set_2(1.0f, (GenFunction_2)coMusicPlayer_playMusic, COMUSIC_3C_MINIGAME_LOSS, 28000);
        timedFunc_set_0(4.0f, (GenFunction_0)func_8038A044);
        timedFunc_set_1(4.0f, (GenFunction_1)chMrVileMinigame_vileWinsRound, (uintptr_t)this->marker);
    }
    if (next_state == 8) {
        chMrVile_setStateTalkToPlayer(local->vile_marker);
        func_80324E38(0.0f, 3);
        timedFunc_set_2(1.0f, (GenFunction_2)coMusicPlayer_playMusic, COMUSIC_3B_MINIGAME_VICTORY, 28000);
        timedFunc_set_0(3.0f, (GenFunction_0)func_8038A044);
        timedFunc_set_1(3.0f, (GenFunction_1)chMrVileMinigame_playerWinsRound, (uintptr_t)this->marker);
    }
    if (next_state == 9) {
        chMrVile_setStateTalkToPlayer(local->vile_marker);
        func_80324E38(0.0f, 3);
        timedFunc_set_2(1.0f, (GenFunction_2)coMusicPlayer_playMusic, COMUSIC_3B_MINIGAME_VICTORY, 28000);
        timedFunc_set_0(3.0f, (GenFunction_0)func_8038A044);
        timedFunc_set_1(3.0f, (GenFunction_1)chMrVileMinigame_vileDropsJiggy, (uintptr_t)this->marker);
    }
    if (next_state == 0xA) {
        chMrVile_setStateTalkToPlayer(local->vile_marker);
        func_80324E38(0.5f, 3);
        timedFunc_set_2(1.0f, (GenFunction_2) coMusicPlayer_playMusic, COMUSIC_3B_MINIGAME_VICTORY, 28000);
        timedFunc_set_1(3.0f, (GenFunction_1) chMrVileMinigame_vileDropsExtraLives, (uintptr_t)this->marker);
    }
    if (next_state == 7) {
        chMrVile_setStateAttackPlayer(local->vile_marker);
    }
    this->state = next_state;
    // [port] Anchor: authority claim/release and round lifecycle react to this event
    CALL_EVENT(OnVileGameStateChange, next_state);
}

void chMrVileMinigame_playerConsumePiece(Actor *this) {
    chMrVileMinigameActor *local;
    bool is_correct_type;
    f32 sp44[3];
    struct vilegame_piece *begin;
    struct vilegame_piece *end;
    struct vilegame_piece *i_ptr;

    local = (chMrVileMinigameActor *)&this->local;

    begin = (struct vilegame_piece *)bk_vector_getBegin(local->game_pieces);
    end = (struct vilegame_piece *) bk_vector_getEnd(local->game_pieces);
    if ((end != begin) && chMrVileMinigame_didPlayerConsumePiece(sp44)){
        sp44[1] = 0.0f;
        for(i_ptr = begin; i_ptr < end; i_ptr++){
            if ((ml_vec3f_distance(i_ptr->position, sp44) < 65.25) && chyumblie_is_edible(i_ptr->marker)) {
                // [port] Anchor: followers route the chomp to the authority as an eat request
                if (!EventSystem_Should(VB_VILE_PLAYER_EAT_PIECE, true, i_ptr->position)) {
                    return;
                }
                is_correct_type = ((local->current_type != YUMBLIE) && (i_ptr->type != YUMBLIE)) || (((local->current_type == YUMBLIE) && i_ptr->type == YUMBLIE));
                if (is_correct_type) {
                    local->player_score++;
                    if (local->player_score == 35) {
                        item_inc(ITEM_16_LIFE);
                        coMusicPlayer_playMusic(COMUSIC_15_EXTRA_LIFE_COLLECTED, 0x7FF8);
                    }
                    timedFunc_set_1(0.0f, (GenFunction_1)func_802FDCB8, ITEM_1A_PLAYER_VILE_SCORE);
                    timedFunc_set_1(0.5f, (GenFunction_1)func_802FDCB8, ITEM_1A_PLAYER_VILE_SCORE);
                    timedFunc_set_1(1.0f, (GenFunction_1)func_802FDCB8, ITEM_1A_PLAYER_VILE_SCORE);
                }
                func_8028F6B8(BS_INTR_17, (i_ptr->type != YUMBLIE) ? ASSET_3F7_MODEL_GRUMBLIE : ASSET_3F6_MODEL_YUMBLIE);
                if (!is_correct_type) {
                    func_8028F66C(BS_INTR_18_CROC_ATE_WRONG);
                }
                func_8038B684(i_ptr->marker);
                return;
            }
        }
    }
}

bool chMrVileMinigame_mrVileConsumePiece(ActorMarker *marker, f32 position[3]) {
    Actor *this;
    chMrVileMinigameActor *local;
    struct vilegame_piece *begin;
    struct vilegame_piece *end;
    struct vilegame_piece *i_ptr;

    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;
    if (this->state != 5){
        return false;
    }
    begin = bk_vector_getBegin(local->game_pieces);
    end = bk_vector_getEnd(local->game_pieces);
    for(i_ptr = begin; i_ptr < end; i_ptr++){
        if ((ml_vec3f_distance(i_ptr->position, position) < 50.0f) && func_8038B684(i_ptr->marker)) {
            local->vile_score++;
            timedFunc_set_1(0.0f, (GenFunction_1)func_802FDCB8, ITEM_1B_VILE_VILE_SCORE);
            timedFunc_set_1(0.5f, (GenFunction_1)func_802FDCB8, ITEM_1B_VILE_VILE_SCORE);
            timedFunc_set_1(1.0f, (GenFunction_1)func_802FDCB8, ITEM_1B_VILE_VILE_SCORE);
            return true;
        }
    }
    return false;
}

BKModelBin *chvilegame_get_grumblie_model(ActorMarker *marker){
    Actor *this;
    chMrVileMinigameActor *local;


    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;
    return local->grumblie_model_bin;
}

s32 chMrVileMinigame_getPieceCount(ActorMarker *marker){
    Actor *this;
    chMrVileMinigameActor *local;


    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;
    return bk_vector_size(local->game_pieces);
}

s32 chMrVileMinigame_getDialogIndex(ActorMarker *marker){
    Actor *this;
    chMrVileMinigameActor *local;


    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;
    return local->dialogIndex;
}

s32 chMrVileMinigame_getScoreDifference(ActorMarker *marker){
    Actor *this;
    chMrVileMinigameActor *local;


    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;
    return local->vile_score - local->player_score;
}

bool chMrVileMinigame_findClosestPiece(ActorMarker *marker, f32 position[3], f32 yaw, f32 dst[3]) {
    f32 piece_direction[3];
    f32 target_direction[3];
    Actor *this;
    chMrVileMinigameActor *local;
    struct vilegame_piece *closest_piece;
    struct vilegame_piece *begin;
    struct vilegame_piece *end;
    struct vilegame_piece *i_ptr;
    f32 distance;
    f32 angle_diff;

    this = marker_getActor(marker);
    local = (chMrVileMinigameActor *)&this->local;
    target_direction[0] = 0.0f;
    target_direction[1] = 0.0f;
    target_direction[2] = 100.0f;
    ml_vec3f_yaw_rotate_copy(target_direction, target_direction, yaw);
    closest_piece = NULL;
    begin = (struct vilegame_piece *) bk_vector_getBegin(local->game_pieces);
    end = (struct vilegame_piece *) bk_vector_getEnd(local->game_pieces);
    for(i_ptr = begin; i_ptr < end; i_ptr++){
        if( ((local->current_type != YUMBLIE) && (i_ptr->type != YUMBLIE)) 
            || ((local->current_type == YUMBLIE) && (i_ptr->type == YUMBLIE))
        ){
            piece_direction[0] = i_ptr->position[0] - position[0];
            piece_direction[1] = i_ptr->position[1] - position[1];
            piece_direction[2] = i_ptr->position[2] - position[2];
            distance = ml_vec3f_distance(i_ptr->position, position);
            angle_diff = func_80256AB4(target_direction[0], target_direction[2], piece_direction[0], piece_direction[2]);
            if( (distance > 300.0f) 
                || ((-0.8 < angle_diff) && (angle_diff < 0.8) && ((piece_direction[0]*target_direction[0] + piece_direction[1]*target_direction[1] + piece_direction[2]*target_direction[2]) >= 0.0f))
            ) {
                if ((closest_piece == NULL) || (distance < ml_vec3f_distance(position, closest_piece->position))){
                    closest_piece = i_ptr;
                }
            }
        }
    }
    if (closest_piece != NULL) {
        dst[0] = closest_piece->position[0];
        dst[1] = closest_piece->position[1];
        dst[2] = closest_piece->position[2];
        return true;
    }
    return false;
}

void chMrVileMinigame_newPiece(ActorMarker *game_marker, ActorMarker *piece_marker, f32 position[3], u32 yumblie_type){
    Actor *this;
    chMrVileMinigameActor *local;
    struct vilegame_piece *temp_v0;

    this = marker_getActor(game_marker);
    local = (chMrVileMinigameActor *)&this->local;
    temp_v0 = (struct vilegame_piece *)bk_vector_pushBackNew(&local->game_pieces);
    temp_v0->type = yumblie_type;
    temp_v0->marker = piece_marker;
    temp_v0->position[0] = position[0];
    temp_v0->position[1] = position[1];
    temp_v0->position[2] = position[2];
    temp_v0->position[1] = 0.0f;
}

void chMrVileMinigame_free(Actor *this){
    chMrVileMinigameActor *local;

    local = (chMrVileMinigameActor *)&this->local;
    // [port] Teardown ordering hazard: if Mr Vile is freed before this controller, its skeletal
    // anim (unk148) is already NULL and chMrVileMinigame_setState(this, 0) -> skeletalAnim_set crashes. Skip it.
#if 0
    chMrVileMinigame_setState(this, 0);
#endif
    bk_vector_free(local->game_pieces);
    assetcache_release(local->grumblie_model_bin);
}

void chvilegame_remove_piece(ActorMarker *game_marker, ActorMarker *piece_marker) {
    Actor *this;
    chMrVileMinigameActor *local;
    struct vilegame_piece *begin;
    struct vilegame_piece *end;
    struct vilegame_piece *i_ptr;

    this = marker_getActor(game_marker);
    local = (chMrVileMinigameActor *)&this->local;
    begin = (struct vilegame_piece *)bk_vector_getBegin(local->game_pieces);
    end = (struct vilegame_piece *)bk_vector_getEnd(local->game_pieces);
    for(i_ptr = begin; i_ptr < end; i_ptr++){
        if (piece_marker == i_ptr->marker) {
            bk_vector_remove(local->game_pieces, i_ptr - begin);
            return;
        }
    }
}

void chMrVileMinigame_update(Actor *this) {
    chMrVileMinigameActor *local;
    f32 sp50;
    f32 sp4C;
    u8 temp_v0;
    s32 sp30[6];
    s32 sp2C;

    sp50 = time_getDelta();
    local = (chMrVileMinigameActor *)&this->local;
    if (!this->volatile_initialized) {
        this->volatile_initialized = true;
        this->marker->actorFreeFunc = &chMrVileMinigame_free;
        local->game_pieces = bk_vector_new(sizeof(struct vilegame_piece), 0x20);
        local->grumblie_model_bin = assetcache_get(0x3F7);
        local->dialogIndex = 0;
        local->vile_marker = NULL;
        if (this->state == 0) {
            local->mode = 0;
        } else {
            this->state = 0;
        }
        if (jiggyscore_isSpawned(JIGGY_28_BGS_MR_VILE)) {
            local->dialogIndex = 3;
            local->mode = 3;
        }
        if (volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME)) {
            local->dialogIndex = 2;
            local->mode = 3;
        }
        chMrVileMinigame_setState(this, 1);
        return;
    }
    if (local->vile_marker == NULL) {
        local->vile_marker = actorArray_findClosestActorFromActorId(this->position, 0x13A, -1, &sp4C)->marker;
    }
    // [port] Anchor: followers are driven by snapshots; only local chomp detection (for
    // eat requests) and HUD mirroring run here.
    if (!EventSystem_Should(VB_VILE_GAME_UPDATE, true)) {
        if ((this->state == 5) && (local->dialogIndex != 7)) {
            chMrVileMinigame_playerConsumePiece(this);
            if (local->type_change_timer > 3.5) {
                if (local->current_type != 0) {
                    item_adjustByDiffWithHud(ITEM_1D_GRUMBLIE, false);
                } else {
                    item_adjustByDiffWithHud(ITEM_1E_YUMBLIE, false);
                }
            }
            ml_timer_update(&local->type_change_timer, sp50);
            item_set(ITEM_1A_PLAYER_VILE_SCORE, local->player_score);
            item_set(ITEM_1B_VILE_VILE_SCORE, local->vile_score);
        }
        return;
    }
    if (this->state == 1) {
        if (volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME)) {
            if (volatileFlag_get(VOLATILE_FLAG_3)) {
                chMrVileMinigame_setState(this, 5);
            }
        } else if (chMrVile_playerWithinRange(local->vile_marker)) {
            chMrVileMinigame_setState(this, 2);
        }
    }
    if (this->state == 5) {
        if (local->dialogIndex == 7) {
            controller_copyFaceButtons(0, sp30);
            if ((sp30[FACE_BUTTON(BUTTON_B)] > 0) && chMrVile_playerWithinRange(local->vile_marker)) {
                chMrVileMinigame_setState(this, 0xA);
            }
        } else {
            chMrVileMinigame_playerConsumePiece(this);
            if ((local->dialogIndex == 3) || (local->dialogIndex == 6)) {
                if (ml_timer_update(&local->type_change_timer, sp50)) {
                    local->current_type = !local->current_type;
                    if (volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME)) {
                        local->type_change_timer = 5.0f;
                    } else {
                        local->type_change_timer = 10.0f;
                    }
                }
                if (local->type_change_timer > 3.5) {
                    if (local->current_type != 0) {
                        item_adjustByDiffWithHud(ITEM_1D_GRUMBLIE, false);
                    } else {
                        item_adjustByDiffWithHud(ITEM_1E_YUMBLIE, false);
                    }
                }
            }
            sp2C = item_getCount(ITEM_1A_PLAYER_VILE_SCORE);
            item_set(ITEM_1A_PLAYER_VILE_SCORE, local->player_score);
            item_set(ITEM_1B_VILE_VILE_SCORE, local->vile_score);
            if ((sp2C != 0) && (local->player_score == 0)) {
                itemPrint_reset();
            }
            if (item_empty(ITEM_6_HOURGLASS)) {
                if (volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME)) {
                    volatileFlag_set(VOLATILE_FLAG_3, 0);
                    volatileFlag_set(VOLATILE_FLAG_5_FF_MINIGAME_WON, BOOL(local->vile_score < local->player_score));
                    chMrVileMinigame_setState(this, 1);
                } else if (local->vile_score < local->player_score) {
                    if (local->dialogIndex == 3) {
                        chMrVileMinigame_setState(this, 9);
                    } else {
                        chMrVileMinigame_setState(this, 8);
                    }
                } else {
                    chMrVileMinigame_setState(this, 6);
                }
            }
        }
    }
    if ((this->state == 7) && (chMrVile_isInitialIdle(local->vile_marker) != 0)) {
        chMrVileMinigame_setState(this, 1);
    }
}

// [port] Anchor sync entry points.

// Fills a snapshot from the live controller. Only the stable states (idle, playing, and
// the round-end results) are broadcast; dialog states cannot be reconstructed remotely.
bool chvilegame_netGather(Actor *this, VileGameSnapshot *dst){
    chMrVileMinigameActor *local = (chMrVileMinigameActor *)&this->local;

    switch (this->state) {
        case 1:   // idle
        case 5:   // playing
        case 6:   // round lost
        case 8:   // round won
        case 9:   // final round won (jiggy)
        case 0xA: // extra challenge won
            break;
        default:
            return false;
    }
    dst->gameState = this->state;
    dst->round = local->dialogIndex;
    dst->maxRound = local->mode;
    dst->currentType = local->current_type;
    dst->typeChangeTimer = local->type_change_timer;
    dst->playerScore = local->player_score;
    dst->vileScore = local->vile_score;
    dst->hourglassRemaining = item_getCount(ITEM_0_HOURGLASS_TIMER);
    return true;
}

// Forces the controller to match an authoritative snapshot (followers + late joiners).
// Mirrors the state-5 side effects of chMrVileMinigame_setState without the dialog/round bookkeeping.
void chvilegame_netApply(Actor *this, const VileGameSnapshot *src){
    chMrVileMinigameActor *local = (chMrVileMinigameActor *)&this->local;
    s32 drift;

    local->dialogIndex = src->round;
    local->mode = src->maxRound;
    local->current_type = src->currentType;
    local->type_change_timer = src->typeChangeTimer;
    local->player_score = src->playerScore;
    local->vile_score = src->vileScore;
    item_set(ITEM_1A_PLAYER_VILE_SCORE, local->player_score);
    item_set(ITEM_1B_VILE_VILE_SCORE, local->vile_score);

    if (this->state != src->gameState) {
        if (src->gameState == 5) {
            item_set(ITEM_0_HOURGLASS_TIMER, src->hourglassRemaining);
            item_set(ITEM_6_HOURGLASS, true);
            mapSpecificFlags_set(6, true);
            if (local->vile_marker != NULL) {
                chMrVile_setStatePlayMinigame(local->vile_marker);
            }
            func_8025A58C(0, 4000);
            timedFunc_set_2(1.0f, (GenFunction_2)coMusicPlayer_playMusic, COMUSIC_55_BGS_MR_VILE, 28000);
        } else {
            item_set(ITEM_6_HOURGLASS, false);
            mapSpecificFlags_set(6, false);
            // Round-end result presentation, mirroring chMrVileMinigame_setState's timed callbacks
            // (dialogs and the static camera stay on the authority).
            if (src->gameState == 6) {
                timedFunc_set_2(1.0f, (GenFunction_2)coMusicPlayer_playMusic, COMUSIC_3C_MINIGAME_LOSS, 28000);
                timedFunc_set_0(4.0f, (GenFunction_0)func_8038A044);
            } else if ((src->gameState == 8) || (src->gameState == 9) || (src->gameState == 0xA)) {
                timedFunc_set_2(1.0f, (GenFunction_2)coMusicPlayer_playMusic, COMUSIC_3B_MINIGAME_VICTORY, 28000);
                timedFunc_set_0(3.0f, (GenFunction_0)func_8038A044);
            } else {
                func_8025A58C(-1, 400);
            }
        }
        this->state = src->gameState;
    } else if (this->state == 5) {
        // tolerance band: only snap the hourglass when drift would be noticeable
        drift = item_getCount(ITEM_0_HOURGLASS_TIMER) - src->hourglassRemaining;
        if ((drift > 15) || (drift < -15)) {
            item_set(ITEM_0_HOURGLASS_TIMER, src->hourglassRemaining);
        }
    }
}

// Authority-side: consume the piece at position on behalf of a remote player. Mirrors
// chMrVileMinigame_playerConsumePiece without reading the local player's mouth position.
// position[1] must be 0 to match piece positions (see chMrVileMinigame_newPiece).
// On success, reports the consumed piece's type and whether it matched the required
// type so the requesting client can replay the croc's eat feedback (see
// chvilegame_netPlayEatFeedback).
bool chvilegame_netConsumeRemote(Actor *this, f32 position[3], s32 *out_piece_type, s32 *out_correct_type){
    chMrVileMinigameActor *local = (chMrVileMinigameActor *)&this->local;
    bool is_correct_type;
    struct vilegame_piece *begin;
    struct vilegame_piece *end;
    struct vilegame_piece *i_ptr;

    if (this->state != 5) {
        return false;
    }
    begin = (struct vilegame_piece *)bk_vector_getBegin(local->game_pieces);
    end = (struct vilegame_piece *)bk_vector_getEnd(local->game_pieces);
    for(i_ptr = begin; i_ptr < end; i_ptr++){
        if ((ml_vec3f_distance(i_ptr->position, position) < 65.25) && chyumblie_is_edible(i_ptr->marker)) {
            is_correct_type = ((local->current_type != YUMBLIE) && (i_ptr->type != YUMBLIE)) || (((local->current_type == YUMBLIE) && i_ptr->type == YUMBLIE));
            if (is_correct_type) {
                local->player_score++;
            }
            if (out_piece_type != NULL) {
                *out_piece_type = i_ptr->type;
            }
            if (out_correct_type != NULL) {
                *out_correct_type = is_correct_type;
            }
            func_8038B684(i_ptr->marker);
            return true;
        }
    }
    return false;
}

// Replays the local player's eat feedback after the minigame authority confirms a
// remote eat request: the croc chomp animation (which carries the chomp SFX) plus
// the wrong-type reaction when the eaten piece didn't match the required type. The
// score and piece removal are authority-driven and arrive separately; this is the
// cosmetic half of chMrVileMinigame_playerConsumePiece that the follower otherwise skips.
void chvilegame_netPlayEatFeedback(s32 piece_type, s32 correct_type){
    func_8028F6B8(BS_INTR_17, (piece_type != YUMBLIE) ? ASSET_3F7_MODEL_GRUMBLIE : ASSET_3F6_MODEL_YUMBLIE);
    if (!correct_type) {
        func_8028F66C(BS_INTR_18_CROC_ATE_WRONG);
    }
}
