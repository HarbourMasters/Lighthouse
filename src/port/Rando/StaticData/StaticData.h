#ifndef RANDO_STATIC_DATA_H
#define RANDO_STATIC_DATA_H

#include "port/UI/UIWidgets.hpp"
#include <map>
#include <array>
#include <string>
#include "port/Rando/Types.h"

#include "enums.h"

namespace Rando {

namespace StaticData {

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
extern std::unordered_map<actor_e, UIWidgets::Colors> ItemColors;
extern std::unordered_map<RandoItemId, UIWidgets::Colors> SnsColors;
extern std::unordered_map<int16_t, RandoCheckId> JinjoJiggyChecks;

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
