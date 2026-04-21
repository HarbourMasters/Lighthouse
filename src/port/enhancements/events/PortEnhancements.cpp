#include "PortEnhancements.h"
#include "port/save/SaveManager.h"

#define INIT_EVENT_IDS

#include "port/enhancements/events/hooks/Events.h"
#include "port/nametag.h"

void PortEnhancements_Init() {
    PortEnhancements_Register();
}

void PortEnhancements_Register() {
    // Register engine events
    REGISTER_EVENT(GameFrameUpdate);
    REGISTER_EVENT(FrameDrawEnd);
    REGISTER_EVENT(VanillaBehavior);
    REGISTER_EVENT(OnMapLoad);
    REGISTER_EVENT(OnActorTick);
    REGISTER_EVENT(OnPropTick);
    REGISTER_EVENT(OnSpritePropTick);

    // Register behavior events
    REGISTER_EVENT(OnBeakSwimVelocitySet);
    REGISTER_EVENT(OnBoggyRaceSetSpeed);
    REGISTER_EVENT(OnBootLogosCheck);
    REGISTER_EVENT(OnFurnaceFunDialog);
    REGISTER_EVENT(OnGruntyJinjonatorComplete);
    REGISTER_EVENT(OnIntroCutsceneCheck);
    REGISTER_EVENT(OnMumboTokenUpdate);
    REGISTER_EVENT(OnWaterPyramidTimer);
    REGISTER_EVENT(OnNametagDraw);

    // Register game events
    REGISTER_EVENT(OnGameLoad);
    REGISTER_EVENT(OnGameSave);
    REGISTER_EVENT(OnSaveFileLoad);
    REGISTER_EVENT(OnSaveFileSave);
    REGISTER_EVENT(OnPropInit);

    // Register game events
    REGISTER_EVENT(MapTransitionEnd);

    // Register rando events
    REGISTER_EVENT(OnActorSpawn);
    REGISTER_EVENT(OnActorCollision);
    REGISTER_EVENT(OnActorDestroy);
}

void PortEnhancements_Exit() {
    // @port TODO
}
