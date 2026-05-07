#ifndef RANDO_LOGIC_H
#define RANDO_LOGIC_H

#include "port/Rando/Rando.h"
#include "port/ShipUtils.h"

extern "C" {
s32 item_getCount(enum item_e item);

int ability_isUnlocked(enum ability_e uid);

bool fileProgressFlag_get(enum file_progress_e index);
s32 __transformation_getCost(enum transformation_e trans_id);
}

namespace Rando {

namespace Logic {
extern std::vector<Rando::StaticData::RandoShuffledPool> shuffledPool;

void GenerateShufflePool();

inline bool IsCheckShuffled(RandoCheckId randoCheckId) {
    bool isShuffled = false;

    for (auto& object : shuffledPool) {
        if (object.randoCheckId == randoCheckId) {
            isShuffled = object.isShuffled;
            break;
        }
    }

    return isShuffled;
}

inline Rando::StaticData::RandoShuffledPool GetShuffledObject(RandoCheckId randoCheckId) {
    Rando::StaticData::RandoShuffledPool shuffledObject;
    shuffledObject.randoCheckId = RC_UNKNOWN;

    if (!IsCheckShuffled(randoCheckId)) {
        return shuffledObject;
    }

    for (auto& object : shuffledPool) {
        if (object.randoCheckId == randoCheckId) {
            shuffledObject = object;
            break;
        }
    }

    return shuffledObject;
}

inline bool IsCheckObtained(RandoCheckId randoCheckId) {
    bool isObtained = false;

    for (auto& object : shuffledPool) {
        if (object.randoCheckId == randoCheckId) {
            isObtained = object.obtained;
            break;
        }
    }

    return isObtained;
}

inline bool ShouldSpawnJinjoJiggy(int16_t levelId) {
    bool shouldSpawn = false;
    int16_t jinjoCount = 0;

    for (auto& pool : Rando::Logic::shuffledPool) {
        if (!pool.obtained) {
            continue;
        }

        if (Rando::StaticData::Checks[pool.shuffleCheckId].worldId != levelId) {
            continue;
        }

        if (pool.randoItemId >= RI_JINJO_BLUE && pool.randoItemId <= RI_JINJO_YELLOW) {
            jinjoCount++;
        }
    }

    if (jinjoCount == 5) {
        shouldSpawn = true;
    }

    return shouldSpawn;
}

// Regions
inline std::string LogicString(std::string condition) {
    if (condition == "true")
        return "";

    return condition;
}

struct RandoRegion {
    const char* regionName;
    int16_t levelId;
    std::map<RandoCheckId, std::pair<std::function<bool()>, std::string>> checks;
    std::map<RandoRegionId, std::pair<std::function<bool()>, std::string>> connections;
};

extern std::map<RandoRegionId, RandoRegion> Regions;

#define CHECK(check, condition)                               \
    {                                                         \
        check, {                                              \
            [] { return condition; }, LogicString(#condition) \
        }                                                     \
    }

#define CONNECTION(region, condition)                         \
    {                                                         \
        region, {                                             \
            [] { return condition; }, LogicString(#condition) \
        }                                                     \
    }

// Check Logic
inline bool CanUseTransformation(transformation_e transId) {
    file_progress_e progressId = (file_progress_e)((transId - TRANSFORM_2_TERMITE) + FILEPROG_90_PAID_TERMITE_COST);

    if (fileProgressFlag_get(progressId)) {
        return true;
    } else {
        if (__transformation_getCost(transId) <= item_getCount(ITEM_1C_MUMBO_TOKEN)) {
            return true;
        }
    }
    return false;
}

#define CAN_USE_ABILITY(abilityId) ability_isUnlocked(abilityId)
#define CAN_USE_TRANSFORMATION(transId) CanUseTransformation(transId)
#define CAN_EXTEND_JUMP_DISTANCE                                                           \
    (CAN_USE_ABILITY(ABILITY_7_FEATHERY_FLAP) || CAN_USE_ABILITY(ABILITY_B_RATATAT_RAP) || \
     CAN_USE_ABILITY(ABILITY_10_TALON_TROT))
#define CAN_ATTACK                                                                                                  \
    (CAN_USE_ABILITY(ABILITY_B_RATATAT_RAP) || CAN_USE_ABILITY(ABILITY_4_CLAW_SWIPE) ||                             \
     CAN_USE_ABILITY(ABILITY_6_EGGS) || CAN_USE_ABILITY(ABILITY_2_BEAK_BUSTER) || CAN_USE_ABILITY(ABILITY_C_ROLL) || \
     CAN_USE_ABILITY(ABILITY_12_WONDERWING))


} // namespace Logic

} // namespace Rando

#endif // RANDO_LOGIC_H