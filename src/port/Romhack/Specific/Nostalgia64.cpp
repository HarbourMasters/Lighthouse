#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/Shared/HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "actor.h"

extern ActorInfo D_80393378;
extern ActorInfo chGruntlingBlack;
extern ActorInfo D_8038BC4C;
extern ActorInfo D_8038BD48;

void chGruntling_initialize(Actor* thisx);
void func_802D3CE8(Actor* thisx);
}

struct HitboxRow {
    u16 markerId;
    u16 flags;
    u16 vals[11];
};
extern "C" HitboxRow D_80370AC0[];

namespace {

// Black Gruntling is recycled to use the Beta Vent enemy
bool sBlackGruntlingAttack = false;

extern "C" void Nostalgia64_VentUpdate(Actor* thisx) {
    Humanoid_Baddies_Actor* local = (Humanoid_Baddies_Actor*)&thisx->local;
    if (!thisx->volatile_initialized) {
        local->baddieSpecific = (thisx->modelCacheIndex == ACTOR_367_GRUNTLING_RED)    ? 2
                                : (thisx->modelCacheIndex == ACTOR_3BF_GRUNTLING_BLUE) ? 1
                                                                                       : 0;
        chGruntling_initialize(thisx);
    }
    humanoidBaddie_update(thisx);
    if (thisx->state == 5) {
        bool swipeFrame = actor_animationIsAt(thisx, 0.18f);
        if (swipeFrame) {
            sfx_playFadeShorthandDefault(SFX_2_CLAW_SWIPE, 1.0f, 28000, thisx->position, 1250, 2500);
        }
        sBlackGruntlingAttack = true;
        if (swipeFrame) {
            sfx_playFadeShorthandDefault(SFX_1F_HITTING_AN_ENEMY_3, 1.0f, 28000, thisx->position, 1250, 2500);
        }
    }
}

// Banjo takes damage while the vent is open
extern "C" void Nostalgia64_VentSmokeUpdate(Actor* thisx) {
    func_802D3CE8(thisx);
    for (int i = 0; i < 0xBB; i++) {
        if (D_80370AC0[i].markerId == MARKER_1F1_GRUNTLING_BLACK) {
            D_80370AC0[i].vals[10] = 0x7000;
            break;
        }
    }
    if (sBlackGruntlingAttack) {
        item_adjustByDiffWithHud(ITEM_14_HEALTH, -item_getCount(ITEM_14_HEALTH));
        bs_setState(BS_41_DIE);
        sBlackGruntlingAttack = false;
    }
}

// The 1881 Barrel Top was swapped for a breakable door
extern "C" void Nostalgia64_BreakableDoorUpdate(Actor* thisx) {
    func_802D3CE8(thisx);
    for (int i = 0; i < 0xBB; i++) {
        if (D_80370AC0[i].markerId == MARKER_9A_1881_BARREL_TOP) {
            D_80370AC0[i].flags = 0x0804;
            break;
        }
    }
}

} // namespace

void RegisterNostalgia64Patches() {
    D_80393378.markerId = MARKER_65_SHRAPNEL;

    chGruntlingBlack.update_func = Nostalgia64_VentUpdate;
    D_8038BC4C.update_func = Nostalgia64_VentSmokeUpdate;
    D_8038BD48.update_func = Nostalgia64_BreakableDoorUpdate;

    // The N64 cube stands in for the note signs
    HackShared_EnableNoteSignSuppression(ACTOR_93_INTRO_N64_CUBE);
}
