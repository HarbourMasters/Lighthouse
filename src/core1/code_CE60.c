#include "core1/core1.h"
#include "functions.h"
#include "variables.h"
#include <ultra64.h>

//musicManager.c

typedef struct bounding_box_s {
  s16 x_min;
  s16 x_max;
  s16 y_min;
  s16 y_max;
  s16 z_min;
  s16 z_max;
} BoundingBox;

static s32 currentMusicTrack = -1;
static s32 previousMusicTrack = 0;
static u8 isPlayerInRange = FALSE;
static u8 wasPlayerInRange = FALSE;
static BoundingBox sBoundingBoxes[] = {
    {-6000, -1580, -9000, 9000, -6200, -1228},
    {-1700, -397, -9000, 9000, -6500, -2287},
    {-1405, -85, -9000, 9000, -6010, -4101},
    {-5500, -1957, -9000, 9000, -2000, -624},
    {-320, 320, 1500, 2400, -6000, -389},
    {-700, 1496, -700, 1730, -978, 978},
    {-6000, -1580, -9000, 9000, -6200, -1228}};

static s32 sUnusedCounter; // is only incremented and decremented, but not used
static bool isMusicEventActive;
static s32 sPlayerPosition[4];
static enum comusic_e sTrackId[4];

void map_setChanMask(s32 chan_mask) {
  musicTrack_setChannelMask(0, chan_mask, 3.0f);
}

void map_setChanMaskWithValue(s32 chan_mask, f32 value) {
  musicTrack_setChannelMask(0, chan_mask, value);
}

bool map_isPlayerInRange(s32 x, s32 z, s32 distance) {
  return (x - sPlayerPosition[0]) * (x - sPlayerPosition[0]) +
             (z - sPlayerPosition[2]) * (z - sPlayerPosition[2]) <
         distance * distance;
}

f32 map_getPlayerDistance(f32 x, f32 z) {
  return gu_sqrtf((x - sPlayerPosition[0]) * (x - sPlayerPosition[0]) +
                  (z - sPlayerPosition[2]) * (z - sPlayerPosition[2]));
}

bool map_isPlayerInsideBoundingBox(s32 box_idx) {
  return ml_vec3w_inside_box_w(
      sPlayerPosition, sBoundingBoxes[box_idx].x_min,
      sBoundingBoxes[box_idx].y_min, sBoundingBoxes[box_idx].z_min,
      sBoundingBoxes[box_idx].x_max, sBoundingBoxes[box_idx].y_max,
      sBoundingBoxes[box_idx].z_max);
}

void map_updateMusicTracks(s32 arg0) {
  if (!comusic_isMainTrack(sTrackId[0]) && sTrackId[2]) {
    comusic_setMainTrack(sTrackId[0], 0);
  }
  comusic_updateTrackWithArgs(sTrackId[2], arg0 ? arg0 : 0x1f4, 4);
  if (comusic_isTrackQueued(sTrackId[1]) || sTrackId[3]) {
    comusic_fadeTrack(sTrackId[1], sTrackId[3], arg0 ? arg0 : 0x1f4, 4);
  }
  comusic_stopTrackIfDone(sTrackId[1]);
}

void map_updateTrackVolumes(void) {
  f32 player_position[3];
  f32 player_distance;

  player_getPosition_s32(sPlayerPosition);
  player_getPosition(player_position);

  sTrackId[0] = func_8032274C();
  sTrackId[1] = func_80322758();
  sTrackId[2] = sTrackId[3] = COMUSIC_0_DING_A;

  if (0 <= sTrackId[0])
    sTrackId[2] = musicTrack_getVolume(sTrackId[0]);
  if (0 <= sTrackId[1])
    sTrackId[3] = musicTrack_getVolume(sTrackId[1]);
  switch (map_get()) {
  case MAP_7_TTC_TREASURE_TROVE_COVE:
    sTrackId[2] =
        ml_map_f(4700 - sPlayerPosition[1], 0.0f, 900.0f, 0.0f, sTrackId[2]);
    sTrackId[3] =
        ml_map_f(4700 - sPlayerPosition[1], 0.0f, 900.0f, sTrackId[3], 0.0f);
    break;
  case MAP_B_CC_CLANKERS_CAVERN:
    player_distance = map_getPlayerDistance(13909.0f, -26.0f);
    sTrackId[2] =
        ml_map_f(player_distance, 1500.0f, 1800.0f, 0.0f, sTrackId[2]);
    sTrackId[3] =
        ml_map_f(player_distance, 1500.0f, 1800.0f, sTrackId[3], 0.0f);
    break;
  case MAP_1_SM_SPIRAL_MOUNTAIN:
    if (map_isPlayerInsideBoundingBox(4)) {
      sTrackId[2] = COMUSIC_0_DING_A;
    } else {
      sTrackId[3] = COMUSIC_0_DING_A;
    }
    break;
  case MAP_1B_MMM_MAD_MONSTER_MANSION:
    if (checkCollisionWithModel(player_position, 1)) {
      sTrackId[2] = COMUSIC_0_DING_A;
    } else {
      sTrackId[3] = COMUSIC_0_DING_A;
    }
    break;
  case MAP_41_FP_BOGGYS_IGLOO:
    if (jiggyscore_isCollected(JIGGY_2E_FP_PRESENTS) ||
        (levelSpecificFlags_get(LEVEL_FLAG_11_FP_UNKNOWN) &&
         levelSpecificFlags_get(LEVEL_FLAG_12_FP_UNKNOWN) &&
         levelSpecificFlags_get(LEVEL_FLAG_13_FP_UNKNOWN))) {
      sTrackId[2] = COMUSIC_0_DING_A;
    } else {
      sTrackId[3] = COMUSIC_0_DING_A;
    }
    break;
  case MAP_1D_MMM_CELLAR:
    if (sns_get_item_state(SNS_ITEM_EGG_CYAN, 1) &&
        sPlayerPosition[0] >= 0x23A) {
      sTrackId[2] = COMUSIC_0_DING_A;
    } else {
      sTrackId[3] = COMUSIC_0_DING_A;
    }
    break;
  case MAP_7F_FP_WOZZAS_CAVE:
    if (sns_get_item_state(SNS_ITEM_ICE_KEY, 1) &&
        map_isPlayerInRange(0x619, 0x97a, 0x69a)) {
      sTrackId[2] = COMUSIC_0_DING_A;
    } else {
      sTrackId[3] = COMUSIC_0_DING_A;
    }
    break;
  case MAP_45_CCW_AUTUMN:
  case MAP_46_CCW_WINTER:
    if (map_isPlayerInsideBoundingBox(5)) {
      sTrackId[2] = COMUSIC_0_DING_A;
    } else {
      sTrackId[3] = COMUSIC_0_DING_A;
    }
    break;
  }
}

void map_updateMusicState(bool arg0) {
  map_updateTrackVolumes();
  if (arg0)
    comusic_stopAllTracksIfNotPlaying();
  if (0 < sTrackId[0] && 0 < sTrackId[2])
    comusic_setMainTrack(sTrackId[0], sTrackId[2]);
  if (0 < sTrackId[1] && 0 < sTrackId[3])
    comusic_playTrackWithDefaultVolume(sTrackId[1], sTrackId[3]);
}

void map_resetMusicState(void) {
  sUnusedCounter = 0;
  currentMusicTrack = -1;
  previousMusicTrack = 0;
  wasPlayerInRange = isPlayerInRange =
      map_isPlayerInRange(2883, -10520, 6480) ? TRUE : FALSE;
}

void map_resetState(void) {
  currentMusicTrack = -1;
  previousMusicTrack = 0;
  isPlayerInRange = FALSE;
  wasPlayerInRange = FALSE;
}

void map_setChanMaskFromWaterState(s32 chan_mask_underwater,
                                   s32 chan_mask_surface) {
  if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
    map_setChanMask(chan_mask_underwater);
  } else {
    map_setChanMask(chan_mask_surface);
  }
}

void map_handleMusicEvents(void) {
  if (!isMusicEventActive || volatileFlag_get(VOLATILE_FLAG_1) ||
      volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE) ||
      isInTransitionMap() || player_isDead() || gctransition_8030BDC0() ||
      game_getMode() == GAME_MODE_A_SNS_PICTURE) {
    return;
  }
  player_getPosition_s32(sPlayerPosition);
  map_updateTrackVolumes();
  switch (map_get()) {
  case MAP_2_MM_MUMBOS_MOUNTAIN:
    if (map_isPlayerInRange(-4450, 4550, 1900) ||
        map_isPlayerInRange(-6007, 6176, 620)) {
      map_setChanMask(0x1cc0);
    } else if (map_isPlayerInRange(4511, -1888, 2000)) {
      map_setChanMask(0xb0c0);
    } else if (map_isPlayerInRange(300, -858, 2200)) {
      map_setChanMask(0x513f);
    } else {
      map_setChanMaskFromWaterState(0x200, 0x103f);
    }
    break;
  case MAP_7_TTC_TREASURE_TROVE_COVE:
    map_updateMusicTracks(0);
    if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      map_setChanMask(0x600);
    } else if (map_isPlayerInRange(-300, 1420, 2150) ||
               (map_isPlayerInRange(-300, 1420, 3100) &&
                sPlayerPosition[1] < 1300 &&
                !map_isPlayerInRange(-73, 11331, 7180) &&
                !map_isPlayerInRange(1736, 3588, 455) &&
                !map_isPlayerInRange(1380, 3994, 400))) {
      map_setChanMask(0x7800);
    } else {
      map_setChanMask(0x60ff);
    }
    break;
  case MAP_B_CC_CLANKERS_CAVERN:
    map_updateMusicTracks(0);
    if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      if (!(sPlayerPosition[1] < 651)) {
        map_setChanMaskWithValue(0x8180, 5.0f);
      } else {
        map_setChanMaskWithValue(0x3e00, 5.0f);
      }
    } else {
      map_setChanMask(0x407f);
    }
    break;
  case MAP_12_GV_GOBIS_VALLEY:
    if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      map_setChanMask(0x8020);
    } else if (map_isPlayerInRange(-3990, 5670, 3700)) {
      map_setChanMask(0x797f);
    } else {
      map_setChanMask(0x67fe);
    }
    break;
  case MAP_3_UNUSED:
    map_setChanMaskFromWaterState(0x1800, 0x67fe);
    break;
  case MAP_5_TTC_BLUBBERS_SHIP:
    map_setChanMaskFromWaterState(0x600, 0x7800);
    break;
  case MAP_1_SM_SPIRAL_MOUNTAIN:
    if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      map_setChanMask(0x9000);
    } else {
      if (chmole_learnedAllSpiralMountainAbilities()) {
        map_updateMusicTracks(0);
      }

      map_setChanMask(0x6fff);
    }
    break;
  case MAP_21_CC_WITCH_SWITCH_ROOM:
  case MAP_22_CC_INSIDE_CLANKER:
  case MAP_23_CC_GOLDFEATHER_ROOM:
    map_setChanMaskFromWaterState(0x3c00, 0x61ff);
    break;
  case MAP_25_MMM_WELL:
  case MAP_2F_MMM_WATERDRAIN_BARREL:
    map_setChanMaskFromWaterState(0x1000, 0xcfff);
    break;
  case MAP_13_GV_MEMORY_GAME:
  case MAP_14_GV_SANDYBUTTS_MAZE:
  case MAP_15_GV_WATER_PYRAMID:
  case MAP_16_GV_RUBEES_CHAMBER:
  case MAP_1A_GV_INSIDE_JINXY:
    map_setChanMaskFromWaterState(0x3ffe, 0x4ffe);
    break;
  case MAP_1B_MMM_MAD_MONSTER_MANSION:
    if (!mapSpecificFlags_get(1) &&
        !comusic_isTrackQueued(COMUSIC_4_MMM_CLOCK_VERSION) &&
        !comusic_isTrackQueued(COMUSIC_3C_MINIGAME_LOSS)) {
      map_updateMusicTracks(0);
    }

    map_setChanMask((mapSpecificFlags_get(1) ? 0x2000 : 0) + 0xcfff);
    break;
  case MAP_D_BGS_BUBBLEGLOOP_SWAMP:
    if (map_isPlayerInRange(1890, -1346, 1400) ||
        map_isPlayerInRange(-133, 2008, 900) ||
        map_isPlayerInRange(-3629, -535, 1400)) {
      map_setChanMaskWithValue(0x2f4f, 2.0f);
    } else {
      map_setChanMaskWithValue(0x6f4f, 2.0f);
    }
    break;
  case MAP_31_RBB_RUSTY_BUCKET_BAY:
    if ((-4200 <= sPlayerPosition[0] && sPlayerPosition[0] < -3700) &&
        (-900 <= sPlayerPosition[2] && sPlayerPosition[2] < 900)) {
      map_setChanMask(0x51ff);
    } else {
      map_setChanMaskFromWaterState(0x800, 0x71bf);
    }
    break;
  case MAP_35_RBB_WAREHOUSE:
  case MAP_36_RBB_BOATHOUSE:
  case MAP_37_RBB_CONTAINER_1:
  case MAP_38_RBB_CONTAINER_3:
  case MAP_3E_RBB_CONTAINER_2:
    map_setChanMaskFromWaterState(0x800, 0xfe);
    break;
  case MAP_40_CCW_HUB:
    if (map_isPlayerInRange(0, 0, 2050)) {
      map_setChanMaskWithValue(7, 2.0f);
    } else if (1450 <= sPlayerPosition[2]) {
      map_setChanMaskWithValue(0x407, 2.0f);
    } else if (sPlayerPosition[0] < -1449) {
      map_setChanMaskWithValue(0x707, 2.0f);
    } else if (sPlayerPosition[2] < -1449) {
      map_setChanMaskWithValue(0x1067, 2.0f);
    } else if (1450 <= sPlayerPosition[0]) {
      map_setChanMaskWithValue(0x7007, 2.0f);
    }
    break;
  case MAP_A_TTC_SANDCASTLE:
    map_setChanMaskFromWaterState(0xCE, 0x3C);
    break;
  case MAP_43_CCW_SPRING:
    map_setChanMaskFromWaterState(0x400, 0x7bbf);
    break;
  case MAP_45_CCW_AUTUMN:
    if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      map_setChanMask(0x600);
    } else {
      map_updateMusicTracks(0);
      map_setChanMask(0x7BEF);
    }
    break;
  case MAP_54_UNUSED:
  case MAP_55_UNUSED:
  case MAP_57_UNUSED:
  case MAP_58_UNUSED:
  case MAP_59_UNUSED:
    map_setChanMaskFromWaterState(0xc, 0x307b);
    break;
  case MAP_56_UNUSED:
    map_setChanMaskFromWaterState(0x10, 0x4f6f);
    break;
  case MAP_27_FP_FREEZEEZY_PEAK:
    if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      map_setChanMask(0x400);
    } else if (mapSpecificFlags_get(0) &&
               !jiggyscore_isCollected(JIGGY_2F_FP_XMAS_TREE)) {
      map_setChanMask(0x4bff);
    } else {
      map_setChanMask(0x43ff);
    }
    break;
  case MAP_65_CCW_SPRING_WHIPCRACK_ROOM:
    map_setChanMask(0x107);
    break;
  case MAP_66_CCW_SUMMER_WHIPCRACK_ROOM:
    map_setChanMask(0x1C7);
    break;
  case MAP_67_CCW_AUTUMN_WHIPCRACK_ROOM:
    map_setChanMask(0xC07);
    break;
  case MAP_68_CCW_WINTER_WHIPCRACK_ROOM:
    map_setChanMask(0x1407);
    break;
  case MAP_5E_CCW_SPRING_NABNUTS_HOUSE:
    map_setChanMask(0x41fe);
    break;
  case MAP_5F_CCW_SUMMER_NABNUTS_HOUSE:
    map_setChanMask(0x71fe);
    break;
  case MAP_60_CCW_AUTUMN_NABNUTS_HOUSE:
    map_setChanMask(0x7fe);
    break;
  case MAP_61_CCW_WINTER_NABNUTS_HOUSE:
    map_setChanMask(0xbfe);
    break;
  case MAP_63_CCW_AUTUMN_NABNUTS_WATER_SUPPLY:
    map_setChanMaskFromWaterState(1, 0x7fe);
    break;
  case MAP_64_CCW_WINTER_NABNUTS_WATER_SUPPLY:
    map_setChanMaskFromWaterState(1, 0xbfe);
    break;
  case MAP_69_GL_MM_LOBBY:
    if (map_isPlayerInRange(3510, -1630, 1250)) {
      map_setChanMask(0x7c00);
    } else {
      map_setChanMask(0x41ff);
    }
    break;
  case MAP_6A_GL_TTC_AND_CC_PUZZLE:
    if (696 <= sPlayerPosition[1] && map_isPlayerInRange(-1557, -905, 477)) {
      map_setChanMask(0x8e40);
    } else if (map_isPlayerInRange(1500, -890, 550) ||
               map_isPlayerInRange(1876, -1107, 370)) {
      map_setChanMask(0xf040);
    } else {
      map_setChanMask(0x81ff);
    }
    break;
  case MAP_6B_GL_180_NOTE_DOOR:
    if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      map_setChanMask(0x8800);
    } else if (map_isPlayerInRange(4331, 1269, 1840)) {
      map_setChanMask(0x8640);
    } else if (map_isPlayerInRange(-1318, 1911, 293) ||
               map_isPlayerInRange(-1301, 2168, 234)) {
      map_setChanMask(0xf000);
    } else {
      map_setChanMask(0x81bf);
    }
    break;
  case MAP_6C_GL_RED_CAULDRON_ROOM:
    map_setChanMask(0x81bf);
    break;
  case MAP_6D_GL_TTC_LOBBY:
    map_setChanMask(0xf000);
    break;
  case MAP_70_GL_CC_LOBBY:
    if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      map_setChanMask(0x8100);
    } else if (map_isPlayerInRange(-6614, -467, 1822)) {
      map_setChanMask(0xf047);
    } else {
      map_setChanMask(0x8e41);
    }
    break;
  case MAP_6E_GL_GV_LOBBY:
    if (map_isPlayerInRange(0, -6550, 3650)) {
      map_setChanMask(0xfe00);
    } else {
      map_setChanMask(0x81ff);
    }
    break;
  case MAP_6F_GL_FP_LOBBY:
    if (3701 <= sPlayerPosition[2]) {
      map_setChanMask(0xe040);
    } else {
      map_setChanMask(0x81bf);
    }
    break;
  case MAP_74_GL_GV_PUZZLE:
    if (map_isPlayerInRange(-2633, -31, 515)) {
      map_setChanMask(0xe600);
    } else {
      map_setChanMask(0x81ff);
    }
    break;
  case MAP_75_GL_MMM_LOBBY:
  case MAP_7A_GL_CRYPT:
    map_setChanMask(0xd800);
    break;
  case MAP_71_GL_STATUE_ROOM:
    if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      map_setChanMask(0x200);
    } else if (map_isPlayerInRange(-3854, -346, 770)) {
      map_setChanMask(0x9c00);
    } else {
      map_setChanMask(0x81bf);
    }
    break;
  case MAP_72_GL_BGS_LOBBY:
    if (currentMusicTrack == -1 && isPlayerInRange == FALSE) {
      currentMusicTrack = func_802F9AA8(0x410);
      func_802F9F80(currentMusicTrack, 3.0f, 16777216.0f, 0.0f);
      func_802FA060(currentMusicTrack, 3500, 3500, 0.0f);

      previousMusicTrack = func_802F9AA8(0x411);
      func_802F9F80(previousMusicTrack, 3.0f, 16777216.0f, 0.0f);
      func_802FA060(previousMusicTrack, 3500, 3500, 0.0f);
    }
    if (wasPlayerInRange != isPlayerInRange) {
      wasPlayerInRange = isPlayerInRange;
      if (isPlayerInRange) {
        func_802F9FD0(currentMusicTrack, 0.0f, 0.0f, 3.0f);
        func_802F9FD0(previousMusicTrack, 0.0f, 0.0f, 3.0f);
        currentMusicTrack = -1;
        previousMusicTrack = 0;
      }
    }
    if (map_isPlayerInRange(2883, -10520, 6480)) {
      isPlayerInRange = TRUE;
      map_setChanMask(0xe040);
    } else {
      isPlayerInRange = FALSE;
      map_setChanMask(0x9c00);
    }
    break;
  case MAP_76_GL_640_NOTE_DOOR:
    map_setChanMaskFromWaterState(0x8200, 0x81bf);
    break;
  case MAP_77_GL_RBB_LOBBY:
    map_setChanMaskFromWaterState(0x8200, 0xf000);
    break;
  case MAP_78_GL_RBB_AND_MMM_PUZZLE:
    if (player_getWaterState() == BSWATERGROUP_2_UNDERWATER) {
      map_setChanMask(0x8200);
    } else if (346 <= sPlayerPosition[1] &&
               map_isPlayerInRange(-2202, 538, 872)) {
      map_setChanMask(0x8C00);
    } else if (map_isPlayerInRange(-54, 20, 2652)) {
      map_setChanMask(0x81bf);
    } else {
      map_setChanMask(0xf000);
    }
    break;
  case MAP_79_GL_CCW_LOBBY:
    if (796 <= sPlayerPosition[1] && sPlayerPosition[1] < 1100 &&
        map_isPlayerInRange(28, 4553, 1073)) {
      map_setChanMask(0x81bf);
    } else if (-4 <= sPlayerPosition[1] && sPlayerPosition[1] < 360 &&
               map_isPlayerInRange(135, 4979, 2156)) {
      map_setChanMask(0x81bf);
    } else {
      map_setChanMask(0x9e00);
    }
    break;
  case MAP_80_GL_FF_ENTRANCE:
    if (sPlayerPosition[2] < 0x4e2)
      map_setChanMask(0xe040);
    else
      map_setChanMask(0x9e00);
    break;
  case MAP_7F_FP_WOZZAS_CAVE:
    map_updateMusicTracks((sns_get_item_state(SNS_ITEM_ICE_KEY, 1) &&
                           map_isPlayerInRange(1561, 2426, 1690))
                              ? 0x7ff8
                              : 0);
    map_setChanMaskFromWaterState(0x20, 0x1f);
    break;
  case MAP_8B_RBB_ANCHOR_ROOM:
    map_setChanMaskFromWaterState(0x800, 0x51ff);
    break;
  case MAP_34_RBB_ENGINE_ROOM:
    map_setChanMaskFromWaterState(0x800, 0x43fe);
    break;
  case MAP_91_FILE_SELECT:
    if (!func_802C5A30()) {
      map_setChanMaskWithValue(0x200, 0.5f);
    } else {
      map_setChanMaskWithValue(0x1ff, 0.5f);
    }
    break;
  case MAP_8C_SM_BANJOS_HOUSE:
    map_setChanMaskWithValue(0x1ff, 0.5f);
    break;
  case MAP_1D_MMM_CELLAR:
    map_updateMusicTracks((sns_get_item_state(SNS_ITEM_EGG_CYAN, 1) &&
                           !(sPlayerPosition[0] < 570))
                              ? 0x7ff8
                              : 0);
    break;
  case MAP_46_CCW_WINTER:
    map_updateMusicTracks(0);
    break;
  }
}

void map_worthlessCounter(bool increment) {
  if (increment) {
    sUnusedCounter++;
  } else {
    sUnusedCounter--;
  }
}

void map_setMusicFlag(s32 arg0, s32 arg1) {
  if (arg1 == 3)
    return;
  if (arg1 == 2) {
    isMusicEventActive = TRUE;
  } else {
    isMusicEventActive = FALSE;
  }
}
