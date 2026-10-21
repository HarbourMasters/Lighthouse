#include "CosmeticsEditor.h"
#include "PlayerColors.h"

#include "port/UI/LighthouseGui.hpp"
#include "port/UI/UIWidgets.hpp"

static const char* playerColorLabels[BK_COLOR_CHANNEL_COUNT] = {
    "Banjo's fur", "Banjo's shorts", "Kazooie's feathers", "Kazooie's beak & legs", "Banjo's backpack", "Banjo's skin",
};

// Channel order is the wire order; this is the order the rows are listed in.
static const int playerColorOrder[BK_COLOR_CHANNEL_COUNT] = {
    BK_COLOR_BANJO_FUR,      BK_COLOR_BANJO_SKIN,       BK_COLOR_BANJO_SHORTS,
    BK_COLOR_BANJO_BACKPACK, BK_COLOR_KAZOOIE_FEATHERS, BK_COLOR_KAZOOIE_BEAK,
};

void CosmeticsEditorWindow::DrawElement() {
    ImGui::SeparatorText("Player Colors");
    ImGui::TextWrapped("Tick a part and pick a color to recolor Banjo & Kazooie. Players in your Anchor room see your "
                       "choices. Doesn't apply to transformations or custom models from mods.");
    ImGui::Spacing();

    for (int row = 0; row < BK_COLOR_CHANNEL_COUNT; row++) {
        const int i = playerColorOrder[row];
        u8 r, g, b;
        PlayerColors_getVanilla(i, &r, &g, &b);
        const Color_RGBA8 vanilla = { r, g, b, 255 };

        ImGui::PushID(i);
        UIWidgets::CVarColorPicker("##Color", PlayerColors_getChannelCVar(i), vanilla, false,
                                   UIWidgets::ColorPickerResetButton | UIWidgets::ColorPickerRandomButton, THEME_COLOR);
        ImGui::SameLine();
        UIWidgets::CVarCheckbox(playerColorLabels[i], PlayerColors_getChannelEnabledCVar(i),
                                UIWidgets::CheckboxOptions().DefaultValue(false).Color(THEME_COLOR));
        ImGui::PopID();
    }

    if (PlayerColors_hasPackTextures()) {
        ImGui::Spacing();
        UIWidgets::CVarCheckbox(
            "Use Texture Pack", CVAR_PLAYER_PACK_TEXTURES,
            UIWidgets::CheckboxOptions()
                .DefaultValue(true)
                .Color(THEME_COLOR)
                .Tooltip("Off draws Banjo & Kazooie's vanilla textures, so your colors reach every part."));
    }
}
