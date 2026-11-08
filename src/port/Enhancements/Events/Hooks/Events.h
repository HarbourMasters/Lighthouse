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

    // A hit enemy dropping its bundle.
    // #### `result`
    // ```c
    // sp6C != 0
    // ```
    // #### `args`
    // - `s32` (the enemy's actor id, promoted from `s16`)
    VB_ENEMY_BECOME_BUNDLE,

    // #### `result`
    // ```c
    // item_getCount(ITEM_D_EGGS) != 0        // blue eggs
    // item_getCount(ITEM_F_RED_FEATHER) != 0 // red feathers
    // ```
    // #### `args`
    // - `s32` (the dropped item's actor id)
    // - `s32` (how many of them can lie on the ground)
    VB_YUMYUM_DROP,
    VB_MM_CHIMPY_STUMP_RUMBLE,

    // #### `result`
    // ```c
    // subaddie_playerIsWithinSphereAndActive(this, 700) && !gcdialog_hasCurrentTextId()
    // ```
    // #### `args`
    // - None
    VB_MM_CHIMPY_NOISE,

    // A listener that returns false skips the 134-unit drop and sets the stump's state.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*Actor` (the stump)
    VB_MM_CHIMPY_STUMP_START_LOWERED,
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

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*Actor` (this)
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
    VB_DOOR_OPEN_CAMERA, // Suppresses door-open camera lock when the flag came from a teammate, not us.

    // Once he's done coughing, Boggy waits for the rescue dialog to end before he runs off. A listener that returns
    // false skips the wait and starts the run with func_803877A8 (arg2 2), since no dialog will.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*Actor` (Boggy)
    VB_FP_BOGGY_WAIT_FOR_DIALOG,

    // Whether Banjo can board the scarf sled he's touching.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_FP_SCARF_SLED_BOARD,

    // A puzzle step's camera, and the look-at that holds the player through it. A teammate's step replayed here goes
    // without both.
    // #### `result`
    // ```c
    // true                                  // Croctus, Boggy, Blubber, the snow managers, Tanktup's look-at and exit
    // true                                  // Juju's landing shakes
    // true                                  // Napper
    // true                                  // the flower, the egg, baby Eyrie, Nabnut, Gnawty's boulder
    // true                                  // the crane, the egg tolls
    // true                                  // the lair's grate and third pipe, a note door's end
    // true                                  // a warp cauldron's look-at
    // actor_animationIsAt(this, 0.1f) != 0  // Tanktup's camera
    // gsworld_getMap() == MAP_43_CCW_SPRING // the spring flower's camera
    // this->modelCacheIndex == 0x215        // the CC lobby pipes' camera
    // ```
    // #### `args`
    // - `*Actor` (whose step it is, or NULL from FP's snowman-button and Sir Slush managers, which have none)
    VB_PLAY_PUZZLE_CUTSCENE,

    // A puzzle step's dialog. A teammate's step replayed here shows it only within earshot.
    // #### `result`
    // ```c
    // true                                       // Croctus's second dialog, Boggy, Tanktup's last dialog
    // true                                       // Napper, Jinxy, the GV and church doors
    // true                                       // baby Eyrie, Nabnut, Gnawty
    // (!this->unk138_23) && (!local->unk14)      // Tanktup's hit dialog
    // !jiggyscore_isCollected(JIGGY_1B_CC_TOOTH) // a Clanker tooth's
    // ```
    // #### `args`
    // - `*Actor` (whose step it is)
    VB_PLAY_PUZZLE_DIALOG,

    // A puzzle step's jingle or sound. A teammate's step replayed here plays it only within earshot.
    // #### `result`
    // ```c
    // true                                  // Croctus, Tanktup, the pink egg, Blubber, the teeth, the snow buttons
    // true                                  // Juju, FP ice tree
    // true                                  // Napper, Jinxy, the pyramid, the GV and church doors
    // true                                  // the flower, the egg, baby Eyrie, the season doors
    // true                                  // Gnawty's boulder and its shake
    // true                                  // the crane, the egg tolls, the CC grates, the breakables
    // true                                  // the lair's grate, pipes, level doors, Door of Grunty and statue hat
    // actor_animationIsAt(this, 0.55f) != 0 // Tanktup's cough
    // !this->unk38_31                       // the church door shutting
    // gsworld_getMap() == MAP_43_CCW_SPRING // the spring flower's fanfare
    // ```
    // #### `args`
    // - `*Actor` (whose step it is, or NULL from FP's snowman-button manager, which has none)
    VB_PLAY_PUZZLE_JINGLE,

    // A puzzle's jiggy. A teammate's step leaves it to the teammate.
    // #### `result`
    // ```c
    // true                                                                // Boggy, the pink egg, the snow managers
    // true                                                                // Juju's, with the solve's shake
    // true                                                                // the flower and Nabnut
    // actor_animationIsAt(this, 0.4f) != 0                                // Tanktup's
    // nodeProp_findPositionFromActorId(ACTOR_14B_TTC_BLUBBER_UNKNOWN, sp24) // Blubber's
    // ```
    // #### `args`
    // - `*Actor` (whose puzzle it is, or NULL from FP's snowman-button and Sir Slush managers, which have none)
    VB_SPAWN_PUZZLE_JIGGY,

    // The first Croctus's dialog, whose callback wakes the second. A listener that returns false skips it and has to
    // call chCroctus_activateNextCroctus.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*Actor` (the first Croctus)
    VB_CROCTUS_FIRST_FEED_DIALOG,

    // After the second Croctus's dialog, the first feed's camera exits and its look-at lets go. A listener that returns
    // false skips both, for a feed that set neither.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_CROCTUS_FIRST_FEED_RELEASE,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*u8` (the round to save)
    VB_TIPTUP_SAVE_ROUND,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_CLANKER_LIFT_CLOSE_DIALOG,

    // #### `result`
    // ```c
    // maClankerRings.currentActiveRing != 0
    // ```
    // #### `args`
    // - `*u8` (the active ring)
    VB_CC_RINGS_ACTIVE,

    // #### `result`
    // ```c
    // !jiggyscore_isCollected(JIGGY_16_CC_SNIPPETS)
    // ```
    // #### `args`
    // - None
    VB_SNIPPETS_DROP_JIGGY,

    // #### `result`
    // ```c
    // levelSpecificFlags_get(LEVEL_FLAG_26_FP_WALRUS_JIGGY_DROPPED) // Wozza
    // jiggyscore_isCollected(JIGGY_32_FP_WOZZA) ||
    //     levelSpecificFlags_get(LEVEL_FLAG_26_FP_WALRUS_JIGGY_DROPPED) // Wozza in his cave
    // ```
    // #### `args`
    // - None
    VB_WOZZA_JIGGY_DROPPED,

    // #### `result`
    // ```c
    // !this->initialized
    // ```
    // #### `args`
    // - `*Actor` (the Ancient One)
    VB_ANCIENT_ONE_UNSEEN,

    // A listener that returns true has to end the game.
    // #### `result`
    // ```c
    // !matchingGame.state
    // ```
    // #### `args`
    // - `s32` (matchingGame.state, promoted from `u8`)
    VB_MATCHING_GAME_IDLE,

    // #### `result`
    // ```c
    // arg0->jiggy_marker != NULL
    // ```
    // #### `args`
    // - None
    VB_TUMBLAR_HOLDS_JIGGY,

    // #### `result`
    // ```c
    // arg0->state == TUMBLAR_STATE_1_CONGRATULATING && mapSpecificFlags_get(MMM_SPECIFIC_FLAG_TUMBLAR_BROKEN)
    // ```
    // #### `args`
    // - `s32` (the Tumblar's state, promoted from `u8`)
    VB_TUMBLAR_BREAK,

    // A listener that returns true has to end the game.
    // #### `result`
    // ```c
    // Me.state == 0
    // ```
    // #### `args`
    // - `s32` (Me.state, promoted from `u8`)
    // - `*ActorMarker` (Motzhand)
    VB_ORGAN_GAME_IDLE,

    // An empty honeycomb a beaten enemy drops, or a honeycomb switch pops out.
    // #### `result`
    // ```c
    // this->unk38_31                                                                // the Colliwobble's
    // this->partnerActor && codeBF0_shouldSpawnQuarrieHoneyComb(this->partnerActor) // the Quarries'
    // true                                                                          // a switch's
    // ```
    // #### `args`
    // - `s32` (honeycomb_e)
    VB_SPAWN_EMPTY_HONEYCOMB,

    // The crane's hourglass starting as the cage finishes rising. A listener that returns false has to hold the cage
    // up, since chCageUpSwitch_update lowers it once the hourglass timer is empty.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `s32` (the hourglass ticks)
    VB_RBB_CRANE_HOURGLASS_START,

    // The jiggy cage's motor and slams as it rises, lowers and settles, about 3000 from its switch.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*Actor` (the cage's switch)
    VB_RBB_CRANE_CAGE_SOUND,

    // The BGS puzzle grate or a CC lobby pipe set up already up: the grate despawns and the pipe stays raised.
    // #### `result`
    // ```c
    // fileProgressFlag_get(FILEPROG_1E_LAIR_GRATE_TO_BGS_PUZZLE_OPEN)
    // fileProgressFlag_get(this->modelCacheIndex == 0x215 ? FILEPROG_1F_CC_LOBBY_PIPE_1_RAISED
    //                                                     : FILEPROG_20_CC_LOBBY_PIPE_2_RAISED)
    // fileProgressFlag_get(FILEPROG_21_CC_LOBBY_PIPE_3_RAISED)
    // ```
    // #### `args`
    // - `*Actor` (the grate or pipe)
    VB_LAIR_GRATE_OR_PIPE_ALREADY_UP,

    // A note door still closed, which shows the note count and opens on enough notes.
    // #### `result`
    // ```c
    // !fileProgressFlag_get(this->actorTypeSpecificField + FILEPROG_39_CCW_OPEN) &&
    //     player_isAbilityUnlocked(ABILITY_13_1ST_NOTEDOOR)
    // ```
    // #### `args`
    // - `*Actor` (the door)
    VB_NOTEDOOR_CLOSED,

    // #### `result`
    // ```c
    // itemscore_noteScores_getTotal() >= D_8039347C[this->actorTypeSpecificField - 1]
    // ```
    // #### `args`
    // - `*Actor` (the door)
    VB_NOTEDOOR_ENOUGH_NOTES,

    // A level door opening in the cutscene after its jigsaw.
    // #### `result`
    // ```c
    // func_802D677C(-1) && (func_802D677C(-1) == gsworld_getMap()) && (func_802D67AC(-1) >= 8) &&
    //     (func_802D67AC(-1) < 0x12) && (func_802D67DC(-1) == this->modelCacheIndex)
    // ```
    // #### `args`
    // - `*Actor` (the door)
    VB_LEVELDOOR_OPENING,

    // FP's witch switch pressed, which its lair door waits on.
    // #### `result`
    // ```c
    // volatileFlag_get(VOLATILE_FLAG_BB_WITCH_SWITCH_PRESSED_FP)
    // ```
    // #### `args`
    // - None
    VB_FP_WITCH_SWITCH_PRESSED,

    // CCW's puzzle podium, set up before its switch is pressed. A listener that returns false keeps it and has to hide
    // it.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*Actor` (the podium)
    VB_CCW_PODIUM_DESPAWN,

    // The player stepping onto a jigsaw podium.
    // #### `result`
    // ```c
    // isBanjoOnPodium(this->marker) && this->has_met_before && !isPictureComplete(this) &&
    //     (player_movementGroup() == BSGROUP_0_NONE || player_movementGroup() == BSGROUP_8_TROT)
    // ```
    // #### `args`
    // - `*Actor` (the podium)
    VB_JIGSAW_PODIUM_ENTER,

    // Furnace Fun's question pattern, rolled the first time Brentilda sets up. A listener that returns false has to set
    // FILEPROG_D3_FF_PATTERN.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - None
    VB_FF_PATTERN_ROLL,

    // A yumblie hole's piece, rolled as it appears. A listener that returns false has to set isGrumblie.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*Actor` (the yumblie)
    VB_VILE_PIECE_ROLL,

    // Grunty's brain in the final fight, each phase's update.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*Actor` (Grunty)
    VB_FINAL_BOSS_BRAIN,

    // An egg or attack landing on Grunty in the final fight.
    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `s32` (local->phase, promoted from `u8`)
    // - `*ActorMarker` (what hit her; NULL when replayed)
    VB_FINAL_BOSS_HIT,

    // An egg landing on a Jinjo statue's base in the final fight.
    // #### `result`
    // ```c
    // actor_bossjinjobase->state != CHBOSSJINJOBASE_STATE_3_SPAWNED_BOSS_JINJO   // a Jinjo statue's
    // actor_jinjonatorbase->state != CHBOSSJINJOBASE_STATE_3_SPAWNED_BOSS_JINJO  // the Jinjonator's
    // ```
    // #### `args`
    // - `s32` (ch_bossjinjo_e, the statue)
    // - `s32` (the pad, for the Jinjonator's)
    // - `*ActorMarker` (the egg; NULL when replayed)
    VB_JINJO_STATUE_EGG_HIT,

    // The intro or bridge Bottles in Spiral Mountain, with the player close enough to talk. A listener that returns
    // false for the intro has to set SM_SPECIFIC_FLAG_1_TALKED_TO_BOTTLES.
    // #### `result`
    // ```c
    // ((ml_vec3f_distance(plyr_pos, this->unk1C) < this->actor_specific_1_f) && player_isStableWithExtraSteps()) ||
    //     mapSpecificFlags_get(SM_SPECIFIC_FLAG_10)
    // ```
    // #### `args`
    // - `*Actor` (the Bottles)
    VB_SM_TUTORIAL_CHOICE_OPEN,

    // A Spiral Mountain Bottles in reach of the player, who can wake him.
    // #### `result`
    // ```c
    // subaddie_playerIsWithinSphereAndActive(this, 250)
    // ```
    // #### `args`
    // - `*Actor` (the Bottles)
    VB_SM_MOLEHILL_ACTIVE,

    // The attack tutorial finding the claw swipe, roll and rat-a-tat rap already known, which ends it.
    // #### `result`
    // ```c
    // __chAttackTutorial_areLearnableAbilitiesUnlocked()
    // ```
    // #### `args`
    // - `*Actor` (the attack tutorial)
    VB_ATTACK_TUTORIAL_MOVES_KNOWN,

    // FP's Twinkly box, hit to start its minigame.
    // #### `result`
    // ```c
    // this->state == 1 || this->state == 2
    // ```
    // #### `args`
    // - `*Actor` (the box)
    VB_FP_TWINKLY_START,

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

    // A listener that returns false keeps the dialog from showing, and its flag stays unset.
    // #### `result`
    // ```c
    // fileProgressFlag_get(progress_flag) == 0
    // ```
    // #### `args`
    // - `s32` (progress_flag)
    VB_PROGRESS_FLAG_DIALOG,

    // Grunty's random lair taunts.
    // #### `result`
    // ```c
    // level_get() == LEVEL_6_LAIR
    // ```
    // #### `args`
    // - None
    VB_LAIR_GRUNTY_TAUNTS,
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

    // What Brentilda says after her hint: her offer to heal, or ASSET_D38_DIALOG_BOTTLES_ALL_MOVES_LEARNED when
    // there's nothing to heal.
    // #### `result`
    // ```c
    // text_id == ((ActorLocal_lair_5640 *)&this->local)->unk0 + 2
    // ```
    // #### `args`
    // - None
    VB_BRENTILDA_HEAL_DIALOG,
    VB_SKY_UPDATE,
    VB_SKY_DRAW_BACKDROP_RECT,
    VB_CCW_SEASON_DOOR_RAISED_INIT,
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

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `s32` (item)
    // - `*s32` (diff)
    VB_ITEM_ADJUST_BY_DIFF,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*Actor` (the hut)
    VB_MM_HUT_SPAWN_EXPLOSION,

    // #### `result`
    // ```c
    // true
    // ```
    // #### `args`
    // - `*ActorMarker` (Nabnut)
    VB_NABNUT_SNORE_SFX,

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
bool EventSystem_ShouldActorUpdate(Actor* actor);
#ifdef __cplusplus
}
#endif
