/*
 * Nostalgia 64 Fun Facts
 *
 * The black gruntling raises a flag whenever it attacks, and a cellar hatch
 * watches for that flag and finishes Banjo off. Two hacky prop-update hooks
 * also re-poke the attack-interaction table every frame (Brentilda's row and
 * a Nabnut-window row), and the lockless iron gate is marker-swapped onto the
 * unused shrapnel slot so its setups can place it.
 *
 * The hack's gruntling hook overwrites the second actor_animationIsAt(0.7f)
 * call with its flag-setter, so the branch after it reuses the stale 0.18f
 * result and SFX_1F rides the claw-swipe frame instead of the 0.7f point.
 * Replicated faithfully below.
 */

#include <libultraship/bridge.h>
#include "port/Enhancements/Events/Hooks/Events.h"
#include "HackShared.h"

extern "C" {
#include "enums.h"
#include "functions.h"
#include "actor.h"

extern ActorInfo D_80393378;
extern ActorInfo chGruntlingBlack;
extern ActorInfo D_8038BC4C;

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

bool sBlackGruntlingAttack = false;

extern "C" void Nostalgia64_GruntlingBlackUpdate(Actor* thisx) {
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

extern "C" void Nostalgia64_CellarHatchUpdate(Actor* thisx) {
    func_802D3CE8(thisx);
    if (sBlackGruntlingAttack) {
        item_adjustByDiffWithHud(ITEM_14_HEALTH, -item_getCount(ITEM_14_HEALTH));
        bs_setState(BS_41_DIE);
        sBlackGruntlingAttack = false;
    }
}

} // namespace

void RegisterNostalgia64Patches() {
    D_80393378.markerId = MARKER_65_SHRAPNEL;

    for (int i = 0; i < 0xBB; i++) {
        if (D_80370AC0[i].markerId == MARKER_1E0_BRENTILDA) {
            D_80370AC0[i].vals[7] = 0x7000;
        } else if (D_80370AC0[i].markerId == MARKER_239_CCW_UNKNOWN_NABNUT_WINDOW) {
            D_80370AC0[i].flags = 0x0804;
        }
    }

    chGruntlingBlack.update_func = Nostalgia64_GruntlingBlackUpdate;
    D_8038BC4C.update_func = Nostalgia64_CellarHatchUpdate;

    HackShared_EnableNoteSignSuppression(ACTOR_93_INTRO_N64_CUBE);
}
