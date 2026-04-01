#include "PortEnhancements.h"
#include "port/rando/Rando.h"
#include "port/save/SaveConverter.h"

#define INIT_EVENT_IDS

#include "port/enhancements/events/hooks/Events.h"

void PortEnhancements_Init() {
    PortEnhancements_Register();
}

void PortEnhancements_Register() {
    // Register engine events
    REGISTER_EVENT(GameFrameUpdate);
    REGISTER_EVENT(VanillaBehavior);

    // Register game events
    REGISTER_EVENT(OnGameFileLoad);
    REGISTER_EVENT(OnSaveFileLoad);
    REGISTER_EVENT(OnSaveFileSave);

    // Register rando events
    REGISTER_EVENT(OnActorSpawn);
    REGISTER_EVENT(OnActorCollision);

    SaveConverter_Init();
    Rando::Init();
}

void PortEnhancements_Exit() {
    // @port TODO
}
