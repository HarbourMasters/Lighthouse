#include "Spoiler.h"
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/Rando/Logic/Logic.h"
#include "port/UI/Notification.h"

extern void RandoSaveCheck_to_json(nlohmann::json& j, const RandoSaveCheck& randoSaveCheck);
extern RandoSaveCheck RandoSaveCheck_from_json(const nlohmann::json& j, RandoSaveCheck& randoSaveCheck);

namespace Rando {

namespace Spoiler {
nlohmann::ordered_json GenerateFromPoolGeneration() {
    nlohmann::ordered_json orderedSpoiler = nlohmann::ordered_json::object();

    orderedSpoiler["type"] = "LIGHTHOUSE_RANDO_SPOILER";
    orderedSpoiler["seed"] = randoFinalSeed; // TODO: Add once Manual Seed Input is finished.

    orderedSpoiler["options"] = nlohmann::json::object();
    for (auto& optionEntry : RANDO_SAVE_OPTIONS) {
        orderedSpoiler["options"][optionEntry.name] = optionEntry.optionValue;
    }

    orderedSpoiler["checks"] = nlohmann::json::object();

    for (auto& checkEntry : RANDO_SAVE_CHECKS) {
        nlohmann::json check = nlohmann::json::object();
        RandoSaveCheck_to_json(check, checkEntry);

        if (checkEntry.randoCheckId == RC_UNKNOWN) {
            checkEntry.name = "RC_UNKNOWN";
        }

        orderedSpoiler["checks"][checkEntry.name] = check;
    }

    orderedSpoiler["loadout"] = nlohmann::json::object();

    orderedSpoiler["loadout"]["abilities"] = nlohmann::json::array();
    orderedSpoiler["loadout"]["abilities"].push_back(ABILITY_A_HOLD_A_JUMP_HIGHER);
    orderedSpoiler["loadout"]["abilities"].push_back(ABILITY_13_1ST_NOTEDOOR);
    for (auto& [abilityId, abilityInfo] : abilityLoadoutMap) {
        if (CVarGetInteger(abilityInfo.second, 0)) {
            orderedSpoiler["loadout"]["abilities"].push_back(abilityId);
        }
    }

    orderedSpoiler["loadout"]["items"] = nlohmann::json::array();
    for (auto& [item, itemInfo] : itemLoadoutMap) {
        if (CVarGetInteger(itemInfo.second, 0)) {
            orderedSpoiler["loadout"]["items"].push_back(item);
        }
    }

    return orderedSpoiler;
}

void GenerateFromSpoiler(nlohmann::json spoiler) {
    Rando::Logic::shuffledPool.clear();

    if (!spoiler.contains("type") || spoiler["type"] != "LIGHTHOUSE_RANDO_SPOILER") {
        Notification::Emit({ .message = "Error: Invalid Spoiler Log.", .messageColor = ImVec4(0.85f, 0.3f, 0, 1) });
        return;
    }

    gameFile_saveData[selectedFileNum].shipSaveData.randoSaveData.seedId = spoiler["seed"];

    if (spoiler.contains("checks") && !spoiler["checks"].empty()) {
        for (auto& data : spoiler["checks"].items()) {
            RandoSaveCheck checkEntry = RandoSaveCheck_from_json(data.value(), checkEntry);
            checkEntry.name = Rando::StaticData::Checks[checkEntry.randoCheckId].name;
            RANDO_SAVE_CHECKS[checkEntry.randoCheckId] = checkEntry;

            Rando::Logic::shuffledPool.push_back(checkEntry);
        }
    }

    if (spoiler.contains("options") && !spoiler["options"].empty()) {
        for (auto& data : spoiler["options"].items()) {
            for (auto& [optionId, staticOption] : Rando::StaticData::Options) {
                if (staticOption.name == data.key()) {
                    RANDO_SAVE_OPTIONS[optionId].optionValue = data.value().get<int32_t>();
                    break;
                }
            }
        }
    }

    if (spoiler.contains("loadout") && !spoiler["loadout"].empty()) {
        if (spoiler["loadout"].contains("abilities") && !spoiler["loadout"]["abilities"].empty()) {
            for (auto& data : spoiler["loadout"]["abilities"].items()) {
                ability_setLearned(data.value(), true);
                ability_setHasUsed(data.value());
            }
        }

        if (spoiler["loadout"].contains("items") && !spoiler["loadout"]["items"].empty()) {
            for (auto& data : spoiler["loadout"]["items"].items()) {
                item_setMaxCount(data.value());
            }
        }
    }

    Rando::Logic::GrantFileProgressFlags();
    Rando::Logic::GrantSpiralMountainChecks();

    Notification::Emit({ .message = "Loaded from Spoiler Log.", .messageColor = ImVec4(0, 0.85f, 0, 1) });
}

} // namespace Spoiler

} // namespace Rando
