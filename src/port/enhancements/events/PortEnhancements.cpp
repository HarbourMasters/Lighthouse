#include "PortEnhancements.h"
#include "port/save/SaveManager.h"
#include "port/rando/Rando.h"
#include "port/ShipUtils.h"

#include <stdarg.h>

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
    REGISTER_EVENT(ViewportFrustumUpdate);
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
    REGISTER_EVENT(OnWarpDispatch);
    REGISTER_EVENT(OnSetJiggyList);

    // Register rando events
    REGISTER_EVENT(OnSaveLoad);
    REGISTER_EVENT(OnActorSpawn);
    REGISTER_EVENT(OnActorSaveState);
    REGISTER_EVENT(OnActorCollision);
    REGISTER_EVENT(OnActorDestroy);

    Rando::Init();
}

void PortEnhancements_Exit() {
    // @port TODO
}

extern "C" bool EventSystem_Should(VBehaviorID id, uint32_t result, ...) {
    // Only the external function can use the Variadic Function syntax.
    // To pass the va args to the next caller must be done using va_list and reading the args into it.
    // Because there can be N subscribers registered to each template call, the subscribers will be responsible for
    // creating a copy of this va_list to avoid incrementing the original pointer between calls.
    va_list args;
    va_start(args, result);

    // Because of default argument promotion, even though our incoming "result" is just a bool, it needs to be typed as
    // an int to be permitted to be used in va_start, otherwise it is undefined behavior.
    // Here we downcast back to a bool for our actual hook handlers.
    bool boolResult = static_cast<bool>(result);

    CALL_EVENT(VanillaBehavior, id, &boolResult, &args);

    va_end(args);
    return boolResult;
}
