#include "PortEnhancements.h"

#define INIT_EVENT_IDS

#include "port/enhancements/events/hooks/Events.h"

void PortEnhancements_Init() {
    PortEnhancements_Register();
}

void PortEnhancements_Register() {
    // Register engine events
    REGISTER_EVENT(GameFrameUpdate);
    REGISTER_EVENT(VanillaBehavior);

    // Register rando events
    REGISTER_EVENT(OnActorSpawn);
}

void PortEnhancements_Exit() {
    // @port TODO
}
