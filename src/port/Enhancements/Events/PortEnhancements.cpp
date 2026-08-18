#include "PortEnhancements.h"
#include "port/Save/SaveManager.h"
#include "port/Rando/Rando.h"
#include "port/ShipUtils.h"

#include <stdarg.h>

#define INIT_EVENT_IDS

#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Nametag/Nametag.h"

void PortEnhancements_Init() {
    PortEnhancements_Register();
    // LoadGuiTextures();
}

void PortEnhancements_Register() {
    // Register engine events
    REGISTER_EVENT(GameFrameUpdate);
    REGISTER_EVENT(FrameDrawEnd);
    REGISTER_EVENT(OnControllerUpdate);
    REGISTER_EVENT(VanillaBehavior);
    REGISTER_EVENT(OnMapLoad);
    REGISTER_EVENT(OnMapLoadStub);
    REGISTER_EVENT(OnDialogLoaded);
    REGISTER_EVENT(OnModelLoad);
    REGISTER_EVENT(OnModelDisplayListLoad);
    REGISTER_EVENT(ViewportFrustumUpdate);
    REGISTER_EVENT(CameraRotationAuthored);
    REGISTER_EVENT(MapModelXluScale);
    REGISTER_EVENT(OnTransitionModelScale);
    REGISTER_EVENT(OnTransitionStateUpdate);
    REGISTER_EVENT(DrawDistanceCubeWidth);
    REGISTER_EVENT(OnActorTick);
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
    REGISTER_EVENT(ResolveBoldFontSlot);

    // Register draw events
    REGISTER_EVENT(OnParadeNameDraw);
    REGISTER_EVENT(OnJinjoHeadDraw);
    REGISTER_EVENT(ResolveSpriteHdPath);
    REGISTER_EVENT(OnBoldFontLetterBuilt);
    REGISTER_EVENT(ResolveBoldFontHd);
    REGISTER_EVENT(OnBoldFontReset);
    REGISTER_EVENT(OnWorldDraw);
    REGISTER_EVENT(OnPlayerDraw);
    REGISTER_EVENT(OnHudDraw);

    // Register behavior events
    REGISTER_EVENT(OnBeakSwimVelocitySet);
    REGISTER_EVENT(OnBoggyRaceSetSpeed);
    REGISTER_EVENT(OnMrVileSetSpeed);
    REGISTER_EVENT(OnFurnaceFunDialog);
    REGISTER_EVENT(OnGeoCull);
    REGISTER_EVENT(OnGruntyJinjonatorComplete);
    REGISTER_EVENT(OnIntroCutsceneCheck);
    REGISTER_EVENT(OnMiscCutscenesCheck);
    REGISTER_EVENT(OnTooieJiggyCollect);
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
    REGISTER_EVENT(OnWarpResolveDest);
    REGISTER_EVENT(OnNewGame);
    REGISTER_EVENT(OnGameStart);
    REGISTER_EVENT(EggHeadSpawn);
    REGISTER_EVENT(OnActorDestroy);
    REGISTER_EVENT(OnLevelReset);
    REGISTER_EVENT(OnCheckSpiralMountainAbilities);
    REGISTER_EVENT(OnReset);
    REGISTER_EVENT(SetAnimSpeedMult);
    REGISTER_EVENT(OnActorUpdate);

    // Register rando events
    REGISTER_EVENT(InitRandoEvents);
    REGISTER_EVENT(OnLoadFileSelect);
    REGISTER_EVENT(OnSaveLoad);
    REGISTER_EVENT(OnWarpDispatch);
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
