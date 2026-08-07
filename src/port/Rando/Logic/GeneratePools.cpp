#include "Logic.h"
// #include "port/Rando/Spoiler/Spoiler.h"
#include "port/UI/Notification.h"
#include "port/UI/cvar_prefixes.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include <sstream>
#include <random>

#include "enums.h"

int32_t randoFinalSeed = 0;

namespace Rando {

namespace Logic {
std::vector<RandoCheckId> checkPool;
std::vector<std::tuple<actor_e, int32_t, RandoCheckId>> itemPool;

std::vector<RandoCheckId> abilityCheckPool;
std::vector<std::tuple<actor_e, int32_t, RandoCheckId>> abilityItemPool;

std::vector<RandoSaveCheck> shuffledPool;

uint32_t GetRandoSeed(const std::string& input) {
    if (CVarGetInteger(CVAR_RANDOMIZER_SETTING("ManualInput"), 0) && !input.empty()) {
        return Ship_Hash(input);
    }
    std::string randoHash = std::to_string(GetUnixTimestamp());
    return Ship_Hash(randoHash);
}

void ShuffleRandoItems(const std::string& input, std::vector<std::tuple<actor_e, int32_t, RandoCheckId>>& pool) {
    uint32_t seed = GetRandoSeed(input);

    std::mt19937 rando(seed);
    std::shuffle(pool.begin(), pool.end(), rando);

    randoFinalSeed = seed;
}

void GenerateShufflePool(SaveData* saveData) {
    checkPool.clear();
    itemPool.clear();
    abilityCheckPool.clear();
    abilityItemPool.clear();
    shuffledPool.clear();

    for (auto& [randoCheckId, randoStaticCheck] : Rando::StaticData::Checks) {
        if (randoStaticCheck.randoCheckType == RCTYPE_BLUE_EGG &&
            CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_BLUE_EGGS].cvar, 0) == RO_GENERIC_OFF) {
            continue;
        }

        if (randoStaticCheck.randoCheckType == RCTYPE_EMPTY_HONEYCOMB &&
            CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_EMPTY_HONEYCOMBS].cvar, 0) == RO_GENERIC_OFF) {
            continue;
        }

        if (randoStaticCheck.randoCheckType == RCTYPE_EXTRA_LIFE &&
            CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_EXTRA_LIVES].cvar, 0) == RO_GENERIC_OFF) {
            continue;
        }

        if (randoStaticCheck.randoCheckType == RCTYPE_HONEYCOMB &&
            CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_BEEHIVE_HONEYCOMBS].cvar, 0) == RO_GENERIC_OFF) {
            continue;
        }

        if (randoStaticCheck.randoCheckType == RCTYPE_JIGGY &&
            CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_JIGGIES].cvar, 0) == RO_GENERIC_OFF) {
            continue;
        }

        if (randoStaticCheck.randoCheckType == RCTYPE_JINJO &&
            CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_JINJOS].cvar, 0) == RO_GENERIC_OFF) {
            continue;
        }

        if (randoStaticCheck.randoCheckType == RCTYPE_MOLEHILL) {
            if (CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_MOLEHILLS].cvar, 0) == RO_GENERIC_ON) {
                abilityCheckPool.push_back(randoCheckId);
                abilityItemPool.push_back({ Rando::StaticData::GetActorIdByRandoItemId(randoStaticCheck.randoItemId),
                                            randoStaticCheck.collectionId, randoCheckId });
            }
            continue;
        }

        if (randoStaticCheck.randoCheckType == RCTYPE_MUMBO_TOKEN &&
            CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_MUMBO_TOKENS].cvar, 0) == RO_GENERIC_OFF) {
            continue;
        }

        if (randoStaticCheck.randoCheckType == RCTYPE_MUSIC_NOTE &&
            CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_MUSIC_NOTES].cvar, 0) == RO_GENERIC_OFF) {
            continue;
        }

        if (randoStaticCheck.randoCheckType == RCTYPE_STOP_N_SWOP &&
            CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_STOP_N_SWOP].cvar, 0) == RO_GENERIC_OFF) {
            continue;
        }

        checkPool.push_back(randoCheckId);
        itemPool.push_back({ Rando::StaticData::GetActorIdByRandoItemId(randoStaticCheck.randoItemId),
                             randoStaticCheck.collectionId, randoCheckId });
    }

    if (!itemPool.empty()) {
        Rando::Logic::ShuffleRandoItems(CVarGetString(CVAR_RANDOMIZER_SETTING("InputSeed"), ""), itemPool);
    }

    if (!abilityItemPool.empty()) {
        Rando::Logic::ShuffleRandoItems(CVarGetString(CVAR_RANDOMIZER_SETTING("InputSeed"), ""), abilityItemPool);
    }

    if (RANDO_SAVE_OPTIONS[RO_LOGIC].optionValue == RO_LOGIC_GLITCHLESS) {
        Rando::Logic::GenerateGlitchlessLogicPool(checkPool, itemPool, abilityCheckPool, abilityItemPool, saveData);
    } else if (RANDO_SAVE_OPTIONS[RO_LOGIC].optionValue == RO_LOGIC_NO_LOGIC) {
        Rando::Logic::GenerateNoLogicPool(itemPool, abilityItemPool);
    }

    if (checkPool.size() != itemPool.size()) {
        return;
    }

    for (int i = 0; i < checkPool.size(); i++) {
        RandoSaveCheck randoShuffleEntry = {
            .name = Rando::StaticData::Checks[checkPool[i]].name,
            .randoCheckId = checkPool[i],
            .randoItemId = Rando::StaticData::Checks[std::get<2>(itemPool[i])].randoItemId,
            .randoCollectionId = std::get<1>(itemPool[i]),
            .isShuffled = checkPool[i] == RC_UNKNOWN ? false : true,
            .eligible = false,
            .skipped = false,
        };

        shuffledPool.push_back(randoShuffleEntry);
        RANDO_SAVE_CHECKS[checkPool[i]] = randoShuffleEntry;
    }

    if (!abilityCheckPool.empty()) {
        for (int a = 0; a < abilityCheckPool.size(); a++) {
            RandoSaveCheck randoShuffleEntry = {
                .name = Rando::StaticData::Checks[abilityCheckPool[a]].name,
                .randoCheckId = abilityCheckPool[a],
                .randoItemId = Rando::StaticData::Checks[std::get<2>(abilityItemPool[a])].randoItemId,
                .randoCollectionId = std::get<1>(abilityItemPool[a]),
                .isShuffled = true,
                .eligible = false,
                .skipped = false,
            };

            shuffledPool.push_back(randoShuffleEntry);
            RANDO_SAVE_CHECKS[abilityCheckPool[a]] = randoShuffleEntry;
        }
    }

    for (auto& [optionId, optionValue] : Rando::StaticData::Options) {
        RANDO_SAVE_OPTIONS[optionId].name = optionValue.name;
        RANDO_SAVE_OPTIONS[optionId].optionValue = CVarGetInteger(optionValue.cvar, optionValue.defaultValue);
    }

    saveData->shipSaveData.randoSaveData.seedId = randoFinalSeed;
}

void GeneratePoolFromSaveData(SaveData* saveData) {
    for (int i = RC_UNKNOWN; i < RC_MAX; i++) {
        RandoSaveCheck randoSaveCheck = saveData->shipSaveData.randoSaveData.randoSaveCheck[i];

        if (randoSaveCheck.isShuffled == false) {
            continue;
        }

        RandoSaveCheck shuffledObject = {
            .name = Rando::StaticData::Checks[(RandoCheckId)i].name,
            .randoCheckId = randoSaveCheck.randoCheckId,
            .randoItemId = randoSaveCheck.randoItemId,
            .randoCollectionId = randoSaveCheck.randoCollectionId,
            .isShuffled = randoSaveCheck.isShuffled,
            .eligible = randoSaveCheck.eligible,
            .received = randoSaveCheck.received,
            .skipped = randoSaveCheck.skipped,
        };

        Rando::Logic::shuffledPool.push_back(shuffledObject);
    }
}

} // namespace Logic

} // namespace Rando