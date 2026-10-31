#include "PortEnhancements.h"
#include "port/Save/SaveManager.h"
#include "port/Rando/Rando.h"
#include "port/ShipUtils.h"

#include <libultraship/bridge/consolevariablebridge.h>
#include <stdarg.h>
#include <unordered_map>

#define INIT_EVENT_IDS

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Nametag/Nametag.h"

void PortEnhancements_Init() {
    PortEnhancements_Register();
    LoadGuiTextures();
}

void PortEnhancements_Register() {
    // Register engine events
    REGISTER_EVENT(GameFrameUpdate);
    REGISTER_EVENT(FrameDrawEnd);
    REGISTER_EVENT(OnControllerRead);
    REGISTER_EVENT(OnControllerUpdate);
    REGISTER_EVENT(OnMapLoad);
    REGISTER_EVENT(OnMapLoaded);
    REGISTER_EVENT(OnMapLoadStub);
    REGISTER_EVENT(OnDialogLoaded);
    REGISTER_EVENT(OnDialogUpdate);
    REGISTER_EVENT(OnModelLoad);
    REGISTER_EVENT(OnModelDisplayListLoad);
    REGISTER_EVENT(OnModelBinBuilt);
    REGISTER_EVENT(OnModelDraw);
    REGISTER_EVENT(OnModelRenderBegin);
    REGISTER_EVENT(OnModelRenderEnd);
    REGISTER_EVENT(ViewportFrustumUpdate);
    REGISTER_EVENT(OnViewportProjection);
    REGISTER_EVENT(CameraRotationAuthored);
    REGISTER_EVENT(OnDynamicCameraUpdate);
    REGISTER_EVENT(MapModelXluScale);
    REGISTER_EVENT(OnTransitionModelScale);
    REGISTER_EVENT(OnTransitionStateUpdate);
    REGISTER_EVENT(DrawDistanceCubeWidth);
    REGISTER_EVENT(CubeDrawRange);
    REGISTER_EVENT(MapUnderwaterTint);
    REGISTER_EVENT(OnActorTick);
    REGISTER_EVENT(OnActorDrawBegin);
    REGISTER_EVENT(OnActorDrawEnd);
    REGISTER_EVENT(ShouldActorDraw);
    REGISTER_EVENT(OnPropTick);
    REGISTER_EVENT(OnSpritePropTick);
    REGISTER_EVENT(LocalizeUiString);
    REGISTER_EVENT(OnFileSelectInfoBuild);

    // Register localization events
    REGISTER_EVENT(LocalizeFileSelectPrompt);
    REGISTER_EVENT(OnFileSelectLanguageRefresh);
    REGISTER_EVENT(OnFileSelectPortrait);
    REGISTER_EVENT(LocalizeParade);
    REGISTER_EVENT(ParadeCreditDialogId);
    REGISTER_EVENT(PresentDialogId);
    REGISTER_EVENT(ResolveBoldFontSlot);

    // Register draw events
    REGISTER_EVENT(OnParadeNameDraw);
    REGISTER_EVENT(OnJinjoHeadDraw);
    REGISTER_EVENT(ResolveSpriteHdPath);
    REGISTER_EVENT(ResolveAnimTextureHd);
    REGISTER_EVENT(OnBoldFontLetterBuilt);
    REGISTER_EVENT(ResolveBoldFontHd);
    REGISTER_EVENT(OnBoldFontReset);
    REGISTER_EVENT(OnDialogFontLoad);
    REGISTER_EVENT(OnWorldDrawBegin);
    REGISTER_EVENT(OnWorldDraw);
    REGISTER_EVENT(OnPlayerDraw);
    REGISTER_EVENT(OnHudDraw);
    REGISTER_EVENT(OnPauseMenuDraw);
    REGISTER_EVENT(OnPauseMenuFree);
    REGISTER_EVENT(OnZoomboxTextDraw);

    // Register behavior events
    REGISTER_EVENT(OnBeakSwimVelocitySet);
    REGISTER_EVENT(OnBoggyRaceSetSpeed);
    REGISTER_EVENT(OnMrVileSetSpeed);
    REGISTER_EVENT(OnFurnaceFunDialog);
    REGISTER_EVENT(OnQuizStateChange);
    REGISTER_EVENT(OnQuizUpdate);
    REGISTER_EVENT(OnLairRelease);
    REGISTER_EVENT(OnRollInit);
    REGISTER_EVENT(OnRollUpdate);
    REGISTER_EVENT(OnGeoCull);
    REGISTER_EVENT(OnGruntyJinjonatorComplete);
    REGISTER_EVENT(OnIntroCutsceneCheck);
    REGISTER_EVENT(OnMiscCutscenesCheck);
    REGISTER_EVENT(OnJigsawPodiumInput);
    REGISTER_EVENT(OnMumboTokenUpdate);
    REGISTER_EVENT(OnMumboTokenIdResolve);
    REGISTER_EVENT(OnNametagDraw);
    REGISTER_EVENT(OnPlayerAnimChange);
    REGISTER_EVENT(OnPlayerAnimReset);
    REGISTER_EVENT(OnPlayerAnimSubRangeChange);
    REGISTER_EVENT(OnPlayerTransformChange);
    REGISTER_EVENT(OnWaterPyramidTimer);
    REGISTER_EVENT(OnVileHoleStateChange);
    REGISTER_EVENT(OnVileGameStateChange);

    // Register game events
    REGISTER_EVENT(OnGameFlagSet);
    REGISTER_EVENT(OnItemCountChanged);
    REGISTER_EVENT(OnCollectibleCollected);
    REGISTER_EVENT(OnConsumableCollected);
    REGISTER_EVENT(OnNoteCollected);
    REGISTER_EVENT(OnJinjoCollected);
    REGISTER_EVENT(OnCarryThrow);
    REGISTER_EVENT(OnAbilityLearned);
    REGISTER_EVENT(OnJiggySpawned);
    REGISTER_EVENT(OnHoneycombDropSpawn);
    REGISTER_EVENT(OnTimedJiggyExpired);
    REGISTER_EVENT(OnPlayerDeath);
    REGISTER_EVENT(OnGameFileErase);
    REGISTER_EVENT(OnGameLoad);
    REGISTER_EVENT(OnGameSave);
    REGISTER_EVENT(OnGameErase);
    REGISTER_EVENT(OnBottlesBonusComplete);
    REGISTER_EVENT(OnSaveFileLoad);
    REGISTER_EVENT(OnSaveFileSave);
    REGISTER_EVENT(OnSaveClear);
    REGISTER_EVENT(OnPropInit);
    REGISTER_EVENT(OnWarpDispatch);
    REGISTER_EVENT(OnNamedWarp);
    REGISTER_EVENT(OnWarpMusic);
    REGISTER_EVENT(OnMapChangeRequest);
    REGISTER_EVENT(OnMapTransitionIn);
    REGISTER_EVENT(OnMapTransitionOut);
    REGISTER_EVENT(OnSpawnableActorsReset);
    REGISTER_EVENT(OnItemMax);
    REGISTER_EVENT(OnLevelItemMax);
    REGISTER_EVENT(OnNewGame);
    REGISTER_EVENT(OnDefaultBootMap);
    REGISTER_EVENT(OnGetMapSkyInfo);
    REGISTER_EVENT(OnGetMapModelDescription);
    REGISTER_EVENT(OnGetMapMusicTrack);
    REGISTER_EVENT(OnGetMapLevel);
    REGISTER_EVENT(OnGameStart);
    REGISTER_EVENT(EggHeadSpawn);
    REGISTER_EVENT(OnActorDestroy);
    REGISTER_EVENT(OnActorArrayFree);
    REGISTER_EVENT(OnMapFileLoad);
    REGISTER_EVENT(OnLevelReset);
    REGISTER_EVENT(OnCheckSpiralMountainAbilities);
    REGISTER_EVENT(OnReset);
    REGISTER_EVENT(SetAnimSpeedMult);
    REGISTER_EVENT(OnActorUpdate);

    // Register rando events
    REGISTER_EVENT(InitRandoEvents);
    REGISTER_EVENT(OnLoadFileSelect);
    REGISTER_EVENT(OnSaveLoad);
    REGISTER_EVENT(OnActorSpawn);
    REGISTER_EVENT(OnLoadActorSaveState);
    REGISTER_EVENT(OnSaveActorSaveState);
    REGISTER_EVENT(OnActorCollision);
    REGISTER_EVENT(OnFindActorFromActorId);
    REGISTER_EVENT(OnFindActorMarkerFromJiggyId);
    REGISTER_EVENT(OnFindClosestActorFromActorId);
    REGISTER_EVENT(OnSetJiggyList);
    REGISTER_EVENT(OnGetLevelSpecificFlag);
    REGISTER_EVENT(OnIsJiggyScoreCollected);
    REGISTER_EVENT(OnIsJiggyScoreSpawned);
    REGISTER_EVENT(SetRandoInfFlag);
    REGISTER_EVENT(OnRandoCheckObtained);
    REGISTER_EVENT(OnIsHoneycombScoreCollected);
    REGISTER_EVENT(ClearBundleDespawnQueue);
    REGISTER_EVENT(OnIsMumboTokenScoreCollected);
    REGISTER_EVENT(OnSnSItemState);

    Rando::Init();
}

void PortEnhancements_Exit() {
    // @port TODO
}

// Registration runs at boot or on the game thread, which also dispatches, so nothing here is locked.
namespace {
struct HookEvent {
    EventID event = -1;
    int32_t listeners = 0;
};

HookEvent sBehaviorHooks[VB_COUNT];

struct ActorEventHooks {
    ListenerID forwarder = -1;
    int32_t listeners = 0;
    std::unordered_map<int32_t, HookEvent> byActor;
};

std::unordered_map<EventID*, ActorEventHooks> sActorEventHooks;
} // namespace

ListenerID VanillaBehavior_RegisterListener(VBehaviorID id, const char* name, EventPriority priority,
                                            EventCallback callback, const char* file, int line) {
    HookEvent& hook = sBehaviorHooks[id];
    if (hook.event == -1) {
        hook.event = EventSystemRegisterEvent(name);
    }
    hook.listeners++;
    return EventSystemRegisterListener(hook.event, callback, priority, file, line);
}

void VanillaBehavior_UnregisterListener(VBehaviorID id, ListenerID listenerId) {
    HookEvent& hook = sBehaviorHooks[id];
    EventSystemUnregisterListener(hook.event, listenerId);
    hook.listeners--;
}

ListenerID ActorEvent_RegisterListener(EventID* event, EventCallback forward, const char* name, int32_t actorId,
                                       EventCallback callback, EventPriority priority, const char* file, int line) {
    ActorEventHooks& hooks = sActorEventHooks[event];
    if (hooks.listeners++ == 0) {
        hooks.forwarder = EventSystemRegisterListener(*event, forward, EVENT_PRIORITY_NORMAL, __FILE__, __LINE__);
    }
    HookEvent& hook = hooks.byActor[actorId];
    if (hook.event == -1) {
        hook.event = EventSystemRegisterEvent(name);
    }
    hook.listeners++;
    return EventSystemRegisterListener(hook.event, callback, priority, file, line);
}

void ActorEvent_UnregisterListener(EventID* event, int32_t actorId, ListenerID listenerId) {
    ActorEventHooks& hooks = sActorEventHooks[event];
    HookEvent& hook = hooks.byActor[actorId];
    EventSystemUnregisterListener(hook.event, listenerId);
    hook.listeners--;
    if (--hooks.listeners == 0) {
        EventSystemUnregisterListener(*event, hooks.forwarder);
        hooks.forwarder = -1;
    }
}

void ActorEvent_Forward(EventID* event, IEvent* payload) {
    Actor* actor = reinterpret_cast<ActorEventPayload*>(payload)->actor;
    if (actor == nullptr) {
        return;
    }
    const ActorEventHooks& hooks = sActorEventHooks.at(event);
    auto it = hooks.byActor.find(actor->modelCacheIndex); // the ACTOR_* id
    if (it != hooks.byActor.end() && it->second.listeners > 0) {
        EventSystemCallEvent(it->second.event, payload, __FILE__, __LINE__, FILE_AND_LINE);
    }
}

extern "C" bool EventSystem_Should(VBehaviorID id, uint32_t result, ...) {
    // Because of default argument promotion, even though our incoming "result" is just a bool, it needs to be typed as
    // an int to be permitted to be used in va_start, otherwise it is undefined behavior.
    // Here we downcast back to a bool for our actual hook handlers.
    bool boolResult = static_cast<bool>(result);

    const HookEvent& hook = sBehaviorHooks[id];
    if (hook.listeners == 0) {
        return boolResult;
    }

    // Only the external function can use the Variadic Function syntax.
    // To pass the va args to the next caller must be done using va_list and reading the args into it.
    // Because there can be N subscribers registered to each template call, the subscribers will be responsible for
    // creating a copy of this va_list to avoid incrementing the original pointer between calls.
    va_list args;
    va_start(args, result);

    VanillaBehavior event = { { false }, id, &boolResult, &args };
    EventSystemCallEvent(hook.event, &event, __FILE__, __LINE__, FILE_AND_LINE);

    va_end(args);
    return boolResult;
}

extern "C" bool EventSystem_ShouldActorDraw(Actor* actor) {
    bool should = true;
    CALL_EVENT(ShouldActorDraw, actor, &should);
    return should;
}

// 1, or -1 when the player inverted the control.
extern "C" int EventSystem_InvertControl(InvertControlType type) {
    switch (type) {
        case INVERT_CAMERA_X:
            return CVarGetInteger(CVAR_ENHANCEMENT("Camera.InvertX"), 0) ? -1 : 1;
        case INVERT_CAMERA_Y:
            return CVarGetInteger(CVAR_ENHANCEMENT("Camera.InvertY"), 0) ? -1 : 1;
        case INVERT_FIRST_PERSON_X:
            return CVarGetInteger(CVAR_ENHANCEMENT("Camera.FirstPerson.InvertX"), 0) ? -1 : 1;
    }
    return 1;
}
