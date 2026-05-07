#include "MiscBehavior.h"
#include "port/enhancements/events/hooks/Events.h"

// #include "port/Rando/Logic/Logic.h"

// Entry point for the module, run once on game boot
void Rando::MiscBehavior::Init() {
    Rando::MiscBehavior::OnFileLoad();
    // Rando::MiscBehavior::OnFileSave();
}
