#include "MiscBehavior.h"
#include "port/Enhancements/Events/Hooks/Events.h"

// #include "port/Rando/Logic/Logic.h"

// Entry point for the module, run once on game boot
void Rando::MiscBehavior::Init() {
    Rando::MiscBehavior::InitWorldStateBehavior();
    Rando::MiscBehavior::InitFileSelectBehavior();
    Rando::MiscBehavior::OnFileLoad();
    // Rando::MiscBehavior::OnFileSave();
}
