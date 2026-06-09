#pragma once

#include <libultraship/libultraship.h>
#include "UIWidgets.hpp"
#include "Menu.h"
#include <fast/backends/gfx_rendering_api.h>
#include "port/UI/cvar_prefixes.h"
#include "port/UI/enhancementTypes.h"

namespace LighthouseGui {
class LighthouseMenu : public Ship::Menu {
public:
    LighthouseMenu(const std::string& consoleVariable, const std::string& name);

    void InitElement() override;
    void DrawElement() override;
    void UpdateElement() override;
    void Draw() override;

    void AddSidebarEntry(std::string sectionName, std::string sidbarName, uint32_t columnCount);
    WidgetInfo& AddWidget(WidgetPath& pathInfo, std::string widgetName, WidgetType widgetType);
    void AddMenuSettings();
    void AddMenuEnhancements();
    void AddMenuNetwork();
    void AddMenuDevTools();

private:
    char mGitCommitHashTruncated[8];
    bool mIsTaggedVersion;
};
} // namespace LighthouseGui
