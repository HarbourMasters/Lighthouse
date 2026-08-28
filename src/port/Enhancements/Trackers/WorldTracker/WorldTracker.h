#ifndef WORLD_TRACKER_H
#define WORLD_TRACKER_H

#include "port/Rando/Rando.h"
#include <ship/window/gui/GuiWindow.h>

#include "enums.h"

typedef struct {
    int32_t noteLevelTotal;
    int32_t jiggyLevelTotal;
    int32_t honeycombLevelTotal;
    int32_t tokenLevelTotal;
    int32_t hasJinjo[5];
} WorldTrackerObject;

namespace WorldTracker {

extern WorldTrackerObject worldTrackerObject[LEVEL_C_BOSS];
extern WorldTrackerObject worldTrackerTotal;
void UpdateWorldTracker();

void Init();

class WorldTrackerWindow : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override{};
    void Draw() override;
    void UpdateElement() override{};
};

class SettingsWindow : public Ship::GuiWindow {
public:
    using GuiWindow::GuiWindow;

    void InitElement() override{};
    void DrawElement() override;
    void UpdateElement() override{};
};

} // namespace WorldTracker

#endif // WORLD_TRACKER_H