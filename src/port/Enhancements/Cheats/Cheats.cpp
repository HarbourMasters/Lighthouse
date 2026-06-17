// Cheats
//
// Cheat enhancement hooks.

#include <libultraship/bridge.h>
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"
#include "port/ShipInit.hpp"
#include "port/ShipUtils.h"

extern "C" {
#include "enums.h"
#include "core2/statetimer.h"
#include "bs_funcs.h"
#include "functions.h"

s32 port_getRomhackMaxEggs(void);
s32 port_getRomhackMaxGoldFeathers(void);
s32 port_getRomhackMaxRedFeathers(void);
}

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
            bool inTalonTrot = bsbtrot_inSet((enum bs_e)currentState) || bslongleg_inSet(0); // Also in longleg state

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

// Levitate — Hold L to float upward with gravity disabled
void RegisterLevitate_Init() {
    static bool levitateActive = false;
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_LEVITATE, 0), [](IEvent* event) {
        if (bakey_held(BUTTON_L)) {
            baphysics_set_gravity(0.0f);
            f32 pos[3];
            player_getPosition(pos);
            pos[1] += 20.0f;
            player_setPosition(pos);
            levitateActive = true;
        } else {
            // Only reset gravity once when L is released, not every frame
            if (levitateActive) {
                baphysics_reset_gravity();
                levitateActive = false;
            }
        }
    });
}

// ============================================================================
// TRANSFORMATION CHEATS
// ============================================================================

// Transformation cycling with D-pad Up/Down
// D-pad Up: Cycle forward through transformations (Banjo -> Mumbo -> Termite -> ... -> Wishy -> Banjo)
// D-pad Down: Cycle backward through transformations (Banjo -> Wishy -> ... -> Termite -> Mumbo -> Banjo)
void RegisterCycleTransform_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_CYCLE_TRANSFORM, 0), [](IEvent* event) {
        s32 currentTransform = (s32)player_getTransformation();

        // D-pad Up: Cycle forward through transformations
        if (bakey_pressed(BUTTON_D_UP)) {
            currentTransform++;
            if (currentTransform > TRANSFORM_7_WISHWASHY) {
                currentTransform = TRANSFORM_1_BANJO;
            }
            func_8028FB88((enum transformation_e)currentTransform);
        }
        // D-pad Down: Cycle backward through transformations
        else if (bakey_pressed(BUTTON_D_DOWN)) {
            currentTransform--;
            if (currentTransform < TRANSFORM_1_BANJO) {
                currentTransform = TRANSFORM_7_WISHWASHY;
            }
            func_8028FB88((enum transformation_e)currentTransform);
        }
    });
}

// Fast Transformation — speeds up Mumbo transformation animation by 3x
void RegisterFastTransform_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_FAST_TRANSFORM, 0), [](IEvent* event) {
        // Check if currently transforming
        if (baflag_isTrue(BA_FLAG_1B_TRANSFORMING)) {
            // Speed up the transformation timer (timer 0 controls animation progress)
            f32 remaining = batimer_get(0);
            if (remaining > 0.0f) {
                // Reduce timer by 2x remaining time per frame (3x total speed)
                batimer_incrementBy(0, -remaining * 2.0f);
            }
        }
    });
}

// ============================================================================
// MUMBO CHEATS
// ============================================================================

// Disable Mumbo untransform when going too far
void RegisterNoMumboUntransform_Init() {
    COND_HOOK(GameFrameUpdate, EVENT_PRIORITY_NORMAL, CVarGetInteger(CVAR_NO_MUMBO_UNTRANSFORM, 0), [](IEvent* event) {
        // Prevent Mumbo from triggering untransform dialog/warn
        // These functions check game state and show dialog
        // We disable them by setting a flag they check
        volatileFlag_set((enum volatile_flags_e)207, 1); // Prevents detransform warning
    });
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
