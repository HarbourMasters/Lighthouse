// Egg Aim, backported from Banjo-Tooie
// BanjoRecomp mod by ProxySaw

#include <libultraship/bridge.h>

#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/UI/Crosshair.h"

#include "enums.h"
#include "functions.h"
#include "core2/commonParticle.h"
#include "core2/abilityprogress.h"

extern "C" bool __ncba1p_fullyZoomedIn(void);

#define CVAR_NAME CVAR_ENHANCEMENT("Backports.EggAim")

namespace {

constexpr int kFireWindup = 10;

int sFiring = 0;

bool canAimEggs() {
    return can_egg() && player_getTransformation() == TRANSFORM_1_BANJO && !player_inWater();
}

} // namespace

void RegisterEggAimSpawn_Init() {
    REGISTER_LISTENER(EggHeadSpawn, EVENT_PRIORITY_NORMAL, [](IEvent* event) {
        if (!CVarGetInteger(CVAR_NAME, 0) || sFiring == 0) {
            return;
        }
        auto* ev = reinterpret_cast<EggHeadSpawn*>(event);
        float rotation[3];
        ncba1p_getZoomedInRotation(rotation);
        *ev->pitch = -rotation[0];
        *ev->spawnHeight = 100.0f;
        *ev->minVerticalVelocity = -99999.0f;
        *ev->yawBias = 0.0f;
        *ev->flattenTrajectory = 0;
    });
}

static RegisterShipInitFunc eggAimSpawnInitFunc(RegisterEggAimSpawn_Init);

void RegisterEggAim_Init() {
    // A wind-up frozen by turning Egg Aim off would otherwise mis-flag the next ordinary egg shot.
    sFiring = 0;

    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NAME, 0), [](IEvent*) {
        if (getGameMode() != GAME_MODE_3_NORMAL || balookat_getState() != 0) {
            sFiring = 0;
            return;
        }

        if (!canAimEggs()) {
            return;
        }

        // From where the zoom hides Banjo, not from the end of the zoom.
        if (ncba1p_getState() == FIRSTPERSON_STATE_1_ENTER && __ncba1p_fullyZoomedIn()) {
            Crosshair::ShowThisFrame();
        }

        if (ncba1p_getState() == FIRSTPERSON_STATE_2_IDLE) {
            Crosshair::ShowThisFrame();

            if (sFiring == 0 && bakey_pressed(BUTTON_Z)) {
                if (!item_empty(ITEM_D_EGGS)) {
                    sFiring = kFireWindup;
                } else {
                    item_adjustByDiffWithHud(ITEM_D_EGGS, 0);
                    coMusicPlayer_playMusic(COMUSIC_2C_BUZZER, 22000);
                }
            }
        }

        if (sFiring) {
            if (sFiring == kFireWindup) {
                func_8030E760(SFX_46_KAZOOIE_CHOKING_UP, 1.0f, 0x7fff);
            }
            if (sFiring == 1) {
                func_8030E760(SFX_57_KAZOOIE_HEGH, 1.0f, 0x7fff);
                commonParticle_new(COMMON_PARTICLE_1_EGG_HEAD, 1);
                item_dec(ITEM_D_EGGS);
                ability_setUsedWithDialog(ABILITY_USED_7_EGG);
            }
            sFiring--;
        }
    });
}

static RegisterShipInitFunc eggAimInitFunc(RegisterEggAim_Init, { CVAR_NAME });
