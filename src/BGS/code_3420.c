#include "functions.h"
#include "bk_math.h"
#include "variables.h"
#include <ultra64.h>

extern f32 *chVile_getPostion(ActorMarker *);
extern void bundle_setRandomVelocity(f32);
extern void func_802FDCB8(s32);

enum chvilegame_piece_type_e { YUMBLIE, GRUMBLIE };

struct vilegame_piece {
  enum chvilegame_piece_type_e type;
  f32 position[3];
  ActorMarker *marker; // yumblie ptr;
};

typedef struct {
  u8 current_type;
  // u8 pad1[3];
  bk_vector(struct vilegame_piece) * game_pieces;
  BKModelBin *grumblie_model_bin;
  u8 unkC;
  u8 unkD;
  u8 player_score;
  u8 vile_score;
  f32 type_change_timer;
  ActorMarker *vile_marker;
} ActorLocal_BGS_3420;

void chvilegame_setState(Actor *this, s32 next_state);

/* .data */
ActorInfo D_80390960 = {MARKER_C6_VILE_GAME_CTRL,
                        ACTOR_138_VILE_GAME_CTRL,
                        0,
                        0,
                        NULL,
                        chvilegame_update,
                        NULL,
                        func_80325340,
                        0,
                        0,
                        0.0f,
                        0};

// Vile Wins
enum asset_e D_80390984[] = {0,
                             ASSET_C66_DIALOG_MR_VILE_WINS_ROUND_1,
                             ASSET_C68_DIALOG_MR_VILE_WINS_ROUND_2,
                             ASSET_C6A_DIALOG_MR_VILE_WINS_ROUND_3,
                             ASSET_C92_DIALOG_MR_VILE_WINS_EXTRA_CHALLENGE_2,
                             ASSET_C93_DIALOG_MR_VILE_WINS_EXTRA_CHALLENGE_3,
                             ASSET_C94_DIALOG_MR_VILE_WINS_EXTRA_CHALLENGE_4,
                             0};

// Player Wins
enum asset_e D_803909A4[] = {0,
                             ASSET_C67_DIALOG_MR_VILE_ROUND_2_START,
                             ASSET_C69_DIALOG_MR_VILE_ROUND_3_START,
                             0,
                             ASSET_C95_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_1,
                             ASSET_C96_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_2,
                             ASSET_C97_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_3};

// Player Wins Rematch
enum asset_e D_803909C0[] = {0,
                             ASSET_C6E_DIALOG_MR_VILE_LOSE_ROUND_2_REMATCH,
                             ASSET_C6F_DIALOG_MR_VILE_LOSE_ROUND_3_REMATCH,
                             0,
                             ASSET_C95_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_1,
                             ASSET_C96_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_2,
                             ASSET_C97_DIALOG_MR_VILE_LOSES_EXTRA_CHALLENGE_3};

// Player Declines
enum asset_e D_803909DC[] = {
    ASSET_C65_DIALOG_MR_VILE_PLAYER_DECLINES,
    ASSET_C65_DIALOG_MR_VILE_PLAYER_DECLINES,
    ASSET_C65_DIALOG_MR_VILE_PLAYER_DECLINES,
    ASSET_C8F_DIALOG_MR_VILE_PLAYER_DECLINES_EXTRA_CHALLENGE,
    0,
    0,
    0};

// Round 1 Regular & Extra Challenge
enum asset_e BGS_D_803909F8[] = {
    ASSET_C64_DIALOG_MR_VILE_ROUND_1_START,
    0,
    0,
    ASSET_C8E_DIALOG_MR_VILE_WINS_EXTRA_CHALLENGE_1,
    0,
    0,
    0,
    0};

enum asset_e D_80390A18[] = {ASSET_C6D_DIALOG_MR_VILE_LOSE_ROUND_1_REMATCH,
                             ASSET_C70_DIALOG_MR_VILE_WIN_ROUND_2_REMATCH,
                             ASSET_C71_DIALOG_MR_VILE_WIN_ROUND_3_REMATCH,
                             ASSET_C8E_DIALOG_MR_VILE_WINS_EXTRA_CHALLENGE_1,
                             0,
                             0,
                             0,
                             0};

/* .code */
bool chvilegame_isPlayerInRange(f32 arg0[3]) {
  if (player_movementGroup() != BSGROUP_7_CROC_ATTACK) {
    return FALSE;
  }
  func_8028E9C4(2, arg0);
  return TRUE;
}

void chvilegame_resetGamePieces(Actor *this, s32 arg1) {
  ActorLocal_BGS_3420 *local;
  Actor *vile;
  f32 sp94[3];
  f32 sp88[3];
  f32 sp7C[3];
  s32 var_s0;
  s32 var_v0;

  local = (ActorLocal_BGS_3420 *)&this->local;
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
      bundle_setYaw(ml_vec2f_angle_between_points(sp7C[0], sp7C[2]));
      jiggy_spawn(JIGGY_28_BGS_MR_VILE, sp94);
    }
  }
  for (var_s0 = 2; var_s0 != 0x3C; var_s0++) {
    var_v0 = ((var_s0 & 1)) ? -(var_s0 / 2) * 0xA : (var_s0 / 2) * 0xA;
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
    if (ml_vec3f_distance(sp94, sp7C) <= 1000.0f) {
      break;
    }
  }
  ncStaticCamera_setPositionAndRotation(sp94, sp88);
}

void chvilegame_handleDialogResponse(ActorMarker *marker, enum asset_e text_id,
                                     s32 arg2) {
  Actor *this;

  this = marker_getActor(marker);
  if (arg2 == 1) {
    chvilegame_setState(this, 4);
  } else {
    chvilegame_setState(this, 3);
  }
}

void chvilegame_startRound(ActorMarker *marker, enum asset_e text_id,
                           s32 arg2) {
  Actor *this;

  this = marker_getActor(marker);
  chvilegame_setState(this, 1);
}

void chvilegame_handleRoundEnd(ActorMarker *marker, enum asset_e text_id,
                               s32 arg2) {
  Actor *this;

  this = marker_getActor(marker);
  chvilegame_setState(this, 5);
}

void chvilegame_handleVileWin(ActorMarker *marker, enum asset_e text_id,
                              s32 arg2) {
  Actor *this;
  ActorLocal_BGS_3420 *local;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  timed_exitStaticCamera(0.0f);
  setCameraModeAtTime(0.0f, 0);
  local->unkC--;
  chvilegame_setState(this, 7);
}

void chvilegame_showVileWinDialog(ActorMarker *marker) {
  Actor *this;
  ActorLocal_BGS_3420 *local;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  chvilegame_resetGamePieces(this, 0);
  gcdialog_showText(D_80390984[local->unkC], 0xF,
                    chVile_getPostion(local->vile_marker), this->marker,
                    chvilegame_handleVileWin, NULL);
}

void chvilegame_handleDialogEnd(ActorMarker *marker, enum asset_e text_id, s32 arg2) {
  Actor *this;

  this = marker_getActor(marker);
  timed_exitStaticCamera(0.0f);
  setCameraModeAtTime(0.0f, 0);
  chvilegame_setState(this, 5);
}

void chvilegame_showPlayerWinDialog(ActorMarker *marker) {
  Actor *this;
  ActorLocal_BGS_3420 *local;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  chvilegame_resetGamePieces(this, 0);
  if (local->unkC == local->unkD) {
    gcdialog_showText(D_803909A4[local->unkC], 0xF,
                      chVile_getPostion(local->vile_marker), this->marker,
                      chvilegame_handleDialogEnd, NULL);
  } else {
    gcdialog_showText(D_803909C0[local->unkC], 0xF,
                      chVile_getPostion(local->vile_marker), this->marker,
                      chvilegame_handleDialogEnd, NULL);
  }
  func_80347A14(0);
}

void chvilegame_startRoundDialogEnd(ActorMarker *marker, enum asset_e text_id, s32 arg2) {
  Actor *this;

  this = marker_getActor(marker);
  timed_exitStaticCamera(0.0f);
  setCameraModeAtTime(0.0f, 0);
  chvilegame_setState(this, 1);
}

void chvilegame_showRoundStartDialog(ActorMarker *marker) {
  Actor *this;
  ActorLocal_BGS_3420 *local;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;

  chvilegame_resetGamePieces(this, 1);
  gcdialog_showText(0xC6B, 0xF, chVile_getPostion(local->vile_marker),
                    this->marker, chvilegame_startRoundDialogEnd, NULL);
}

void chvilegame_handleExtraLife(ActorMarker *marker, enum asset_e text_id,
                                s32 arg2) {
  Actor *this;
  ActorLocal_BGS_3420 *local;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  timed_exitStaticCamera(0.0f);
  setCameraModeAtTime(0.0f, 0);
  local->unkC = 3;
  chvilegame_setState(this, 1);
}

void chvilegame_spawnExtraLife(ActorMarker *marker) {
  Actor *vile;
  Actor *this;
  ActorLocal_BGS_3420 *local;
  s32 i;
  s32 var_s2;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  chvilegame_resetGamePieces(this, 0);
  var_s2 = actorArray_actorCount(ACTOR_49_EXTRA_LIFE);
  vile = marker_getActor(local->vile_marker);
  if (var_s2 > 0) {
    comusic_playTrackWithVolumeOverride(COMUSIC_15_EXTRA_LIFE_COLLECTED, 0x7FF8);
  }
  for (i = 0; i < 3; i++) {
    if (var_s2 < 3) {
      bundle_setYaw(vile->yaw + (f32)(i * 30));
      bundle_setRandomVelocity(2.0f);
      bundle_spawn_f32(BUNDLE_6_MM_HUT_EXTRA_LIFE, vile->position);
      var_s2 += 1;
    } else {
      item_inc(ITEM_16_LIFE);
    }
  }
  gcdialog_showText(0xC98, 0xF, chVile_getPostion(local->vile_marker),
                    this->marker, chvilegame_handleExtraLife, NULL);
}

void chvilegame_stopMusic(void) { playMusicWithFade(-1, 400); }

void chvilegame_setState(Actor *this, s32 next_state) {
  ActorLocal_BGS_3420 *local;

  local = (ActorLocal_BGS_3420 *)&this->local;
  mapSpecificFlags_set(6, FALSE);
  if (next_state == 1) {
    if (local->vile_marker != NULL) {
      chvile_setState5(local->vile_marker);
    }
  }
  if (next_state == 2) {
    chvile_setState2(local->vile_marker);
    if (local->unkC == 3) {
      if (local->unkD >= 4) {
        gcdialog_showText(0xC91, 0xE, chVile_getPostion(local->vile_marker),
                          this->marker, chvilegame_handleDialogResponse, NULL);
      } else {
        gcdialog_showText((local->unkC == local->unkD) ? 0xC8D : 0xC90, 0xE,
                          chVile_getPostion(local->vile_marker), this->marker,
                          chvilegame_handleDialogResponse, NULL);
      }
    } else {
      gcdialog_showText((local->unkC == local->unkD) ? 0xC63 : 0xC6C, 0xE,
                        chVile_getPostion(local->vile_marker), this->marker,
                        chvilegame_handleDialogResponse, NULL);
    }
  }
  if (next_state == 3) {
    gcdialog_showText(D_803909DC[local->unkC], 4,
                      chVile_getPostion(local->vile_marker), this->marker,
                      chvilegame_startRound, NULL);
  }
  if (next_state == 4) {
    if (local->unkC == local->unkD) {
      gcdialog_showText(BGS_D_803909F8[local->unkC],
                        0xE | ((BGS_D_803909F8[local->unkC] == 0xC8E) ? 1 : 0) |
                            0xE,
                        chVile_getPostion(local->vile_marker), this->marker,
                        chvilegame_handleRoundEnd, NULL);
    } else {
      gcdialog_showText(D_80390A18[local->unkC], 0xF,
                        chVile_getPostion(local->vile_marker), this->marker,
                        chvilegame_handleRoundEnd, NULL);
    }
    func_80347A14(0);
  }
  if (next_state == 5) {
    local->unkC++;
    if (local->unkD < local->unkC) {
      local->unkD = local->unkC;
    }
    if (local->unkC == 7) {
      chvile_setState6(local->vile_marker);
    } else {
      local->current_type = YUMBLIE;
      local->player_score = 0;
      local->vile_score = 0;
      if (volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME) != 0) {
        local->type_change_timer = 5.0f;
      } else {
        local->type_change_timer = 10.0f;
      }
      item_set(ITEM_0_HOURGLASS_TIMER, 3600 - 1);
      item_set(ITEM_6_HOURGLASS, TRUE);
      mapSpecificFlags_set(6, TRUE);
      chvile_setState3(local->vile_marker);
      playMusicWithFade(0, 4000);
      timedFunc_set_2(1.0f, (GenFunction_2)comusic_playTrackWithVolumeOverride,
                      COMUSIC_55_BGS_MR_VILE, 28000);
    }
  }
  if (this->state == 5) {
    if (local->unkC != 7) {
      item_set(ITEM_6_HOURGLASS, FALSE);
      if ((next_state != 6) && (next_state != 8) && (next_state != 9)) {
        chvilegame_stopMusic();
      }
    }
    chvile_setState1(local->vile_marker);
    func_80347A14(1);
  }
  if (next_state == 6) {
    chvile_setState2(local->vile_marker);
    setCameraModeAtTime(0.0f, 3);
    timedFunc_set_2(1.0f, (GenFunction_2)comusic_playTrackWithVolumeOverride,
                    COMUSIC_3C_MINIGAME_LOSS, 28000);
    timedFunc_set_0(4.0f, (GenFunction_0)chvilegame_stopMusic);
    timedFunc_set_1(4.0f, (GenFunction_1)chvilegame_showVileWinDialog,
                    (s32)this->marker);
  }
  if (next_state == 8) {
    chvile_setState2(local->vile_marker);
    setCameraModeAtTime(0.0f, 3);
    timedFunc_set_2(1.0f, (GenFunction_2)comusic_playTrackWithVolumeOverride,
                    COMUSIC_3B_MINIGAME_VICTORY, 28000);
    timedFunc_set_0(3.0f, (GenFunction_0)chvilegame_stopMusic);
    timedFunc_set_1(3.0f, (GenFunction_1)chvilegame_showPlayerWinDialog,
                    (s32)this->marker);
  }
  if (next_state == 9) {
    chvile_setState2(local->vile_marker);
    setCameraModeAtTime(0.0f, 3);
    timedFunc_set_2(1.0f, (GenFunction_2)comusic_playTrackWithVolumeOverride,
                    COMUSIC_3B_MINIGAME_VICTORY, 28000);
    timedFunc_set_0(3.0f, (GenFunction_0)chvilegame_stopMusic);
    timedFunc_set_1(3.0f, (GenFunction_1)chvilegame_showRoundStartDialog, (s32)this->marker);
  }
  if (next_state == 0xA) {
    chvile_setState2(local->vile_marker);
    setCameraModeAtTime(0.5f, 3);
    timedFunc_set_2(1.0f, (GenFunction_2)comusic_playTrackWithVolumeOverride,
                    COMUSIC_3B_MINIGAME_VICTORY, 28000);
    timedFunc_set_1(3.0f, (GenFunction_1)chvilegame_spawnExtraLife,
                    (s32)this->marker);
  }
  if (next_state == 7) {
    chvile_setState4(local->vile_marker);
  }
  this->state = next_state;
}

void chvilegame_player_consume_piece(Actor *this) {
  ActorLocal_BGS_3420 *local;
  bool is_correct_type;
  f32 sp44[3];
  struct vilegame_piece *begin;
  struct vilegame_piece *end;
  struct vilegame_piece *i_ptr;

  local = (ActorLocal_BGS_3420 *)&this->local;

  begin = (struct vilegame_piece *)vector_getBegin(local->game_pieces);
  end = (struct vilegame_piece *)vector_getEnd(local->game_pieces);
  if ((end != begin) && chvilegame_isPlayerInRange(sp44)) {
    sp44[1] = 0.0f;
    for (i_ptr = begin; i_ptr < end; i_ptr++) {
      if ((ml_vec3f_distance(i_ptr->position, sp44) < 65.25) &&
          chyumblie_is_edible(i_ptr->marker)) {
        is_correct_type =
            ((local->current_type != YUMBLIE) && (i_ptr->type != YUMBLIE)) ||
            (((local->current_type == YUMBLIE) && i_ptr->type == YUMBLIE));
        if (is_correct_type) {
          local->player_score++;
          if (local->player_score == 35) {
            item_inc(ITEM_16_LIFE);
            comusic_playTrackWithVolumeOverride(COMUSIC_15_EXTRA_LIFE_COLLECTED, 0x7FF8);
          }
          timedFunc_set_1(0.0f, (GenFunction_1)func_802FDCB8,
                          ITEM_1A_PLAYER_VILE_SCORE);
          timedFunc_set_1(0.5f, (GenFunction_1)func_802FDCB8,
                          ITEM_1A_PLAYER_VILE_SCORE);
          timedFunc_set_1(1.0f, (GenFunction_1)func_802FDCB8,
                          ITEM_1A_PLAYER_VILE_SCORE);
        }
        func_8028F6B8(BS_INTR_17, (i_ptr->type != YUMBLIE)
                                      ? ASSET_3F7_MODEL_GRUMBLIE
                                      : ASSET_3F6_MODEL_YUMBLIE);
        if (!is_correct_type) {
          func_8028F66C(BS_INTR_18_CROC_ATE_WRONG);
        }
        chyumblie_onEaten(i_ptr->marker);
        return;
      }
    }
  }
}

bool chvilegame_cpu_consume_piece(ActorMarker *marker, f32 position[3]) {
  Actor *this;
  ActorLocal_BGS_3420 *local;
  struct vilegame_piece *begin;
  struct vilegame_piece *end;
  struct vilegame_piece *i_ptr;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  if (this->state != 5) {
    return FALSE;
  }
  begin = vector_getBegin(local->game_pieces);
  end = vector_getEnd(local->game_pieces);
  for (i_ptr = begin; i_ptr < end; i_ptr++) {
    if ((ml_vec3f_distance(i_ptr->position, position) < 50.0f) &&
        chyumblie_onEaten(i_ptr->marker)) {
      local->vile_score++;
      timedFunc_set_1(0.0f, (GenFunction_1)func_802FDCB8,
                      ITEM_1B_VILE_VILE_SCORE);
      timedFunc_set_1(0.5f, (GenFunction_1)func_802FDCB8,
                      ITEM_1B_VILE_VILE_SCORE);
      timedFunc_set_1(1.0f, (GenFunction_1)func_802FDCB8,
                      ITEM_1B_VILE_VILE_SCORE);
      return TRUE;
    }
  }
  return FALSE;
}

BKModelBin *chvilegame_get_grumblie_model(ActorMarker *marker) {
  Actor *this;
  ActorLocal_BGS_3420 *local;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  return local->grumblie_model_bin;
}

s32 chvilegame_get_piece_count(ActorMarker *marker) {
  Actor *this;
  ActorLocal_BGS_3420 *local;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  return vector_size(local->game_pieces);
}

s32 chvilegame_getCurrentRound(ActorMarker *marker) {
  Actor *this;
  ActorLocal_BGS_3420 *local;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  return local->unkC;
}

s32 chvilegame_get_score_difference(ActorMarker *marker) {
  Actor *this;
  ActorLocal_BGS_3420 *local;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  return local->vile_score - local->player_score;
}

bool chvilegame_find_closest_piece(ActorMarker *marker, f32 position[0],
                                   f32 yaw, f32 dst[3]) {
  f32 piece_direction[3];
  f32 target_direction[3];
  Actor *this;
  ActorLocal_BGS_3420 *local;
  struct vilegame_piece *closest_piece;
  struct vilegame_piece *begin;
  struct vilegame_piece *end;
  struct vilegame_piece *i_ptr;
  f32 distance;
  f32 angle_diff;

  this = marker_getActor(marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  target_direction[0] = 0.0f;
  target_direction[1] = 0.0f;
  target_direction[2] = 100.0f;
  ml_vec3f_yaw_rotate_copy(target_direction, target_direction, yaw);
  closest_piece = NULL;
  begin = (struct vilegame_piece *)vector_getBegin(local->game_pieces);
  end = (struct vilegame_piece *)vector_getEnd(local->game_pieces);
  for (i_ptr = begin; i_ptr < end; i_ptr++) {
    if (((local->current_type != YUMBLIE) && (i_ptr->type != YUMBLIE)) ||
        ((local->current_type == YUMBLIE) && (i_ptr->type == YUMBLIE))) {
      piece_direction[0] = i_ptr->position[0] - position[0];
      piece_direction[1] = i_ptr->position[1] - position[1];
      piece_direction[2] = i_ptr->position[2] - position[2];
      distance = ml_vec3f_distance(i_ptr->position, position);
      angle_diff = ml_f_sin_of_angle_between_points_2D(
          target_direction[0], target_direction[2], piece_direction[0],
          piece_direction[2]);
      if ((distance > 300.0f) ||
          ((-0.8 < angle_diff) && (angle_diff < 0.8) &&
           ((piece_direction[0] * target_direction[0] +
             piece_direction[1] * target_direction[1] +
             piece_direction[2] * target_direction[2]) >= 0.0f))) {
        if ((closest_piece == NULL) ||
            (distance < ml_vec3f_distance(position, closest_piece->position))) {
          closest_piece = i_ptr;
        }
      }
    }
  }
  if (closest_piece != NULL) {
    dst[0] = closest_piece->position[0];
    dst[1] = closest_piece->position[1];
    dst[2] = closest_piece->position[2];
    return TRUE;
  }
  return FALSE;
}

void chvilegame_new_piece(ActorMarker *game_marker, ActorMarker *piece_marker,
                          f32 position[3],
                          enum chvilegame_piece_type_e yumblie_type) {
  Actor *this;
  ActorLocal_BGS_3420 *local;
  struct vilegame_piece *temp_v0;

  this = marker_getActor(game_marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  temp_v0 = (struct vilegame_piece *)vector_pushBackNew(&local->game_pieces);
  temp_v0->type = yumblie_type;
  temp_v0->marker = piece_marker;
  temp_v0->position[0] = position[0];
  temp_v0->position[1] = position[1];
  temp_v0->position[2] = position[2];
  temp_v0->position[1] = 0.0f;
}

void chvilegame_free(Actor *this) {
  ActorLocal_BGS_3420 *local;

  local = (ActorLocal_BGS_3420 *)&this->local;
  chvilegame_setState(this, 0);
  vector_free(local->game_pieces);
  assetcache_release(local->grumblie_model_bin);
}

void chvilegame_remove_piece(ActorMarker *game_marker,
                             ActorMarker *piece_marker) {
  Actor *this;
  ActorLocal_BGS_3420 *local;
  struct vilegame_piece *begin;
  struct vilegame_piece *end;
  struct vilegame_piece *i_ptr;

  this = marker_getActor(game_marker);
  local = (ActorLocal_BGS_3420 *)&this->local;
  begin = (struct vilegame_piece *)vector_getBegin(local->game_pieces);
  end = (struct vilegame_piece *)vector_getEnd(local->game_pieces);
  for (i_ptr = begin; i_ptr < end; i_ptr++) {
    if (piece_marker == i_ptr->marker) {
      vector_remove(local->game_pieces, i_ptr - begin);
      return;
    }
  }
}

void chvilegame_update(Actor *actor) {
  ActorLocal_BGS_3420 *local;
  f32 sp50;
  f32 sp4C;
  u8 temp_v0;
  s32 sp30[6];
  s32 sp2C;

  sp50 = time_getDelta();
  local = (ActorLocal_BGS_3420 *)&actor->local;
  if (!actor->volatile_initialized) {
    actor->volatile_initialized = TRUE;
    actor->marker->actorFreeFunc = &chvilegame_free;
    local->game_pieces = vector_new(sizeof(struct vilegame_piece), 0x20);
    local->grumblie_model_bin = assetcache_get(0x3F7);
    local->unkC = 0;
    local->vile_marker = NULL;
    if (actor->state == 0) {
      local->unkD = 0;
    } else {
      actor->state = 0;
    }
    if (jiggyscore_isSpawned(JIGGY_28_BGS_MR_VILE)) {
      local->unkC = 3;
      local->unkD = 3;
    }
    if (volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME)) {
      local->unkC = 2;
      local->unkD = 3;
    }
    chvilegame_setState(actor, 1);
    return;
  }
  if (local->vile_marker == NULL) {
    local->vile_marker =
        actorArray_findClosestActorFromActorId(actor->position, 0x13A, -1, &sp4C)
            ->marker;
  }
  if (actor->state == 1) {
    if (volatileFlag_get(VOLATILE_FLAG_2_FF_IN_MINIGAME)) {
      if (volatileFlag_get(VOLATILE_FLAG_3)) {
        chvilegame_setState(actor, 5);
      }
    } else if (chvile_isPlayerClose(local->vile_marker)) {
      chvilegame_setState(actor, 2);
    }
  }
  if (actor->state == 5) {
    if (local->unkC == 7) {
      controller_copyFaceButtons(0, sp30);
      if ((sp30[FACE_BUTTON(BUTTON_B)] > 0) &&
          chvile_isPlayerClose(local->vile_marker)) {
        chvilegame_setState(actor, 0xA);
      }
    } else {
      chvilegame_player_consume_piece(actor);
      if ((local->unkC == 3) || (local->unkC == 6)) {
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
            item_adjustByDiffWithHud(ITEM_1D_GRUMBLIE, FALSE);
          } else {
            item_adjustByDiffWithHud(ITEM_1E_YUMBLIE, FALSE);
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
          volatileFlag_set(VOLATILE_FLAG_5_FF_MINIGAME_WON,
                           BOOL(local->vile_score < local->player_score));
          chvilegame_setState(actor, 1);
        } else if (local->vile_score < local->player_score) {
          if (local->unkC == 3) {
            chvilegame_setState(actor, 9);
          } else {
            chvilegame_setState(actor, 8);
          }
        } else {
          chvilegame_setState(actor, 6);
        }
      }
    }
  }
  if ((actor->state == 7) && (BGS_chvile_isInState1(local->vile_marker) != 0)) {
    chvilegame_setState(actor, 1);
  }
}
