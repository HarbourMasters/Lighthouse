// Cheats
//
// Cheat enhancement hooks.

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/Romhack/RomhackConfig.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"

#include "enums.h"
#include "core2/statetimer.h"
#include "core2/ba/physics.h"
#include "bs_funcs.h"
#include "functions.h"

// ============================================================================
// CVAR DEFINITIONS
// ============================================================================

#define CVAR_INFINITE_HEALTH CVAR_ENHANCEMENT("Cheats.InfiniteHealth")
#define CVAR_INFINITE_AIR CVAR_ENHANCEMENT("Cheats.InfiniteAir")
#define CVAR_INFINITE_LIVES CVAR_ENHANCEMENT("Cheats.InfiniteLives")
#define CVAR_INFINITE_EGGS CVAR_ENHANCEMENT("Cheats.InfiniteEggs")
#define CVAR_INFINITE_RED_FEATHERS CVAR_ENHANCEMENT("Cheats.InfiniteRedFeathers")
#define CVAR_INFINITE_GOLD_FEATHERS CVAR_ENHANCEMENT("Cheats.InfiniteGoldFeathers")
#define CVAR_INFINITE_BOOTS_SNEAKERS CVAR_ENHANCEMENT("Cheats.InfiniteBootsSneakers")
#define CVAR_TALON_TROT_CYCLE CVAR_ENHANCEMENT("Cheats.TalonTrotCycle")
#define CVAR_LEVITATE CVAR_ENHANCEMENT("Cheats.Levitate")
#define CVAR_NO_MUMBO_UNTRANSFORM CVAR_ENHANCEMENT("Cheats.NoMumboUntransform")
#define CVAR_CYCLE_TRANSFORM CVAR_ENHANCEMENT("Cheats.CycleTransform")
#define CVAR_FAST_TRANSFORM CVAR_ENHANCEMENT("Cheats.FastTransform")

// ============================================================================
// INFINITE ITEMS / STATS
// ============================================================================

// Infinite health — drives the game's native sandcastle infinite-health flag
// so item_adjustByDiff's existing infinite-item gate handles it.
void RegisterInfiniteHealth_Init() {
    volatileFlag_set(VOLATILE_FLAG_94_SANDCASTLE_INFINITE_HEALTH, 0);
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_INFINITE_HEALTH, 0),
              [](IEvent* event) { volatileFlag_set(VOLATILE_FLAG_94_SANDCASTLE_INFINITE_HEALTH, 1); });
}

// Infinite Air — uses native infinite-air flag
void RegisterInfiniteAir_Init() {
    volatileFlag_set(VOLATILE_FLAG_96_SANDCASTLE_INFINITE_AIR, 0);
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_INFINITE_AIR, 0),
              [](IEvent* event) { volatileFlag_set(VOLATILE_FLAG_96_SANDCASTLE_INFINITE_AIR, 1); });
}

// Infinite Lives — caps at 9 (max displayable)
void RegisterInfiniteLives_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_INFINITE_LIVES, 0), [](IEvent* event) {
        // Only refill during actual gameplay (not in file select or demo mode)
        if (gsworld_getMap() != MAP_91_FILE_SELECT && !IsDemoMode()) {
            if (item_getCount(ITEM_16_LIFE) < 9) {
                item_set(ITEM_16_LIFE, 9);
            }
        }
    });
}

// Infinite Eggs — refills to current max (respects Cheato upgrades)
void RegisterInfiniteEggs_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_INFINITE_EGGS, 0), [](IEvent* event) {
        // Only refill during actual gameplay (not in file select or demo mode)
        if (gsworld_getMap() != MAP_91_FILE_SELECT && !IsDemoMode()) {
            s32 currentCount = item_getCount(ITEM_D_EGGS);
            s32 maxCount = port_getRomhackMaxEggs();
            if (maxCount < 0)
                maxCount = 100; // Default when romhack doesn't override
            if (currentCount < maxCount) {
                item_set(ITEM_D_EGGS, maxCount);
            }
        }
    });
}

// Infinite Red Feathers — refills to current max (respects Cheato upgrades)
void RegisterInfiniteRedFeathers_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_INFINITE_RED_FEATHERS, 0), [](IEvent* event) {
        // Only refill during actual gameplay (not in file select or demo mode)
        if (gsworld_getMap() != MAP_91_FILE_SELECT && !IsDemoMode()) {
            s32 currentCount = item_getCount(ITEM_F_RED_FEATHER);
            s32 maxCount = port_getRomhackMaxRedFeathers();
            if (maxCount < 0)
                maxCount = 50; // Default when romhack doesn't override
            if (currentCount < maxCount) {
                item_set(ITEM_F_RED_FEATHER, maxCount);
            }
        }
    });
}

// Infinite Gold Feathers — refills to current max (respects Cheato upgrades)
void RegisterInfiniteGoldFeathers_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_INFINITE_GOLD_FEATHERS, 0),
              [](IEvent* event) {
                  // Only refill during actual gameplay (not in file select or demo mode)
                  if (gsworld_getMap() != MAP_91_FILE_SELECT && !IsDemoMode()) {
                      s32 currentCount = item_getCount(ITEM_10_GOLD_FEATHER);
                      s32 maxCount = port_getRomhackMaxGoldFeathers();
                      if (maxCount < 0)
                          maxCount = 10; // Default when romhack doesn't override
                      if (currentCount < maxCount) {
                          item_set(ITEM_10_GOLD_FEATHER, maxCount);
                      }
                  }
              });
}

// ============================================================================
// TIMERS & ABILITIES
// ============================================================================

// Infinite Boots & Sneakers — Keeps boots and sneakers from expiring
void RegisterBootsAndSneakersTimer_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_INFINITE_BOOTS_SNEAKERS, 0),
              [](IEvent* event) {
                  // STATE_TIMER_2_LONGLEG - Trot Shoes (boots)
                  if (stateTimer_isActive(STATE_TIMER_2_LONGLEG)) {
                      stateTimer_set(STATE_TIMER_2_LONGLEG, 999.0f);
                  }
                  // STATE_TIMER_3_TURBO_TALON - Sneakers
                  if (stateTimer_isActive(STATE_TIMER_3_TURBO_TALON)) {
                      stateTimer_set(STATE_TIMER_3_TURBO_TALON, 999.0f);
                  }
              });
}

// D-pad Talon Trot Cycling — toggle between Normal <-> Boots <-> Sneakers with D-pad
void RegisterTalonTrotCycle_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_TALON_TROT_CYCLE, 0), [](IEvent* event) {
        // D-pad cycling - only works while Talon Trot is active
        if (bakey_pressed(BUTTON_D_RIGHT) || bakey_pressed(BUTTON_D_LEFT)) {
            s32 currentState = bs_getState();
            bool inTalonTrot = bsbtrot_inSet(currentState) || bslongleg_inSet(currentState);

            if (inTalonTrot) {
                bool inBoots = stateTimer_isActive(STATE_TIMER_2_LONGLEG);
                bool inSneakers = stateTimer_isActive(STATE_TIMER_3_TURBO_TALON);

                if (!inBoots && !inSneakers) {
                    // Normal -> Boots (D-pad Right) or Normal -> Sneakers (D-pad Left)
                    if (bakey_pressed(BUTTON_D_RIGHT)) {
                        bs_setState(BS_26_LONGLEG_IDLE);
                        stateTimer_set(STATE_TIMER_2_LONGLEG, 999.0f);
                    } else {
                        stateTimer_set(STATE_TIMER_3_TURBO_TALON, 999.0f);
                    }
                } else if (inBoots) {
                    // Boots -> Sneakers (D-pad Right) or Boots -> Normal (D-pad Left)
                    if (bakey_pressed(BUTTON_D_RIGHT)) {
                        bs_setState(BS_15_BTROT_IDLE);
                        stateTimer_clear(STATE_TIMER_2_LONGLEG);
                        stateTimer_set(STATE_TIMER_3_TURBO_TALON, 999.0f);
                    } else {
                        bs_setState(BS_15_BTROT_IDLE);
                        stateTimer_clear(STATE_TIMER_2_LONGLEG);
                    }
                } else {
                    // Sneakers -> Normal (D-pad Right) or Sneakers -> Boots (D-pad Left)
                    if (bakey_pressed(BUTTON_D_RIGHT)) {
                        stateTimer_clear(STATE_TIMER_3_TURBO_TALON);
                    } else {
                        bs_setState(BS_26_LONGLEG_IDLE);
                        stateTimer_clear(STATE_TIMER_3_TURBO_TALON);
                        stateTimer_set(STATE_TIMER_2_LONGLEG, 999.0f);
                    }
                }
            }
        }
    });
}

// ============================================================================
// MOVEMENT CHEATS
// ============================================================================

// Levitate — Hold L to float straight up; tapping L out of a damaging fall cancels the fall.
void RegisterLevitate_Init() {
    static const f32 LEVITATE_VELOCITY = 500.0f;
    static bool levitateActive = false;
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_LEVITATE, 0), [](IEvent* event) {
        // Suspend levitation while dialog is up.
        // Cancel fall damage and the tumble/splat animation if pressed mid-fall.
        if (bakey_held(BUTTON_L) && !gcdialog_hasCurrentTextId()) {
            if (bakey_pressed(BUTTON_L) && !player_isStable()) {
                s32 fallDamage = 0;
                if (bafalldamage_get_damage(&fallDamage) != 0) {
                    bafalldamage_start();
                    bs_setState(BS_1_IDLE);
                }
            }
            baphysics_set_gravity(0.0f);
            baphysics_set_vertical_velocity(LEVITATE_VELOCITY);
            levitateActive = true;
        } else if (levitateActive) {
            // Only reset gravity once when L is released, not every frame.
            baphysics_reset_gravity();
            levitateActive = false;
        }
    });
}

// ============================================================================
// TRANSFORMATION CHEATS
// ============================================================================

static const s32 BAANIM_WISHYWASHY = 0x80;
static bool isWishyWashyUnlocked() {
    return (baanim_getActiveBottlesBonusMask() & BAANIM_WISHYWASHY) != 0;
}

static bool jiggyCollectInFlight() {
    return baflag_isTrue(BA_FLAG_7_TOUCHING_JIGGY) || bsjig_inJiggyJig((enum bs_e)bs_getState());
}

// Transformation cycling with D-pad Up/Down
// D-pad Up: Cycle forward through transformations (Banjo -> Termite -> ... -> Bee -> [Wishy] -> Banjo)
// D-pad Down: Cycle backward through transformations (Banjo -> [Wishy] -> Bee -> ... -> Termite -> Banjo)
void RegisterCycleTransform_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_CYCLE_TRANSFORM, 0), [](IEvent* event) {
        if (jiggyCollectInFlight()) {
            return;
        }

        s32 currentTransform = (s32)player_getTransformation();

        // D-pad Up: Cycle forward through transformations
        if (bakey_pressed(BUTTON_D_UP)) {
            currentTransform++;
            if (currentTransform > TRANSFORM_7_WISHWASHY) {
                currentTransform = TRANSFORM_1_BANJO;
            }
            if (currentTransform == TRANSFORM_7_WISHWASHY && !isWishyWashyUnlocked()) {
                currentTransform = TRANSFORM_1_BANJO; // skip Wishy Washy -> wrap to Banjo
            }
            player_transform((enum transformation_e)currentTransform);
        }
        // D-pad Down: Cycle backward through transformations
        else if (bakey_pressed(BUTTON_D_DOWN)) {
            currentTransform--;
            if (currentTransform < TRANSFORM_1_BANJO) {
                currentTransform = TRANSFORM_7_WISHWASHY;
            }
            if (currentTransform == TRANSFORM_7_WISHWASHY && !isWishyWashyUnlocked()) {
                currentTransform = TRANSFORM_6_BEE; // skip Wishy Washy -> step to Bee
            }
            player_transform((enum transformation_e)currentTransform);
        }
    });
}

void RegisterFastTransform_Init() {
    COND_HOOK(SetAnimSpeedMult, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_FAST_TRANSFORM, 0), [](IEvent* event) {
        if (baflag_isTrue(BA_FLAG_1B_TRANSFORMING)) {
            SetAnimSpeedMult* ev = reinterpret_cast<SetAnimSpeedMult*>(event);
            if (ev->id == 0) {
                *ev->mult *= 8;
            }
        }
    });

    COND_HOOK(OnActorUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_FAST_TRANSFORM, 0), [](IEvent* event) {
        OnActorUpdate* ev = reinterpret_cast<OnActorUpdate*>(event);
        if (ev->actor->actor_info->actorId != ACTOR_7_MUMBO) {
            return;
        }

        if (ev->actor->state == 4 || ev->actor->state == 5) {
            anctrl_setDuration(ev->actor->anctrl, 7.5f / 8.0f);
        }
    });
}

// ============================================================================
// MUMBO CHEATS
// ============================================================================

// Disable Mumbo untransform when going too far
void RegisterNoMumboUntransform_Init() {
    COND_VB_SHOULD(VB_MUMBO_DETRANSFORM, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NO_MUMBO_UNTRANSFORM, 0),
                   { *should = false; });
}

// ============================================================================
// REGISTRATION
// ============================================================================

static RegisterShipInitFunc initInfiniteHealthFunc(RegisterInfiniteHealth_Init, { CVAR_INFINITE_HEALTH });
static RegisterShipInitFunc initInfiniteAirFunc(RegisterInfiniteAir_Init, { CVAR_INFINITE_AIR });
static RegisterShipInitFunc initInfiniteLivesFunc(RegisterInfiniteLives_Init, { CVAR_INFINITE_LIVES });
static RegisterShipInitFunc initInfiniteEggsFunc(RegisterInfiniteEggs_Init, { CVAR_INFINITE_EGGS });
static RegisterShipInitFunc initInfiniteRedFeathersFunc(RegisterInfiniteRedFeathers_Init,
                                                        { CVAR_INFINITE_RED_FEATHERS });
static RegisterShipInitFunc initInfiniteGoldFeathersFunc(RegisterInfiniteGoldFeathers_Init,
                                                         { CVAR_INFINITE_GOLD_FEATHERS });
static RegisterShipInitFunc initBootsAndSneakersTimerFunc(RegisterBootsAndSneakersTimer_Init,
                                                          { CVAR_INFINITE_BOOTS_SNEAKERS });
static RegisterShipInitFunc initBootCycleFunc(RegisterTalonTrotCycle_Init, { CVAR_TALON_TROT_CYCLE });
static RegisterShipInitFunc initLevitateFunc(RegisterLevitate_Init, { CVAR_LEVITATE });
static RegisterShipInitFunc initCycleTransformFunc(RegisterCycleTransform_Init, { CVAR_CYCLE_TRANSFORM });
static RegisterShipInitFunc initFastTransformFunc(RegisterFastTransform_Init, { CVAR_FAST_TRANSFORM });
static RegisterShipInitFunc initNoMumboUntransformFunc(RegisterNoMumboUntransform_Init, { CVAR_NO_MUMBO_UNTRANSFORM });
