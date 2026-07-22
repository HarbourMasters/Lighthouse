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
    VB_PLAY_BOOT_LOGOS,
    VB_PLAY_INTRO_CUTSCENE,
    VB_DISABLE_SNACKER,
    VB_VOID_OUT_GAME_OVER,
    VB_CCW_GNAWTY_SPRING_ROCK,
    VB_CCW_FLOWER_REPLANT,
    VB_TERMITE_MOUND_SLOPES,
    VB_CLAW_SWIPE_SLIDE,
    VB_BOGGY_RACE_GAME_OVER,
    VB_JINJO_CHARGE_SOUND,
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
    // Anchor: suppresses door-open camera lock when the flag came from a teammate, not us.
    VB_DOOR_OPEN_CAMERA,
    // CC rings water snap on run teardown (Anchor): when a teammate finishes the rings while we're still
    // running, we tear our run down and vanilla snaps the water to its risen height. The isConnected
    // listener (HookHandlers.cpp) suppresses that snap (should=false) because the finisher's WATER_RISE
    // broadcast already started the animated rise on us, so snapping would blink it back to done.
    VB_CC_RINGS_SNAP_WATER,
    // Lair door remote-open "already handled" test (Anchor): the sRemoteOpenDoorActor arm animates a
    // lair door open for teammates already in the room. It disarms when the door's persistent open flag
    // is set (the default, passed in) — correct for the world entrances, whose synced "seen" flag is
    // distinct from their "open" flag. The Grunty door is the exception: it's broadcast on its own open
    // flag (0xE2), already set when the arm arrives, so the isConnected listener (HookHandlers.cpp)
    // overrides it to key off the door's visual state (fully open = 0x1B) so it still animates live.
    // Args: (s32 doorActorId, s32 doorState).
    VB_LEVELDOOR_REMOTE_OPEN_DONE,
} VBehaviorID;

// Door ids for VB_DOOR_OPEN_CAMERA, identifying which flag(s) gate camera ownership.
typedef enum DoorCameraId {
    GV_DOOR_CAM_SUN,      // sun switch (flag 3)
    GV_DOOR_CAM_STAR,     // star switch / trapdoor (flag 5)
    GV_DOOR_CAM_KAZOOIE,  // beak-bomb door (flag 6)
    GV_DOOR_CAM_JINXY,    // Jinxy sneeze (flags 0, 1)
    MMM_DOOR_CAM_CHURCH,  // church door, Tumblar challenge (flag 0)
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
