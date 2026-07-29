#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include <libultraship/bridge/eventsbridge.h>

typedef enum VBehaviorID {
    VB_INIT_RETURN_TO_LAIR,
    VB_PAUSE_MENU_PORTRAIT_DEPTH,
    VB_ZOOMBOX_TEXT_ADJUST, // text scale (shrink pause text) + X nudge (JP kana clearance)
    VB_STATIC_CAMERA_SET,
    VB_STATIC_CAMERA_EXIT,
    VB_CAMERA_LIVE_ASPECT,
    VB_CAMERA_FOLLOW,
    VB_GRUNTY_DEFEATED_FLAG_BOSS,
    VB_PLAY_JIGGY_DANCE,
    VB_PLAY_NOTEDOOR_DANCE,
    VB_PLAY_BOOT_LOGOS,
    VB_PLAY_INTRO_CUTSCENE,
    VB_DISABLE_SNACKER,
    VB_VOID_OUT_GAME_OVER,
    VB_SAVE_AND_EXIT,
    VB_CCW_GNAWTY_SPRING_ROCK,
    VB_CCW_FLOWER_REPLANT,
    VB_TERMITE_MOUND_SLOPES,
    VB_CLAW_SWIPE_SLIDE,
    VB_BOGGY_RACE_GAME_OVER,
    VB_JINJO_CHARGE_SOUND,
    // MM Chimpy stump: gates the rumble sfx that starts when the stump begins shaking.
    VB_MM_CHIMPY_STUMP_RUMBLE,
    VB_MM_CHIMPY_NOISE,
    // Gates the sfx a spline path node plays. Args: (Actor *pathWalker).
    VB_SPLINE_PATH_SFX,
    // Cancelling hands Mumbo's transform state to the listener. Args: (Actor *mumbo).
    VB_MUMBO_HUT_TRANSFORM_CUTSCENE,
    VB_YUMYUM_DROP,
    VB_OVERRIDE_BOTTLES_TEXT_CALLBACK,
    VB_OVERRIDE_MOLEHILL_ABILITY,
    VB_OVERRIDE_JIGGY_SPAWN,
    VB_OVERRIDE_PROP_SPAWN,
    VB_OVERRIDE_BUNDLE_SPAWN,
    VB_OVERRIDE_SNS_MAP_CHECK,
    VB_OVERRIDE_TIMED_DIALOGUE,
    VB_ENEMY_BECOME_BUNDLE,
    VB_UPDATE_JINJO_HUD,
    VB_SET_JINJO_COUNT,
    VB_BUNDLE_SPAWN_SET_ACTOR_DATA,
    VB_NAPPER_SET_JIGGY_POSITION,
    VB_RESET_DIALOG_LANGUAGE,
    // Cancellable at the single dialog choke point (gcdialog_showDialogConditional).
    // Listeners receive the dialog's text_id; returning should=true suppresses the popup.
    VB_OVERRIDE_DIALOG_SHOW,
    // Mr. Vile minigame (Anchor authority gating): cancelled on clients following a
    // remote authority so local random logic yields to network state.
    VB_VILE_YUMBLIE_EMERGE,
    VB_VILE_YUMBLIE_HIDE,
    VB_VILE_PLAYER_EAT_PIECE,
    VB_VILE_GAME_UPDATE,
    VB_VILE_CPU_AI,
    // Anchor: lets a remotely-watered CCW flower grow without local camera/fanfare/jiggy.
    VB_CCW_FLOWER_REMOTE_GROW,
    // Anchor: gates FP twinkly minigame start so only one client owns a run at a time.
    VB_FP_TWINKLY_START,
    // Anchor: true while the SM tutorial choice is still open to the local player. Checked
    // when opening the intro offer (first client to reach it claims ownership) and again
    // when an answer lands (a stale opt-out folds into opt-in once the choice closed).
    VB_SM_TUTORIAL_CHOICE_OPEN,
    // Anchor: SM ability molehills stay inert until the tutorial choice is made, so no
    // tutorial sequence can start while the decision is still open.
    VB_SM_MOLEHILL_ACTIVE,
    // Anchor: suppresses door-open camera lock when the flag came from a teammate, not us.
    VB_DOOR_OPEN_CAMERA,
    // CC rings water snap on run teardown (Anchor): suppressed when a teammate finished the rings.
    VB_CC_RINGS_SNAP_WATER,
    // Lair door remote-open "already handled" test (Anchor). Args: (s32 doorActorId, s32 doorState).
    VB_LEVELDOOR_REMOTE_OPEN_DONE,
    // Romhack port gates. Cancel to override the vanilla behavior;
    // args carry the site-specific data.
    VB_JIGGYSCORE_LEVEL_TOTAL,
    VB_PAUSEMENU_LEVEL_TO_PAGE,
    VB_PAUSEMENU_SET_NEXT_PAGE,
    VB_MAP_CHANGE_REQUEST,
    VB_VOID_OUT_RESPAWN_TRANSITION,
    VB_MAP_TRANSITION_IN_INDEX,
    VB_CCW_SEASON_SWITCH_PRESSED_INIT,
    VB_PAUSEMENU_DRAW_JOYSTICKS,
    VB_PAUSEMENU_BOLD_FONT_TEXTURE,
    VB_PAUSEMENU_LEVEL_NAME_X,
    VB_PAUSEMENU_ROW_VISIBLE,
    VB_NOTEDOOR_DRAW_NUMBER,
    VB_JIGGY_COLLECT_TUTORIAL,
    VB_HONEYCOMB_PUMPKIN_REQUIREMENT,
    VB_SKY_UPDATE,
    VB_SKY_DRAW_BACKDROP_RECT,
    VB_BRENTILDA_HEAL_DIALOG,
    VB_GROUND_HAZARD_ACTIVE,
    VB_GAMESELECT_START_NEW_GAME,
    VB_WARP_DISPATCH,
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
