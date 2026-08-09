#ifndef RANDO_STATIC_DATA_H
#define RANDO_STATIC_DATA_H

#include <map>
#include <unordered_map>
#include <array>
#include <string>
#include "port/Rando/Types.h"

#include "enums.h"

namespace Rando {

namespace StaticData {
void SendCollisionNotification(RandoCheckId randoCheckId);
void SendRemoteCheckNotification(RandoCheckId randoCheckId, const std::string& collectorName);

struct RandoLogicData {
    const char* name;
    bool canAccess;
    bool isFilled;
    bool isShuffled;
};

struct RandoStaticCheck {
    RandoCheckId randoCheckId;
    const char* name;
    RandoCheckType randoCheckType;
    int32_t worldId;
    RandoItemId randoItemId;
    int32_t collectionId;
    int32_t posX;
    int32_t posY;
    int32_t posZ;
};

RandoCheckId GetCheckByPosition(int32_t posX, int32_t posY, int32_t posZ);
RandoCheckId GetCheckByMumboTokenId(mumbotoken_e tokenId);
RandoCheckId GetCheckByHoneycombId(honeycomb_e honeycombId);
RandoCheckId GetCheckByJiggyId(int32_t jiggyId);
RandoCheckId GetJinjoJiggyCheckByLevelId(int16_t levelId);
RandoCheckId GetCheckByAbilityId(int32_t abilityId);

extern std::map<RandoCheckId, RandoStaticCheck> Checks;
extern std::map<RandoCheckId, std::tuple<int32_t, int32_t, int32_t>> multiSpawnCheckMap;

struct RandoStaticItem {
    RandoItemId randoItemId;
    const char* spoilerName;
    const char* article;
    const char* name;
    RandoItemType randoItemType;
    int16_t actorId;
    int16_t worldId;
};

RandoItemId GetRandoItemByActorId(actor_e actorId);
actor_e GetActorIdByRandoItemId(RandoItemId randoItemId);

extern std::map<RandoItemId, RandoStaticItem> Items;

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

struct RandoStaticFlag {
    RandoInf randoFlagId;
    const char* name;
    int32_t defaultValue;
};

extern std::map<RandoInf, RandoStaticFlag> Flags;

void ModifyRandoInfFlagState(RandoCheckId randoCheckId);

extern std::unordered_map<std::string, RandoCheckId> locationNameToEnum;
extern std::unordered_map<std::string, RandoItemId> itemNameToEnum;

} // namespace StaticData

} // namespace Rando

#endif // RANDO_STATIC_DATA_H
