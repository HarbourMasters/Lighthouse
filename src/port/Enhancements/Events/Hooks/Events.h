#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include <libultraship/bridge/eventsbridge.h>

// Grouped by the system that listens for them. Nothing depends on the numeric values --
// every reference is by name at compile time and none are serialized -- so ids can be
// added to the group they belong to rather than to the end.
typedef enum VBehaviorID {
    // Return to Lair restoration, and the pause-menu adjustments
    VB_INIT_RETURN_TO_LAIR,
    VB_PAUSE_MENU_PORTRAIT_DEPTH,
    VB_ZOOMBOX_TEXT_ADJUST, // text scale (shrink pause text) + X nudge (JP kana clearance)

    // Camera behavior
    VB_CUTSCENE_ACTOR_DRAW,
    VB_CAMERA_LIVE_ASPECT,
    VB_CAMERA_FOLLOW,
    VB_CAMERA_APPLY_SHAKE,

    // Cutscene and celebration skips
    VB_PLAY_BOOT_LOGOS,
    VB_PLAY_INTRO_CUTSCENE,
    VB_PLAY_JIGGY_DANCE,
    VB_PLAY_NOTEDOOR_DANCE,

    // Vanilla bug fixes and corrections
    VB_GRUNTY_DEFEATED_FLAG_BOSS,
    VB_VOID_OUT_GAME_OVER,
    VB_CCW_GNAWTY_SPRING_ROCK,
    VB_CCW_FLOWER_REPLANT,
    VB_TERMITE_MOUND_SLOPES,
    VB_CLAW_SWIPE_SLIDE,
    VB_BOGGY_RACE_GAME_OVER,
    VB_JINJO_CHARGE_SOUND,
    VB_ENEMY_BECOME_BUNDLE,
    VB_YUMYUM_DROP,
    VB_MM_CHIMPY_STUMP_RUMBLE,
    VB_MM_CHIMPY_NOISE,
    VB_SPLINE_PATH_SFX,
    VB_MAP_SAVESTATE_USE,

    // Gameplay options and cheats
    VB_DISABLE_SNACKER,
    VB_SAVE_AND_EXIT,
    VB_MUMBO_DETRANSFORM,

    // Rando object behavior
    VB_OVERRIDE_BOTTLES_TEXT_CALLBACK,
    VB_OVERRIDE_MOLEHILL_ABILITY,
    VB_OVERRIDE_JIGGY_SPAWN,
    VB_OVERRIDE_PROP_SPAWN,
    VB_OVERRIDE_BUNDLE_SPAWN,
    VB_BUNDLE_SPAWN_SET_ACTOR_DATA,
    VB_NAPPER_SET_JIGGY_POSITION,
    VB_OVERRIDE_SNS_MAP_CHECK,
    VB_OVERRIDE_TIMED_DIALOGUE,
    VB_UPDATE_JINJO_HUD,
    VB_SET_JINJO_COUNT,
    VB_COLLECTABLE_ALREADY_HELD,
    VB_XMAS_TREE_JIGGY_COLLIDABLE,

    // Dialog and localization
    VB_RESET_DIALOG_LANGUAGE,
    VB_OVERRIDE_DIALOG_SHOW,

    // Rendering and performance
    VB_CUBE_PROP_SORT,
    VB_GCLIGHTS_RECOLOR,
    VB_SNOW_CAMERA_ROTATION,
    VB_SNOW_RENDER_STATE,
    VB_MODEL_DRAWDIST_FADE_ALPHA,
    VB_SPRITE_RESTORE_ALPHA_COMPARE,
    VB_DRAWDIST_BOX_CULL,
    VB_PICTUREBOX_TARGET_FB,
    VB_PICTUREBOX_SUBMIT_FRAME,

    // Anchor
    VB_VILE_YUMBLIE_EMERGE,
    VB_VILE_YUMBLIE_HIDE,
    VB_VILE_PLAYER_EAT_PIECE,
    VB_VILE_GAME_UPDATE,
    VB_VILE_CPU_AI,
    VB_CCW_FLOWER_REMOTE_GROW,  // Lets a remotely-watered CCW flower grow without local camera/fanfare/jiggy.
    VB_FP_TWINKLY_START,        // Gates FP twinkly minigame start so only one client owns a run at a time.
    VB_SM_TUTORIAL_CHOICE_OPEN, // True while the SM tutorial choice is still open to the local player.
    VB_SM_MOLEHILL_ACTIVE,      // SM ability molehills stay inert until the tutorial choice is made.
    VB_DOOR_OPEN_CAMERA,        // Suppresses door-open camera lock when the flag came from a teammate, not us.
    VB_CC_RINGS_SNAP_WATER,     // CC rings water snap on run teardown: suppressed when a teammate finished the rings.
    VB_LEVELDOOR_REMOTE_OPEN_DONE, // Lair door remote-open "already handled" test.
    VB_CCW_PODIUM_DESPAWN,    // Despawn the unrevealed CCW puzzle podium: suppressed so a teammate's switch press can
                              // reveal it live.
    VB_JIGSAW_PICTURE_RESYNC, // Rebuild a lair podium's cached piece state when the synced flags disagree with it.

    // Romhack port gates
    VB_JIGGYSCORE_LEVEL_TOTAL,
    VB_HONEYCOMBSCORE_LEVEL_TOTAL,
    VB_MUMBO_HUT_INTERACT,
    VB_LEVEL_LOAD_RESET_SCORES,
    VB_LEVEL_LOAD_RESET_MAP_SETPIECES,
    VB_CUTSCENE_SKIP_REQUIRE_PROGRESS,
    VB_GAME_OVER_RETURN_MAP,
    VB_MAP_SAVESTATE_CLEAR_ALL,
    VB_LEVEL_LOAD_SAVESTATE_INIT,
    VB_RACE_VOID_OUT_FULL_TRANSITION,
    VB_PROGRESS_FLAG_DIALOG, // showProgressFlagDialog(): false = never show (and never set) progress dialogs.
    VB_LAIR_GRUNTY_TAUNTS,   // Grunty's random lair taunt timer block: false = skip entirely.
    VB_AMBIENCE_MAP_UPDATE,  // Per-map ambience/chan-mask switch: false = a listener replaced it.
    VB_PAUSEMENU_SNS_ITEMS,  // Overrides the collected-SNS-item count that gates the Stop 'N' Swop page.
    VB_PAUSEMENU_SNS_DRAW,   // Stop 'N' Swop totals display: false = a listener drew its own version.
    VB_PAUSEMENU_LEVEL_TO_PAGE,
    VB_PAUSEMENU_SET_NEXT_PAGE,
    VB_PAUSEMENU_DRAW_JOYSTICKS,
    VB_PAUSEMENU_BOLD_FONT_TEXTURE,
    VB_PAUSEMENU_LEVEL_NAME_X,
    VB_PAUSEMENU_ROW_VISIBLE,
    VB_MAP_CHANGE_REQUEST,
    VB_MAP_TRANSITION_IN_INDEX,
    VB_VOID_OUT_RESPAWN_TRANSITION,
    VB_EGG_FIRE_SFX,
    VB_WARP_KEEPS_MUSIC,
    VB_BUMP_REBOUNDS_PLAYER,
    VB_WARP_DISPATCH,
    VB_GAMESELECT_START_NEW_GAME,
    VB_NOTEDOOR_DRAW_NUMBER,
    VB_JIGGY_COLLECT_TUTORIAL,
    VB_HONEYCOMB_PUMPKIN_REQUIREMENT, // also read by rando
    VB_BRENTILDA_HEAL_DIALOG,
    VB_SKY_UPDATE,
    VB_SKY_DRAW_BACKDROP_RECT,
    VB_GROUND_HAZARD_ACTIVE,
    VB_CCW_SEASON_SWITCH_PRESSED_INIT,
    VB_XMAS_TREE_ICE_UPDATE,
    VB_BOGGY_HOME_VISIBLE,
} VBehaviorID;

typedef enum DoorCameraId {
    GV_DOOR_CAM_SUN,     // sun switch (flag 3)
    GV_DOOR_CAM_STAR,    // star switch / trapdoor (flag 5)
    GV_DOOR_CAM_KAZOOIE, // beak-bomb door (flag 6)
    GV_DOOR_CAM_JINXY,   // Jinxy sneeze (flags 0, 1)
    MMM_DOOR_CAM_CHURCH, // church door, Tumblar challenge (flag 0)
} DoorCameraId;

DEFINE_EVENT(VanillaBehavior, VBehaviorID id; bool* should; va_list * originalArgs;);

#ifdef __cplusplus
extern "C" {
#endif
extern bool EventSystem_Should(VBehaviorID id, uint32_t result, ...);
#ifdef __cplusplus
}
#endif

// Lighthouse variant of CALL_CANCELLABLE_RETURN_EVENT: returns the event payload's
// `result` field when a listener cancels, rather than a bare void return.
#undef CALL_CANCELLABLE_RETURN_EVENT
#define CALL_CANCELLABLE_RETURN_EVENT(eventType, ...)                                      \
    eventType eventType##_ = { { false }, __VA_ARGS__ };                                   \
    EventSystemCallEvent(eventType##ID, &eventType##_, __FILE__, __LINE__, FILE_AND_LINE); \
    if (eventType##_.Event.Cancelled) {                                                    \
        return eventType##_.result;                                                        \
    }

#define REGISTER_VB_SHOULD(idToCheck, priority, body)                \
    REGISTER_LISTENER(VanillaBehavior, priority, [](IEvent* event) { \
        auto* ev = reinterpret_cast<VanillaBehavior*>(event);        \
        if (ev->id == idToCheck) {                                   \
            bool* should = ev->should;                               \
            va_list args;                                            \
            va_copy(args, *ev->originalArgs);                        \
            body;                                                    \
            va_end(args);                                            \
        }                                                            \
    })

#define COND_HOOK(eventId, priority, condition, body)                \
    {                                                                \
        static ListenerID listenerId = -1;                           \
        if (listenerId != -1) {                                      \
            UNREGISTER_LISTENER(eventId, listenerId);                \
            listenerId = -1;                                         \
        }                                                            \
        if (condition) {                                             \
            listenerId = REGISTER_LISTENER(eventId, priority, body); \
        }                                                            \
    }
#define COND_ID_HOOK(eventId, id, priority, condition, body)         \
    {                                                                \
        static ListenerID listenerId = -1;                           \
        if (listenerId != -1) {                                      \
            UNREGISTER_LISTENER(eventId, listenerId);                \
            listenerId = -1;                                         \
        }                                                            \
        if (condition) {                                             \
            listenerId = REGISTER_LISTENER(eventId, priority, body); \
        }                                                            \
    }
#define COND_VB_SHOULD(id, priority, condition, body)            \
    {                                                            \
        static ListenerID listenerId = -1;                       \
        if (listenerId != -1) {                                  \
            UNREGISTER_LISTENER(VanillaBehavior, listenerId);    \
            listenerId = -1;                                     \
        }                                                        \
        if (condition) {                                         \
            listenerId = REGISTER_VB_SHOULD(id, priority, body); \
        }                                                        \
    }

#include "List/EngineEvent.h"
#include "List/BehaviorEvent.h"
#include "List/GameEvent.h"
#include "List/RandoEvent.h"
