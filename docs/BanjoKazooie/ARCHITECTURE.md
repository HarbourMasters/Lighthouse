# Banjo-Kazooie Game Architecture

This document describes the major game systems in the Banjo-Kazooie codebase and how they interconnect. It is intended as a reference for collaborators working on any subsystem.

## Table of Contents

- [Levels, Maps, and Overlays](#levels-maps-and-overlays)
- [Props and Spatial Cubes](#props-and-spatial-cubes)
- [Actor System](#actor-system)
- [Player State Machine (bs/ba)](#player-state-machine)
- [Animation System](#animation-system)
- [Collision System](#collision-system)
- [Camera System](#camera-system)
- [Particle System](#particle-system)
- [Sound and Music](#sound-and-music)
- [Collectibles and Scoring](#collectibles-and-scoring)
- [Model Rendering](#model-rendering)
- [Sprite Rendering](#sprite-rendering)
- [Model/Sprite Asset Interchangeability (N64 vs PC)](#modelsprite-asset-interchangeability-n64-vs-pc)
- [Cutscenes](#cutscenes)
- [Save Data](#save-data)

---

## Levels, Maps, and Overlays

The game world is organized in three layers: **levels**, **maps**, and **overlays**.

### Levels

A level is a player-facing world (Mumbo's Mountain, Treasure Trove Cove, etc.). There are 13 levels defined in `enum level_e` (enums.h). Each level contains one or more maps.

### Maps

A map is an individual room or zone. There are ~140 maps defined in `enum map_e` (~129 non-unused). A level like Mad Monster Mansion has separate maps for the main area, the church, the cellar, etc.

Each map contains:
- **3D geometry** — display lists and vertex data for the environment
- **Cubes** — spatial grid cells containing spawn data (see below)
- **Camera nodes** — predefined camera positions and behaviors
- **Lighting** — directional lights, ambient color, fog parameters

Map transitions happen through `func_802E4078(map, exit, transition)`. The flow:
1. Save current state (actor positions, flags)
2. Free old map (cubes, models, cameras, lighting)
3. Load new map data from assets
4. Determine and activate the overlay for the new level
5. Spawn player at the exit position
6. Process the actor spawn queue

### Overlays

On N64, overlays were dynamically loaded code segments — one per level region. On PC, all code is statically linked. The overlay manager (`overlaymanager.c`) tracks the active overlay ID and dispatches callbacks.

Each overlay defines five callbacks:
- `init()` — called when the overlay loads
- `update()` — called each frame
- `release()` — called when unloading
- `draw(Gfx**, Mtx**, Vtx**)` — render level-specific elements
- `unk14(s32, s32)` — undocumented callback

There are 14 overlays (`enum overlay_e`): one stub (`overlay_stub/`), one per game world, plus cutscenes, Grunty's lair, and the final battle.

The map-to-overlay mapping is in `map/overlay.c`. Multiple maps within the same level share one overlay.

---

## Props and Spatial Cubes

### Cubes

Every map is divided into a spatial grid of **Cubes** (`struct cube_s` in `prop.h`). Each cube represents a region of 3D space and contains two arrays:

- **prop1 (NodeProp)** — 20-byte spawn markers used by the spline path system, cameras, and special triggers. Contains position, radius, yaw, scale, and flag bitfields.
- **prop2 (Prop)** — Actor, model, and sprite placement data.

Cubes are loaded from assets in `actor_cubepropsystem.c` and `actor_cubebounds.c`, which read NodeProp and Prop arrays and then initialize actor markers from the ActorProp entries.

### Actor Spawning from NodeProps

Collectible actors (notes, eggs, feathers, honeycombs, extra lives, etc.) spawn through the NodeProp/CubeProp system rather than through direct spawn calls.

1. **Map loading** (`map/lifecycle.c` -> `cubeList_fromFile()`) parses the map binary and reads Cube data containing NodeProp entries.
2. Each NodeProp has an `actorId` field — e.g., notes use `ACTOR_51_MUSIC_NOTE` (0x51).
3. When the player enters a cube's proximity, the CubePropSystem (`actor_cubepropsystem.c`) iterates the cube's NodeProps and calls `__actor_spawnWithYaw_s32()` in `actor_cubebounds.c`.
4. This function looks up the actor ID in the `sSpawnableActorList` (populated at boot by `spawn_queue.c`) and calls the registered spawn function with position and yaw from the NodeProp.
5. Collectible actors are registered with `ACTOR_FLAG_UNKNOWN_21`, which marks them as proximity-spawned collectibles.

This means **note positions (and all collectible positions) are defined in the map binary data**, not in C code. They come from NodeProp entries embedded in each map's cube grid.

For hooking collectible spawns, the `OnActorSpawn` event in `__actor_spawnWithYaw_s32()` fires for every NodeProp-triggered spawn with the actor ID, position (x, y, z), and yaw. For collision, notes use `MARKER_5F_MUSIC_NOTE` — the collection handler is in `ba/ba_marker.c`.

### Prop Union

`Prop` is a union (`union prop_s` in `prop.h`) with three variants:

| Variant | Purpose | Key Fields |
|---------|---------|------------|
| `ActorProp` | Spawnable actor | `marker` pointer, position (s16 x/y/z), flags |
| `SpriteProp` | Billboard sprite | sprite asset ID, position, display params |
| `ModelProp` | Static 3D model | model asset ID, position, rotation, flags |

On N64 all variants are 12 bytes. On 64-bit, ActorProp grows due to its 8-byte marker pointer; other variants use `_pad64` to match.

---

## Actor System

Actors are the runtime instances of spawnable game objects — enemies, NPCs, collectibles, triggers, and effects.

### ActorInfo

Every actor type has an `ActorInfo` struct (`prop.h`) that serves as its template:

```
markerId          — asset ID type (enum marker_e)
actorId           — actor type ID (enum actor_e)
modelId           — 3D model to render
startAnimation    — initial animation state
animations        — animation table
update_func       — per-frame logic
update2_func      — secondary update (often actor_update_func_80326224)
draw_func         — rendering function
unk18             — unknown field
draw_distance     — culling distance
shadow_scale      — shadow circle size
unk20             — unknown field
```

Level-specific actor implementations live in `src/<LEVEL>/ch/` directories. Each `ch/` file typically defines one `ActorInfo` and its update/draw functions.

### ActorMarker

`ActorMarker` is the runtime handle linking an actor to the spatial system. Key fields:

- `propPtr` — backpointer to the ActorProp in its cube
- `cubePtr` — backpointer to the cube it belongs to
- `drawFunc`, `collisionFunc`, `dieFunc` — behavior callbacks
- `actorUpdateFunc` — per-frame update
- `actrArrayIdx` — index into the global actor array
- `yaw`, `pitch`, `roll` — orientation (bitfields)
- `modelId` — model asset for rendering
- `unk48` — loaded BKModel data

### Actor

`Actor` (`prop.h`, ~0x180 bytes) is the full runtime state of a spawned actor:

- Position, velocity, rotation (yaw, pitch, roll)
- `state` — 6-bit state machine index
- `anctrl` — animation controller
- `actor_info` — pointer to ActorInfo template
- `scale`, `alpha` — visual properties
- Local data union — actor-specific state (varies by type)
- `marker` — backpointer to ActorMarker
- Particle emitters, SFX source index, despawn flag

### Lifecycle

1. **Spawn**: `actor_new()` allocates an Actor in `suBaddieActorArray`, calls `marker_init()` to create the marker, sets up callbacks from ActorInfo
2. **Update**: `marker->actorUpdateFunc(actor)` called each frame
3. **Despawn**: `marker_despawn()` sets `despawn_flag`, actor is freed in batch by `func_803283D4()`

The actor array starts at 20 slots and grows by 5 via realloc when full.

---

## Player State Machine

The player (Banjo-Kazooie) uses a dedicated state machine split across two subsystems.

### BS (Banjo States)

Files in `src/core2/bs/` implement the state machine core. There are **~153 states** explicitly defined in `enum bs_e` (enums.h), spanning indices 0 through 0xA5 with gaps:

- Movement: Idle, Walk (slow/normal/fast), Jump, Crouch, Skid
- Attacks: Claw Swipe, Beak Buster (`beak_buster.c`), Beak Barge (`beak_barge.c`), Beak Peck (`beak_peck.c`), Wonderwing (`wonderwing.c`)
- Abilities: Talon Trot (`talon_trot.c`), Beak Bomb (`beak_bomb.c`), Swimming (`beak_swim.c`), Diving, Climbing
- Transformations: Termite, Pumpkin, Walrus, Crocodile, Bee (`bee.c`/`bee_fly.c`/`bee_main.c`) (each with enter/idle/walk/exit states)
- Special: Scripted movement (`scripted.c`, `scripted_enter.c`, `scripted_goto.c`, `scripted_look.c`, `scripted_vanish.c`, `scripted_transform.c`), Death, Locked, Carrying objects

Each state has four handlers:
- `init_func` — enter state
- `update_func` — per-frame logic
- `end_func` — leave state
- `interrupt_func` — handle interrupts (e.g., taking damage, collecting a jiggy)

State transitions go through `bs_setState(state_id)`, which calls end on the old state and init on the new one. `bs_checkInterrupt()` tests if the current state handles a given interrupt type.

### BA (Banjo Attributes)

Files in `src/core2/ba/` are ~40 decomposed subsystems that each manage one aspect of the player:

| File | Purpose |
|------|---------|
| `ba_position.c` | XYZ coordinates |
| `ba_yaw.c` | Facing direction |
| `ba_momentum.c` | Velocity and acceleration |
| `ba_stick.c` | Joystick input processing |
| `ba_stickinterp.c` | Joystick smoothing |
| `ba_health.c` | Health and damage |
| `ba_falldamage.c` | Fall damage calculation |
| `ba_falling.c` | Falling state |
| `ba_underwater.c` | Underwater behavior |
| `ba_groundsurface.c` | Surface type detection |
| `ba_hitbox.c` | Collision volumes |
| `ba_animcache.c` | Animation caching |
| `ba_animstate.c` | Current animation |
| `ba_modelselect.c` | Character model switching |
| `ba_eyes.c` / `ba_eyeblink.c` | Eye animation |
| `ba_sfx.c` / `ba_sfxintensity.c` | Sound effects |
| `ba_scripted.c` | Scripted movement control (type, position, duration) |
| `ba_flag.c` / `ba_statusflags.c` | Internal state flags |
| `ba_key.c` / `ba_input.c` | Input mapping |
| `ba_lookat.c` / `ba_lookdir.c` | Look targeting |
| `ba_recoil.c` | Knockback |
| `ba_bounds.c` | Bounding box |
| `ba_timer.c` | Frame timers |
| `ba_state.c` | State-to-handler mapping table |
| `ba_musicstate.c` | Turbo trainers/wading boots music |

### Stored State

`bs_storedstate.c` persists state across map transitions:
- Wading Boots timer remaining
- Turbo Trainers timer remaining
- Current transformation type (Banjo, Termite, Pumpkin, Walrus, Croc, Bee, Wishwashy)

---

## Animation System

### Skeletal Animation

The skeletal animation pipeline:

1. `SkeletalAnimation` holds current progress (0.0–1.0), duration, behavior (loop/once/backwards/stopped), and a callback list
2. `skeletalAnim_update()` advances progress by `dt / duration` each frame
3. `AnimationFile` binary data contains per-bone keyframe tracks. Each `AnimationFileElement` stores a bone ID, component (rotation/scale/translation), and keyframe array
4. `animationFile_getBoneTransformList()` interpolates keyframes using Catmull-Rom splines
5. `BoneTransformList` (array of `BoneTransform`: quaternion rotation, scale[3], position[3]) is passed to the renderer

The system supports smooth transitions between animations using double/triple buffered bone transform slots and `boneTransformList_interpolate()` for crossfading.

### Animation Cache

`animCache` (`anim/anim_cache.c`) pools 340 cached bone transform results. Each entry has a lifetime counter; stale entries are evicted incrementally by `animCache_flushStale()`. This avoids redundant computation when multiple actors share the same animation at the same frame.

### Glspline (Path Animation)

`spline_pathfollow.c` handles spline path animation for objects that follow predefined paths (file select screen objects, map decorations). It casts `NodeProp*` data to `Union_glspline*` structs, reinterpreting the 20-byte NodeProp through different bitfield boundaries to extract animation timing, path control points, and behavior flags.

---

## Collision System

Files in `src/core2/collision/` implement spatial collision queries. The collision dispatch table (`collision/dispatch.c`) routes queries through registered collision function sets per actor.

### Data Structures

- `BKCollisionList` — spatial acceleration grid stored per-model. Contains bounding box, grid dimensions, and arrays of `BKCollisionGeo` (cell references) and `BKCollisionTri` (triangle primitives)
- `BKCollisionTri` — triangle with vertex indices and surface type flags
- `BKCollisionGeo` — cell entry pointing to a range of triangles

### Query Types

Each actor's `ActorMarker` has a `Struct6Cs` with four collision function pointers:

| Function | Purpose |
|----------|---------|
| `unk0` | **Raycast**: origin + direction → first hit triangle + normal |
| `unk4` | **Sphere cast**: origin + movement + radius → swept collision |
| `unk8` | **Sphere check**: origin + radius → overlapping triangle |
| `unkC` | **Distance query**: origin + radius → nearest surface distance |

### Process

1. Spatial grid lookup (O(1) cell find using stride arithmetic)
2. Triangle intersection tests against all triangles in the cell
3. Surface type extraction from the triangle's flags field
4. Normal vector written to caller-provided buffer

Collision is toggled per-actor with `actor_collisionOn()` / `actor_collisionOff()`.

---

## Camera System

Files in `src/core2/camera/` manage the game camera.

### Camera Nodes

Each map defines up to 70 camera nodes (`CameraNode` in `camera.h`), loaded from assets. The `type` field selects behavior:

| Type | Purpose | Key Fields |
|------|---------|------------|
| 0 | Empty/unset | default state on init and after removal |
| 1 | Dynamic follow | position, horizontal/vertical speed, acceleration, rotation, flags |
| 2 | Static | fixed position and pitch/yaw/roll |
| 3 | Tracked follow | extends Type1 with close/far distance limits |
| 4 | Special behavior | control flags only |

### Camera Motors

Two motor systems (`camera_motor1.c`, `camera_motor2.c`) handle smooth interpolation. Each motor tracks:
- Current position component
- Target position
- Velocity with damping (typically 0.65)
- Maximum speed
- Frame time delta

Motors smooth all camera transitions so cuts don't pop.

### Pipeline

1. Load camera nodes from map assets
2. Select active node based on player position and game state
3. Interpolate position/rotation through motor system
4. Apply FOV and fog parameters
5. Feed final transform to the rendering viewport

---

## Particle System

Files in `src/core2/particle/` implement a factory-based particle system.

### Architecture

- **Emitter pool** (`particle/accel.c`): Manages 16 emitter slots. `func_802F0F78(cnt)` allocates a slot; `func_802F0EF0(slot)` returns or creates the `ParticleEmitter`.
- **ParticleEmitter** (~0x130 bytes): Defines all spawn parameters — position, velocity ranges, acceleration, lifetime, scale, color, sprite or model to render, spawn interval, fade in/out, draw mode.
- **Particle** (~0x60 bytes): Individual particle instance with position, velocity, acceleration, rotation, angular velocity, scale, age, and lifetime.

### Lifecycle

1. **Create emitter**: Set ranges for velocity, scale, lifetime, color, etc.
2. **Spawn**: `__particleEmitter_initParticle()` randomizes fields from emitter ranges. Supports Cartesian or spherical velocity modes.
3. **Update**: `func_802F10A4()` ages particles, defragments dead ones, frees idle emitters after a cooldown.
4. **Draw**: `__particleEmitter_drawOnPass()` renders each particle as either a model (`modelRender_draw`) or sprite billboard (`func_80344720`). Supports alpha fade, RGB overlay, and depth mode.

---

## Sound and Music

### SFX Sources

`sfx/source.c` manages up to 35 concurrent spatial sound sources (`SfxSource`). Each source has:
- 3D position
- Inner/outer fade radii (distance attenuation)
- SFX ID, sample rate, volume, priority
- Play/stop state

Key functions: `sfxsource_createSfxsourceAndReturnIndex()`, `sfxsource_setSfxId()`, `sfxsource_set_position()`, `func_8030E2C4()` (play).

### Audio Manager

`audio_manager.c` (core1) runs the audio synthesis thread. `audio_soundplayer.c` handles voice allocation and event processing. The N64 audio system uses an event queue with types for MIDI events, note on/off, and voice state changes.

### Music

`sfx/musicplay.c` wraps the N_AL audio API for background music. Music tracks are associated with asset IDs, sample rates, and reverb types.

---

## Collectibles and Scoring

### State Tracking

Collectible state is tracked across several files:

| File | Purpose |
|------|---------|
| `bundle.c` | Spawns groups of collectibles in a burst (e.g., 5 notes from a hut). Defines 36 bundle types with physics parameters. |
| `score_honeycomb.c` | Honeycomb piece collection bitfield |
| `score_jiggy.c` | Jiggy collection bitfield |
| `score_mumbo.c` | Mumbo token collection bitfield |
| `flags_bitfield.c` | File progress flags and volatile flags (bitfield get/set operations) |
| `gamestate.c` | Item counts (notes, eggs, feathers, lives, etc.) |

### HUD Score Queue System

Files in `src/core2/scorequeue/` manage the on-screen score display:

| File | Purpose |
|------|---------|
| `manager.c` | Manages 44 item print effects with new/update/draw/free callbacks |
| `dispatch.c` | Dispatch table routing item types to their queue pool |
| `queue_notes.c` | Queue pool for music note pickups |
| `queue_honeycombs.c` | Queue pool for honeycomb pickups |
| `queue_collectibles.c` | Queue pool for tokens, bullions, oranges, presents |
| `queue_major_items.c` | Queue pool for health, jinjos, jiggies |
| `queue_totals.c` | Queue pool for total displays |
| `queue_timers.c` | Queue pool for timer displays |
| `queue_lives.c` | Queue pool for extra life displays |
| `drawui_sprites.c` | Sprite-based score popup renderer |
| `drawui_models.c` | 3D model score popup renderer |
| `drawui_stub.c` | No-op renderer for items with no visual |
| `drawui_air.c` | Air meter display |
| `drawui_health.c` | Health honeycomb bar |
| `drawui_honeycarrier.c` | Empty honeycomb carrier display |
| `drawui_jinjo.c` | Jinjo silhouette display |
| `drawui_lives.c` | Extra lives counter |

### Collectible Actors

The spawnable actor implementations (the pickup objects themselves) live in `src/core2/ch/` — e.g., `ch/honeycomb.c`, `ch/musicnote.c`, `ch/feather.c`, `ch/extralife.c`. These define ActorInfo structs with models, animations, and pickup behavior.

---

## Model Rendering

### Geometry Layout

Models use a command-based geometry layout. `GeoCmd` structures define a sequence of operations:

| Command | Purpose |
|---------|---------|
| GeoCmd0 | Load matrix (position, rotation, scale) |
| GeoCmd1 | Billboard transform |
| GeoCmd2 | Draw indexed mesh |
| GeoCmd3+ | Matrix push/pop, texture load, light setup |

`model/render.c` dispatches these commands to build display lists.

### Skinning

`model/skinning.c` applies skeletal animation to mesh vertices:
1. For each mesh, look up the bone's animation matrix via `animMtxList_get()`
2. Transform each vertex position through the bone matrix with `mlMtx_apply_vec3s()`
3. Flush vertex cache with `osWritebackDCache()`

### Lighting

`model/lighting.c` and `vtx/lighting.c` apply per-map directional and ambient lighting to models. Light configuration is loaded as part of the map data.

---

## Sprite Rendering

Sprites are 2D billboard images used for dialog portraits, particles, trees, and UI elements.

### BKSprite Structure

`BKSprite` (structs.h) contains:
- Frame count, type, display width/height
- Animation metadata (speed, direction, flip) in `unkC` bitfield
- Array of frame pointers, each containing a display list and vertex array

### Rendering

`sprite/render.c` handles billboard rendering:
1. Compute vector from camera to sprite position
2. Project onto camera direction for distance
3. Cull if beyond 3000 units or below minimum pixel size
4. Set up billboard matrix (always face camera)
5. Submit frame's display list with vertex data in segment 1

`anim/anim_spriteframe.c` advances the current frame based on the animation speed and direction defined in the sprite's metadata.

---

## Model/Sprite Asset Interchangeability (N64 vs PC)

### N64 Behavior

On N64, model and sprite assets were raw binary blobs stored at contiguous ROM addresses. Because the game accessed them through direct ROM-to-RAM DMA, the same data pointer could be interpreted as either a model or a sprite — they shared the same memory space with no type enforcement. The prop system distinguishes model props from sprite props using the `unk8_1` flag (bit 1 of the setup data flags u16): a value of 1 indicates a model prop, 0 indicates a sprite prop.

Some actors have `unk8_1=1` in their N64 setup data despite their underlying asset being a sprite. Mumbo Tokens are a notable example. On N64 this was harmless: `func_80330F50` would return data from the asset, and because the sprite data happened to have `animType=0`, the animation code in `func_8032D510` would exit early without attempting model-specific animation operations on the sprite data.

### PC Port Incompatibility

On PC, models and sprites are loaded through separate resource factories (`ModelFactory` and `SpriteFactory`) that produce distinct runtime types. An asset exported as a sprite cannot be accessed through the model loading path and vice versa. This breaks the N64's implicit interchangeability: when the prop system sees `unk8_1=1` and treats the prop as a model, it attempts to load the asset through `ModelFactory`, which fails because the asset was exported by `SpriteFactory`.

### Fix Strategy

The fix introduces a runtime asset type check rather than relying solely on the setup data flag:

1. **`ResourceMgr_IsModelAsset()`** queries the resource manager to determine whether a given asset ID was actually exported as a model or a sprite, regardless of what the prop's `unk8_1` flag claims.

2. **Animation path expansion in `func_8032D510`**: The original code only entered the sprite animation path for props with `unk8_1=0`. The fix expands the condition so that marker/actor props with `unk8_1=1` also enter the sprite animation path when `ResourceMgr_IsModelAsset()` reports the asset is not a model.

3. **Division-by-zero guard in `func_8032CD60`**: Actors that newly enter the sprite animation path (because the expanded condition now routes them there) may have `speed=0` or a zero frame range. A guard prevents the resulting division by zero. This is a port-specific safety measure for the expanded code path, not a change to original decomp logic.

This approach preserves the original decomp behavior for all correctly-flagged props while handling the edge cases where N64's memory model allowed type mismatches to silently succeed.

---

## Cutscenes

Cutscene-related code is distributed across several locations:

- `map/cutscene_skip.c` — cutscene skip condition checks and map warp helpers
- `map/cutscene_triggers.c` — per-map cutscene trigger checks
- `map/warp_dispatch.c` — master warp/door function dispatch table (NodeProp callbacks for map transitions)
- `camera/camera_focustarget.c` — camera focus target selector (used during cutscenes and gameplay)
- `cutscenes/cutscene_trigger.c` — first-visit cutscene trigger actor
- `cutscenes/character_parade.c` — ending credits good-ending music actor
- `cutscenes/sfx_sequencer.c` — cutscene SFX playback sequencer actor
- `cutscenes/cutscene_animsequence.c` — cutscene animation sequence data

Level-specific cutscene code lives in `src/cutscenes/`. The `context/parade.c` file manages the character parade end-of-game sequence.

---

## Save Data

### Structure

Save data (`include/save.h`) is organized as:

```
SaveData (120 bytes per slot)
  magic, slotIndex
  data[0x70]:
    jiggy collection state
    honeycomb collection state
    mumbo token state
    note high scores per level
    time scores
    progress flags (~291 indices)
    saved items
    ability unlocks
```

A separate `GlobalData` struct stores cross-file data (SNS items).

### Flags

Two flag systems track game state:

- **File progress flags** (`enum file_progress_e`, ~291 indices): Persistent across saves. Track map unlocks, jiggy/honeycomb collection, ability learns, NPC interactions, puzzle completion.
- **Volatile flags** (`enum volatile_flags_e`, ~200 flags): Reset on load. Track current map, minigame state, boss phase, quiz answers, cheat activation.

### I/O

`savedata.c` handles read/write through `eeprom_readBlocks()` / `eeprom_writeBlocks()` (stubbed on PC to use file I/O).