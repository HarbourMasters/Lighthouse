#include "Traps.h"

#include <algorithm>
#include <random>
#include <libultraship/bridge/consolevariablebridge.h>
#include "port/Rando/StaticData/StaticData.h"

#include "enums.h"
#include "functions.h"
#include "bsint.h"
#include "core2/core2.h"
#include "core2/gsworld.h"
#include "core2/statetimer.h"
#include "core2/gc/transition.h"

// Last fall's damage, which the splat state re-applies on entry (src/core2/ba/falldamage.c)
extern "C" {
extern struct {
    s32 damage;
    s32 state;
} D_8037C1C0;
}

namespace Rando {
namespace Traps {

static const RandoItemId kTraps[] = {
    RI_TRAP_DAMAGE,
    RI_TRAP_SLIDE,
    RI_TRAP_STUN,
};

static const f32 kSlideSeconds = 3.0f;

void Place(std::vector<RandoSaveCheck>& pool, uint32_t seed) {
    if (CVarGetInteger(Rando::StaticData::Options[RO_TRAPS].cvar, RO_GENERIC_OFF) == RO_GENERIC_OFF) {
        return;
    }

    std::vector<size_t> filler;
    for (size_t i = 0; i < pool.size(); i++) {
        RandoItemId id = pool[i].randoItemId;
        if (id == RI_BLUE_EGG || id == RI_EXTRA_LIFE || id == RI_HONEYCOMB) {
            filler.push_back(i);
        }
    }

    const auto& percentOption = Rando::StaticData::Options[RO_TRAP_PERCENTAGE];
    int32_t percent = std::clamp(CVarGetInteger(percentOption.cvar, percentOption.defaultValue), 0, 100);
    size_t count = filler.size() * percent / 100;

    std::mt19937 rng(seed);
    std::shuffle(filler.begin(), filler.end(), rng);
    for (size_t i = 0; i < count; i++) {
        pool[filler[i]].randoItemId = kTraps[rng() % std::size(kTraps)];
    }
}

static bool PlayerIsFree() {
    if (getGameMode() != GAME_MODE_3_NORMAL || gctransition_active() || gcdialog_hasCurrentTextId() ||
        map_getLevel(gsworld_getMap()) == LEVEL_D_CUTSCENE) {
        return false;
    }

    enum bs_e state = (enum bs_e)bs_getState();
    return !player_isDead() && !bsjig_inJiggyJig(state) && !baflag_isTrue(BA_FLAG_1B_TRANSFORMING) &&
           player_movementGroup() != BSGROUP_D_TRANSFORMING;
}

static bool BanjoOnGround() {
    return player_getTransformation() == TRANSFORM_1_BANJO && player_getWaterState() == BSWATERGROUP_0_NONE &&
           player_movementGroup() == BSGROUP_0_NONE && player_isStable();
}

bool CanApply(RandoItemId randoItemId) {
    if (!PlayerIsFree()) {
        return false;
    }

    switch (randoItemId) {
        case RI_TRAP_DAMAGE:
            // Same check the hazard interrupt makes; waiting keeps i-frames from eating the trap
            return baiFrame_getState() != 3 && baMarker_isCollidable();
        case RI_TRAP_SLIDE:
            return BanjoOnGround() && !stateTimer_isActive(STATE_TIMER_5_UNKNOWN);
        case RI_TRAP_STUN:
            return BanjoOnGround() && bs_getState() != BS_72_SPLAT;
        default:
            return true;
    }
}

void Apply(RandoItemId randoItemId) {
    switch (randoItemId) {
        case RI_TRAP_DAMAGE:
            // Mirrors hazards_update: the hurt interrupt alone doesn't kill at 0 health
            player_checkHazardInterrupt(0xD);
            if (item_empty(ITEM_14_HEALTH)) {
                bs_checkInterrupt(BS_INTR_13_FF_DEATH_SQUARE);
            }
            break;
        case RI_TRAP_SLIDE:
            stateTimer_set(STATE_TIMER_5_UNKNOWN, kSlideSeconds);
            break;
        case RI_TRAP_STUN:
            D_8037C1C0.damage = 0;
            bs_setState(BS_72_SPLAT);
            break;
        default:
            break;
    }
}

} // namespace Traps
} // namespace Rando
