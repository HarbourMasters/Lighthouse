// Always Update Actors
//
// Actors stop updating once Banjo is out of range. AUA lifts that for the actors in UpdatesAtAnyDistance, so a level
// seen from far away keeps moving. Each range check goes through VB_ACTOR_UPDATE_DISTANCE:
//   - core2/actor_array.c
//   - TTC/ch/blubber.c
//   - core2/ch/beeswarm.c
//   - core2/ch/trainers.c
//   - core2/ch/snowman.c
//   - core2/collision/scarabbeetle.c
//   - core2/collision/enemy.c
//
// Behavior changes:
//   - Enemies and NPCs keep moving as far as Banjo can see them. Their own range checks still decide when they react
//     to him.
//   - Sir Slush's 3150 wake range is lifted too. He idles before Banjo first comes near. With this on he keeps idling
//     and turning toward Banjo instead of playing his DIE animation and holding the last frame.
//   - Snippets and termites share two footstep sounds a frame per kind, handed out in update order (crab.c,
//     termite.c). Far walkers would take them from near ones, so they go to the two nearest Banjo instead.
//   - Chump fish bubbles and Lockup dust stay within the original update range (chumpfish.c, lockup.c).
//   - Scarab beetles and Buzzbombs only hold an sfx source within 4000, so far ones fly silently instead of draining
//     the shared pool (scarabbeetle.c, enemy.c).
//
// Left out:
//   - Actors that only react when Banjo comes near: Nibblys, Ice Cubes, Snarebears, Grabba, Slappa, and Rippers.
//   - Actors whose only far work is particles, which keep their distance checks: Mumbo tokens, fires, sparkles, smoke.
//   - Actors with nothing to show far away: Boggy, presents, the race sled, Twinkly Munchers, the Christmas tree
//     switch, magic carpets, Rubee and his egg pot, Histup, sarcophagi, Toots, Jinxy, Loggo, Quarrie, drips, Cheato,
//     Sir Slush's hat.
//   - Wozza, Brentilda and the bear cubs: they only turn toward Banjo.
//   - Purple Tee-Hee: only in Tumblar's Shed.
//   - lair/ff_lava_fireball.c: far spawners would use up Furnace Fun's two-fireball budget.
//   - FP/ch/twinklybox.c: its 800 check starts the box shaking. It isn't an update gate.
//   - FP/ch/boggyraceflag1.c, boggyraceflag2.c: their check also turns on the race gate's collision.

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Patches/Patches.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"

#include "functions.h"
extern "C" {
#include "enums.h"
}

#include <algorithm>
#include <unordered_map>
#include <utility>
#include <vector>

#define CVAR_ALWAYS_UPDATE_ACTORS CVAR_ENHANCEMENT("Gameplay.AlwaysUpdateActors")

namespace {

bool UpdatesAtAnyDistance(const Actor* actor) {
    switch (actor->actor_info->actorId) {
        case ACTOR_4_BIGBUTT:
        case ACTOR_5_TERMITE:
        case ACTOR_6_GRUBLIN:
        case ACTOR_A_CHUMP_FISH:
        case ACTOR_E_SMALL_BULL:
        case ACTOR_2C_TURBO_TALON_TRAINERS:
        case ACTOR_56_SHRAPNEL:
        case ACTOR_67_SNIPPET:
        case ACTOR_69_CLAM:
        case ACTOR_CA_TEEHEE:
        case ACTOR_E6_GLOOP:
        case ACTOR_F2_BLACK_SNIPPET:
        case ACTOR_F5_MUTIE_SNIPPET:
        case ACTOR_115_BLUBBER:
        case ACTOR_124_SIR_SLUSH:
        case ACTOR_134_BUZZBOMB:
        case ACTOR_151_LOCKUP_SLOW:
        case ACTOR_152_LOCKUP_MEDIUM:
        case ACTOR_153_LOCKUP_FAST:
        case ACTOR_166_TOPPER_THE_CARROT_A:
        case ACTOR_34D_BEE_SWARM:
        case ACTOR_34E_LIMBO:
        case ACTOR_34F_MUMMUM:
        case ACTOR_350_SEAMAN_GRUBLIN:
        case ACTOR_367_GRUNTLING_RED:
        case ACTOR_36F_TOPPER_THE_CARROT_B:
        case ACTOR_375_GRUBLIN_HOOD:
        case ACTOR_380_SCARAB_BEETLE:
        case ACTOR_3BF_GRUNTLING_BLUE:
        case ACTOR_3C0_GRUNTLING_BLACK:
            return true;
        default:
            return false;
    }
}

bool InView(Actor* actor) {
    f32 camera[3];
    viewport_getPosition_vec3f(camera);
    f32 range = 4000.0f * GraphicsPatches_GetDrawDistanceMultiplier() + 1000.0f;
    if (ml_distanceSquared_vec3f(actor->position, camera) > range * range) {
        return false;
    }
    return viewport_func_8024DB50(actor->position, 500.0f);
}

struct FootstepSlots {
    s32 frame = 0;
    std::vector<std::pair<f32, ActorMarker*>> walkers;
    ActorMarker* nearest[2] = {};
};

std::unordered_map<void (*)(Actor*), FootstepSlots> sFootstepSlots;

bool TakeFootstepSlot(Actor* actor) {
    FootstepSlots& slots = sFootstepSlots[actor->actor_info->update_func];
    s32 frame = globalTimer_getTime();
    if (slots.frame != frame) {
        size_t ranked = std::min<size_t>(2, slots.walkers.size());
        std::partial_sort(slots.walkers.begin(), slots.walkers.begin() + ranked, slots.walkers.end(),
                          [](const auto& a, const auto& b) { return a.first < b.first; });
        for (size_t i = 0; i < 2; i++) {
            slots.nearest[i] = i < ranked ? slots.walkers[i].second : nullptr;
        }
        slots.walkers.clear();
        slots.frame = frame;
    }

    f32 player[3];
    player_getPosition(player);
    slots.walkers.emplace_back(ml_distanceSquared_vec3f(actor->position, player), actor->marker);
    return actor->marker == slots.nearest[0] || actor->marker == slots.nearest[1];
}

} // namespace

void RegisterAlwaysUpdateActors_Init() {
    const bool enabled = CVarGetInteger(CVAR_ALWAYS_UPDATE_ACTORS, 0) && !RomhackConfig_IsActive();

    // To make the game world feel more alive with added draw distance,
    // these actors can always run their updates.
    COND_VB_SHOULD(VB_ACTOR_UPDATE_DISTANCE, EVENT_PRIORITY_NORMAL, enabled, {
        Actor* actor = va_arg(args, Actor*);
        if (!*should && !IsDemoMode() && UpdatesAtAnyDistance(actor) &&
            (actor->actor_info->unk18 == 0 || InView(actor))) {
            *should = func_8028F098() && !volatileFlag_get(VOLATILE_FLAG_BF);
        }
    });

    // Tracks actor footstep slots so a far actor can't take up slots
    // and end up silencing footsteps for actors actually close to Banjo.
    COND_VB_SHOULD(VB_ACTOR_FOOTSTEP_SLOT, EVENT_PRIORITY_NORMAL, enabled, {
        Actor* actor = va_arg(args, Actor*);
        if (!IsDemoMode()) {
            *should = TakeFootstepSlot(actor);
        }
    });

    // To save on performance, particles are controlled separately.
    COND_VB_SHOULD(VB_ACTOR_AMBIENT_PARTICLES, EVENT_PRIORITY_NORMAL, enabled, {
        Actor* actor = va_arg(args, Actor*);
        *should = func_803296D8(actor, actor->actor_info->unk18);
    });
}

static RegisterShipInitFunc initFunc(RegisterAlwaysUpdateActors_Init, { CVAR_ALWAYS_UPDATE_ACTORS });
