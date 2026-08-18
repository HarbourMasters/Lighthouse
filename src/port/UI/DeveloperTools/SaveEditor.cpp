#include "SaveEditor.h"
#include "SaveEditorTooltips.h"
#include "port/Rando/Rando.h"
#include "port/Rando/Logic/Logic.h"
#include "port/Rando/CustomObject/CustomObject.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/UI/UIWidgets.hpp"
#include "port/UI/Notification.h"
#include "port/Save/Types.h"
#include "port/ShipUtils.h"

#include <string>
#include <imgui.h>
#include <libultraship/libultraship.h>
#include "port/UI/LighthouseGui.hpp"
#include "port/UI/cvar_prefixes.h"

#include "enums.h"
#include "prop.h"

#define DEFAULT_MAX_HEALTH 8
#define DEFAULT_MAX_EGGS 100
#define DEFAULT_MAX_RED_FEATHERS 50
#define DEFAULT_MAX_GOLD_FEATHERS 10

extern "C" {
bool ability_isUnlocked(enum ability_e uid);
void jiggyscore_setCollected(s32 indx, s32 val);
void honeycombscore_set(enum honeycomb_e indx, bool val);
void mumboscore_set(enum mumbotoken_e indx, bool val);
s32 itemscore_noteScores_get(enum level_e lvl_id);
extern u8 D_80385FF0[0xE];

extern struct {
    u8 D_803832C0[0xD];
    u8 D_803832CD[0xD];
} jiggyscore;

extern u8 sHoneycombScore[3];
extern u8 sMumboTokenScore[16];
}

std::vector<std::string> warpCauldronList = {
    "Lower Pink Cauldron",
    "Upper Pink Cauldron",
    "Lower Green Cauldron",
    "Upper Green Cauldron",
    "Lower Red Cauldron",
    "Upper Red Cauldron",
    "Unused1",
    "Unused2",
    "Lower Yellow Cauldron",
    "Upper Yellow Cauldron",
};

std::vector<std::tuple<item_e, std::string, int32_t>> ammoDetailList = {
    { ITEM_D_EGGS, "Blue Eggs", DEFAULT_MAX_EGGS },
    { ITEM_F_RED_FEATHER, "Red Feathers", DEFAULT_MAX_RED_FEATHERS },
    { ITEM_10_GOLD_FEATHER, "Gold Feathers", DEFAULT_MAX_GOLD_FEATHERS },
};

std::unordered_map<int32_t, int32_t> progressToLevelMap = {
    { FILEPROG_31_MM_OPEN, LEVEL_1_MUMBOS_MOUNTAIN },      { FILEPROG_32_TTC_OPEN, LEVEL_2_TREASURE_TROVE_COVE },
    { FILEPROG_33_CC_OPEN, LEVEL_3_CLANKERS_CAVERN },      { FILEPROG_34_BGS_OPEN, LEVEL_4_BUBBLEGLOOP_SWAMP },
    { FILEPROG_35_FP_OPEN, LEVEL_5_FREEZEEZY_PEAK },       { FILEPROG_36_GV_OPEN, LEVEL_7_GOBIS_VALLEY },
    { FILEPROG_37_MMM_OPEN, LEVEL_A_MAD_MONSTER_MANSION }, { FILEPROG_38_RBB_OPEN, LEVEL_9_RUSTY_BUCKET_BAY },
    { FILEPROG_39_CCW_OPEN, LEVEL_8_CLICK_CLOCK_WOOD },
};

std::vector<level_e> levelOrder = {
    LEVEL_B_SPIRAL_MOUNTAIN,     LEVEL_6_LAIR,
    LEVEL_1_MUMBOS_MOUNTAIN,     LEVEL_2_TREASURE_TROVE_COVE,
    LEVEL_3_CLANKERS_CAVERN,     LEVEL_4_BUBBLEGLOOP_SWAMP,
    LEVEL_5_FREEZEEZY_PEAK,      LEVEL_7_GOBIS_VALLEY,
    LEVEL_A_MAD_MONSTER_MANSION, LEVEL_9_RUSTY_BUCKET_BAY,
    LEVEL_8_CLICK_CLOCK_WOOD,
};

bool SaveEditor_IsJiggyCollected(jiggy_e jiggyId) {
    return (jiggyscore.D_803832C0[(jiggyId - 1) / 8] & (1 << (jiggyId & 7))) != 0;
}

bool SaveEditor_IsHoneycombCollected(honeycomb_e honeycombId) {
    return (sHoneycombScore[(honeycombId - 1) / 8] & (1 << (honeycombId & 7))) != 0;
}

bool SaveEditor_IsMumboTokenCollected(mumbotoken_e tokenId) {
    return (sMumboTokenScore[(tokenId - 1) / 8] & (1 << (tokenId & 7))) != 0;
}

static const WorldDef* SaveEditor_GetWorldDef(int32_t levelId) {
    for (const WorldDef& world : kWorlds) {
        if (world.levelId == levelId) {
            return &world;
        }
    }
    return nullptr;
}

void SaveEditor_UpdateCheckTracker(RandoSaveCheck randoSaveCheck) {
    if (randoSaveCheck.eligible) {
        CustomObject::CheckObtainedEX(randoSaveCheck.randoCheckId);
    }

    for (auto& pool : Rando::Logic::shuffledPool) {
        if (pool.randoCheckId == randoSaveCheck.randoCheckId) {
            pool.isShuffled = randoSaveCheck.isShuffled;
            pool.eligible = randoSaveCheck.eligible;
            pool.skipped = randoSaveCheck.skipped;
            break;
        }
    }

    int32_t itemIncr = randoSaveCheck.eligible ? 1 : -1;

    Rando::StaticData::RandoStaticItem randoItem = Rando::StaticData::Items[randoSaveCheck.randoItemId];
    switch (randoItem.randoItemType) {
        case RITYPE_JIGGY:
            jiggyscore_setCollected(randoSaveCheck.randoCollectionId, randoSaveCheck.eligible);
            item_adjustByDiffWithoutHud(ITEM_26_JIGGY_TOTAL, itemIncr);
            break;
        case RITYPE_EMPTY_HONEYCOMB:
            honeycombscore_set((honeycomb_e)randoSaveCheck.randoCollectionId, randoSaveCheck.eligible);
            break;
        case RITYPE_MOLEHILL:
            if (randoSaveCheck.eligible) {
                ability_unlock((ability_e)randoSaveCheck.randoCollectionId);
            } else {
                ability_setLearned((ability_e)randoSaveCheck.randoCollectionId, 0);
            }
            break;
        case RITYPE_MUMBO_TOKEN:
            mumboscore_set((mumbotoken_e)randoSaveCheck.randoCollectionId, randoSaveCheck.eligible);
            item_adjustByDiffWithoutHud(ITEM_1C_MUMBO_TOKEN, itemIncr);
            break;
        default:
            break;
    }
}

void SaveEditor_DrawUnlocks() {
    if (ImGui::BeginChild("UnlockChild")) {
        if (ImGui::BeginTable("UnlocksTable", 3, ImGuiTableFlags_SizingFixedSame)) {
            ImGui::TableNextColumn();

            ImGui::SeparatorText("Ability Unlocks");
            for (int i = ABILITY_0_BARGE; i <= ABILITY_12_WONDERWING; i++) {
                ImGui::PushID(i);
                bool isUnlocked = ability_isUnlocked((ability_e)i);
                std::string abilName = "Unlock " + abilityNameList[i];
                if (UIWidgets::Checkbox(abilName.c_str(), &isUnlocked)) {
                    if (ability_isUnlocked((ability_e)i)) {
                        ability_setLearned((ability_e)i, false);
                    } else {
                        ability_setLearned((ability_e)i, true);
                    }
                }
                ImGui::PopID();
            }
            ImGui::TableNextColumn();

            ImGui::SeparatorText("World Unlocks");
            for (int i = FILEPROG_31_MM_OPEN; i <= FILEPROG_39_CCW_OPEN; i++) {
                ImGui::PushID(i);
                bool isUnlocked = fileProgressFlag_get((file_progress_e)i);
                std::string worldName = "Unlock " + worldNameList[progressToLevelMap.at(i) - 1];
                if (UIWidgets::Checkbox(worldName.c_str(), &isUnlocked)) {
                    if (fileProgressFlag_get((file_progress_e)i)) {
                        fileProgressFlag_set((file_progress_e)i, false);
                    } else {
                        fileProgressFlag_set((file_progress_e)i, true);
                    }
                }
                ImGui::PopID();
            }
            ImGui::TableNextColumn();

            ImGui::SeparatorText("Cauldron Unlocks");
            for (int i = FILEPROG_49_PINK_CAULDRON_1_ACTIVE; i <= FILEPROG_52_YELLOW_CAULDRON_2_ACTIVE; i++) {
                if (i == FILEPROG_4F_UNUSED_CAULDRON_1_ACTIVE || i == FILEPROG_50_UNUSED_CAULDRON_2_ACTIVE) {
                    continue;
                }

                ImGui::PushID(i);
                bool isUnlocked = fileProgressFlag_get((file_progress_e)i);
                std::string cauldronName = "Unlock " + warpCauldronList[i - FILEPROG_49_PINK_CAULDRON_1_ACTIVE];
                if (UIWidgets::Checkbox(cauldronName.c_str(), &isUnlocked)) {
                    if (fileProgressFlag_get((file_progress_e)i)) {
                        fileProgressFlag_set((file_progress_e)i, false);
                    } else {
                        fileProgressFlag_set((file_progress_e)i, true);
                    }
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        ImGui::EndChild();
    }
}

void SaveEditor_DrawGeneralTab() {
    if (ImGui::BeginChild("GeneralChild")) {
        ImGui::SeparatorText("Health");
        bool dblHealthUnlocked = fileProgressFlag_get(FILEPROG_B9_DOUBLE_HEALTH);
        int32_t curLives = item_getCount(ITEM_16_LIFE);
        if (ImGui::BeginTable("PlayerHealth", 2)) {
            ImGui::TableNextColumn();
            int32_t curHealth = item_getCount(ITEM_14_HEALTH);
            int32_t maxHealth = item_getCount(ITEM_15_HEALTH_TOTAL);

            if (UIWidgets::SliderInt("curHealth", &curHealth,
                                     {
                                         .format = "Current: %i",
                                         .min = 0,
                                         .max = item_getCount(ITEM_15_HEALTH_TOTAL),
                                         .clamp = true,
                                         .labelPosition = UIWidgets::LabelPositions::None,
                                         .color = THEME_COLOR,
                                     })) {
                item_set(ITEM_14_HEALTH, curHealth);
            }
            ImGui::TableNextColumn();
            if (UIWidgets::SliderInt("maxHealth", &maxHealth,
                                     {
                                         .format = "Maximum: %i",
                                         .min = 1,
                                         .max = fileProgressFlag_get(FILEPROG_B9_DOUBLE_HEALTH) ? 16 : 8,
                                         .clamp = true,
                                         .labelPosition = UIWidgets::LabelPositions::None,
                                         .color = THEME_COLOR,
                                     })) {
                item_set(ITEM_15_HEALTH_TOTAL, maxHealth);
            }
            ImGui::EndTable();
        }
        if (UIWidgets::Checkbox("Double Health", &dblHealthUnlocked)) {
            if (fileProgressFlag_get(FILEPROG_B9_DOUBLE_HEALTH)) {
                fileProgressFlag_set(FILEPROG_B9_DOUBLE_HEALTH, false);
                if (item_getCount(ITEM_14_HEALTH) > DEFAULT_MAX_HEALTH) {
                    item_set(ITEM_14_HEALTH, item_getCount(ITEM_15_HEALTH_TOTAL));
                }
            } else {
                fileProgressFlag_set(FILEPROG_B9_DOUBLE_HEALTH, true);
            }
        }
        if (UIWidgets::SliderInt("extraLives", &curLives,
                                 {
                                     .format = "Lives: %i",
                                     .min = 0,
                                     .max = 99,
                                     .clamp = true,
                                     .labelPosition = UIWidgets::LabelPositions::None,
                                     .color = THEME_COLOR,
                                 })) {
            item_set(ITEM_16_LIFE, curLives);
        }

        ImGui::SeparatorText("Ammo");
        for (auto& [id, name, value] : ammoDetailList) {
            int32_t curAmmo = item_getCount(id);
            int32_t cheatoValue = (id >= ITEM_F_RED_FEATHER ? (((id - 1) - ITEM_D_EGGS) + FILEPROG_BE_CHEATO_BLUEEGGS)
                                                            : (id - ITEM_D_EGGS) + FILEPROG_BE_CHEATO_BLUEEGGS);
            bool isCheato = fileProgressFlag_get((file_progress_e)cheatoValue);

            ImGui::PushID(id);
            ImGui::Text("%s", name.c_str());
            if (ImGui::BeginTable("PlayerAmmo", 2)) {
                ImGui::TableNextColumn();
                if (UIWidgets::Checkbox("Enable Cheato", &isCheato)) {
                    if (fileProgressFlag_get((file_progress_e)cheatoValue)) {
                        fileProgressFlag_set((file_progress_e)cheatoValue, false);
                        if (item_getCount(id) > value) {
                            item_set(id, value);
                        }
                    } else {
                        fileProgressFlag_set((file_progress_e)cheatoValue, true);
                    }
                }
                ImGui::TableNextColumn();
                if (UIWidgets::SliderInt(
                        name.c_str(), &curAmmo,
                        {
                            .format = "%i",
                            .min = 0,
                            .max = fileProgressFlag_get((file_progress_e)cheatoValue) ? (value * 2) : value,
                            .clamp = true,
                            .labelPosition = UIWidgets::LabelPositions::None,
                            .color = THEME_COLOR,
                        })) {
                    item_set(id, curAmmo);
                }
                ImGui::EndTable();
            }
            ImGui::Separator();
            ImGui::PopID();
        }

        ImGui::EndChild();
    }
}

void SaveEditor_DrawProgressTab() {
    if (ImGui::BeginChild("ProgressChild")) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        for (auto& level : levelOrder) {
            const WorldDef* world = SaveEditor_GetWorldDef(level);
            if (world == nullptr) {
                continue;
            }

            ImGui::SeparatorText(worldNameList[level - 1].c_str());
            if (ImGui::BeginTable("WorldTable", 2, ImGuiTableFlags_SizingFixedFit)) {
                ImGui::TableNextColumn();

                if (world->hasNoteScore) {
                    ImGui::Text("100 Notes");
                    ImGui::TableNextColumn();
                    bool hasAllNotes = itemscore_noteScores_get(level) >= 100;
                    std::string noteLabel = "##notes" + std::to_string(level);

                    ImGui::SameLine();
                    if (UIWidgets::Checkbox(noteLabel.c_str(), &hasAllNotes,
                                            { .labelPosition = UIWidgets::LabelPositions::None })) {
                        D_80385FF0[level] = hasAllNotes ? 100 : 0;
                    }
                    ImGui::TableNextColumn();
                }

                if (world->jiggyCount > 0) {
                    ImGui::Text("Jiggies");
                    ImGui::TableNextColumn();
                    for (int i = world->jiggyStart; i < (world->jiggyStart + world->jiggyCount); i++) {
                        std::string labelStr = "##jiggy" + std::to_string(i);
                        bool isCollected = SaveEditor_IsJiggyCollected((jiggy_e)i);
                        int32_t curJiggyCount = item_getCount(ITEM_26_JIGGY_TOTAL);

                        ImGui::SameLine();
                        if (UIWidgets::Checkbox(labelStr.c_str(), &isCollected,
                                                UIWidgets::CheckboxOptions()
                                                    .LabelPosition(UIWidgets::LabelPositions::None)
                                                    .Tooltip(SaveEditor_GetJiggyTooltip((jiggy_e)i)))) {
                            if (SaveEditor_IsJiggyCollected((jiggy_e)i)) {
                                jiggyscore_setCollected(i, false);
                                item_set(ITEM_26_JIGGY_TOTAL, (curJiggyCount - 1));
                            } else {
                                jiggyscore_setCollected(i, true);
                                item_set(ITEM_26_JIGGY_TOTAL, (curJiggyCount + 1));
                            }
                        }
                    }
                } else {
                    ImGui::TableNextColumn();
                }

                ImGui::TableNextColumn();
                if (world->mumboCount > 0) {
                    ImGui::Text("Mumbo Tokens");
                    ImGui::TableNextColumn();
                    int32_t tokenCount = world->mumboCount;
                    if (level == LEVEL_A_MAD_MONSTER_MANSION &&
                        CVarGetInteger(CVAR_ENHANCEMENT("Fixes.MumboTokenMMM"), 0)) {
                        tokenCount++;
                    }
                    for (int i = 1; i <= tokenCount; i++) {
                        int32_t tokenId =
                            (i > world->mumboCount) ? RP_MUMBO_TOKEN_INSIDE_LOGGO : world->mumboStart + (i - 1);
                        std::string labelStr = "##token" + std::to_string(tokenId);
                        bool isCollected = SaveEditor_IsMumboTokenCollected((mumbotoken_e)tokenId);
                        int32_t curTokenCount = item_getCount(ITEM_1C_MUMBO_TOKEN);

                        // Wrap every 10 so the wider worlds don't run off the window.
                        if (i == 1 || (i % 10) != 1) {
                            ImGui::SameLine();
                        }
                        if (UIWidgets::Checkbox(labelStr.c_str(), &isCollected,
                                                UIWidgets::CheckboxOptions()
                                                    .LabelPosition(UIWidgets::LabelPositions::None)
                                                    .Tooltip(SaveEditor_GetMumboTokenTooltip((mumbotoken_e)tokenId)))) {
                            if (SaveEditor_IsMumboTokenCollected((mumbotoken_e)tokenId)) {
                                mumboscore_set((mumbotoken_e)tokenId, false);
                                item_set(ITEM_1C_MUMBO_TOKEN, (curTokenCount - 1));
                            } else {
                                mumboscore_set((mumbotoken_e)tokenId, true);
                                item_set(ITEM_1C_MUMBO_TOKEN, (curTokenCount + 1));
                            }
                        }
                    }
                } else {
                    ImGui::TableNextColumn();
                }

                ImGui::TableNextColumn();
                if (world->honeycombCount > 0) {
                    ImGui::Text("Empty Honeycombs");
                    ImGui::TableNextColumn();
                    for (int combId = world->honeycombStart; combId < (world->honeycombStart + world->honeycombCount);
                         combId++) {
                        std::string labelStr = "##comb" + std::to_string(combId);
                        bool isCollected = SaveEditor_IsHoneycombCollected((honeycomb_e)combId);

                        ImGui::SameLine();
                        if (UIWidgets::Checkbox(labelStr.c_str(), &isCollected,
                                                UIWidgets::CheckboxOptions()
                                                    .LabelPosition(UIWidgets::LabelPositions::None)
                                                    .Tooltip(SaveEditor_GetHoneycombTooltip((honeycomb_e)combId)))) {
                            if (SaveEditor_IsHoneycombCollected((honeycomb_e)combId)) {
                                honeycombscore_set((honeycomb_e)combId, false);
                            } else {
                                honeycombscore_set((honeycomb_e)combId, true);
                            }
                        }
                    }
                }
                ImGui::EndTable();
            }
        }
        ImGui::PopStyleVar(1);

        ImGui::EndChild();
    }
}

void DrawRandoFlagEditor() {
    ImGui::SeparatorText("Rando INF Flags");
    if (selectedFileNum == DEFAULT_FILE_NUM) {
        ImGui::Text("No Save File Loaded");
        return;
    }
    if (ImGui::BeginChild("RandoFlagChild")) {
        for (int f = RANDO_INF_UNKNOWN; f < RANDO_INF_MAX; f++) {
            ImGui::PushID(f);
            bool flagState = RANDO_SAVE_FLAGS[f].flagState;
            if (UIWidgets::Checkbox(Rando::StaticData::Flags[(RandoInf)f].name, &flagState)) {
                CALL_EVENT(SetRandoInfFlag, (RandoInf)f, !RANDO_SAVE_FLAGS[f].flagState);
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
    }
}

void DrawRandoCheckEditor() {
    if (selectedFileNum == DEFAULT_FILE_NUM || Rando::Logic::shuffledPool.empty()) {
        ImGui::Text("No Rando Save Data");
    } else {
        static ImGuiTextFilter rcFilter;
        UIWidgets::PushStyleCombobox();
        rcFilter.Draw("##filter", ImGui::GetContentRegionAvail().x);
        UIWidgets::PopStyleCombobox();
        if (!rcFilter.IsActive()) {
            ImGui::SameLine(18.0f);
            ImGui::Text("Search");
        }

        if (ImGui::BeginChild("RandoToolsChild", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            if (ImGui::BeginTable("RandoSaveEditorTable", 7)) {
                ImGui::TableSetupColumn("shuffled", ImGuiTableColumnFlags_WidthFixed, 34.0f);
                ImGui::TableSetupColumn("eligible", ImGuiTableColumnFlags_WidthFixed, 34.0f);
                ImGui::TableSetupColumn("received", ImGuiTableColumnFlags_WidthFixed, 34.0f);
                ImGui::TableSetupColumn("skipped", ImGuiTableColumnFlags_WidthFixed, 34.0f);
                ImGui::TableSetupColumn("checkName", ImGuiTableColumnFlags_WidthFixed,
                                        (ImGui::GetContentRegionAvail().x - 34.0f) * 0.60f);
                ImGui::TableSetupColumn("itemName", ImGuiTableColumnFlags_WidthFixed,
                                        (ImGui::GetContentRegionAvail().x - 34.0f) * 0.25f);
                ImGui::TableSetupColumn("collectionId", ImGuiTableColumnFlags_WidthFixed, 34.0f);
                ImGui::TableNextColumn();

                for (auto& check : RANDO_SAVE_CHECKS) {
                    auto checkEntry = Rando::StaticData::Checks.find(check.randoCheckId);
                    if (checkEntry == Rando::StaticData::Checks.end()) {
                        continue;
                    }
                    const char* checkName = checkEntry->second.name;

                    auto itemEntry = Rando::StaticData::Items.find(check.randoItemId);
                    const char* itemName =
                        (itemEntry != Rando::StaticData::Items.end()) ? itemEntry->second.name : nullptr;

                    if (!rcFilter.PassFilter(checkName) && !rcFilter.PassFilter(itemName)) {
                        continue;
                    }

                    ImGui::PushID(check.randoCheckId);
                    bool isChanged = false;
                    bool isShuffled = check.isShuffled;
                    bool eligible = check.eligible;
                    bool received = check.received;
                    bool skipped = check.skipped;

                    std::string checkId = std::to_string((uint32_t)check.randoCheckId);

                    if (UIWidgets::Checkbox(
                            ("isShuffled##" + checkId).c_str(), &isShuffled,
                            UIWidgets::CheckboxOptions().LabelPosition(UIWidgets::LabelPositions::None))) {
                        RANDO_SAVE_CHECKS[check.randoCheckId].isShuffled = !check.isShuffled;
                        isChanged = true;
                    }
                    ImGui::TableNextColumn();
                    if (UIWidgets::Checkbox(
                            ("eligible##" + checkId).c_str(), &eligible,
                            UIWidgets::CheckboxOptions().LabelPosition(UIWidgets::LabelPositions::None))) {
                        RANDO_SAVE_CHECKS[check.randoCheckId].eligible = !check.eligible;
                        isChanged = true;
                    }
                    ImGui::TableNextColumn();
                    if (UIWidgets::Checkbox(
                            ("received##" + checkId).c_str(), &received,
                            UIWidgets::CheckboxOptions().LabelPosition(UIWidgets::LabelPositions::None))) {
                        RANDO_SAVE_CHECKS[check.randoCheckId].received = !check.received;
                        isChanged = true;
                    }
                    ImGui::TableNextColumn();
                    if (UIWidgets::Checkbox(
                            ("skipped##" + checkId).c_str(), &skipped,
                            UIWidgets::CheckboxOptions().LabelPosition(UIWidgets::LabelPositions::None))) {
                        RANDO_SAVE_CHECKS[check.randoCheckId].skipped = !check.skipped;
                        isChanged = true;
                    }

                    if (isChanged) {
                        SaveEditor_UpdateCheckTracker(check);
                    }
                    ImGui::TableNextColumn();

                    ImGui::TextWrapped("%s", checkName);
                    ImGui::TableNextColumn();

                    ImGui::TextWrapped(Rando::StaticData::Items[check.randoItemId].name);
                    ImGui::TableNextColumn();

                    Rando::StaticData::RandoStaticItem randoItem = Rando::StaticData::Items[check.randoItemId];
                    if (randoItem.randoItemType != RITYPE_JINJO && randoItem.randoItemType != RITYPE_MUSIC_NOTE) {
                        TableCellCenteredText(std::to_string(check.randoCollectionId).c_str());
                    }
                    ImGui::TableNextColumn();

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
        }
    }
}

void DrawRandoTabBar() {
    if (ImGui::BeginTabBar("RandoTabBar")) {
        if (ImGui::BeginTabItem("Flag Editor")) {
            DrawRandoFlagEditor();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Check Editor")) {
            DrawRandoCheckEditor();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void SaveEditor_DrawTabBar() {
    UIWidgets::PushStyleTabs(THEME_COLOR);
    if (ImGui::BeginTabBar("SaveEditorTabBar")) {
        if (ImGui::BeginTabItem("General")) {
            SaveEditor_DrawGeneralTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Unlocks")) {
            SaveEditor_DrawUnlocks();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("World Progress")) {
            SaveEditor_DrawProgressTab();
            ImGui::EndTabItem();
        }
        if (IS_RANDO) {
            if (ImGui::BeginTabItem("Rando Save Editor")) {
                DrawRandoTabBar();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    UIWidgets::PopStyleTabs();
}

void SaveEditorWindow::DrawElement() {
    SaveEditor_DrawTabBar();
}