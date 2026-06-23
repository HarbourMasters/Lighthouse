#include "MiscBehavior.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/UI/Notification.h"

// #include "port/Rando/Logic/Logic.h"
// #include "port/Rando/Spoiler/Spoiler.h"

void Rando::MiscBehavior::OnFileSave() {
    REGISTER_LISTENER(OnGameSave, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        OnGameSave* ev = (OnGameSave*)event;

        if (!IS_RANDO) {
            return;
        }
    });
}