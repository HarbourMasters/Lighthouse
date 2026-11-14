#include <libultraship/libultraship.h>
#include <atomic>

#include "port/ShipInit.hpp"
#include "port/UI/cvar_prefixes.h"
#include "port/Patches/Patches.h"

#define CVAR_MUSIC_VOLUME CVAR_SETTING("Volume.Music")
#define CVAR_SFX_VOLUME CVAR_SETTING("Volume.SFX")

static std::atomic<float> sMusicVolume = 1.0f;
static std::atomic<float> sSfxVolume = 1.0f;

float AudioVolume_GetMusic(void) {
    return sMusicVolume.load(std::memory_order_relaxed);
}

float AudioVolume_GetSfx(void) {
    return sSfxVolume.load(std::memory_order_relaxed);
}

static void RegisterAudioVolume_Init() {
    sMusicVolume = CVarGetInteger(CVAR_MUSIC_VOLUME, 100) / 100.0f;
    sSfxVolume = CVarGetInteger(CVAR_SFX_VOLUME, 100) / 100.0f;
}

static RegisterShipInitFunc initAudioVolume(RegisterAudioVolume_Init, { CVAR_MUSIC_VOLUME, CVAR_SFX_VOLUME });
