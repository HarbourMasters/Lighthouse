#include "MiscBehavior.h"
#include "port/Enhancements/Events/Hooks/Events.h"

// Entry point for the module, run once on game boot
void Rando::MiscBehavior::Init() {
    Rando::MiscBehavior::InitFileSelectBehavior();
    Rando::MiscBehavior::OnFileLoad();
}
