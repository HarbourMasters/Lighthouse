#pragma once

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <libultraship/bridge/eventsbridge.h>

// Grouped by the system that listens for them. Nothing depends on the numeric values --
// every reference is by name at compile time and none are serialized -- so ids can be
// added to the group they belong to rather than to the end.
typedef enum VBehaviorID {
    // Return to Lair restoration, and the pause-menu adjustments

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*struct1As` (D_8036C4E0, the main menu's options)
    VB_INIT_RETURN_TO_LAIR,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `**Gfx`
    // - `**Mtx`
    // - `**Vtx`
    // - `**GcZoombox` (the four menu boxes)
    // - `s32` (menu, promoted from `u8`)
    // - `s32` (selection, promoted from `u8`)
    VB_PAUSEMENU_DRAW_ZOOMBOXES,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_PAUSEMENU_UPDATE,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `s32` (page_id)
    VB_PAUSEMENU_PRINT_LEVEL_NAME,

    // Camera behavior
    VB_CAMERA_FOLLOW,
    VB_CAMERA_APPLY_SHAKE,

    // #### `result`
    // ```c
    // !balookat_getState() && player_movementGroup() != BSGROUP_4_LOOK && batimer_get(7) == 0.0f &&
    //     bainput_should_zoom_out_camera()
    // ```
    // #### `args`
    // - None
    VB_CAMERA_CYCLE_ZOOM,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_CAMERA_SWIVEL,

    // Cutscene and celebration skips

    // #### `result`
    // ```c
    // i_ptr->unk4 != 0 && i_ptr->unkC != NULL
    // ```
    // #### `args`
    // - `s32` (i_ptr->unk0, promoted from `s16`)
    VB_CUTSCENE_TRIGGER_UPDATE,
    VB_PLAY_INTRO_CUTSCENE,

    // #### `result`
    // ```c
    // !sp2C
    // ```
    // #### `args`
    // - None
    VB_PLAY_JIGGY_DANCE,

    // A listener that returns false shows the dialog itself.
    // #### `result`
    // ```c
    // s0
    // ```
    // #### `args`
    // - `s32` (s0)
    VB_JIGGY_TUTORIAL_DIALOG,

    // #### `result`
    // ```c
    // comusic_isTrackQueued(COMUSIC_30_5TH_JINJO_COLLECTED)
    // ```
    // #### `args`
    // - None
    VB_JINJO_JINGLE_LOOK_AT,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_JIGGY_JINGLE_DUCK_MUSIC,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_JIGGY_FANFARE_MUSIC_DELAY,
    VB_PLAY_NOTEDOOR_DANCE,

    // Vanilla bug fixes and corrections
    VB_GRUNTY_DEFEATED_FLAG_BOSS,

    // #### `result`
    // ```c
    // item_empty(ITEM_16_LIFE)
    // ```
    // #### `args`
    // - None
    VB_VOID_OUT_GAME_OVER,
    VB_CCW_GNAWTY_SPRING_ROCK,
    VB_CCW_FLOWER_REPLANT,

    // A listener that returns false sets the slide rate itself.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*f32` (sp28)
    VB_TERMITE_MOUND_SLOPES,

    // #### `result`
    // ```c
    // bakey_pressed(BUTTON_B) && can_claw()
    // ```
    // #### `args`
    // - None
    VB_CLAW_SWIPE_SLIDE,

    // Game over at no lives left, or a respawn. A listener that returns false handles the death itself.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_HANDLE_PLAYER_DEATH,
    VB_JINJO_CHARGE_SOUND,
    VB_ENEMY_BECOME_BUNDLE,
    VB_YUMYUM_DROP,
    VB_MM_CHIMPY_STUMP_RUMBLE,
    VB_MM_CHIMPY_NOISE,
    VB_SPLINE_PATH_SFX,

    // #### `result`
    // ```c
    // !volatileFlag_get(VOLATILE_FLAG_1F_IN_CHARACTER_PARADE) // mapSavestate_save
    // true                                                    // mapSavestate_apply
    // ```
    // #### `args`
    // - `s32` (outgoing_mode, or mode for the apply)
    VB_MAP_SAVESTATE_USE,

    // Gameplay options and cheats

    // Whether an actor is near enough to the player to update.
    // #### `result`
    // ```c
    // func_803296D8(actor, temp_s1)                      // the actor array
    // subaddie_playerIsWithinSphereAndActive(this, 2500) // Blubber
    // subaddie_playerIsWithinSphere(this, 0xFA0)         // the bee swarm
    // subaddie_playerIsWithinSphere(this, 4000)          // the snowman, enemies
    // subaddie_playerIsWithinSphereAndActive(this, 3150) // the snowman
    // func_803296D8(this, 2000)                          // the trainers
    // false                                              // the scarab beetles
    // ```
    // #### `args`
    // - `*Actor` (this)
    VB_ACTOR_UPDATE_DISTANCE,

    // Whether this walker takes one of its kind's two footsteps this frame.
    // #### `result`
    // ```c
    // (this->state == CRAB_STATE_2_IDLE_UNK) || (this->state == CRAB_STATE_3_TURN_TO_PLAYER) ||
    //     (this->state == CRAB_STATE_4_CHASE_PLAYER) // the crabs
    // true                                           // the termites
    // ```
    // #### `args`
    // - `*Actor` (this)
    VB_ACTOR_FOOTSTEP_SLOT,
    VB_ACTOR_AMBIENT_PARTICLES,

    // #### `result`
    // ```c
    // anctrl_isAt(sp58, 0.6905f)
    // ```
    // #### `args`
    // - `*AnimCtrl` (sp58)
    // - `*f32` (D_8037D338)
    VB_BEAK_BOMB_WINDUP,

    // #### `result`
    // ```c
    // sp48[0] * sp48[0] + sp48[1] * sp48[1] + sp48[2] * sp48[2] > 16000000.0f
    // ```
    // #### `args`
    // - None
    VB_BEAK_BOMB_RANGE_END,
    VB_DISABLE_SNACKER,

    // #### `result`
    // ```c
    // is_level_id_valid || is_map_game_over
    // ```
    // #### `args`
    // - None
    VB_SAVE_AND_EXIT,
    VB_MUMBO_DETRANSFORM,
    VB_ZOOMBOX_FAST_VOICE,

    // Rando object behavior
    VB_OVERRIDE_BOTTLES_TEXT_CALLBACK,
    VB_OVERRIDE_MOLEHILL_ABILITY,
    VB_OVERRIDE_JIGGY_SPAWN,
    VB_JIGGY_SPAWN_JINGLE,
    VB_CC_TOOTH_PLACE_JIGGY,
    VB_OVERRIDE_PROP_SPAWN,
    VB_APPLY_CUBE_PROP_DATA,
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
    VB_SPRITE_RESTORE_ALPHA_COMPARE,

    // #### `result`
    // ```c
    // viewport_isBoundingBoxInFrustum(scaled_min, scaled_max)
    // ```
    // #### `args`
    // - `*f32` (scaled_min)
    // - `*f32` (scaled_max)
    VB_DRAWDIST_BOX_CULL,

    // #### `result`
    // ```c
    // (!found && (cmd->flags & BK_GEO_CMD_CAMERA_IS_OUTSIDE_BIT)) ||
    //     (found && (cmd->flags & BK_GEO_CMD_CAMERA_IS_INSIDE_BIT))         // CAMERA
    // (cmd->min < dist) && (dist <= cmd->max)                               // LOD
    // viewport_func_8024DB50(pos_scaled, dist_scaled) && cmd->branch_offset // UNKE
    // ```
    // #### `args`
    // - `s32` (OCCLUSION_CMD_CAMERA, OCCLUSION_CMD_LOD or OCCLUSION_CMD_UNKE)
    // - `*void` (the geo command)
    // - `*BKModelBin`
    // - `*u8` (camera-area ids, CAMERA only)
    // - `s32` (camera-area count, promoted from `u8`)
    // - `s32` (CAMERA flags promoted from `u8`, or LOD min)
    // - `s32` (LOD max)
    VB_DRAW_GEO_CMD,
    VB_PICTUREBOX_TARGET_FB,
    VB_PICTUREBOX_SUBMIT_FRAME,
    VB_ITEMPRINT_DRAW_COUNT,

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

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_ABILITY_CLEAR_ALL,
    VB_MUMBO_HUT_INTERACT,

    // A listener that returns false skips player_transform, and Mumbo carries on as if the cast worked.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `s32` (the transformation)
    VB_MUMBO_CAST,

    // #### `result`
    // ```c
    // randf() < 0.01 // the T. rex
    // randf() < 0.05 // the mistake
    // ```
    // #### `args`
    // - None
    VB_MUMBO_RANDOM_EVENT,
    VB_LEVEL_LOAD_RESET_SCORES,
    VB_LEVEL_LOAD_RESET_MAP_SETPIECES,
    VB_CUTSCENE_SKIP_REQUIRE_PROGRESS,
    VB_GAME_OVER_MACHINE_ROOM,
    VB_LEVEL_ENTERED_FROM_LAIR,
    VB_MAP_SAVESTATE_CLEAR_ALL,
    VB_LEVEL_LOAD_SAVESTATE_INIT,
    VB_RACE_VOID_OUT_FULL_TRANSITION,

    // A listener that returns false keeps the dialog from showing, and its flag stays unset.
    // #### `result`
    // ```c
    // fileProgressFlag_get(progress_flag) == 0
    // ```
    // #### `args`
    // - `s32` (progress_flag)
    VB_PROGRESS_FLAG_DIALOG,
    VB_LAIR_GRUNTY_TAUNTS,  // Grunty's random lair taunt timer block: false = skip entirely.
    VB_AMBIENCE_MAP_UPDATE, // Per-map ambience/chan-mask switch: false = a listener replaced it.

    // A listener that returns false skips the Stop 'N' Swop display and can draw a different one.
    // #### `result`
    // ```c
    // D_80383010.sns_alpha != 0
    // ```
    // #### `args`
    // - `**Gfx`
    // - `**Mtx`
    // - `s32` (sns_alpha)
    // - `*s16` (unk3E, the egg angles)
    // - `*BKModelBin` (sns_egg_model)
    // - `*BKModelBin` (ice_key_model)
    VB_PAUSEMENU_SNS_DRAW,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_PAUSEMENU_SET_NEXT_PAGE,
    VB_PAUSEMENU_DRAW_JOYSTICKS,

    // The pink bold font on the current level's page. A listener that returns false sets a different texture.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_PAUSEMENU_BOLD_FONT_TEXTURE,

    // Whether a totals row shows.
    // #### `result`
    // ```c
    // !(i == 0) && !(i == 2) // the Lair page
    // !(i == 0) && !(i == 1) // Spiral Mountain's page
    // 1                      // every other page
    // ```
    // #### `args`
    // - `s32` (the page's level_id)
    // - `s32` (i, the row)
    VB_PAUSEMENU_ROW_VISIBLE,

    // A listener that returns false skips game_transitionToMapResettingLevel and moves the player another way.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `s32` (gVoidOutReturnLocation[0], the map)
    // - `s32` (gVoidOutReturnLocation[1], the exit)
    VB_VOID_OUT_RESPAWN_TRANSITION,
    VB_BUMP_REBOUNDS_PLAYER,

    // #### `result`
    // ```c
    // (codeA5BC0_getNodePropUnkC(arg0) + 1) != global_timer_time
    // ```
    // #### `args`
    // - `s32` (the warp's index in sWarpFunctions)
    VB_WARP_DISPATCH,

    // #### `result`
    // ```c
    // !gameFile_isNotEmpty(game_number)
    // ```
    // #### `args`
    // - `s32` (game_number)
    VB_GAMESELECT_START_NEW_GAME,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*Actor` (the file's game select actor)
    // - `s32` (game_number)
    VB_GAMESELECT_START_GAME,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_GAMESELECT_NEW_GAME_INTRO,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `f64` (function_time, promoted from `f32`)
    VB_GAMESELECT_RESUME_WARP,
    VB_NOTEDOOR_DRAW_NUMBER,

    // #### `result`
    // ```c
    // jiggyscore_total() < 3
    // ```
    // #### `args`
    // - None
    VB_JIGGY_COLLECT_TUTORIAL,
    VB_HONEYCOMB_PUMPKIN_REQUIREMENT, // also read by rando
    VB_BRENTILDA_HEAL_DIALOG,
    VB_SKY_UPDATE,
    VB_SKY_DRAW_BACKDROP_RECT,
    VB_CCW_SEASON_DOOR_RAISED_INIT,
    VB_XMAS_TREE_ICE_UPDATE,
    VB_BOGGY_HOME_VISIBLE,
    VB_LAST_JIGGY_FANFARE,
    VB_JIGSAW_PODIUM_COLLIDE,

    // #### `result`
    // ```c
    // sp50
    // ```
    // #### `args`
    // - `s32` (the map)
    VB_HEALTH_HUD_SHOW,
    VB_MIDI_SET_CHAN_MASK,
    VB_ITEM_ADJUST_BY_DIFF,
    VB_MM_HUT_DESPAWN_ON_SMASH,
    VB_NABNUT_SNORE_SFX,
    VB_NAPPER_WAKE_SFX,
    VB_MM_WITCH_SWITCH_JIGGY_FLUSH,

    VB_COUNT,
} VBehaviorID;

typedef enum DoorCameraId {
    GV_DOOR_CAM_SUN,     // sun switch (flag 3)
    GV_DOOR_CAM_STAR,    // star switch / trapdoor (flag 5)
    GV_DOOR_CAM_KAZOOIE, // beak-bomb door (flag 6)
    GV_DOOR_CAM_JINXY,   // Jinxy sneeze (flags 0, 1)
    MMM_DOOR_CAM_CHURCH, // church door, Tumblar challenge (flag 0)
} DoorCameraId;

typedef enum InvertControlType {
    INVERT_CAMERA_X,
    INVERT_CAMERA_Y,
    INVERT_FIRST_PERSON_X,
} InvertControlType;

DEFINE_EVENT(VanillaBehavior, VBehaviorID id; bool* should; va_list * originalArgs;);

#ifdef __cplusplus
extern "C" {
#endif
extern bool EventSystem_Should(VBehaviorID id, uint32_t result, ...);
extern int EventSystem_InvertControl(InvertControlType type);
#ifdef __cplusplus
}

// functions.h includes this header, and C++ files include functions.h inside extern "C".
extern "C++" {
ListenerID VanillaBehavior_RegisterListener(VBehaviorID id, const char* name, EventPriority priority,
                                            EventCallback callback, const char* file, int line);
void VanillaBehavior_UnregisterListener(VBehaviorID id, ListenerID listenerId);
ListenerID ActorEvent_RegisterListener(EventID* event, EventCallback forward, const char* name, int32_t actorId,
                                       EventCallback callback, EventPriority priority, const char* file, int line);
void ActorEvent_UnregisterListener(EventID* event, int32_t actorId, ListenerID listenerId);
void ActorEvent_Forward(EventID* event, IEvent* payload);
template <EventID* Event> void ActorEvent_ForwardTo(IEvent* payload) {
    ActorEvent_Forward(Event, payload);
}
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

#define REGISTER_VB_SHOULD(idToCheck, priority, ...)              \
    VanillaBehavior_RegisterListener(                             \
        idToCheck, #idToCheck, priority,                          \
        [](IEvent* event) {                                       \
            auto* ev = reinterpret_cast<VanillaBehavior*>(event); \
            bool* should = ev->should;                            \
            va_list args;                                         \
            va_copy(args, *ev->originalArgs);                     \
            __VA_ARGS__;                                          \
            va_end(args);                                         \
        },                                                        \
        __FILE__, __LINE__)

#define COND_HOOK(eventId, priority, condition, ...)                                                          \
    {                                                                                                         \
        static ListenerID listenerId = -1;                                                                    \
        if (listenerId != -1) {                                                                               \
            UNREGISTER_LISTENER(eventId, listenerId);                                                         \
            listenerId = -1;                                                                                  \
        }                                                                                                     \
        if (condition) {                                                                                      \
            listenerId = EventSystemRegisterListener(eventId##ID, __VA_ARGS__, priority, __FILE__, __LINE__); \
        }                                                                                                     \
    }
// Its priority orders it only against other COND_ID_HOOKs on the same actor id; they all run inside one
// NORMAL-priority listener on the event.
#define COND_ID_HOOK(eventId, actorId, priority, condition, ...)                                                   \
    {                                                                                                              \
        static_assert(offsetof(eventId, actor) == offsetof(ActorEventPayload, actor), "needs Actor* actor first"); \
        static ListenerID listenerId = -1;                                                                         \
        if (listenerId != -1) {                                                                                    \
            ActorEvent_UnregisterListener(&eventId##ID, actorId, listenerId);                                      \
            listenerId = -1;                                                                                       \
        }                                                                                                          \
        if (condition) {                                                                                           \
            listenerId = ActorEvent_RegisterListener(&eventId##ID, ActorEvent_ForwardTo<&eventId##ID>, #eventId,   \
                                                     actorId, __VA_ARGS__, priority, __FILE__, __LINE__);          \
        }                                                                                                          \
    }
#define COND_VB_SHOULD(id, priority, condition, ...)                    \
    {                                                                   \
        static ListenerID listenerId = -1;                              \
        if (listenerId != -1) {                                         \
            VanillaBehavior_UnregisterListener(id, listenerId);         \
            listenerId = -1;                                            \
        }                                                               \
        if (condition) {                                                \
            listenerId = REGISTER_VB_SHOULD(id, priority, __VA_ARGS__); \
        }                                                               \
    }

#include "List/EngineEvent.h"
#include "List/BehaviorEvent.h"
#include "List/GameEvent.h"
#include "List/RandoEvent.h"

typedef struct {
    IEvent Event;
    Actor* actor;
} ActorEventPayload;

#ifdef __cplusplus
extern "C" {
#endif
bool EventSystem_ShouldActorDraw(Actor* actor);
#ifdef __cplusplus
}
#endif
