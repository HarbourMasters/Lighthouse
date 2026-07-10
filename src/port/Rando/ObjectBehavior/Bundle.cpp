#include "ObjectBehavior.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "spdlog/spdlog.h"

extern "C" {
extern f32 gBundle_yaw;

bool fileProgressFlag_get(enum file_progress_e index);
void marker_despawn(ActorMarker* marker);
}

bool applyCustomPhysics = false;
std::vector<ActorMarker*> bundleDespawnQueue;
int32_t vileCount = 0;

void Rando::ObjectBehavior::DespawnCollectedBundles() {
    if (bundleDespawnQueue.empty()) {
        return;
    }

    for (auto& bundle : bundleDespawnQueue) {
        marker_despawn(bundle);
    }

    bundleDespawnQueue.clear();
}

void Rando::ObjectBehavior::InitBundleBehavior() {
    COND_HOOK(ClearBundleDespawnQueue, EVENT_PRIORITY_NORMAL, IS_RANDO,
              [](IEvent* event) { bundleDespawnQueue.clear(); })

    COND_VB_SHOULD(VB_BUNDLE_SPAWN_SET_ACTOR_DATA, EVENT_PRIORITY_NORMAL, IS_RANDO, {
        Actor* refActor = va_arg(args, Actor*);

        if (refActor == NULL) {
            *should = true;
        }
    })

    COND_VB_SHOULD(VB_OVERRIDE_BUNDLE_SPAWN, EVENT_PRIORITY_NORMAL, IS_RANDO, {
        bundle_e bundleId = (bundle_e)va_arg(args, int);
        BundleInfo* bundleInfo = va_arg(args, BundleInfo*);
        s32 bundleCount = va_arg(args, s32);
        f32* position = va_arg(args, f32*);
        Actor** actor = va_arg(args, Actor**);

        map_e currentMap = gsworld_getMap();
        level_e currentLevel = map_getLevel(currentMap);

        int32_t spawnPosition[3];
        spawnPosition[0] = (int32_t)position[0];
        spawnPosition[1] = (int32_t)position[1];
        spawnPosition[2] = (int32_t)position[2];

        SPDLOG_INFO("Bundle ID: {} :: {}, {}, {}", (int32_t)bundleId, spawnPosition[0], spawnPosition[1], spawnPosition[2]);

        if (bundleId == BUNDLE_16__HONEYCOMB &&
            (currentMap == MAP_43_CCW_SPRING || currentMap == MAP_B_CC_CLANKERS_CAVERN)) {
            if (CheckEnemyOverlapPosition(spawnPosition)) {
                *should = false;
                return;
            }
        }

        RandoCheckId randoCheckId = RC_UNKNOWN;
        applyCustomPhysics = false;

        switch (currentLevel) {
            case LEVEL_1_MUMBOS_MOUNTAIN:
                switch (bundleId) {
                    case BUNDLE_0_MM_HUT_MUSIC_NOTE:
                        randoCheckId = (RandoCheckId)((int32_t)RC_MM_NOTE_HUT_BUNDLE_1 + bundleCount);
                        applyCustomPhysics = true;
                        break;
                    case BUNDLE_1_MM_HUT_BLUE_EGG:
                        randoCheckId = (RandoCheckId)((int32_t)RC_MM_BLUE_EGG_HUT_BUNDLE_1 + bundleCount);
                        applyCustomPhysics = true;
                        break;
                    case BUNDLE_3_MM_HUT_JINJO_GREEN:
                        randoCheckId = RC_MM_JINJO_GREEN;
                        applyCustomPhysics = true;
                        break;
                    case BUNDLE_4_MM_HUT_JIGGY:
                        if (spawnPosition[1] < 2000) {
                            randoCheckId = RC_MM_JIGGY_ORANGE_PADS;
                        } else {
                            randoCheckId = RC_MM_JIGGY_HUTS;
                            applyCustomPhysics = true;
                        }
                        break;
                    case BUNDLE_6_MM_HUT_EXTRA_LIFE:
                        randoCheckId = RC_MM_EXTRA_LIFE_HUT;
                        applyCustomPhysics = true;
                        break;
                    case BUNDLE_7__JIGGY:
                        randoCheckId = RC_MM_JIGGY_CHIMPY;
                        break;
                    case BUNDLE_10__JIGGY:
                        randoCheckId = RC_MM_JIGGY_JUJU;
                        break;
                    case BUNDLE_15__JIGGY:
                        randoCheckId = RC_MM_JIGGY_CONGA;
                        break;
                    case BUNDLE_18__HONEYCOMB:
                        if (spawnPosition[1] < 400) {
                            randoCheckId = 
                                (RandoCheckId)((int32_t)RC_MM_HONEYCOMB_BEEHIVE_BY_BIG_BUTT_1 + bundleCount);
                        } else {
                            randoCheckId = 
                                (RandoCheckId)((int32_t)RC_MM_HONEYCOMB_BEEHIVE_BY_THE_HUTS_1 + bundleCount);
                        }
                        break;
                    default:
                        break;
                }
                break;
            case LEVEL_2_TREASURE_TROVE_COVE:
                switch (bundleId) {
                    case BUNDLE_4_MM_HUT_JIGGY:
                        randoCheckId = RC_TTC_JIGGY_RED_X;
                        break;
                    case BUNDLE_7__JIGGY:
                        randoCheckId = RC_TTC_JIGGY_BLUBBER;
                        break;
                    default:
                        break;
                    case BUNDLE_18__HONEYCOMB:
                        if (position[1] < 900 && position[1] > 890) {
                            randoCheckId = 
                                (RandoCheckId)((int32_t)RC_TTC_HONEYCOMB_BEEHIVE_NEAR_BLUBBERS_SHIP_1 + bundleCount);
                        } else if (position[1] < 1770 && position[1] > 1765) {
                            randoCheckId = 
                                (RandoCheckId)((int32_t)RC_TTC_HONEYCOMB_BEEHIVE_NEAR_THE_BACK_STAIRS_1 + bundleCount);
                        } else if (position[1] < 1910 && position[1] > 1905) {
                            randoCheckId = 
                                (RandoCheckId)((int32_t)RC_TTC_HONEYCOMB_BEEHIVE_IN_THE_CLIFFSIDE_POOL_1 + bundleCount);
                        } else if (position[1] < 875 && position[1] > 870) {
                            randoCheckId = 
                                (RandoCheckId)((int32_t)RC_TTC_HONEYCOMB_BEEHIVE_ON_THE_BEACH_1 + bundleCount);
                        }
                        break;
                }
                break;
            case LEVEL_3_CLANKERS_CAVERN:
                switch (bundleId) {
                    case BUNDLE_10__JIGGY:
                        switch (spawnPosition[1]) {
                            case 1536:
                                randoCheckId = RC_CC_JIGGY_TOOTH;
                                break;
                            default:
                                randoCheckId = RC_CC_JIGGY_RINGS;
                                break;
                        }
                        break;
                    case BUNDLE_18__HONEYCOMB:
                        if (position[1] < 1405 && position[1] > 1395) {
                            randoCheckId = 
                                (RandoCheckId)((int32_t)RC_CC_HONEYCOMB_BEEHIVE_INSIDE_CLANKER_1 + bundleCount);
                        } else if (position[1] < 5495 && position[1] > 5490) {
                            randoCheckId = 
                                (RandoCheckId)((int32_t)RC_CC_HONEYCOMB_BEEHIVE_NEAR_THE_ENTRANCE_1 + bundleCount);
                        } else if (position[1] < 3820 && position[1] > 3810) {
                            randoCheckId = 
                                (RandoCheckId)((int32_t)RC_CC_HONEYCOMB_BEEHIVE_NEAR_THE_YELLOW_GRATES_1 + bundleCount);
                        }
                        break;
                    default:
                        break;
                }
                break;
            case LEVEL_4_BUBBLEGLOOP_SWAMP:
                switch (bundleId) {
                    case BUNDLE_6_MM_HUT_EXTRA_LIFE:
                        randoCheckId = (RandoCheckId)((int32_t)RC_BGS_EXTRA_LIFE_MR_VILE_1 + vileCount);
                        vileCount++;
                        applyCustomPhysics = true;
                        if (vileCount >= 3) {
                            vileCount = 0;
                        }
                        break;
                    case BUNDLE_7__JIGGY:
                        randoCheckId = RC_BGS_JIGGY_CROCTUS;
                        break;
                    case BUNDLE_8__JIGGY:
                        randoCheckId = RC_BGS_JIGGY_MR_VILE;
                        break;
                    case BUNDLE_9__JIGGY:
                        randoCheckId = RC_BGS_JIGGY_TANKTUP;
                        break;
                    case BUNDLE_B_BGS_HUT_MUSIC_NOTE:
                        randoCheckId = (RandoCheckId)((int32_t)RC_BGS_NOTE_HUT_BUNDLE_1 + bundleCount);
                        break;
                    case BUNDLE_C_BGS_HUT_JIGGY:
                        randoCheckId = RC_BGS_JIGGY_HUTS;
                        break;
                    case BUNDLE_10__JIGGY:
                        switch (spawnPosition[2]) {
                            case 49:
                                randoCheckId = RC_BGS_JIGGY_ELEVATED_WALKWAY;
                                break;
                            case -1386:
                                randoCheckId = RC_BGS_JIGGY_FLIBBITS;
                                break;
                            case 2799:
                                randoCheckId = RC_BGS_JIGGY_PINKEGG;
                                break;
                            case -1020:
                                randoCheckId = RC_BGS_JIGGY_TIPTUP;
                                break;
                            case -6148:
                                randoCheckId = RC_BGS_JIGGY_MAZE;
                                break;
                            default:
                                break;
                        }
                        break;
                    case BUNDLE_18__HONEYCOMB:
                        if (position[1] < 1260 && position[1] > 1250) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_BGS_HONEYCOMB_BEEHIVE_ELEVATED_WALKWAY_1 + bundleCount);
                        } else if (position[1] < 1160 && position[1] > 1150) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_BGS_HONEYCOMB_BEEHIVE_NEAR_MAZE_ENTRANCE_1 + bundleCount);
                        } else if (position[0] < 4390 && position[0] > 4385) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_BGS_HONEYCOMB_BEEHIVE_NEAR_TANKTUP_1 + bundleCount);
                        } else if (position[0] < 2565 && position[0] > 2560) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_BGS_HONEYCOMB_BEEHIVE_NEAR_WARP_PAD_1 + bundleCount);
                        }
                        break;
                    default:
                        break;
                }
                break;
            case LEVEL_5_FREEZEEZY_PEAK:
                switch (bundleId) {
                    case BUNDLE_9__JIGGY:
                        randoCheckId = RC_FP_JIGGY_SLED_TO_BOGGY;
                        break;
                    case BUNDLE_10__JIGGY:
                        if (spawnPosition[1] > 1900) {
                            randoCheckId = RC_FP_JIGGY_BOGGY_RACE_1;
                        } else if (spawnPosition[1] < 850) {
                            randoCheckId = RC_FP_JIGGY_WOZZA;
                        }
                        break;
                    case BUNDLE_7__JIGGY:
                        randoCheckId = RC_FP_JIGGY_BOGGY_RACE_2;
                        break;
                    case BUNDLE_18__HONEYCOMB:
                        if (position[1] < 820 && position[1] > 810) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_FP_HONEYCOMB_BEEHIVE_NEAR_RACE_START_1 + bundleCount);
                        } else if (position[1] < 590 && position[1] > 580) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_FP_HONEYCOMB_BEEHIVE_PRESENT_STACK_1 + bundleCount);
                        } else if (position[1] < 1755 && position[1] > 1745) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_FP_HONEYCOMB_BEEHIVE_SCARF_END_1 + bundleCount);
                        } else if (position[1] < 5760 && position[1] > 5750) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_FP_HONEYCOMB_BEEHIVE_SCARF_START_1 + bundleCount);
                        }
                        break;
                }
                break;
            case LEVEL_6_LAIR:
                switch (bundleId) {
                    case BUNDLE_C_BGS_HUT_JIGGY:
                        if (spawnPosition[1] ==
                            Rando::StaticData::Checks[RC_GL_JIGGY_WITCH_SWITCH_MUMBOS_MOUNTAIN].posY) {
                            randoCheckId = RC_GL_JIGGY_WITCH_SWITCH_MUMBOS_MOUNTAIN;
                            break;
                        }
                        if (spawnPosition[1] ==
                            Rando::StaticData::Checks[RC_GL_JIGGY_WITCH_SWITCH_RUSTY_BUCKET_BAY].posY) {
                            randoCheckId = RC_GL_JIGGY_WITCH_SWITCH_RUSTY_BUCKET_BAY;
                            break;
                        }
                        if (spawnPosition[1] ==
                            Rando::StaticData::Checks[RC_GL_JIGGY_WITCH_SWITCH_CLICK_CLOCK_WOOD].posY) {
                            randoCheckId = RC_GL_JIGGY_WITCH_SWITCH_CLICK_CLOCK_WOOD;
                            break;
                        }
                        if (spawnPosition[1] ==
                            Rando::StaticData::Checks[RC_GL_JIGGY_WITCH_SWITCH_CLANKERS_CAVERN].posY) {
                            randoCheckId = RC_GL_JIGGY_WITCH_SWITCH_CLANKERS_CAVERN;
                            break;
                        }
                        break;
                    case BUNDLE_10__JIGGY:
                        if (fileProgressFlag_get(FILEPROG_1A_TTC_WITCH_SWITCH_JIGGY_PRESSED) &&
                            gsworld_getMap() == MAP_6D_GL_TTC_LOBBY) {
                            randoCheckId = RC_GL_JIGGY_WITCH_SWITCH_TREASURE_TROVE_COVE;
                            auto spawnTuple = Rando::StaticData::multiSpawnCheckMap.at(randoCheckId);
                            spawnPosition[0] = std::get<0>(spawnTuple);
                            spawnPosition[1] = std::get<1>(spawnTuple);
                            spawnPosition[2] = std::get<2>(spawnTuple);
                        } else {
                            applyCustomPhysics = true;
                            randoCheckId = Rando::StaticData::GetCheckByPosition(spawnPosition[0], spawnPosition[1],
                                                                                 spawnPosition[2]);
                        }
                        break;
                    case BUNDLE_18__HONEYCOMB:
                        switch (currentMap) { 
                            case MAP_69_GL_MM_LOBBY:
                                randoCheckId =
                                    (RandoCheckId)((int32_t)RC_GL_HONEYCOMB_BEEHIVE_NEAR_50_NOTE_DOOR_1 + bundleCount);
                                break;
                            case MAP_6D_GL_TTC_LOBBY:
                                randoCheckId =
                                    (RandoCheckId)((int32_t)RC_GL_HONEYCOMB_BEEHIVE_NEAR_TTC_ENTRANCE_1 + bundleCount);
                                break;
                            case MAP_6F_GL_FP_LOBBY:
                                if (position[1] < 295 && position[1] > 285) {
                                    randoCheckId = 
                                        (RandoCheckId)((int32_t)RC_GL_HONEYCOMB_BEEHIVE_BIG_GRUNTY_STATUE_LEFT_1 + bundleCount);
                                } else if (position[1] < 255 && position[1] > 245) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_GL_HONEYCOMB_BEEHIVE_BIG_GRUNTY_STATUE_RIGHT_1 + bundleCount);
                                }
                                break;
                            case MAP_70_GL_CC_LOBBY:
                                randoCheckId =
                                    (RandoCheckId)((int32_t)RC_GL_HONEYCOMB_BEEHIVE_NEAR_CC_ENTRANCE_1 + bundleCount);
                                break;
                            case MAP_71_GL_STATUE_ROOM:
                                randoCheckId =
                                    (RandoCheckId)((int32_t)RC_GL_HONEYCOMB_BEEHIVE_NEAR_FIRST_GRUNTY_STATUE_1 + bundleCount);
                                break;
                            case MAP_72_GL_BGS_LOBBY:
                                randoCheckId =
                                    (RandoCheckId)((int32_t)RC_GL_HONEYCOMB_BEEHIVE_BEHIND_BGS_ENTRANCE_1 + bundleCount);
                                break; 
                            case MAP_75_GL_MMM_LOBBY:
                                randoCheckId =
                                    (RandoCheckId)((int32_t)RC_GL_HONEYCOMB_BEEHIVE_BEHIND_MMM_ENTRANCE_1 + bundleCount);
                                break;
                            case MAP_78_GL_RBB_AND_MMM_PUZZLE:
                                randoCheckId =
                                    (RandoCheckId)((int32_t)RC_GL_HONEYCOMB_BEEHIVE_NEAR_RBB_PUZZLE_1 + bundleCount);
                                break;
                            case MAP_79_GL_CCW_LOBBY:
                                randoCheckId =
                                    (RandoCheckId)((int32_t)RC_GL_HONEYCOMB_BEEHIVE_NEAR_CCW_PUZZLE_SWITCH_1 + bundleCount);
                                break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case LEVEL_7_GOBIS_VALLEY:
                switch (bundleId) {
                    case BUNDLE_10__JIGGY:
                        randoCheckId =
                            Rando::StaticData::GetCheckByPosition(spawnPosition[0], spawnPosition[1], spawnPosition[2]);
                        break;
                    case BUNDLE_D__EMPTY_HONEYCOMB:
                        randoCheckId = RC_GV_EMPTY_HONEYCOMB_GOBI;
                    case BUNDLE_18__HONEYCOMB:
                        if (position[1] < 2990 && position[1] > 2980) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_GV_HONEYCOMB_BEEHIVE_BEHIND_WATER_PYRAMID_1 + bundleCount);
                        } else if (position[1] < 1990 && position[1] > 1980) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_GV_HONEYCOMB_BEEHIVE_NEAR_PUZZLE_PYRAMID_1 + bundleCount);
                        } else if (position[1] < 610 && position[1] > 600) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_GV_HONEYCOMB_BEEHIVE_NEAR_WARP_PAD_1 + bundleCount);
                        }
                        break;
                    default:
                        break;
                }
                break;
            case LEVEL_8_CLICK_CLOCK_WOOD:
                switch (bundleId) {
                    case BUNDLE_18__HONEYCOMB:
                        switch (currentMap) { 
                            case MAP_40_CCW_HUB:
                                if (position[1] < 150 && position[1] > 140) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_ENTRANCE_LEFT_OF_SUMMER_DOOR_1 +
                                                       bundleCount);
                                } else if (position[1] < 160 && position[1] > 150) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_ENTRANCE_RIGHT_OF_SUMMER_DOOR_1 +
                                                       bundleCount);
                                }
                                break;
                            case MAP_43_CCW_SPRING:
                                if (position[0] < 5 && position[0] > -5) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_SPRING_BY_THE_BIG_FLOWER_1 +
                                                       bundleCount);
                                } else if (position[0] < -1545 && position[0] > -1555) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_SPRING_GRASS_NEAR_THE_ENTRANCE_1 +
                                                       bundleCount);
                                } else if (position[0] < 4345 && position[0] > 4340) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_SPRING_UNDER_THE_TREEHOUSE_1 +
                                                       bundleCount);
                                }
                                break;
                            case MAP_44_CCW_SUMMER:
                                if (position[1] < -440 && position[1] > -450) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_DRIED_UP_LAKE_1 +
                                                       bundleCount);
                                } else if (position[1] < 160 && position[1] > 150) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_GRASS_NEAR_THE_ENTRANCE_1 +
                                                       bundleCount);
                                } else if (position[1] < 4580 && position[1] > 4570) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_SUMMER_OUTSIDE_NABNUTS_1 +
                                                       bundleCount);
                                }
                                break;
                            case MAP_45_CCW_AUTUMN:
                                if (position[1] < 1435 && position[1] > 1425) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_ABOVE_THE_LAKE_1 +
                                                       bundleCount);
                                } else if (position[1] < 160 && position[1] > 150) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_GRASS_NEAR_THE_ENTRANCE_1 +
                                                       bundleCount);
                                } else if (position[1] < 4405 && position[1] > 4395) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_AUTUMN_IN_THE_TREEHOUSE_1 +
                                                       bundleCount);
                                }
                                break;
                            case MAP_46_CCW_WINTER:
                                if (position[1] < 1060 && position[1] > 1050) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_WINTER_AROUND_THE_TREE_BASE_1 +
                                                       bundleCount);
                                } else if (position[1] < 4405 && position[1] > 4395) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_WINTER_OUTSIDE_THE_TREEHOUSE_1 +
                                                       bundleCount);
                                }
                                break;
                            case MAP_4D_CCW_WINTER_MUMBOS_SKULL:
                                randoCheckId =
                                    (RandoCheckId)((int32_t)RC_CCW_HONEYCOMB_BEEHIVE_WINTER_INSIDE_MUMBOS_SKULL_1 +
                                                   bundleCount);
                                break;
                        }
                    default:
                        break;
                }
                break;
            case LEVEL_9_RUSTY_BUCKET_BAY:
                switch (bundleId) {
                    case BUNDLE_6_MM_HUT_EXTRA_LIFE:
                        randoCheckId = RC_RBB_EXTRA_LIFE_BOOM_BOXES;
                        applyCustomPhysics = true;
                        break;
                    case BUNDLE_18__HONEYCOMB:
                        if (position[0] == 0 && position[1] == 156 && position[2] == 4800) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_RBB_HONEYCOMB_BEEHIVE_ENGINE_ROOM_ENTRANCE_1 + bundleCount);
                        } else if (position[0] == 0 && position[1] == 156 && position[2] == -178) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_RBB_HONEYCOMB_BEEHIVE_FIRST_SHIPPING_CONTAINER_1 + bundleCount);
                        } else if (position[0] == 7850 && position[1] == -1019 && position[2] == 3550) {
                            randoCheckId =
                                (RandoCheckId)((int32_t)RC_RBB_HONEYCOMB_BEEHIVE_GRATE_ABOVE_PINK_JINJO_1 + bundleCount);
                        }
                        break;
                    default:
                        break;
                }
            case LEVEL_A_MAD_MONSTER_MANSION:
                switch (bundleId) { 
                    case BUNDLE_18__HONEYCOMB:
                        switch (currentMap) { 
                            case MAP_1C_MMM_CHURCH:
                                if (position[0] < 1880 && position[0] > 1870) {
                                    randoCheckId = 
                                        (RandoCheckId)((int32_t)RC_MMM_HONEYCOMB_BEEHIVE_CHURCH_ORGAN_RIGHT_1 + bundleCount);
                                } else if (position[0] < -1870 && position[0] > -1880) {
                                    randoCheckId = 
                                        (RandoCheckId)((int32_t)RC_MMM_HONEYCOMB_BEEHIVE_CHURCH_ORGAN_LEFT_1 + bundleCount);
                                }
                                break;
                            case MAP_1B_MMM_MAD_MONSTER_MANSION:
                                if (position[1] < 160 && position[1] > 150) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_MMM_HONEYCOMB_BEEHIVE_IN_THE_MAZE_1 + bundleCount);
                                } else if (position[1] < 335 && position[1] > 325) {
                                    randoCheckId =
                                        (RandoCheckId)((int32_t)RC_MMM_HONEYCOMB_BEEHIVE_IN_THE_TALL_GRASS_1 +
                                                       bundleCount);
                                }
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case LEVEL_B_SPIRAL_MOUNTAIN:
                switch (bundleId) {
                    case BUNDLE_1F_SM_EMPTY_HONEYCOMB:
                        if (spawnPosition[1] >= 500 && spawnPosition[1] <= 800) {
                            randoCheckId = RC_SM_EMPTY_HONEYCOMB_COLLIWOBBLE;
                        } else {
                            randoCheckId = RC_SM_EMPTY_HONEYCOMB_QUARRIES;
                        }
                        break;
                    case BUNDLE_18__HONEYCOMB:
                        break;
                    default:
                        break;
                }
                break;
            default:
                return;
        }

        if (randoCheckId == RC_UNKNOWN) {
            return;
        }

        if (!Rando::Logic::IsCheckShuffled(randoCheckId)) {
            return;
        }

        RandoSaveCheck randoSaveCheck = RANDO_SAVE_CHECKS[randoCheckId];
        if (randoSaveCheck.obtained) {
            actor_e actorId = (actor_e)Rando::StaticData::Items[randoSaveCheck.randoItemId].actorId;
            *actor =
                CustomObject::SpawnCustomActorEX(randoCheckId, spawnPosition, &actorInfoMap.at((actor_e)actorId).first,
                                                 actorInfoMap.at((actor_e)actorId).second);

            bundleDespawnQueue.push_back((*actor)->marker);
        } else {
            *actor = CustomObject::ShouldCreateCustomActorEX(randoCheckId, spawnPosition, false);

            if (*actor == NULL) {
                *should = true;
                return;
            }

            if (applyCustomPhysics) {
                ApplyCustomActorPhysics(randoCheckId, *actor, false);
            } else {
                ApplyBundleActorPhysics(*actor, bundleId, (BundleInfo*)bundleInfo, gBundle_yaw);
            }
        }

        *should = true;
    })
}
