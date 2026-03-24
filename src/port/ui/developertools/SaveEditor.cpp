#include "SaveEditor.h"
#include "port/ui/UIWidgets.hpp"
#include "port/ui/Notification.h"
#include "port/ShipUtils.h"

#include <string>
#include <imgui.h>
#include <libultraship/libultraship.h>
#include "port/ui/LighthouseGui.hpp"
#include "port/ui/cvar_prefixes.h"

#include "enums.h"
#include "prop.h"

#define DEFAULT_MAX_HEALTH 8
#define DEFAULT_MAX_EGGS 100
#define DEFAULT_MAX_RED_FEATHERS 50
#define DEFAULT_MAX_GOLD_FEATHERS 10

extern "C" {
bool player_is_present(void);
s32 item_getCount(enum item_e item);
void item_set(s32 item, s32 val);
bool fileProgressFlag_get(enum file_progress_e index);
void fileProgressFlag_set(enum file_progress_e index, s32 set);
int ability_isUnlocked(enum ability_e uid);
void ability_setLearned(s32 move, s32 val);
u32 jiggyscore_isCollected(enum jiggy_e jiggy_id);
void jiggyscore_setCollected(s32 indx, s32 val);
bool honeycombscore_get(enum honeycomb_e indx);
void honeycombscore_set(enum honeycomb_e indx, bool val);
}

std::vector<std::string> worldNameList = {
    "Mumbo's Mountian", "Treasure Trove Cove", "Clanker's Cavern", "Bubblegloop Swamp", "Freezeezy Peak",
    "Gobi's Valley",    "Mad Monster Mansion", "Rusty Bucket Bay", "Click Clock Wood",
};

std::vector<std::string> abilityNameList = {
    "Beak Barge",    "Beak Bomb", "Beak Buster", "Camera Control", "Claw Swipe",  "Climb", "Eggs",
    "Feathery Flap", "Flap Flip", "Flight",      "Jump Higher",    "Ratatat Rap", "Roll",  "Shock Jump",
    "Wading Boots",  "Dive",      "Talon Trot",  "Turbo Talon",    "Wonderwing",
};

std::vector<std::tuple<item_e, std::string, int32_t>> ammoDetailList = {
    { ITEM_D_EGGS, "Blue Eggs", DEFAULT_MAX_EGGS },
    { ITEM_F_RED_FEATHER, "Red Feathers", DEFAULT_MAX_RED_FEATHERS },
    { ITEM_10_GOLD_FEATHER, "Gold Feathers", DEFAULT_MAX_GOLD_FEATHERS },
};

void SaveEditor_DrawAbilityUnlocks() {
    if (ImGui::BeginChild("AbilityUnlockChild")) {
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
        ImGui::EndChild();
    }
}

void SaveEditor_DrawWorldUnlocks() {
    if (ImGui::BeginChild("WorldUnlockChild")) {
        for (int i = FILEPROG_31_MM_OPEN; i < FILEPROG_39_CCW_OPEN; i++) {
            ImGui::PushID(i);
            bool isUnlocked = fileProgressFlag_get((file_progress_e)i);
            std::string worldName = "Unlock " + worldNameList[i - FILEPROG_31_MM_OPEN];
            if (UIWidgets::Checkbox(worldName.c_str(), &isUnlocked)) {
                if (fileProgressFlag_get((file_progress_e)i)) {
                    fileProgressFlag_set((file_progress_e)i, false);
                } else {
                    fileProgressFlag_set((file_progress_e)i, true);
                }
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
    }
}

void SaveEditor_DrawGeneralTab() {
    if (ImGui::BeginChild("GeneralChild")) {
        ImGui::SeparatorText("Health");
        bool dblHealthUnlocked = fileProgressFlag_get(FILEPROG_B9_DOUBLE_HEALTH);
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

        ImGui::SeparatorText("Ammo");
            for (auto& [id, name, value] : ammoDetailList) {
                int32_t curAmmo = item_getCount(id);
                int32_t cheatoValue = ((id - ITEM_D_EGGS) + FILEPROG_BE_CHEATO_BLUEEGGS);
                bool isCheato = fileProgressFlag_get((file_progress_e)cheatoValue);

                ImGui::PushID(id);
                ImGui::Text(name.c_str());
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
        
        ImGui::SeparatorText("Mumbo's Mountain");
        if (ImGui::BeginTable("WorldTable", 2, ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableNextColumn();

            ImGui::Text("Jiggies");
            ImGui::TableNextColumn();
            for (int i = JIGGY_01_MM_JINJO; i <= JIGGY_A_MM_CONGA; i++) {
                std::string labelStr = "##jiggy" + std::to_string(i);
                bool isCollected = jiggyscore_isCollected((jiggy_e)i);
                int32_t curJiggyCount = item_getCount(ITEM_26_JIGGY_TOTAL);

                ImGui::SameLine();
                if (UIWidgets::Checkbox(labelStr.c_str(), &isCollected,
                                        { .labelPosition = UIWidgets::LabelPositions::None })) {
                    if (jiggyscore_isCollected((jiggy_e)i)) {
                        jiggyscore_setCollected(i, false);
                        item_set(ITEM_26_JIGGY_TOTAL, (curJiggyCount - 1));
                    } else {
                        jiggyscore_setCollected(i, true);
                        item_set(ITEM_26_JIGGY_TOTAL, (curJiggyCount + 1));
                    }
                }
            }

            ImGui::TableNextColumn();
            ImGui::Text("Empty Honeycombs");
            ImGui::TableNextColumn();
            for (int i = HONEYCOMB_1_MM_HILL; i <= HONEYCOMB_2_MM_JUJU; i++) {
                std::string labelStr = "##comb" + std::to_string(i);
                bool isCollected = honeycombscore_get((honeycomb_e)i);

                ImGui::SameLine();
                if (UIWidgets::Checkbox(labelStr.c_str(), &isCollected,
                                        { .labelPosition = UIWidgets::LabelPositions::None })) {
                    if (honeycombscore_get((honeycomb_e)i)) {
                        honeycombscore_set((honeycomb_e)i, false);
                    } else {
                        honeycombscore_set((honeycomb_e)i, true);
                    }
                }
            }
            ImGui::EndTable();
        }

        ImGui::EndChild();
    }
}

void SaveEditor_DrawTabBar() {
    UIWidgets::PushStyleTabs(THEME_COLOR);
    if (ImGui::BeginTabBar("SaveEditorTabBar")) {
        if (ImGui::BeginTabItem("General")) {
            SaveEditor_DrawGeneralTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("World Unlocks")) {
            SaveEditor_DrawWorldUnlocks();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Ability Unlocks")) {
            SaveEditor_DrawAbilityUnlocks();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Progress")) {
            SaveEditor_DrawProgressTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    UIWidgets::PopStyleTabs();
}

void SaveEditorWindow::DrawElement() {
    // TODO: Check for Save Loaded
    // if (!player_is_present()) {
    //     ImGui::Text("No Save Loaded");
    //     return;
    // }
    SaveEditor_DrawTabBar();
}