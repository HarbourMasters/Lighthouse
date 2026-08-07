#include "Logic.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/UI/Notification.h"

extern "C" f32 itemPrintValues[0x2C];
extern "C" s32 D_80385F30[0x2C];

typedef struct {
    int32_t actorId;
    int32_t collectionId;
    RandoCheckId shuffledCheckId;
} PlacedCheckObject;

typedef struct {
    int32_t noteCount;
    int32_t jiggyCount;
    int32_t mumboTokenCount;
} PlacedItemCounts;

typedef struct {
    int32_t itemType;
    int32_t itemId;
    int32_t itemCount;
} ProgressionItemData;

typedef struct {
    RandoAccessId progID;
    std::vector<int32_t> abilityIds;
    bool isComplete;
} ProgressionAbilityData;

typedef struct {
    RandoAccessId progId;
    std::vector<ProgressionItemData> itemData;
} ProgressionBaseId;

// clang-format off
std::vector<ProgressionBaseId> progressionItems = {
    { RA_NOTE_DOOR_50,  { { RITYPE_MUSIC_NOTE,  ACTOR_51_MUSIC_NOTE,    50 },
                          { RITYPE_JIGGY,       ACTOR_46_JIGGY,         1 },
                          { RITYPE_MUMBO_TOKEN, ACTOR_2D_MUMBO_TOKEN,   5 } } },

    { RA_NOTE_DOOR_180, { { RITYPE_MUSIC_NOTE,  ACTOR_51_MUSIC_NOTE,    180 },
                          { RITYPE_JIGGY,       ACTOR_46_JIGGY,         8 },
                          { RITYPE_MUMBO_TOKEN, ACTOR_2D_MUMBO_TOKEN,   0 } } },
    
    { RA_NOTE_DOOR_260, { { RITYPE_MUSIC_NOTE,  ACTOR_51_MUSIC_NOTE,    260 },
                          { RITYPE_JIGGY,       ACTOR_46_JIGGY,         15 },
                          { RITYPE_MUMBO_TOKEN, ACTOR_2D_MUMBO_TOKEN,   15 } } },
    
    { RA_NOTE_DOOR_450, { { RITYPE_MUSIC_NOTE,  ACTOR_51_MUSIC_NOTE,    450 },
                          { RITYPE_JIGGY,       ACTOR_46_JIGGY,         32 },
                          { RITYPE_MUMBO_TOKEN, ACTOR_2D_MUMBO_TOKEN,   30 } } },
    
    { RA_NOTE_DOOR_640, { { RITYPE_MUSIC_NOTE,  ACTOR_51_MUSIC_NOTE,    640 },
                          { RITYPE_JIGGY,       ACTOR_46_JIGGY,         54 },
                          { RITYPE_MUMBO_TOKEN, ACTOR_2D_MUMBO_TOKEN,   50 } } },
    
    { RA_NOTE_DOOR_765, { { RITYPE_MUSIC_NOTE,  ACTOR_51_MUSIC_NOTE,    765 },
                          { RITYPE_JIGGY,       ACTOR_46_JIGGY,         69 },
                          { RITYPE_MUMBO_TOKEN, ACTOR_2D_MUMBO_TOKEN,   75 } } },
    
    { RA_NOTE_DOOR_810, { { RITYPE_MUSIC_NOTE,  ACTOR_51_MUSIC_NOTE,    810 },
                          { RITYPE_JIGGY,       ACTOR_46_JIGGY,         94 },
                          { RITYPE_MUMBO_TOKEN, ACTOR_2D_MUMBO_TOKEN,   0 } } },
};

std::vector<ProgressionAbilityData> progressionAbilities = {
    { RA_NOTE_DOOR_50,  { ABILITY_10_TALON_TROT, ABILITY_8_FLAP_FLIP, ABILITY_6_EGGS },     false },
    { RA_NOTE_DOOR_180, { ABILITY_2_BEAK_BUSTER, ABILITY_D_SHOCK_JUMP, ABILITY_F_DIVE },    false },
    { RA_NOTE_DOOR_260, { ABILITY_E_WADING_BOOTS, ABILITY_5_CLIMB },                        false },
    { RA_NOTE_DOOR_450, { ABILITY_9_FLIGHT },                                               false },
    { RA_NOTE_DOOR_640, { ABILITY_F_DIVE },                                                 false },
    { RA_NOTE_DOOR_765, { ABILITY_2_BEAK_BUSTER },                                          false },
    { RA_NOTE_DOOR_810, { ABILITY_1_BEAK_BOMB },                                            false },
};
// clang-format on

bool failSafeTrigger = false;
int32_t prevProgressionIndex = -1;
std::vector<RandoCheckId> jinjoCheckIds;

void UpdateSaveDataItemCounts(PlacedItemCounts itemCounts) {
    D_80385F30[ITEM_C_NOTE] = itemCounts.noteCount;
    D_80385F30[ITEM_E_JIGGY] = itemCounts.jiggyCount;
    D_80385F30[ITEM_1C_MUMBO_TOKEN] = itemCounts.mumboTokenCount;
    D_80385F30[ITEM_25_MUMBO_TOKEN_TOTAL] = itemCounts.mumboTokenCount;

    switch (itemCounts.mumboTokenCount) {
        case 5:
            fileProgressFlag_set(FILEPROG_90_PAID_TERMITE_COST, 1);
            break;
        case 10:
            fileProgressFlag_set(FILEPROG_93_PAID_CROC_COST, 1);
            break;
        case 15:
            fileProgressFlag_set(FILEPROG_92_PAID_WALRUS_COST, 1);
            break;
        case 20:
            fileProgressFlag_set(FILEPROG_91_PAID_PUMPKIN_COST, 1);
            break;
        case 25:
            fileProgressFlag_set(FILEPROG_94_PAID_BEE_COST, 1);
            break;
        default:
            break;
    }
}

int32_t GetRandomCheckIndexS(Rando::StaticData::RandoLogicData (&checks)[RC_MAX], RandoCheckType checkType,
                             bool shouldExclude, bool shouldRestrict, bool gameComplete) {
    std::vector<int32_t> availableIndex;
    availableIndex.reserve(RC_MAX);

    for (int i = RC_UNKNOWN; i < RC_MAX; ++i) {
        if (i == RC_UNKNOWN) {
            continue;
        }

        if (checks[i].canAccess || gameComplete) {
            if (!checks[i].isFilled && checks[i].isShuffled) {
                if (shouldExclude) {
                    if (Rando::StaticData::Checks[(RandoCheckId)i].randoCheckType != checkType) {
                        availableIndex.push_back(i);
                        continue;
                    }
                }
                if (shouldRestrict) {
                    if (Rando::StaticData::Checks[(RandoCheckId)i].randoCheckType == checkType) {
                        availableIndex.push_back(i);
                        continue;
                    }
                }
            }
        }
    }

    if (availableIndex.empty()) {
        return -1;
    }

    srand(randoFinalSeed);
    int32_t randomCheck = rand() % availableIndex.size();

    return availableIndex[randomCheck];
}

int32_t GetRandomItemIndexS(std::vector<std::tuple<actor_e, int32_t, RandoCheckId>> items, actor_e actorId) {
    std::vector<int32_t> availableIndex;
    availableIndex.reserve(items.size());

    for (int i = 0; i < items.size(); ++i) {
        if (std::get<2>(items[i]) == RC_UNKNOWN) {
            continue;
        }

        if (actorId != ACTOR_1_UNKNOWN) {
            if (std::get<0>(items[i]) == actorId) {
                availableIndex.push_back(i);
                continue;
            }
        } else {
            availableIndex.push_back(i);
            continue;
        }
    }

    if (availableIndex.empty()) {
        return -1;
    }

    srand(randoFinalSeed);
    int32_t randomItem = rand() % availableIndex.size();

    return availableIndex[randomItem];
}

int32_t GetCheckPoolJinjoJiggyIndexByLevelId(int16_t levelId) {
    for (int i = 0; i < Rando::Logic::checkPool.size(); i++) {
        Rando::StaticData::RandoStaticCheck randoStaticCheck = Rando::StaticData::Checks[Rando::Logic::checkPool[i]];

        if (randoStaticCheck.randoCheckType != RCTYPE_JIGGY) {
            continue;
        }

        if ((randoStaticCheck.collectionId == (10 * levelId) - 9) &&
            (reachableChecks[Rando::Logic::checkPool[i]].canAccess &&
             !reachableChecks[Rando::Logic::checkPool[i]].isFilled)) {
            return Rando::Logic::checkPool[i];
        }
    }

    return -1;
}

int32_t GetItemPoolIndexByJinjoCheck(RandoCheckId randoCheckId) {
    for (int i = 0; i < Rando::Logic::itemPool.size(); i++) {
        if (std::get<2>(Rando::Logic::itemPool[i]) == randoCheckId) {
            return i;
        }
    }

    return -1;
}

int32_t GetCurrentAccessibleChecks() {
    int32_t currentChecks = 0;

    for (auto& check : reachableChecks) {
        if (check.canAccess = true && check.isFilled == false) {
            currentChecks++;
        }
    }

    return currentChecks;
}

void UpdateJinjoChecks(std::vector<RandoCheckId>& jinjoCheckList) {
    for (int i = 0; i < jinjoCheckList.size(); i++) {
        for (int s = 0; s < jinjoCheckIds.size(); s++) {
            if (jinjoCheckList[i] == jinjoCheckIds[s]) {
                jinjoCheckIds.erase(jinjoCheckIds.begin() + s);
                break;
            }
        }
    }

    jinjoCheckList.clear();
}

void PopulateJinjoCheckIds() {
    jinjoCheckIds.clear();

    for (auto& check : Rando::Logic::checkPool) {
        Rando::StaticData::RandoStaticCheck randoStaticCheck = Rando::StaticData::Checks[check];

        if (randoStaticCheck.randoCheckType != RCTYPE_JINJO) {
            continue;
        }

        jinjoCheckIds.push_back(check);
    }
}

void ResetSaveData() {
    // [port] `data` is the 112-byte vanilla payload, not the whole SaveData, which is
    // ~43 KB once shipSaveData (rando checks, note/jinjo retention) is counted.
    for (size_t s = 0; s < sizeof(gameFile_saveData[selectedFileNum].data); s++) {
        gameFile_saveData[selectedFileNum].data[s] = 0;
    }

    for (int a = ABILITY_0_BARGE; a < ABILITY_13_1ST_NOTEDOOR; a++) {
        ability_setLearned((ability_e)a, false);
    }

    for (int f = FILEPROG_90_PAID_TERMITE_COST; f <= FILEPROG_94_PAID_BEE_COST; f++) {
        fileProgressFlag_set((file_progress_e)f, 0);
    }

    D_80385F30[ITEM_C_NOTE] = 0;
    D_80385F30[ITEM_E_JIGGY] = 0;
    D_80385F30[ITEM_26_JIGGY_TOTAL] = 0;
    D_80385F30[ITEM_1C_MUMBO_TOKEN] = 0;
    D_80385F30[ITEM_25_MUMBO_TOKEN_TOTAL] = 0;

    itemscore_noteScores_clear();
}

void SetUnlockedAbility(ability_e abilityId) {
    ability_unlock((ability_e)abilityId);

    if (abilityId == ABILITY_8_FLAP_FLIP) {
        ability_unlock(ABILITY_A_HOLD_A_JUMP_HIGHER);
        ability_unlock(ABILITY_7_FEATHERY_FLAP);
    } else if (abilityId == ABILITY_4_CLAW_SWIPE) {
        ability_unlock(ABILITY_C_ROLL);
        ability_unlock(ABILITY_B_RATATAT_RAP);
    }
}

void SetPlacedItem(int32_t checkIndex, int32_t itemIndex, PlacedItemCounts& placedItems,
                   PlacedCheckObject (&placedCheckItems)[RC_MAX],
                   std::vector<std::tuple<actor_e, int32_t, RandoCheckId>>& pool) {
    if (checkIndex < 0 || itemIndex < 0) {
        return;
    }

    placedCheckItems[checkIndex] = {
        .actorId = std::get<0>(pool[itemIndex]),
        .collectionId = std::get<1>(pool[itemIndex]),
        .shuffledCheckId = std::get<2>(pool[itemIndex]),
    };

    reachableChecks[checkIndex].isFilled = true;

    switch (std::get<0>(pool[itemIndex])) {
        case ACTOR_46_JIGGY:
            placedItems.jiggyCount++;
            break;
        case ACTOR_51_MUSIC_NOTE:
            placedItems.noteCount++;
            break;
        case ACTOR_2D_MUMBO_TOKEN:
            placedItems.mumboTokenCount++;
            break;
        case ACTOR_12C_MOLEHILL:
            SetUnlockedAbility((ability_e)std::get<1>(pool[itemIndex]));
            break;
        default:
            pool.erase(pool.begin() + itemIndex);
            return;
    }

    UpdateSaveDataItemCounts(placedItems);
    pool.erase(pool.begin() + itemIndex);
}

void FlushRemainingPools(PlacedItemCounts placedItems, PlacedCheckObject (&placedCheckItems)[RC_MAX],
                         std::vector<std::tuple<actor_e, int32_t, RandoCheckId>>& itemPool,
                         std::vector<std::tuple<actor_e, int32_t, RandoCheckId>>& abilityItemPool) {
    int32_t checkIndex = 0;
    int32_t itemPoolIndex = 0;

    while (!abilityItemPool.empty()) {
        checkIndex = GetRandomCheckIndexS(reachableChecks, RCTYPE_MOLEHILL, false, true, true);
        itemPoolIndex = GetRandomItemIndexS(abilityItemPool, ACTOR_1_UNKNOWN);

        SetPlacedItem(checkIndex, itemPoolIndex, placedItems, placedCheckItems, abilityItemPool);

        if (checkIndex < 0 || itemPoolIndex < 0) {
            break;
        }
    }
    while (!itemPool.empty()) {
        checkIndex = GetRandomCheckIndexS(reachableChecks, RCTYPE_MOLEHILL, true, false, true);
        itemPoolIndex = GetRandomItemIndexS(itemPool, ACTOR_1_UNKNOWN);

        SetPlacedItem(checkIndex, itemPoolIndex, placedItems, placedCheckItems, itemPool);

        if (checkIndex < 0 || itemPoolIndex < 0) {
            break;
        }
    }
}

bool UpdateAccessibility(RandoRegionId randoRegionId, bool& accessChanged) {
    auto regionData = Rando::Logic::Regions[randoRegionId];

    for (auto& availableConnections : regionData.connections) {
        if (!reachableRegions[availableConnections.first].canAccess && availableConnections.second.first()) {
            reachableRegions[availableConnections.first].canAccess = true;
            accessChanged = true;
        }
    }
    for (auto& availableEvents : regionData.events) {
        if (!reachableEvents[availableEvents.first].canAccess && availableEvents.second()) {
            reachableEvents[availableEvents.first].canAccess = true;
            accessChanged = true;
        }
    }
    for (auto& availableChecks : regionData.checks) {
        if (!reachableChecks[availableChecks.first].canAccess && availableChecks.second.first()) {
            reachableChecks[availableChecks.first].canAccess = true;
            accessChanged = true;
        }
    }

    return accessChanged;
}

namespace Rando {

namespace Logic {

void GenerateGlitchlessLogicPool(std::vector<RandoCheckId>& checkPool,
                                 std::vector<std::tuple<actor_e, int32_t, RandoCheckId>>& itemPool,
                                 std::vector<RandoCheckId>& abilityCheckPool,
                                 std::vector<std::tuple<actor_e, int32_t, RandoCheckId>>& abilityItemPool,
                                 SaveData* saveData) {
    bool isGameComplete = false;
    int32_t checkIndex = 0;
    int32_t itemPoolIndex = 0;
    int32_t progressionIndex = 0;

    PlacedItemCounts placedItems = { .noteCount = 0, .jiggyCount = 0, .mumboTokenCount = 0 };
    PlacedCheckObject placedCheckItems[RC_MAX] = {};

    if (CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_JINJOS].cvar, 0) == RO_GENERIC_ON) {
        PopulateJinjoCheckIds();
    }

    // This doesn't have to exist, added strictly for better development testing.
    for (auto& [regionId, regionData] : Rando::Logic::Regions) {
        reachableRegions[regionId].name = regionData.regionName;
        reachableRegions[regionId].canAccess = false;
        reachableRegions[regionId].isFilled = false;
    }

    for (auto& [checkId, checkData] : Rando::StaticData::Checks) {
        reachableChecks[checkId].name = checkData.name;
        reachableChecks[checkId].canAccess = false;
        reachableChecks[checkId].isFilled = false;
        reachableChecks[checkId].isShuffled = false;
    }

    for (auto& shuffledCheck : checkPool) {
        reachableChecks[shuffledCheck].isShuffled = true;
    }
    for (auto& shuffledAbiity : abilityCheckPool) {
        reachableChecks[shuffledAbiity].isShuffled = true;
    }

    // Starting Initialization
    reachableRegions[RR_SPIRAL_MOUNTAIN_ENTRANCE].canAccess = true;
    Rando::Logic::GrantStartingLoadout();
    for (auto& [ability, abilityInfo] : abilityLoadoutMap) {
        if (!CVarGetInteger(abilityInfo.second, 0)) {
            continue;
        }

        RandoCheckId abilityCheck = Rando::StaticData::GetCheckByAbilityId(ability);

        auto it = std::find(abilityCheckPool.begin(), abilityCheckPool.end(), abilityCheck);
        if (it == abilityCheckPool.end()) {
            continue;
        }

        abilityCheckPool.erase(it);

        // The molehill left the pool, so it has to stop being a valid placement target as well.
        reachableChecks[abilityCheck].isShuffled = false;

        // The player already owns this ability, so its item leaves the pool alongside the molehill.
        auto item = std::find_if(abilityItemPool.begin(), abilityItemPool.end(),
                                 [&](const std::tuple<actor_e, int32_t, RandoCheckId>& abilityItem) {
                                     return std::get<1>(abilityItem) == (int32_t)ability;
                                 });
        if (item != abilityItemPool.end()) {
            abilityItemPool.erase(item);
        }

        UpdateAccessibility(RR_SPIRAL_MOUNTAIN_ENTRANCE, reachableRegions[RR_SPIRAL_MOUNTAIN_ENTRANCE].canAccess);
    }
    UpdateAccessibility(RR_SPIRAL_MOUNTAIN_ENTRANCE, reachableRegions[RR_SPIRAL_MOUNTAIN_ENTRANCE].canAccess);

    while (!isGameComplete) {
        bool accessibilityAdded = false;

        for (int i = RR_UNKNOWN + 1; i < RR_MAX; i++) {
            if (!reachableRegions[i].canAccess) {
                continue;
            }

            accessibilityAdded = UpdateAccessibility((RandoRegionId)i, accessibilityAdded);

            if (i == RR_GRUNTILDAS_LAIR_LOBBY && placedItems.jiggyCount == 0) {
                if (CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_JIGGIES].cvar, 0) == RO_GENERIC_ON) {
                    checkIndex = GetRandomCheckIndexS(reachableChecks, RCTYPE_MOLEHILL, true, false, isGameComplete);
                    itemPoolIndex = GetRandomItemIndexS(itemPool, ACTOR_46_JIGGY);

                    SetPlacedItem(checkIndex, itemPoolIndex, placedItems, placedCheckItems, itemPool);

                    if (checkIndex >= 0 && itemPoolIndex >= 0) {
                        accessibilityAdded = true;
                    } else {
                        Notification::Emit({ .message = "No Checks left for First Jiggy." });
                        RefreshMetrics("No Checks Available for First Jiggy");
                    }
                }
            }
        }

        if (reachableEvents[RA_GAME_COMPLETE].canAccess) {
            FlushRemainingPools(placedItems, placedCheckItems, itemPool, abilityItemPool);

            itemPool.clear();
            abilityItemPool.clear();
            for (auto& check : checkPool) {
                itemPool.push_back({ (actor_e)placedCheckItems[check].actorId, placedCheckItems[check].collectionId,
                                     placedCheckItems[check].shuffledCheckId });
            }
            for (auto& mole : abilityCheckPool) {
                abilityItemPool.push_back({ (actor_e)placedCheckItems[mole].actorId,
                                            placedCheckItems[mole].collectionId,
                                            placedCheckItems[mole].shuffledCheckId });
            }

            RefreshMetrics("Generation Complete");
            ResetSaveData();

            isGameComplete = true;
            Notification::Emit({ .message = "Seed Generation completed!" });
            break;
        }

        if (CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_MOLEHILLS].cvar, 0) == RO_GENERIC_ON) {
            int32_t progCheck = 0;
            for (auto& abilityId : progressionAbilities[progressionIndex].abilityIds) {
                if (ability_isUnlocked((ability_e)abilityId)) {
                    progCheck++;
                    continue;
                }

                for (int a = 0; a < abilityItemPool.size(); a++) {
                    if (std::get<1>(abilityItemPool[a]) == abilityId) {
                        checkIndex =
                            GetRandomCheckIndexS(reachableChecks, RCTYPE_MOLEHILL, false, true, isGameComplete);

                        SetPlacedItem(checkIndex, a, placedItems, placedCheckItems, abilityItemPool);

                        progCheck++;
                        accessibilityAdded = true;
                    }
                }
            }
            if (progCheck == progressionAbilities[progressionIndex].abilityIds.size()) {
                progressionAbilities[progressionIndex].isComplete = true;
            }
        } else {
            switch (progressionAbilities[progressionIndex].progID) {
                case RA_NOTE_DOOR_50:
                    __chSmBottles_skipIntroTutorial();
                    ability_unlock(ABILITY_6_EGGS);
                    ability_unlock(ABILITY_10_TALON_TROT);
                    ability_unlock(ABILITY_2_BEAK_BUSTER);
                    break;
                case RA_NOTE_DOOR_180:
                    ability_unlock(ABILITY_D_SHOCK_JUMP);
                    ability_unlock(ABILITY_12_WONDERWING);
                    ability_unlock(ABILITY_9_FLIGHT);
                    break;
                case RA_NOTE_DOOR_260:
                    ability_unlock(ABILITY_E_WADING_BOOTS);
                    break;
                case RA_NOTE_DOOR_450:
                    ability_unlock(ABILITY_1_BEAK_BOMB);
                    ability_unlock(ABILITY_11_TURBO_TALON);
                    break;
                default:
                    break;
            }

            for (auto& abilityId : progressionAbilities[progressionIndex].abilityIds) {
                SetUnlockedAbility((ability_e)abilityId);
            }
            progressionAbilities[progressionIndex].isComplete = true;
        }

        if (reachableEvents[progressionItems[progressionIndex].progId].canAccess) {
            if (CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_JIGGIES].cvar, 0) == RO_GENERIC_ON) {
                while (placedItems.jiggyCount < progressionItems[progressionIndex].itemData[1].itemCount) {
                    int32_t jinjoChance = rand() % 100;
                    if (jinjoChance >= 45 && GetCurrentAccessibleChecks() >= 6 && !jinjoCheckIds.empty() &&
                        GetRandomItemIndexS(itemPool, ACTOR_46_JIGGY) >= 0) {
                        int32_t selectedIndex = rand() % jinjoCheckIds.size();
                        int32_t selectedLevel = Rando::StaticData::Checks[jinjoCheckIds[selectedIndex]].worldId;

                        std::vector<RandoCheckId> selectedJinjos;
                        for (auto& jinjoCheck : jinjoCheckIds) {
                            if (Rando::StaticData::Checks[jinjoCheck].worldId == selectedLevel) {
                                selectedJinjos.push_back(jinjoCheck);
                            }
                        }

                        for (auto& jinjoPlace : selectedJinjos) {
                            checkIndex =
                                GetRandomCheckIndexS(reachableChecks, RCTYPE_MOLEHILL, true, false, isGameComplete);
                            itemPoolIndex = GetItemPoolIndexByJinjoCheck(jinjoPlace);

                            SetPlacedItem(checkIndex, itemPoolIndex, placedItems, placedCheckItems, itemPool);
                        }

                        UpdateJinjoChecks(selectedJinjos);

                        checkIndex = GetCheckPoolJinjoJiggyIndexByLevelId(selectedLevel);
                        itemPoolIndex = GetRandomItemIndexS(itemPool, ACTOR_46_JIGGY);

                        SetPlacedItem(checkIndex, itemPoolIndex, placedItems, placedCheckItems, itemPool);

                        if (checkIndex >= 0 && itemPoolIndex >= 0) {
                            accessibilityAdded = true;
                        }
                    } else {
                        checkIndex =
                            GetRandomCheckIndexS(reachableChecks, RCTYPE_MOLEHILL, true, false, isGameComplete);
                        itemPoolIndex = GetRandomItemIndexS(itemPool, ACTOR_46_JIGGY);

                        SetPlacedItem(checkIndex, itemPoolIndex, placedItems, placedCheckItems, itemPool);

                        if (checkIndex < 0 || itemPoolIndex < 0) {
                            break;
                        }

                        if (checkIndex >= 0 && itemPoolIndex >= 0) {
                            accessibilityAdded = true;
                        }
                    }
                }
            } else {
                placedItems.jiggyCount = progressionItems[progressionIndex].itemData[1].itemCount;
                UpdateSaveDataItemCounts(placedItems);
            }

            if (CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_MUMBO_TOKENS].cvar, 0) == RO_GENERIC_ON) {
                while (placedItems.mumboTokenCount < progressionItems[progressionIndex].itemData[2].itemCount) {
                    checkIndex = GetRandomCheckIndexS(reachableChecks, RCTYPE_MOLEHILL, true, false, isGameComplete);
                    itemPoolIndex = GetRandomItemIndexS(itemPool, ACTOR_2D_MUMBO_TOKEN);

                    SetPlacedItem(checkIndex, itemPoolIndex, placedItems, placedCheckItems, itemPool);

                    if (checkIndex < 0 || itemPoolIndex < 0) {
                        break;
                    }

                    if (checkIndex >= 0 && itemPoolIndex >= 0) {
                        accessibilityAdded = true;
                    }
                }
            } else {
                placedItems.mumboTokenCount = progressionItems[progressionIndex].itemData[2].itemCount;
                UpdateSaveDataItemCounts(placedItems);
            }

            if (CVarGetInteger(Rando::StaticData::Options[RO_SHUFFLE_MUSIC_NOTES].cvar, 0) == RO_GENERIC_ON) {
                while (placedItems.noteCount < progressionItems[progressionIndex].itemData[0].itemCount) {
                    checkIndex = GetRandomCheckIndexS(reachableChecks, RCTYPE_MOLEHILL, true, false, isGameComplete);
                    itemPoolIndex = GetRandomItemIndexS(itemPool, ACTOR_51_MUSIC_NOTE);

                    SetPlacedItem(checkIndex, itemPoolIndex, placedItems, placedCheckItems, itemPool);

                    if (checkIndex < 0 || itemPoolIndex < 0) {
                        break;
                    }

                    if (checkIndex >= 0 && itemPoolIndex >= 0) {
                        accessibilityAdded = true;
                    }
                }
            } else {
                placedItems.noteCount = progressionItems[progressionIndex].itemData[0].itemCount;
                UpdateSaveDataItemCounts(placedItems);
            }
        }

        if (placedItems.noteCount >= progressionItems[progressionIndex].itemData[0].itemCount &&
            placedItems.jiggyCount >= progressionItems[progressionIndex].itemData[1].itemCount &&
            placedItems.mumboTokenCount >= progressionItems[progressionIndex].itemData[2].itemCount &&
            progressionAbilities[progressionIndex].isComplete) {

            if (progressionIndex < progressionAbilities.size() - 1) {
                progressionIndex++;
            }
        }

        if (!accessibilityAdded) {
            if (!failSafeTrigger) {
                failSafeTrigger = true;
                for (int a = 0; a < abilityItemPool.size(); a++) {
                    if (!ability_isUnlocked((ability_e)std::get<1>(abilityItemPool[a]))) {
                        checkIndex =
                            GetRandomCheckIndexS(reachableChecks, RCTYPE_MOLEHILL, false, true, isGameComplete);
                        SetPlacedItem(checkIndex, a, placedItems, placedCheckItems, abilityItemPool);
                        if (checkIndex >= 0) {
                            a = 0;
                            failSafeTrigger = false;
                        }
                    }
                }
            } else {
                if (prevProgressionIndex == progressionIndex) {
                    Notification::Emit({ .message = "Seed Configuration impossible, failed to generate." });
                    RefreshMetrics("Seed Failed to Generate");
                    ResetSaveData();
                    break;
                } else {
                    prevProgressionIndex = progressionIndex;
                }
            }
        }
    }
}

} // namespace Logic

} // namespace Rando