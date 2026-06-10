#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include <libultraship/bridge/eventsbridge.h>

typedef enum VBehaviorID {
    VB_INIT_RETURN_TO_LAIR,
    VB_STATIC_CAMERA_SET,
    VB_STATIC_CAMERA_EXIT,
    VB_CAMERA_LIVE_ASPECT,
    VB_GRUNTY_DEFEATED_FLAG_BOSS,
    VB_PLAY_JIGGY_DANCE,
    VB_VOID_OUT_GAME_OVER,
    VB_CCW_GNAWTY_SPRING_ROCK,
    VB_CCW_FLOWER_REPLANT,
    VB_TERMITE_MOUND_SLOPES,
    VB_CLAW_SWIPE_SLIDE,
    VB_BOGGY_RACE_GAME_OVER,
    VB_JINJO_CHARGE_SOUND,
    VB_POSITIONAL_SFX_PAN,
    VB_YUMYUM_DROP,
    VB_RESET_DIALOG_LANGUAGE,
} VBehaviorID;

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
