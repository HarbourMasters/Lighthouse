# Source File Directory

**Subdirectory conventions:**
- `core2/anim/`: animation system
- `core2/ba/`: player banjo-kazooie subsystems — original short names (`physics.c`, `marker.c`) and prefixed files (`ba_state.c`, `ba_position.c`, etc.)
- `core2/bs/`: behavior state machines — original short names (`bFlap.c`, `stand.c`) and prefixed files (`bs_list.c`, `bs_statemachine.c`, etc.)
- `core2/ch/`: spawnable actor implementations — characters, overlays, FX emitters, collectible actors (`jinjo.c`, `jiggy.c`, `mumbotoken.c`, `beeswarm.c`)
- `core2/collectible/`: collectible state tracking and UI (`bundle.c`, `printui.c`, `musicnotedata.c`) — actors are in `ch/`
- `core2/collision/`: collision detection and raycasting
- `core2/cutscene/`: cutscene playback and flags
- `core2/dialog/`: dialog binary loading
- `core2/font/`: font rendering and kerning
- `core2/frame/`: framebuffer and render memory
- `core2/fx/`: effects and score display (`effect_*.c`, `score_*.c`)
- `core2/gc/`: game controller UI (pause menu, dialog, zoombox)
- `core2/gfx/`: display list data and init
- `core2/level/`: per-level metadata and flags
- `core2/lighting/`: lighting compute, setup, apply
- `core2/map/`: map model, save state, flags, audio config
- `core2/model/`: model rendering and data access
- `core2/camera/`: camera system (renamed from `nc/`)
- `core2/particle/`: particle emitter system
- `core2/quiz/`: Furnace Fun quiz system
- `core2/sfx/`: sound effects and audio streaming
- `core2/sprite/`: sprite rendering and animation
- `core2/texture/`: texture copy and data access
- `core2/vtx/`: vertex operations (color, normal, transform, UV)
- Overlay `ch/` subfolders: character/actor implementations
- Overlay `ma/` subfolders: map-specific mechanics (slalom, castle puzzles)
- Overlay roots: system/level files (`crc.c`, `actor_spawninit.c`, minigames)

---

## src/core1/

| File | Content |
|------|---------|
| `init.c` | Boot/init entry point |
| `initthread.c` | Thread initialization |
| `memory.c` | Memory heap (malloc/free/realloc) |
| `overlay.c` | Overlay loading system |
| `overlaymanager.c` | Overlay manager (tracks loaded overlay ID) |
| `display_list.c` | GFX/MTX/VTX stack management, scissor setup, framebuffer init |
| `graphics_thread.c` | RSP/graphics task submission thread, audio/gfx task queue (mostly `#if 0`) |
| `ml.c` | Math library core |
| `mlmtx.c` | Matrix operations (get/set/push/multiply) |
| `ml_mtxconv.c` | Matrix conversion (MtxF↔Mtx), scale/rotate/translate, perspective/frustum |
| `collision.c` | Core1 collision system |
| `crccalc.c` | CRC calculation |
| `depthbuffer.c` | Depth buffer management |
| `framebufferdraw.c` | CPU framebuffer drawing (transitions, screen captures) |
| `viewport.c` | Viewport setup |
| `vimgr.c` | VI manager (video interface) |
| `pimanager.c` | PI manager (peripheral interface) |
| `inflate.c` | Decompression (inflate) |
| `rarezip.c` | Rare proprietary compression |
| `defragmanager.c` | Memory defragmentation manager |
| `lookup.c` | Asset lookup tables |
| `sprite.c` | Core1 sprite system |
| `ll.c` | Linked list operations |
| `ll_cvt.c` | Linked list conversion |
| `debugtext.c` | Debug text rendering |
| `syncprintf.c` | Synchronous printf |
| `sns.c` | Stop 'N' Swop data |
| `stopnswop.c` | Stop 'N' Swop system |
| `ucode.c` | RSP microcode loading (stubbed) |
| `eeprom.c` | EEPROM save data |
| `data_1E820.c` | Static data segment |
| `stub_2FA0.c` | Stub function |
| `stub_3A70.c` | Stub function |
| `stub_13640.c` | Stub function |
| `stub_1D590.c` | Stub function |
| `audio_manager.c` | Audio synthesis thread, heap, oscillator state, DMA loading |
| `audio_soundplayer.c` | N64 sound player, voice allocation, envelope processing, MIDI events |
| `audio_ambience.c` | Context-aware music ducking, region-based layer mixing, map-specific audio |
| `audio_instruments.c` | Music instrument data, bank/soundfont init, 176 track metadata entries |
| `audio_musicplayer.c` | Music player control |
| `audio_sfx.c` | Sound effects playback |
| `ba_motor.c` | Rumble/motor pak control |

### src/core1/gu/

| File | Content |
|------|---------|
| `gu_mtx.c` | guScale, guPerspective, guRotate, frustum — matches N64 SDK `gu*` naming |

N64 SDK math library files (`cosf.c`, `sinf.c`, `sqrtf.c`, etc.) removed — replaced by libultraship.

### src/core1/io/

| File | Content |
|------|---------|
| `pfs_manager.c` | Controller pak persistence manager, input polling, button/joystick state |

N64 SDK controller/IO library files (`pfs_checker.c`, `pfs_init.c`, `controller.c`, `vi*.c`, etc.) removed — replaced by libultraship.

### src/core1/audio/

N64 audio synthesis library. Files prefixed `n_` are the "new" (optimized) audio pipeline.

| File | Content |
|------|---------|
| `csp_postfloat.c` | Posts `AL_UNK18_EVT` with two float params to sequencer |
| `csp_getchanstate.c` | Returns byte from `chanState[chan].unkA` |
| `csp_controlchange.c` | Posts MIDI control change event (0x7D) |
| `csp_chanmode.c` | Posts MIDI channel mode messages (0x7E, 0x7F) |
| `cspgettempo.c` | Get sequencer tempo |
| `cspplay.c` | Start sequence playback |
| `cspsetbank.c` | Set instrument bank |
| `cspsetseq.c` | Set sequence data |
| `cspsettempo.c` | Set sequencer tempo |
| `cspsetvol.c` | Set sequencer volume |
| `cspstop.c` | Stop sequence playback |
| `synthesizer.c` | Audio synthesizer driver |
| `drvrNew.c` | Audio driver initialization |
| `event.c` | Audio event queue |
| `heapalloc.c` | Audio heap allocation |
| `heapinit.c` | Audio heap initialization |
| `load.c` | Audio sample/bank loading |
| `seq.c` | Sequence player |
| `sl.c` | Sound library core |
| `env.c` | Envelope generator |
| `filter.c` | Audio filter |
| `mainbus.c` | Main audio bus mixing |
| `auxbus.c` | Auxiliary audio bus |
| `reverb.c` | Reverb effect |
| `resample.c` | Audio resampling |
| `save.c` | Audio state save |
| `copy.c` | Audio buffer copy |
| `bnkf.c` | Bank file parsing |
| `cents2ratio.c` | Cents-to-frequency ratio conversion |
| `cseq.c` | Compact sequence format |
| `synallocfx.c` | Synth FX allocation |
| `syndelete.c` | Synth voice deletion |
| `n_synthesizer.c` | New synthesizer driver |
| `n_drvrNew.c` | New audio driver init |
| `n_csplayer.c` | New compact sequence player |
| `n_csq.c` | New compact sequence |
| `n_seq.c` | New sequence player |
| `n_seqplayer.c` | New sequence playback |
| `n_sl.c` | New sound library |
| `n_env.c` | New envelope generator |
| `n_envresample.c` | New envelope + resampling |
| `n_load.c` | New sample loading |
| `n_mainbus.c` | New main bus mixing |
| `n_auxbus.c` | New auxiliary bus |
| `n_reverb.c` | New reverb |
| `n_resample.c` | New resampling |
| `n_adpcm.c` | ADPCM audio decoding |
| `n_save.c` | New audio state save |
| `n_synaddplayer.c` | Add player to synth |
| `n_synallocfx.c` | New synth FX allocation |
| `n_synallocvoice.c` | Synth voice allocation |
| `n_syndelete.c` | New synth deletion |
| `n_synfreevoice.c` | Free synth voice |
| `n_synsetfxmix.c` | Set FX mix level |
| `n_synsetpan.c` | Set voice pan |
| `n_synsetpitch.c` | Set voice pitch |
| `n_synsetpriority.c` | Set voice priority |
| `n_synsetvol.c` | Set voice volume |
| `n_synstartvoice.c` | Start voice playback |
| `n_synstartvoiceparam.c` | Start voice with parameters |
| `n_synstopvoice.c` | Stop voice playback |

### src/core1/os/

N64 OS stubs — threading, messaging, timers (reimplemented for PC).

| File | Content |
|------|---------|
| `initialize.c` | OS initialization |
| `createthread.c` | Thread creation |
| `destroythread.c` | Thread destruction |
| `startthread.c` | Start thread |
| `stopthread.c` | Stop thread |
| `yieldthread.c` | Yield thread |
| `getthreadpri.c` | Get thread priority |
| `settreadpri.c` | Set thread priority |
| `thread.c` | Thread management |
| `createmesgqueue.c` | Create message queue |
| `sendmesg.c` | Send message |
| `recvmesg.c` | Receive message |
| `jammesg.c` | Jam (priority) message |
| `seteventmesg.c` | Set event message |
| `settimer.c` | Set timer |
| `stoptimer.c` | Stop timer |
| `timerintr.c` | Timer interrupt handler |
| `gettime.c` | Get system time |
| `pidma.c` | PI DMA transfer |
| `virtualtophysical.c` | Virtual-to-physical address (identity on PC) |
| `setglobalintmask.c` | Set global interrupt mask |
| `resetglobalintmask.c` | Reset global interrupt mask |
| `kdebugserver.c` | Kernel debug server stub |

---

## src/core2/

### Player State & Control (core2/ba/, core2/bs/)

| File | Content |
|------|---------|
| `ba/ba_state.c` | Player BK state machine init and management |
| `ba/ba_position.c` | Player position storage/getters (`player_position[3]`) |
| `ba/ba_stickinterp.c` | Analog stick input interpretation for movement |
| `ba/ba_yaw.c` | Player yaw management (`yaw_get`, `yaw_set`, `yaw_setIdeal`) |
| `ba/ba_eyeblink.c` | Eye animation and blinking state |
| `ba/ba_eyeblink_data.c` | Eye blink animation timing data tables |
| `ba/ba_eyemouth_data.c` | Eye/mouth animation timing tables (actor states) |
| `ba/ba_lookdir.c` | Player look/gaze direction control |
| `ba/ba_health.c` | Health points tracking |
| `ba/ba_sfxintensity.c` | SFX intensity levels for player actions |
| `ba/ba_intensity.c` | Player state intensity/scale parameters |
| `ba/ba_momentum.c` | Player momentum and impulse calculations |
| `ba/ba_recoil.c` | Recoil position, throw target, talk target, turbo duration |
| `ba/ba_falldamage.c` | Fall height tracking with death check |
| `ba/ba_musicstate.c` | Turbo trainers/wading boots/bonus music state management |
| `ba/ba_statusflags.c` | Player status flags |
| `ba/ba_falling.c` | Fall detection and falling state logic |
| `ba/ba_hitbox.c` | Attack hitbox detection (beaks, claws, projectiles) |
| `ba/ba_underwater.c` | Underwater/water state flags |
| `ba/ba_groundsurface.c` | Ground surface detection/properties |
| `ba/ba_animcache.c` | Common animation cache for persistent player animations |
| `ba/ba_modelselect.c` | Player model asset selection by transformation/map |
| `ba/ba_animstate.c` | Animation state flags for player appearance |
| `ba/ba_bounds.c` | Player bounding box |
| `ba/ba_buzz.c` | Bee buzzing state |
| `ba/ba_eyes.c` | Eye state control |
| `ba/ba_flag.c` | Player action flags |
| `ba/ba_input.c` | Player input processing |
| `ba/ba_key.c` | Key/button state |
| `ba/ba_lookat.c` | Head/eye look-at target |
| `ba/ba_sfx.c` | Player sound effects |
| `ba/ba_stick.c` | Analog stick state |
| `ba/ba_timer.c` | Player action timers |
| `ba/ba_anim.c` | Player animation control |
| `ba/ba_carry.c` | Carry/hold object state |
| `ba/ba_carriedobj.c` | Carried object tracking |
| `ba/ba_drone.c` | Player drone/transformation state |
| `ba/ba_flap.c` | Wing flap state |
| `ba/ba_marker.c` | Player actor marker |
| `ba/ba_model.c` | Player model state |
| `ba/ba_physics.c` | Player physics |
| `bs/bs_list.c` | Behavior state list |
| `bs/bs_statemachine.c` | Core behavior state management: `bs_setState`, `bs_getState` |
| `bs/bs_storedstate.c` | Stored behavior state |

### Behavior States (core2/bs/)

Original decomp behavior state files (not prefixed):

| File | Content |
|------|---------|
| `ant.c` | Ant transformation |
| `bBarge.c` | Beak Barge |
| `bbuster.c` | Beak buster |
| `bee.c` | Bee transformation |
| `beeFly.c` | Bee flight |
| `beeMain.c` | Bee main state |
| `bEggAss.c` | Rear egg fire |
| `bEggHead.c` | Forward egg spit |
| `bFlap.c` | Feathery flap |
| `bFlip.c` | Flip jump |
| `bFly.c` | Flight state |
| `bLongLeg.c` | Talon trot |
| `bPeck.c` | Rat-a-tat rap |
| `bShock.c` | Shock spring jump |
| `bSwim.c` | Swimming |
| `bTrot.c` | Trot state |
| `bWhirl.c` | Wonderwing |
| `carry.c` | Carry state |
| `claw.c` | Claw swipe |
| `climb.c` | Climbing |
| `croc.c` | Crocodile transformation |
| `crouch.c` | Crouching |
| `die.c` | Death state |
| `drone.c` | Drone state dispatch (scripted player movement) |
| `droneenter.c` | Drone: enter building/area sequence |
| `dronegoto.c` | Drone: move player to target position |
| `dronelook.c` | Drone: force player look direction |
| `dronevanish.c` | Drone: player vanish/disappear |
| `dronexform.c` | Drone: Mumbo transformation sequence |
| `jig.c` | Jiggy dance |
| `jump.c` | Jump |
| `ow.c` | Hurt/ow state |
| `pumpkin.c` | Pumpkin transformation |
| `rebound.c` | Rebound |
| `rest.c` | Rest/idle |
| `sled.c` | Sled ride |
| `slide.c` | Slide |
| `splat.c` | Splat landing |
| `stand.c` | Standing |
| `surf.c` | Surfing |
| `swim.c` | Swimming variant |
| `talk.c` | Talking |
| `throw.c` | Throw |
| `timeout.c` | Idle timeout |
| `turn.c` | Turning |
| `twirl.c` | Twirl |
| `walk.c` | Walking |
| `walrus.c` | Walrus transformation |
| `washy.c` | Washing |

### Camera System (core2/camera/)

| File | Content |
|------|---------|
| `camera_nodemanager.c` | Camera node type detection and position retrieval |
| `camera_focus.c` | Camera focus/target actor tracking |
| `camera_position.c` | Camera position and viewport frustum calc |
| `camera_fog.c` | Fog color/alpha blending by map |
| `camera_ease.c` | Smooth camera ease-in/ease-out transitions |
| `camera_motor1.c` | Camera motor/actuator control 1 |
| `camera_motor2.c` | Camera motor/actuator control 2 |
| `camera_interp.c` | Camera position interpolation |
| `camera_offset.c` | Camera offset calculation |
| `camera_set.c` | Direct camera position setter |
| `cameranode_type1.c` | Camera node type 1 |
| `cameranode_type2.c` | Camera node type 2 |
| `cameranode_type3.c` | Camera node type 3 |
| `cameranode_type4.c` | Camera node type 4 |
| `cameranodelist.c` | Camera node list management |
| `dynamicCamera.c` | Dynamic camera base |
| `dynamicCam1.c` | Dynamic camera mode 1 |
| `dynamicCam3.c` | Dynamic camera mode 3 |
| `dynamicCam4.c` | Dynamic camera mode 4 |
| `dynamicCam5.c` | Dynamic camera mode 5 |
| `dynamicCam8.c` | Dynamic camera mode 8 |
| `dynamicCam9.c` | Dynamic camera mode 9 |
| `dynamicCam10.c` | Dynamic camera mode 10 |
| `dynamicCam11.c` | Dynamic camera mode 11 |
| `dynamicCam12.c` | Dynamic camera mode 12 |
| `dynamicCam13.c` | Dynamic camera mode 13 |
| `dynamicCamA.c` | Dynamic camera mode A |
| `dynamicCamB.c` | Dynamic camera mode B |
| `dynamicCamC.c` | Dynamic camera mode C |
| `dynamicCamD.c` | Dynamic camera mode D |
| `dynamicCamF.c` | Dynamic camera mode F |
| `firstpersoncamera.c` | First-person camera |
| `randomCamera.c` | Random camera placement |
| `staticCamera.c` | Static/fixed camera |

### Collision (core2/collision/)

| File | Content |
|------|---------|
| `climbsurface.c` | Climbing collision detection and plane management |
| `raycast.c` | BK collision ray casting and triangle detection |
| `spherecast.c` | Collision sphere cast from vertex data |
| `tricheck.c` | Collision triangle intersection checking |
| `raycastmap.c` | Map-specific collision ray casting |
| `filter.c` | Collision detection filtering |
| `cachesize.c` | Collision detection cache sizing |
| `polydetect.c` | Polygon collision detection with UV mapping |
| `init.c` | Collision system initialization |
| `funcs.c` | Function pointers for collision detection |
| `enemy.c` | Enemy collision/damage detection |
| `geometrybounds.c` | Point-to-bounding-box calculations |
| `hitboxdata.c` | Hitbox/collision mapping data (187-entry marker lookup) |

### Actor/Prop System

| File | Content |
|------|---------|
| `actor_cubepropsystem.c` | Cube/prop loading, caching, actor prop initialization |
| `actor_array.c` | Actor array management, spawn/despawn, memcpy |
| `actor_cubebounds.c` | Cube spatial bounding box checking, prop array `D_803820B8` |
| `actor_updatestub.c` | Stub actor update function |
| `actor_cleanup.c` | Actor cleanup and despawn |
| `load_zone.c` | Loadzone entry/map transition handling |
| `actor_motioncurve.c` | Actor motion/movement curve |
| `spawn_queue.c` | Entity spawn queue |
| `actor_pool35.c` | Actor pool manager (35-unit spacing) |
| `actor_pool40.c` | Actor pool manager (40-unit spacing) |
| `actor_pool48.c` | Actor pool manager (48-unit spacing) |

### Collectibles (core2/collectible/) — state/data only, actors in ch/

| File | Content |
|------|---------|
| `bundle.c` | Collectible bundle/group state |
| `printui.c` | Item display/print UI (jiggy, note count) |
| `printdraw.c` | Item rendering/drawing functions |
| `musicnotedata.c` | Music note collection data |
| `honeycombdata.c` | Honeycomb collectible data |

### Effects (core2/fx/)

| File | Content |
|------|---------|
| `effect_sparkle.c` | Sparkle sprite effect (turbo trainers, wading boots) |
| `effect_generic.c` | Generic effect actor initialization |
| `effect_sparkleemit.c` | Sparkle particle emitter effect |
| `effect_ice.c` | Ice/cold particle effect |
| `effect_modelparticle.c` | Model-based particle effect |
| `effect_simplesprite.c` | Simple sprite effect rendering |
| `effect_spriteanim.c` | Animated sprite effect |
| `effect_particle.c` | Generic particle effect actor |
| `effect_enemyrender.c` | Enemy actor rendering (collision/bounds) |
| `effect_ripple.c` | Water ripple/wave particle effect |
| `effect_soundemit.c` | Sound-based effect/emitter |
| `effect_colordata.c` | Effect color RGB data |
| `effect_playerspray.c` | Player particle spray (recoil/knockback) |
| `effect_eggshatter.c` | Egg shatter effect |
| `score_honeycomb.c` | Honeycomb score display |
| `score_jiggy.c` | Jiggy score display |
| `score_mumbo.c` | Mumbo token score display |
| `sparkle.c` | Sparkle effect helpers |
| `ripple.c` | Ripple effect helpers |
| `airscore.c` | Air meter score display |
| `lifescore.c` | Life counter score display |
| `healthscore.c` | Health bar score display |
| `jinjoscore.c` | Jinjo counter score display |
| `honeycarrierscore.c` | Honey carrier score display |
| `common1score.c` | Common score display 1 |
| `common2score.c` | Common score display 2 |
| `common3score.c` | Common score display 3 |

### Particle System (core2/particle/)

| File | Content |
|------|---------|
| `particle.c` | Core particle system |
| `emitter1.c` | Particle emitter control 1 |
| `emitter2.c` | Particle emitter control 2 |
| `spawn.c` | Create/spawn particle emitters |
| `velocityset.c` | Particle velocity setter |
| `samplerate.c` | Audio sample rate for particle sounds |
| `colordefault.c` | Default particle colors |
| `accel.c` | Particle acceleration settings |
| `lifescale.c` | Particle lifetime and scale settings |
| `typeindex.c` | Particle type indexing |
| `velocityrange.c` | Particle velocity ranges |
| `scale1.c` | Particle emitter scale set 1 |
| `scale2.c` | Particle emitter scale set 2 |
| `emitterstruct.c` | Particle emitter data structure |
| `factory.c` | Particle type factory functions |
| `initcallback.c` | Particle initialization callbacks |
| `positionset.c` | Particle position offset setter |
| `playertrail.c` | Particle emitter for player action effects |
| `bathroom.c` | MMM bathroom dust/splash particle system |
| `droplet.c` | Bouncy droplet particle effect actor |

### Audio/SFX (core2/sfx/)

| File | Content |
|------|---------|
| `source.c` | SFX source pool (35 slots), distance fade, 3D positioning |
| `pitched.c` | Play SFX with pitch/frequency variation |
| `randompitch.c` | Random pitch variation for sound effects |
| `init.c` | SFX source initialization/setup |
| `streamctrl.c` | Audio streaming control |
| `playstate.c` | Audio playback state tracking |
| `volume.c` | Audio volume/mixing control |
| `filedata.c` | Audio/music file data structure |
| `sequenceindex.c` | Music sequence indexing |
| `effectplay.c` | Play sound for effect/event |
| `musicplay.c` | Play music/melody for effect |

### Model (core2/model/)

| File | Content |
|------|---------|
| `rendernormal.c` | Normals-based model rendering with matrix transforms |
| `datalookup.c` | Model vertex/mesh data navigation |
| `lighting.c` | Per-vertex lighting/color calculation |
| `skinning.c` | Model matrix transforms for rigged rendering |
| `meshbounds.c` | Model mesh bounding box calculation |
| `matrixrotate.c` | Matrix rotation operations |
| `render.c` | Model rendering pipeline |

### Map (core2/map/)

| File | Content |
|------|---------|
| `model.c` | Map model rendering |
| `savestate.c` | Map save state persistence |
| `specificflags.c` | Per-map state flags |
| `exit.c` | Map exit/transition handling |
| `audioconfig.c` | Per-map light/sound configuration (22 maps) |

### Texture (core2/texture/)

| File | Content |
|------|---------|
| `dataaccess.c` | Texture list data pointer access |
| `copy.c` | TMEM texture copy operations |

### Sprite (core2/sprite/)

| File | Content |
|------|---------|
| `rendertex.c` | Sprite rendering with TMEM tile setup |
| `render.c` | Sprite rendering with segment-based drawing |
| `displaydata.c` | BKSprite display data accessors |
| `texblock.c` | Sprite texture block memory pointer |
| `animstep.c` | Sprite animation frame stepping |
| `screenoverlay.c` | Full-screen sprite overlay rendering |
| `actordefault.c` | Default sprite actor handler |
| `dialogdata.c` | Dialog sprite display data |

### Vertex Ops (core2/vtx/)

| File | Content |
|------|---------|
| `listutils.c` | Vertex list utility functions |
| `meshcount.c` | Count vertices across mesh lists |
| `colorapply.c` | Apply vertex colors/lighting |
| `transform.c` | Vertex position transformation |
| `colorset.c` | Set vertex color values |
| `uvset.c` | Set vertex UV coordinates |
| `normalset.c` | Set vertex normal vectors |
| `convert.c` | Vertex format conversion |
| `renderstart.c` | Model rendering initialization |
| `alphablend.c` | Vertex alpha blending |
| `scale.c` | Scale vertex positions |
| `positionset.c` | Set vertex position values |
| `normalcompute.c` | Vertex normal computation from mesh |
| `colorintensity.c` | Vertex color lighting intensity modifier |

### Animation (core2/anim/)

| File | Content |
|------|---------|
| `anim_buffer.c` | Animation double/triple buffering and interpolation |
| `anim_bonetransform.c` | Bone animation file parsing, Catmull-Rom interpolation |
| `anim_defrag.c` | Memory defragmentation for animation/asset caches |
| `anim_sequencestruct.c` | Animation sequence data structure |
| `anim_driver.c` | Actor animation playback driver |
| `anim_sequencehandler.c` | Animation sequence playing/management |
| `anim_spriteframe.c` | Sprite frame animation control |
| `anim_cache.c` | Animation data cache |
| `anim_texturecache.c` | Animation texture cache |
| `sprite.c` | Animation sprite helpers |

### Game Controller UI (core2/gc/)

| File | Content |
|------|---------|
| `text_dialogactor.c` | Text/dialog display actor |
| `zoombox.c` | Zoombox UI widget |
| `dialog.c` | Dialog system |
| `pauseMenu.c` | Pause menu |
| `section.c` | UI section management |
| `sky.c` | Sky rendering |
| `transition.c` | Screen transitions |
| `bound.c` | UI bounds/layout |
| `parade.c` | Character parade sequence |

### Lighting (core2/lighting/)

| File | Content |
|------|---------|
| `compute.c` | Lighting calculation and application |
| `setup.c` | Lighting structure initialization |
| `apply.c` | Apply lighting to scene |

### Level (core2/level/)

| File | Content |
|------|---------|
| `metadata.c` | Per-level metadata (colors, music, geometry) |
| `collectible.c` | Per-level collectible tracking |
| `specificflags.c` | Per-level state flags |
| `lightconfig.c` | Per-level lighting configuration by map ID |

### Cutscene (core2/cutscene/)

| File | Content |
|------|---------|
| `lair.c` | Lair entrance cutscene detection/skipping |
| `nodeupdate.c` | Cutscene node property updates |
| `flag.c` | Cutscene playback flag |
| `actorstruct.c` | Cutscene actor data structure |
| `animated.c` | Animated cutscene actor |
| `ctrl.c` | Cutscene actor animation control |

### Font (core2/font/)

| File | Content |
|------|---------|
| `render.c` | Font rendering system (sprite texture block cache, letter mapping) |
| `print.c` | Font kerning/spacing lookup tables, print buffer management |

### Dialog (core2/dialog/)

| File | Content |
|------|---------|
| `binload.c` | Dialog binary asset loader with fallback |
| `code_CF3E0.c` | Dialog system helpers |

### Display/Frame (core2/gfx/, core2/frame/)

| File | Content |
|------|---------|
| `gfx/displaylistinit.c` | Display list initialization/setup |
| `gfx/displaylistdata.c` | Display list static data arrays |
| `frame/rendermem.c` | Frame rendering memory/buffering |
| `frame/bufferreadback.c` | CPU framebuffer readback for transitions |

### Quiz (core2/quiz/)

| File | Content |
|------|---------|
| `game.c` | Quiz/trivia minigame actor updates |
| `bitfield.c` | Quiz question asked bitfield tracking |
| `questionmanager.c` | Quiz question selection and validation |
| `minigamemanager.c` | Quiz minigame state management |
| `storage.c` | Quiz data storage and persistence |

### Characters & Actors (core2/ch/)

All spawnable actor implementations live here — characters, UI overlays, FX emitters, collectible actors.

| File | Content |
|------|---------|
| `badShad.c` | Bad shadow actor |
| `bat.c` | Bat enemy |
| `beehive.c` | Beehive actor |
| `beeswarm.c` | Bee swarm actor (quiz/minigame context) |
| `bigbutt.c` | Big Butt enemy |
| `bottlesbonus.c` | Bottles' Bonus minigame actor |
| `bottlesbonuscursor.c` | Bottles' Bonus cursor actor |
| `bubble.c` | Bubble actor |
| `clankerwhipcrack.c` | Clanker whip crack effect |
| `climbBase.c` | Climbable base actor |
| `clucker.c` | Clucker enemy |
| `collectible.c` | Generic collectible actor |
| `crab.c` | Crab enemy |
| `drips.c` | Water drips particle emitter |
| `extralife.c` | Extra life collectible actor |
| `feather.c` | Feather collectible actor |
| `firefx.c` | Fire effect particle emitter |
| `flotsam.c` | Flotsam actor |
| `ghost.c` | Ghost enemy |
| `gloop.c` | Gloop actor |
| `goldfeather.c` | Gold feather collectible actor |
| `gravestone.c` | Gravestone actor |
| `grill_chompa.c` | Grill Chompa enemy (bite attack, animation) |
| `honeycomb.c` | Honeycomb collectible actor |
| `icecube.c` | Ice cube actor |
| `jiggy.c` | Jiggy collectible actor |
| `jigsawdance.c` | Jigsaw dance celebration |
| `jinjo.c` | Jinjo character actor |
| `mmm_veggies.c` | MMM vegetable actors (Topper, Bawl, Collywobble) |
| `mole.c` | Mole enemy |
| `molehill.c` | Molehill (Bottles tutorial) actor |
| `mumbotoken.c` | Mumbo token collectible actor |
| `musicnote.c` | Music note actor |
| `musicnote_container.c` | Music note container actor (ACTOR 0x183) |
| `overlaycopyright.c` | Copyright overlay actor |
| `overlaynocontroller.c` | No-controller overlay actor |
| `overlaypressstart.c` | Press Start overlay actor |
| `shrapnel.c` | Shrapnel/debris actor |
| `snacker.c` | Snacker shark actor |
| `snowball.c` | Snowball actor |
| `snowman.c` | Snowman actor |
| `snowmanhat.c` | Snowman hat actor |
| `soundsource.c` | Positioned sound emitter actor |
| `termite.c` | Termite transformation actor |
| `trainers.c` | Turbo Trainers pickup actor |
| `wadingboots.c` | Wading Boots pickup actor |
| `whipcrack.c` | Whip crack effect |

### Misc (core2/ root)

| File | Content |
|------|---------|
| `abilityprogress.c` | Ability unlock progress tracking |
| `anctrl.c` | Animation controller |
| `anim_bonetransformlist.c` | Bone transform list operations (quaternion/scale) |
| `anseq.c` | Animation sequence system |
| `audio_sfxinstruments.c` | SFX instrument data |
| `background_decor.c` | Background decoration actor |
| `climb.c` | Climbing mechanics |
| `coords.c` | Coordinate conversion utilities |
| `crc_bootvalidation.c` | Boot segment CRC validation (anti-tamper, stubbed) |
| `debug_assert.c` | Debug assertion/error output |
| `demo_input.c` | Demo playback input |
| `equipped_items.c` | Equipped item tracking (16-slot bitfield) |
| `file.c` | File/save data management |
| `fileselect.c` | File select screen |
| `fla.c` | FLA (full-screen animation) playback |
| `game_complete.c` | Game completion/end sequence |
| `gameSelect.c` | Game select screen |
| `glspline.c` | GL spline path animation system |
| `matrix_interp.c` | Matrix/quaternion interpolation |
| `mesh_dataaccess.c` | Mesh data structure access helpers |
| `minigame_fish.c` | Fish minigame control (2D swimming AI) |
| `misc_updates.c` | Miscellaneous update functions |
| `mumbo_sign.c` | Mumbo's skull sign |
| `mumbo_transforms.c` | Mumbo transformation system |
| `overlay.c` | Core2 overlay management |
| `physics_force.c` | Force/acceleration physics |
| `pitch.c` | Pitch/angle calculation |
| `placeholder_stub.c` | Placeholder/dummy stub function |
| `playerutils.c` | Player utility functions |
| `projectile_blueegg.c` | Blue egg projectile |
| `prop_decoration.c` | Prop decoration/scenery rendering |
| `propeller_engine.c` | Propeller/engine movement system |
| `rand.c` | Random number generator |
| `roll.c` | Roll mechanics |
| `rope_swing.c` | Rope/vine swinging mechanics |
| `savedata.c` | Save data system |
| `shock_jumppad.c` | Shock/electric jump pad effect |
| `skeletal_anim.c` | Skeletal animation system |
| `snacker_ctl.c` | Snacker shark control |
| `snacker_pool1.c` | Snacker shark pool 1 (5-slot, 40-unit spacing) |
| `snacker_pool2.c` | Snacker shark pool 2 (3-slot, 40-unit spacing) |
| `spawn_queue.c` | Entity spawn queue |
| `spline_bezier.c` | Bezier spline curve evaluation |
| `state_timer.c` | State transition timer |
| `string.c` | String utilities |
| `terrain_material.c` | Terrain surface material type |
| `text_renderdata.c` | Text rendering coordinate/offset data |
| `time.c` | Time management |
| `time_delta.c` | Frame-based time delta |
| `time_scale.c` | Global time scale/slowdown |
| `timed_funcqueue.c` | Timed callback queue |
| `vla.c` | Variable-length array |
| `vtxlist.c` | Vertex list management |
| `yaw.c` | Yaw angle utilities |

---

## src/CC/ (Clanker's Cavern)

| File | Content |
|------|---------|
| `actor_spawninit.c` | Actor spawn registration for CC |
| `bss_pad.c` | BSS padding |
| `crc.c` | Anti-tamper CRC validation |
| `challenge_ctrl.c` | Challenge/minigame control (timer, puzzle) |
| `platform_physics.c` | Complex collision/physics platform actor |
| `model_renderstate.c` | Model rendering with state-based transforms |
| `ch/screw.c` | Screw actor init/state machine |
| `ch/key.c` | CC key actor mechanics (pickup, state, sfx) |
| `ch/token_teeth.c` | Clanker token/jiggy teeth exterior actor |
| `ch/collectibles.c` | Token teeth, jiggy teeth items |
| `ch/sawblade.c` | Saw blade hazard |
| `ch/tooth.c` | Clanker tooth actor |

## src/BGS/ (Bubblegloop Swamp)

| File | Content |
|------|---------|
| `actor_spawninit.c` | Actor spawn registration for BGS |
| `bss_pad.c` | BSS padding |
| `crc.c` | Anti-tamper CRC validation |
| `minigame_vile.c` | Mr. Vile minigame controller |
| `ch/bigalligator.c` | Big alligator actor |
| `ch/choir_turtle.c` | Choir turtle behavior (idle/sing/hurt) |
| `ch/croctus.c` | Croctus NPC |
| `ch/flibbit.c` | Flibbit enemy |
| `ch/frogminigame.c` | Frog minigame actor |
| `ch/leafboat.c` | Leaf boat actor |
| `ch/mrvile.c` | Mr. Vile NPC |
| `ch/mudhut.c` | Mud hut actor |
| `ch/pinkegg.c` | Pink egg actor |
| `ch/swamp_switch.c` | Swamp switch actors (long/short press) |
| `ch/tanktup.c` | Tanktup NPC |
| `ch/tanktup_legs.c` | Tanktup legs animation & prop data |
| `ch/tiptup.c` | Tiptup NPC state machine |
| `ch/yellowflibbit.c` | Yellow Flibbit enemy |
| `ch/yumblie.c` | Yumblie enemy |

## src/TTC/ (Treasure Trove Cove)

| File | Content |
|------|---------|
| `actor_spawninit.c` | Actor spawn registration + misc ocean actors |
| `crc.c` | Anti-tamper CRC validation |
| `ch/blubber.c` | Captain Blubber NPC |
| `ch/clam.c` | Clam actor |
| `ch/leaky.c` | Leaky bucket actor |
| `ch/lockup.c` | Lockup cage actor |
| `ch/nipper.c` | Nipper crab boss |
| `ch/treasure.c` | Treasure chest actor |
| `ch/treasurehunt.c` | Treasure hunt minigame |
| `ma/castle.c` | Sandcastle puzzle mechanics |

## src/MM/ (Mumbo's Mountain)

| File | Content |
|------|---------|
| `actor_spawninit.c` | Actor spawn registration (9 animal actors) |
| `crc.c` | Anti-tamper CRC validation |
| `ch/chimpystump.c` | Chimpy stump actor |
| `ch/conga.c` | Conga ape boss |
| `ch/grublin.c` | Grublin enemy |
| `ch/hut.c` | Hut actor |
| `ch/juju.c` | Juju totem actor |
| `ch/jujuhitbox.c` | Juju hitbox actor |
| `ch/lmonkey.c` | Little monkey actor |
| `ch/orange.c` | Orange projectile |
| `ch/orangepad.c` | Orange pad actor |

## src/MMM/ (Mad Monster Mansion)

| File | Content |
|------|---------|
| `actor_spawninit.c` | Actor registry/factory for MMM NPCs |
| `bss_pad.c` | BSS padding |
| `crc.c` | Anti-tamper CRC validation |
| `napper_room.c` | Napper room control (sleep state, dialog) |
| `minigame_organ.c` | Organ minigame note/key mappings |
| `minigame_shed.c` | Tumbar's Shed minigame |
| `ch/flowerpot.c` | Cemetery flowerpot collectible actor |
| `ch/loggo.c` | Loggo toilet NPC |
| `ch/motzhand.c` | Motzand piano hand boss |
| `ch/napper.c` | Napper ghost character actor |
| `ch/portrait.c` | Grunty portrait actors (6 variants) |
| `ch/portrait_chompa.c` | Portrait Chompa enemy with bite/death mechanics |
| `ch/tumblar.c` | Tumblar character (jiggy collector states) |
| `ch/color_puzzle.c` | Color puzzle cube with RGB fade effects |

## src/FP/ (Freezeezy Peak)

| File | Content |
|------|---------|
| `actor_spawninit.c` | FP actor registry and misc prop/triggers |
| `bss_pad.c` | BSS padding |
| `wozza_fire.c` | Wozza's cave fire ambience sound management |
| `ch/bearcub.c` | Bear cub NPC |
| `ch/boggy1.c` | Boggy NPC (race 1) |
| `ch/boggy2.c` | Boggy NPC (race 2) |
| `ch/boggy3.c` | Boggy NPC (race 3) |
| `ch/boating.c` | Boating actor (slalom course detection) |
| `ch/boating2.c` | Boating actor variant 2 (collision callback) |
| `ch/cavewozza.c` | Cave Wozza NPC |
| `ch/fp_trigger.c` | Simple collision-off trigger actor |
| `ch/present.c` | Present/gift actor |
| `ch/racesled.c` | Race sled actor |
| `ch/scarfsled.c` | Boggy's scarf sled |
| `ch/snowmanbutton.c` | Snowman button actor |
| `ch/twinkly.c` | Christmas light twinkly |
| `ch/twinklybox.c` | Twinkly box container |
| `ch/twinklymuncher.c` | Twinkly Muncher enemy |
| `ch/wozza.c` | Wozza NPC |
| `ch/wozzasjig.c` | Wozza's jiggy actor |
| `ch/xmastree.c` | Christmas tree actor |
| `ch/xmastreeeggtoll.c` | Christmas tree egg toll |
| `ch/xmastreeice.c` | Christmas tree ice variant |
| `ch/xmastreestar.c` | Christmas tree star |
| `ch/xmastreeswitch.c` | Christmas tree switch |
| `ma/slalom.c` | Slalom race course mechanics |
| `ma/snowbutton.c` | Snow button mechanics |
| `ma/snowy.c` | Snowy environment mechanics |

## src/GV/ (Gobi's Valley)

| File | Content |
|------|---------|
| `actor_spawninit.c` | GV master actor registry + door/switch handlers |
| `bss_pad.c` | BSS padding |
| `crc.c` | Anti-tamper CRC + turbo talon state clamp |
| `matchinggame.c` | Memory matching minigame |
| `waterctrl.c` | Water level control |
| `smoke_particle.c` | Smoke particle effect for water pyramid |
| `pyramid_raise.c` | Buried pyramid raising mechanism |
| `pyramid_trigger.c` | Pyramid collision trigger |
| `gv_helpers.c` | Helper functions (ding sound, state) |
| `water_pyramidrot.c` | Water pyramid rotation based on jiggy count |
| `water_pyramidactivate.c` | Water pyramid activation sequence |
| `ch/ancientone.c` | Ancient One NPC |
| `ch/buriedpyramid.c` | Buried pyramid actor |
| `ch/carpet_shadow.c` | Magic carpet shadow rendering |
| `ch/carpet1.c` | Magic carpet 1 transport with shadow |
| `ch/carpet2.c` | Magic carpet 2 with dynamic shadow |
| `ch/egg_pot.c` | Rubee's egg pot actor |
| `ch/gobi1.c` | Gobi NPC encounter 1 |
| `ch/gobi2.c` | Gobi NPC encounter 2 |
| `ch/gobi3.c` | Gobi NPC encounter 3 |
| `ch/gobirock.c` | Gobi's rock actor |
| `ch/gobirope.c` | Gobi's rope actor |
| `ch/grabba.c` | Grabba hand enemy |
| `ch/gv_decor.c` | Stateless decorative actors |
| `ch/gv_enemy.c` | GV enemy state machine |
| `ch/histup.c` | Histup NPC |
| `ch/jinxy.c` | Jinxy NPC dialog interaction |
| `ch/jinxy_head.c` | Jinxy head with lip smack sounds |
| `ch/mazectrl.c` | Maze control actor |
| `ch/rubee.c` | Rubee NPC with dialog and jiggy tracking |
| `ch/sarcophagus.c` | Sarcophagus actor |
| `ch/slappa.c` | Slappa actor (dust particles, animation) |
| `ch/toots.c` | Toots snake charmer NPC |
| `ch/trunker.c` | Trunker tree NPC |

## src/RBB/ (Rusty Bucket Bay)

| File | Content |
|------|---------|
| `actor_spawninit.c` | Actor spawn registration |
| `bss_pad.c` | BSS padding |
| `propellorctrl.c` | Propeller control system |
| `crane_ctrl.c` | Crane animation control and color/sound feedback |
| `crane_particle.c` | Crane particle effects and crushing sequences |
| `life_balloontrigger.c` | Extra life bundle spawn trigger |
| `ch/anchor.c` | Anchor actor |
| `ch/anchorctrl.c` | Anchor control |
| `ch/anchorswitch.c` | Anchor switch |
| `ch/axle.c` | Axle actor |
| `ch/bellbuoy.c` | Bell buoy actor |
| `ch/boombox.c` | Boom Box enemy |
| `ch/bossboombox.c` | Boss Boom Box boss |
| `ch/bossboomboxctrl.c` | Boss Boom Box controller |
| `ch/cog.c` | Cog actor |
| `ch/dolphin.c` | Dolphin NPC |
| `ch/eggtoll1.c` | Egg toll actor |
| `ch/enginefan.c` | Engine fan hazard |
| `ch/enginefanswitch.c` | Engine fan switch |
| `ch/engineparts.c` | Engine parts actor |
| `ch/grimlet.c` | Grimlet enemy |
| `ch/propellor.c` | Propeller hazard |
| `ch/propellorswitch.c` | Propeller switch |
| `ch/rarewareflag.c` | Rareware flag actor |
| `ch/rowboat.c` | Rowboat actor |
| `ch/siren.c` | Pirate siren/alarm actor |
| `ch/whistle.c` | Whistle actor |
| `ch/whistlectrl.c` | Whistle control |
| `ch/whistleswitch.c` | Whistle switch |

## src/CCW/ (Click Clock Wood)

| File | Content |
|------|---------|
| `actor_spawninit.c` | CCW master actor factory declarations |
| `bss_pad.c` | BSS padding |
| `minigame_zubba.c` | Zubba minigame boss activation |
| `seasonal_switch.c` | Multi-season puzzle switches |
| `ch/autumn_flower.c` | Autumn flower actor (state-based visibility) |
| `ch/caterpillar.c` | Caterpillar NPC (follows player, despawns) |
| `ch/cracking_actor.c` | Cracking animation with depth effects |
| `ch/dancing_npc.c` | Dancing character with synced sound |
| `ch/destruct_rock.c` | Rock destruction with particles |
| `ch/eyrie_egg.c` | Eyrie egg hatching sequence |
| `ch/eyrie_summer.c` | Summer Eyrie NPC |
| `ch/eyrie_winter.c` | Winter Eyrie NPC |
| `ch/fairy.c` | Fairy NPC with particles and seasonal appearance |
| `ch/gobi_ccw.c` | Gobi NPC CCW variant (spitting, dialog) |
| `ch/grublinhood.c` | Grublin Hood enemy |
| `ch/idle_npc.c` | NPC with random idle animation |
| `ch/moving_creature.c` | Moving creature actor with sound attenuation |
| `ch/nabnut.c` | Nabnut NPC (acorn collection quest) |
| `ch/season_dialoguenpc.c` | Season-aware NPC with conditional dialog |
| `ch/seasonal_boss.c` | Seasonal boss with multi-phase animations |
| `ch/seasonal_collect.c` | Seasonal collectible with proximity detection |
| `ch/seasonal_npc.c` | Seasonal character with animation and jiggy |
| `ch/seasonal_variant.c` | Seasonally-variant character actors |
| `ch/snare_bear.c` | Snare Bear decorative NPC |
| `ch/snare_beartrap.c` | Snare Bear trap that attacks bee form |
| `ch/splash_item.c` | Splashable item actor (orange splat) |
| `ch/toad.c` | Toad NPC with random voice lines |
| `ch/vacationtexttrigger.c` | Vacation text trigger |
| `ch/zubba.c` | Zubba (wasp) enemy with homing flight |

## src/SM/ (Spiral Mountain)

| File | Content |
|------|---------|
| `anim_callbacks.c` | Animation sequence sound trigger callbacks |
| `crc.c` | Anti-tamper CRC + ability patching (stubbed) |
| `model_visibility.c` | Map-specific model visibility control |
| `version_compat.c` | Regional version compatibility stubs |
| `version_compat.us.v10.c` | US v1.0 specific compatibility |
| `ch/attacktutorial.c` | Attack tutorial actor |
| `ch/furniture.c` | Banjo's house furniture actors |
| `ch/glasses_toad.c` | Proximity trigger for Glasses Toad event |
| `ch/jumptutorial.c` | Jump tutorial actor |
| `ch/quarrie.c` | Quarrie NPC and post-boss rock actor |
| `ch/quarrie_honeycomb.c` | Quarrie honeycomb spawn controller |
| `ch/smbottles.c` | Spiral Mountain Bottles NPC |
| `ch/vegetables.c` | SM vegetable actors |

## src/lair/ (Grunty's Lair)

| File | Content |
|------|---------|
| `actor_spawninit.c` | Actor spawn registration (60+ actors) |
| `bss_pad.c` | BSS padding |
| `cheato.c` | Cheato NPC |
| `consumable_refills.c` | Consumable item refill stations |
| `ff_manager.c` | Furnace Fun game manager |
| `puzzle_door.c` | Puzzle door system with cost/jiggy rewards |
| `fall_damagedetect.c` | Collision detection fallback (fall damage) |
| `lair_particle.c` | Environmental particle effects for Lair |
| `minigame_ffprize.c` | Furnace Fun minigame prize/reward actors |
| `ch/brentilda.c` | Brentilda NPC |
| `ch/cauldron.c` | Cauldron warp point actor |
| `ch/flying_spawner.c` | Flying enemy spawner with waypoints |
| `ch/gruntling.c` | Gruntling enemies (Red, Blue, Black) |
| `ch/grunty_final.c` | Grunty final form cutscene actor |
| `ch/tooty_parade.c` | Tooty trigger for character parade |

## src/cutscenes/

| File | Content |
|------|---------|
| `actor_spawninit.c` | Cutscene actor registry and declarations |
| `bss_pad.c` | BSS padding |
| `cutscene_animsequence.c` | Cutscene animation sequence framework |
| `ch/sparkle.c` | Cutscene sparkle particle effect |
| `ch/glow_sparkle.c` | Dual-layer cutscene glow + sparkle |
| `ch/smoke.c` | Cutscene smoke/dust effects |

## src/emptyLvl/

| File | Content |
|------|---------|
| `empty_level.c` | Empty level stub (dummy byte + BSS + placeholder func) |

---

## Headers moved to include/

| From | To | Content |
|------|----|---------|
| `src/core2/code_B6EA0.h` | `include/core2/sprite_displaydata.h` | ParticleStruct0s, projectile prototypes |
| `include/core2/code_6DA30.h` | `include/core2/text_print.h` | Text printing function prototypes |
| `src/core2/quiz_storage.h` | `include/core2/quiz_storage.h` | Furnace Fun quiz structs (FF_StorageStruct) |
| `src/core2/snackerctl.h` | `include/core2/snackerctl.h` | SnackerCtlState enum, snacker control prototypes |
| `src/core2/ch/snacker.h` | `include/core2/ch/snacker.h` | Snacker actor info, init/state prototypes |
| `src/core2/gc/zoombox.h` | `include/core2/gc/zoombox.h` | GcZoombox struct, dialog portrait sprites enum |

---

## Notes

1. **`actor_spawninit.c`** appears in nearly every overlay — it's the per-level actor registry. Each level has exactly one.
2. **`bss_pad.c`** appears in most overlays — BSS section padding for overlay alignment.
3. **`crc.c`** exists in overlays with anti-tamper checks (CC, BGS, MMM, MM, TTC, GV, SM).
4. **No `code_*.c` files remain** (except `dialog/code_CF3E0.c`) — all source files have been renamed to descriptive names.
5. **Headers**: Decomp headers shared across subdirectories moved from `src/` to `include/` with full-path includes.
6. **Level-local headers** (`CC.h`, `fight.h`) remain in their source directories (only used by sibling files).
7. **`ch/` convention**: All spawnable actors (anything with `ActorInfo`) live in `ch/`. State tracking and data tables live in thematic subdirectories (`collectible/`, `fx/`, etc.).
8. **`ma/` convention**: Map-specific mechanics (slalom courses, sandcastle puzzles) in overlay `ma/` subdirectories.
