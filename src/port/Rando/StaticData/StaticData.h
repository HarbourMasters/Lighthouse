#ifndef RANDO_STATIC_DATA_H
#define RANDO_STATIC_DATA_H

#include <map>
#include <array>
#include <string>
#include "port/Rando/Types.h"

#include "enums.h"

namespace Rando {

namespace StaticData {

struct RandoShuffledPool {
    RandoCheckId randoCheckId;
    RandoCheckId shuffleCheckId;
    RandoItemId randoItemId;
    int32_t randoCollectionId;
    bool isShuffled;
    bool obtained;
    bool skipped;
};

struct RandoStaticCheck {
    RandoCheckId randoCheckId;
    const char* name;
    RandoCheckType randoCheckType;
    int32_t worldId;
    int32_t actorId;
    int32_t collectionId;
    int32_t posX;
    int32_t posY;
    int32_t posZ;
};

RandoCheckId GetCheckByPosition(int32_t posX, int32_t posY, int32_t posZ);
RandoCheckId GetCheckByJiggyId(int32_t jiggyId);
RandoCheckId GetJinjoJiggyCheckByLevelId(int16_t levelId);

extern std::map<RandoCheckId, RandoStaticCheck> Checks;
// extern RandoStaticCheck GetShuffledRandoStaticCheck(s16 x, s16 y, s16 z);

struct RandoStaticItem {
    RandoItemId randoItemId;
    const char* spoilerName;
    const char* article;
    const char* name;
    RandoItemType randoItemType;
    int16_t actorId;
};

RandoItemId GetRandoItemByActorId(actor_e actorId);

extern std::map<RandoItemId, RandoStaticItem> Items;

// int16_t GetModelByRandoItem(RandoItemId randoItem);
// const BehaviorScript *GetBehaviorByModel(int16_t modelId);
// int16_t GetModelByBehavior(const BehaviorScript* behavior);
// RandoItemId GetShuffledRandoItem(RandoCheckId randoCheckId);
// RandoAct GetShuffledRandoAct(RandoCheckId randoCheckId);

// struct RandoStaticEntrance {
//     RandoEntranceId randoEntranceId;
//     const char* name;
//     int16_t destinationId;
//     RandoEntranceType randoEntranceType;
//     WarpNodes deathWarpId;
//     int16_t deathArea;
// };

// extern std::map<RandoEntranceId, RandoStaticEntrance> Entrances;

// RandoEntranceId GetEntranceIdFromDestination(int16_t destinationId);

struct RandoStaticOption {
    RandoOptionId randoOptionId;
    const char* name;
    const char* cvar;
    int32_t defaultValue;
};

extern std::map<RandoOptionId, RandoStaticOption> Options;
// extern std::unordered_map<int32_t, const char*> logicOptions;

RandoOptionId GetOptionIdFromName(const char* name);

// TODO: Add Logic and Regions
// struct RandoStaticRegion {
//     RandoRegionId randoRegionId;
//     const char* name;
//     int16_t levelId;
//     std::map<RandoCheckId, std::function<bool()>> checks;
//     std::map<RandoRegionId, std::function<bool()>> regions;
// };

// extern std::map<RandoRegionId, RandoStaticRegion> Regions;

} // namespace StaticData

} // namespace Rando

#endif // RANDO_STATIC_DATA_H
