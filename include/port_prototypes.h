#ifndef PORT_PROTOTYPES_H
#define PORT_PROTOTYPES_H

#include <ultra64.h>
#include "structs.h"
#include "enums.h"
#include "model.h"
#include "core2/camera.h"
#include "core2/anim/sprite.h"
#include "core2/ba/anim.h" // [port] needed for enum baanim_update_type_e in baanim_setUpdateType prototype
#include "core2/nc/camera.h" // [port] needed for enum camera_type_e in camera_setType prototype
// FuncUnk40 defined in core2/sprite_displaydata.h — forward-typedef here to avoid circular include
#ifndef FUNCUNK40_DEFINED
#define FUNCUNK40_DEFINED
typedef s32 (*FuncUnk40)(ActorMarker *, s32, f32[3]);
#endif

// ============================================================
// POINTER-RETURNING FUNCTIONS (crash without prototypes)
// ============================================================

// --- core2/camera/camera_interp.c ---
BKModelBin *func_8030A4B4(s32 arg0);
BKSpriteDisplayData *func_8030A4D4(s32 arg0);
BKSprite *func_8030A55C(s32 arg0);

// --- core2/actor_cubepropsystem.c ---
ActorMarker *func_8032DCAC(void);
BKVertexList *func_80330CFC(Actor *self, s32 arg1);
BKModelBin *func_80330E28(Actor *self);
BKSpriteDisplayData *func_80330E54(ActorMarker *marker, BKSprite **sprite_ptr);
BKSpriteDisplayData *func_80330F30(ActorMarker *marker);
BKSprite *func_80330F50(ActorMarker *marker);
BKSprite *func_8033B6C4(enum asset_e sprite_id, BKSpriteDisplayData **arg1);
NodeProp *codeA5BC0_getPropNodeAtIndex(Cube *cube, s32 prop_index);

// --- core2/actor_cubebounds.c ---
NodeProp *cubeList_findNodePropByActorIdAndPosition_s32(enum actor_e actor_id, s32 position[3]);
NodeProp *func_80305510(s32 arg0);
BKCollisionTri *func_80303800(f32 volume_p1[3], f32 volume_p2[3], f32 arg2[3], u32 arg3);
NodeProp *nodeprop_findByActorIdAndPosition_s16(enum actor_e actor_id, s16 *position);

// --- core2/map/model.c ---
BKCollisionTri *func_80309B48(f32 startPoint[3], f32 endPoint[3], f32 arg2[3], u32 flagFilter);
// NOTE: func_802E76B0, func_802E805C, func_802E8E88, func_802E9118,
// func_802E92AC, func_802E9DD8 return BKCollisionTri* but have conflicting
// local externs (bool/s32/void) in decomp source files. Files that need
// the pointer type already have correct local externs.
BKCollisionTri *func_802E76B0(BKCollisionList *collisionList, BKVertexList *vertexList, f32 startPoint[3], f32 endPoint[3], f32 arg4[3], u32 flagFilter);

// --- core2/actor_cubepropsystem.c ---
BKCollisionTri *func_803311D4(Cube *cube, f32 arg1[3], f32 arg2[3], f32 arg3[3], u32 arg4);

// --- core2/misc_updates.c ---
BKCollisionTri *func_8029463C(void);
BKModelBin *func_802946A8(void);
BKCollisionTri *func_802946CC(void);

// --- core2/collision/raycast.c ---
BKCollisionTri *func_8031BABC(f32 *arg0, f32 arg1, f32 arg2, u32 arg3, struct86s *arg4);
BKCollisionTri *func_8031BBA0(f32 *self, f32 arg1, f32 arg2, u32 arg3, struct86s *arg4);
BKCollisionTri *func_8031C5EC(struct0 *self);
BKCollisionTri *func_8031C5F4(struct0 *self);
BKModelBin *func_8031C5DC(struct0 *self);

// --- core2/collision/funcs.c ---
BKCollisionTri *func_80320B98(f32 arg0[3], f32 arg1[3], f32 arg2[3], u32 arg3);
// func_80320C94, func_80320DB0 return BKCollisionTri* but
// have conflicting local externs (bool/s32/int) in decomp source files.
void *func_803209EC(void);

// --- core1/collision.c ---
BKCollisionTri *func_80244E54(f32 arg0[3], f32 arg1[3], f32 arg2[3], u32 arg3, f32 arg4, f32 arg5);
BKCollisionTri *func_8024575C(f32 arg0[3], f32 arg1[3], f32 arg2, f32 arg3[3], s32 arg4, u32 arg5);
BKCollisionTri *func_802457C4(f32 arg0[3], f32 arg1[3], f32 arg2, f32 arg3, f32 arg4[3], s32 arg5, u32 arg6);

// --- core2/actor_array.c ---
BKModelBin *func_803257B4(ActorMarker *marker);
Actor *actorArray_findActorFromMarkerId(enum marker_e marker_id);
void *actors_appendToSavestate(void *begin, uintptr_t end);

// --- core2/glspline.c ---
struct56s *func_80342038(s32 indx);
struct56s *func_80341EF0(f32 arg0[3]);
struct56s *func_80341F64(s32 arg0);
struct56s *func_80343F00(s32 indx, f32 arg1[3]);

// --- core2/collision/polydetect.c ---
Struct83s *func_803406B0(void);
Struct83s *func_803406D4(Struct83s *self);

// --- core2/vtxlist.c ---
BKVertexList *vtxList_clone(BKVertexList *vtxList);

// --- core2/spline_bezier.c ---
struct5Bs *func_8034A2C8(void);
struct5Bs *func_8034A348(struct5Bs *self);

// --- core2/mapModel.c ---
struct5Bs *func_803097A0(void);
BKCollisionList *model_getCollisionList(BKModelBin *arg0);

// --- core2/modelRender.c ---
BKModelUnk14List *func_8033A12C(BKModelBin *self);

// --- core2/vtx/colorapply.c ---
Struct70s *func_8034C344(s32 arg0);
Struct70s *func_8034C448(s32 arg0);
Struct70s *func_8034C630(void *arg0); // [port] was s32 — receives pointer on 64-bit

// --- core2/particle/samplerate.c ---
Struct5Ds *func_802F47D0(void);
Struct5Ds *func_802F499C(Struct5Ds *self);

// --- core2/font/render.c ---
BKSpriteTextureBlock *func_802E4D5C(s32 arg0, char arg1);

// --- core2/font/print.c ---
BKSpriteTextureBlock *func_802F5494(s32 letterId, s32 *fontType);

// --- core2/anim/anim_spriteframe.c / particle/typeindex.c etc. ---
ParticleEmitter *func_802EDD8C(f32 pos[3], f32 xz_range, f32 arg2);
ParticleEmitter *func_802F1EC8(f32 *position);
ParticleEmitter *func_802F3E98(f32 pos[3], enum asset_e sprite_id);
ParticleEmitter *func_802F4274(f32 arg0[3]);
ParticleEmitter *func_802F0EF0(u8 arg0);

// --- core2/collectible/bundle.c ---
Actor *bundle_spawn_f32(enum bundle_e bundle_id, f32 position[3]);
Actor *bundle_spawn_s32(enum bundle_e bundle_id, s32 position[3]);

// --- core2/actor_array.c (jiggy actors) ---
Actor **actorArray_findJiggyActors(void);

// --- core2/ba/ba_animcache.c ---
#ifndef ANIMATION_H
typedef struct animation_file_s AnimationFile;
#endif
AnimationFile *animBinCache_get(enum asset_e asset_id);

// --- core2/anim/anim_buffer.c ---
BoneTransformList *anim_getTransform(Animation *self, s32 index);
BoneTransformList *animcache_getCurrentTransform(Animation *self);
BoneTransformList *anim_getStartTransform(Animation *self);
BoneTransformList *anim_getTargetTransform(Animation *self);

// --- core1/audio_manager.c ---
void *func_802403B8(void *state);
void *audioManager_getThread_PAL(void);

// --- core2/anim/anim_sequencehandler.c ---
u8 *func_8032479C(void);

// --- core2/anim/anim_defrag.c (animMtxList) ---
// AnimMtxList already typedef'd via anctrl.h (included earlier via prop.h)
// BoneTransformList needs forward declaration (bonetransform.h not in include chain)
#ifndef _BONE_TRANSFORMATION_H_
typedef struct bone_transform_list_s BoneTransformList;
#endif
MtxF *animMtxList_get(AnimMtxList *self, s32 arg1);
AnimMtxList *animMtxList_new(void);
void animMtxList_free(AnimMtxList *self);
s32 animMtxList_len(AnimMtxList *self);
void animMtxList_setBoneless(AnimMtxList **this_ptr, BKAnimationList *anim_list);
void animMtxList_setBoned(AnimMtxList **this_ptr, BKAnimationList *anim_list, BoneTransformList *arg2);
AnimMtxList *animMtxList_defrag(AnimMtxList *self);

// ============================================================
// FLOAT-RETURNING FUNCTIONS
// ============================================================

f32 alCents2Ratio(s32 cents);
f32 babuzz_80290890(f32 arg0);
f32 babuzz_80290920(f32 arg0, f32 arg1, f32 arg2);
f32 baeyes_getEyePosition(s32 id);
f32 baanim_getTimer(void);
f32 batimer_get(s32 id);
f32 bafalldamage_get_distance_fallen(void);
f32 bastick_calculateZonePosition(f32 arg0, f32 arg1, f32 arg2);
f32 bastick_getX(void);
f32 bastick_getAngle(void);
f32 baModel_getYaw(void);
f32 baphysics_get_gravity(void);
f32 baphysics_get_target_horizontal_velocity(void);
f32 baphysics_get_target_vertical_velocity(void);
f32 baphysics_get_target_yaw(void);
f32 baphysics_get_vertical_velocity(void);
f32 baphysics_get_horizontal_velocity(void);
f32 baphysics_get_horizontal_velocity_percentage(void);
f32 climbGetRadius(void);
f32 func_8029B3B0(f32 arg0);
f32 func_8029B56C(f32 arg0, f32 arg1, f32 arg2, f32 arg3);
f32 func_8029B9C0(void);
f32 func_8029B9FC(void);
f32 func_8029BA44(void);
f32 func_8029CED0(void);
f32 func_802BB34C(s32 arg0);
f32 func_802BB938(f32 arg0[3], f32 arg1[3]);
f32 func_802BBD48(void);
f32 func_802BBEA4(f32 arg0[3], f32 arg1[3], f32 arg2, s32 arg3, s32 arg4);
f32 func_802A2858(void);
f32 func_802A88B0(void);
f32 func_802A8900(void);
f32 func_802A8934(void);
f32 func_802A8984(void);
f32 func_802A716C(void);
f32 func_802B051C(s32 arg0, f32 arg1, f32 arg2, f32 arg3);
f32 __bscroc_getMaxVelocity(void);
f32 func_802D7038(Actor *self);
f32 player_getYaw(void);

// ============================================================
// POINTER-RETURNING FUNCTIONS
// ============================================================

// These return pointers but are commonly listed with utility functions.
// Only functions that return pointers or have no conflicting local externs.
NodeProp *cube_findNodePropByActorId(Cube *cube, enum actor_e actor_id);
AnimTexture *model_getAnimTextureList(BKModelBin *);

// --- core2/nc/cameranodelist.c ---
CameraNodeType1 *ncCameraNodeList_getCameraNodeType1(int camera_node_index);
CameraNodeType2 *ncCameraNodeList_getCameraNodeType2(int camera_node_index);
CameraNodeType3 *ncCameraNodeList_getCameraNodeType3(int camera_node_index);
CameraNodeType4 *ncCameraNodeList_getCameraNodeType4(int camera_node_index);

// ============================================================
// [port] MISC POINTER-RETURNING + OS + GBI PROTOTYPES
// ============================================================

s16 *func_8030C704(void);

// ============================================================
// COMMONLY MISSING PROTOTYPES
// ============================================================

// --- core2/cutscene/flag.c ---
enum map_e map_get(void);
s32 exit_get(void);
s32 func_80334904(void);

// --- core2/fx/score_jiggy.c ---
u32 jiggyscore_isCollected(enum jiggy_e jiggy_id);
int jiggyscore_isSpawned(enum jiggy_e jiggy_id);

// --- core2/fx/score_honeycomb.c ---
bool honeycombscore_get(enum honeycomb_e indx);

// --- core2/map/specificflags.c ---
s32 mapSpecificFlags_get(s32 i);

// --- core2/level/specificflags.c ---
s32 levelSpecificFlags_get(s32 i);

// --- core2/terrain_material.c (item system) ---
s32 item_empty(enum item_e item);
void item_set(s32 item, s32 val);

// --- core2/game_complete.c ---
bool fileProgressFlag_get(enum file_progress_e index);
s32 fileProgressFlag_getN(enum file_progress_e offset, s32 numBits);
s32 volatileFlag_get(enum volatile_flags_e index);

// --- core2/dialog/code_CF3E0.c ---
void volatileFlag_setAndTriggerDialog_0(enum volatile_flags_e arg0);

// --- core1/init.c ---
s32 globalTimer_getTime(void);

// --- core2/actor_array.c ---
bool func_80329530(Actor *self, s32 dist);
s32 func_803297C8(Actor *arg0, f32 arg1[3]);
s32 func_80329054(Actor *arg0, s32 arg1); // decomp has s32 arg0 but callers pass Actor*
void subaddie_set_ideal_yaw(Actor *self, int arg1);
bool subaddie_playerIsWithinSphere(Actor *self, s32 dist);
Actor *actor_spawnWithYaw_s32(enum actor_e id, s32 (*pos)[3], s32 rot);

// --- core2/actor_cubebounds.c ---
bool nodeProp_findPositionFromActorId(enum actor_e actor_id, f32 *arg1);

// --- core2/ba/ba_lookdir.c ---
enum bsgroup_e player_movementGroup(void);
enum hitbox_e player_getActiveHitbox(ActorMarker *marker);
bool player_isDead(void);
bool func_8028F20C(void);
s32 func_8028F66C(enum bs_interrupt_e arg0);
void func_8028F8F8(s32 arg0, bool arg1);
void func_8028F918(s32 arg0);
void func_8028FA14(enum map_e map_id, s32 exit_id);

// --- core2/frame/bufferreadback.c ---
s32 getGameMode(void);

// --- core2/sfx/source.c ---
void func_8030DD90(u8 indx, s32 arg1);
int func_8030E3FC(u8 indx);

// --- core2/camera/camera_motor1.c ---
void func_802BAFE4(s32 arg0);
bool func_802BB270(void);

// --- core2/gc/dialog.c ---
void func_803114D0(void);
int func_803114B0(void);

// --- core2/cutscene/lair.c ---
void func_8031CD20(NodeProp *arg0, s32 arg1, s32 arg2);

// --- core2/collision/polydetect.c ---
void func_80340690(Struct83s *self);

// --- core2/actor_cubebounds.c ---
// [port] bool-return functions MUST have prototypes: without one, caller assumes int (4 bytes)
// but bool is 1 byte on MSVC x64 — upper 3 bytes of garbage make false look like true
bool func_803077FC(f32 arg0[3], s32 *arg1, s32 *arg2, s32 arg3, u32 arg4);
bool func_80305C30(s32 arg0);
bool func_80308F54(s32 cube_index);
bool nodeprop_findPositionFromActorId(enum actor_e actor_id, s32 *position);
s32 func_80306D40(s32 arg0);
s32 func_80306EF4(s32 arg0[3], s32 arg1, s32 arg2);
s32 func_80307504(f32 arg0[3], s32 arg1, s32 arg2, s32 arg3, s32 arg4);

// --- core2/actor_cubepropsystem.c ---
// [port] bool-return: must prototype to avoid int ABI mismatch
bool func_8032E398(Cube *cube, bool (*arg1)(NodeProp *), bool (*arg2)(Prop *));
bool func_80330534(Actor *actor);
bool func_8033056C(Actor *actor);
bool func_80331158(ActorMarker *arg0, f32 *arg1, f32 *arg2);

// --- core2/ba/ba_lookdir.c ---
bool func_8028F280(void); // [port] bool-return: called in cube collision handler without prototype

// --- core2/ba/ba_anim.c ---
bool baanim_isStopped(void);
bool baanim_isAnimID(enum asset_e anim_id);

// --- core2/ba/ba_groundsurface.c ---
bool func_8029CFA0(void);
bool func_8029D66C(void);
bool canTakeGroundDamage(void);

// --- core2/ba/ba_animstate.c ---
bool func_8029DFE0(void);

// --- core2/ba/ba_carriedobj.c ---
bool player_setCarryObjectPose(enum actor_e actor_id, Actor **arg1);

// --- core2/camera/camera_nodemanager.c ---
bool func_802BB720(s32 arg0, f32 arg1[3], f32 arg2[3], s32 *arg3);
bool func_802BB884(f32 arg0[3], f32 *arg1);
bool func_802BC428(void);

// --- core2/camera/camera_fog.c ---
bool func_802BEF64(void);

// --- core2/camera/camera_focus.c ---
bool func_802BAC1C(void);

// --- core2/camera/dynamicCamera.c ---
bool func_802BC640(f32 arg0[3], f32 arg1[3], f32 arg2, s32 arg3);
bool func_802BCE0C(f32 arg0[3], f32 arg1[3]);

// --- core2/camera/dynamicCam12.c ---
bool __is_flying_in_FP(void);
bool func_802C189C(void);

// --- core2/camera/dynamicCam13.c ---
bool func_802C0640(void);

// --- core2/camera/dynamicCamA.c ---
bool ncDynamicCamA_func_802C1EE0(void);

// --- core2/camera/firstpersoncamera.c ---
bool __ncFirstPersonCamera_fullyZoomedIn(void);

// --- core2/collision/funcs.c ---
void func_80320B24(void *arg0, void *arg1, void *arg2); // [port] Method_Core2_999A0_* → void* (function pointers passed opaquely)
void func_80320B44(void *arg0, void *arg1, void *arg2, void *arg3); // [port] same

// --- core2/collision/climbsurface.c ---
s32 func_8029453C(void);

// --- core2/spline_bezier.c ---
void func_8034A2A8(struct5Bs *self);

// --- core2/vtxlist.c ---
void vtxList_free(BKVertexList *vtxList);

// --- core2/ba/ba_model.c / assetcache ---
void assetcache_release(void *);

// --- core2/glspline.c ---
void func_80343DEC(Actor *self);

// --- core2/model/render.c ---
s32 func_8033A0F0(s32 arg0);

// --- core2/spawn_queue.c ---
void __spawnQueue_add_0(void (*arg0)(void));
void __spawnQueue_add_2(void (*arg0)(void), uintptr_t arg1, uintptr_t arg2);

// --- core2/sprite/displaydata.c ---
void func_8033E73C(ActorMarker *arg0, s32 arg1, FuncUnk40 arg2); // [port] fixed: FuncUnk40 to match definition
int func_8033E3F0(enum common_particle_e particle_id, int arg1);

// --- core2/particle/particle.c ---
void particleEmitter_setModel(ParticleEmitter *self, enum asset_e model_id);
void particleEmitter_setSfx(ParticleEmitter *self, enum sfx_e sfx_id, s32 arg2);
void particleEmitter_setAlpha(ParticleEmitter *self, s32 arg1);
void particleEmitter_setVelocityAccelerationAndPositionRanges(ParticleEmitter *self, ParticleSettingsVelocityAccelerationPosition *settings);
void func_802EFC28(ParticleEmitter *self, ParticleSettingsScaleAndLifetimeDrawModeEmitCount *settings);

// --- core2/quiz/game.c ---
void func_802D68F0(s32 seconds);
void func_802D4A9C(Actor *self, s32 arg1);
void func_802D4AC0(Actor *self, s32 arg1, enum file_progress_e arg2);

// --- core2/projectile_blueegg.c ---
void func_80353580(ActorMarker *marker);

// --- core1/audio_musicplayer.c ---
void func_8025A58C(u32 arg0, u32 arg1);
void func_8025AABC(enum comusic_e track_id);
void func_8025AEA0(enum comusic_e track_id, s32 arg1);

// --- core1/stopnswop.c ---
bool sns_get_item_state(s32 item, s32 set);

// --- FP/ma/slalom.c ---
bool maSlalom_isActive(void);

// --- FP/ch/boggy2.c ---
bool func_8038A1A0(ActorMarker *marker);

// func_80389F5C omitted: decomp defines as (void) but callers pass Actor*

// --- CC/model_renderstate.c ---
void CC_func_80387D4C(void);

// OS function prototypes
extern uintptr_t osVirtualToPhysical(void *addr);
extern void *osPhysicalToVirtual(uintptr_t addr);

// GBI function prototypes (implementations in src/port/stub.c)
void gSPSegment(void* value, int segNum, uintptr_t target);
void gSPSegmentLoadRes(void* value, int segNum, uintptr_t target);
void gSPDisplayList(Gfx* pkt, Gfx* dl);
void gSPDisplayListOffset(Gfx* pkt, Gfx* dl, int offset);
void gSPVertex(Gfx* pkt, uintptr_t v, int n, int v0);
void gSPInvalidateTexCache(Gfx* pkt, uintptr_t texAddr);
int ResourceMgr_OTRSigCheck(char* imgData);

// --- src/BGS/actor_spawninit.c ---
void BGS_func_8038F1E0(void);

// --- src/BGS/ch/croctus.c ---
void BGS_func_803885DC(void);

// --- src/BGS/ch/frogminigame.c ---
void BGS_func_8038CED0(void);
void func_8038CE88(void);
void func_8038CEA0(void);
void func_8038CEB8(void);

// --- src/BGS/ch/mrvile.c ---
bool BGS_func_8038C338(ActorMarker *marker);
bool func_8038C2A8(ActorMarker *marker);
void BGS_func_8038C434(ActorMarker *marker);
void BGS_func_8038C460(ActorMarker *arg0);
void func_8038C384(ActorMarker *marker);
void func_8038C3B0(ActorMarker *marker);
void func_8038C3DC(ActorMarker *marker);
void func_8038C408(ActorMarker *marker);

// --- src/BGS/ch/mudhut.c ---
void func_8038EA90(void);

// --- src/BGS/ch/tanktup.c ---
// [port] func_8038F570 removed: defined with (s16*) but cutscene/lair.c calls with no args (decomp pattern)
// [port] func_8028F94C removed: polymorphic calls (chfinalboss.c passes 3 args, definition takes 2)
void func_8038F51C(Actor *self);

// --- src/BGS/ch/tiptup.c ---
s32 func_80388E70(ActorMarker *self);
void chTiptup_choirHitReaction(ActorMarker *self, s32 arg1);
void func_80388FFC(ActorMarker *self, s32 *arg1, f32* arg2);

// --- src/BGS/ch/yumblie.c ---
bool chyumblie_is_edible(ActorMarker * arg0);
bool func_8038B684(ActorMarker * arg0);

// --- src/BGS/minigame_vile.c ---
bool chvilegame_cpu_consume_piece(ActorMarker *marker, f32 position[3]);
s32 chvilegame_get_piece_count(ActorMarker *marker);
s32 chvilegame_get_score_difference(ActorMarker *marker);
s32 func_8038A9E0(ActorMarker *marker);
void chvilegame_new_piece(ActorMarker *game_marker, ActorMarker *piece_marker, f32 position[3], u32 yumblie_type); // [port] enum chvilegame_piece_type_e is file-local; use u32 to avoid incomplete-type conflict
void chvilegame_remove_piece(ActorMarker *game_marker, ActorMarker *piece_marker);

// --- src/CC/actor_spawninit.c ---
void CC_func_80387DA0(void);

// --- src/CC/ch/token_teeth.c ---
void CC_func_803870E0(void);
void func_803870EC(s32 arg0);

// --- src/CC/challenge_ctrl.c ---
void func_803880D4(void);
void func_80388104(void);
void func_8038817C(void);

// --- src/CC/platform_physics.c ---
int CC_func_80388CA0(void);
void CC_func_8038868C(void);
void CC_func_80388760(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void CC_func_80388F4C(void);
void func_80388B4C(f32 arg0[3]);
void func_80388B78(f32 arg0[3], f32 arg1[3]);
void func_80388BBC(f32 arg0[3], f32 arg1[3]);
void func_80388CB4(void);
void func_80388D54(void);
void func_80388ED4(s32 arg0);
void func_803894A0(void);

// --- src/CCW/actor_spawninit.c ---
void CCW_func_8038DB6C(void);

// --- src/CCW/ch/nabnut.c ---
void func_8038BC50(f32 dst[3]);

// --- src/CCW/ch/seasonal_boss.c ---
void func_80389BD8(f32 dst[3]);

// --- src/CCW/ch/seasonal_npc.c ---
bool func_80388438();
void func_803883F4();

// --- src/CCW/minigame_zubba.c ---
void func_803867C8(ActorMarker *marker);
void func_80386814(ActorMarker *marker);
void func_80386840(ActorMarker *marker, s32 *score, s32 *total);

// --- src/FP/actor_spawninit.c ---
void FP_func_80391324(void);

// --- src/FP/ch/boggy2.c ---
void func_8038A09C(f32 arg0[3]);

// --- src/FP/ch/twinkly.c ---
void func_8038C398(f32 position[3], enum marker_e marker_id);

// --- src/FP/ch/twinklybox.c ---
// [port] func_8038DD14 removed: defined as (void) but fp_trigger.c passes Actor* arg (decomp pattern)
bool func_8038DD34(ActorMarker *marker);

// --- src/FP/ma/slalom.c ---
bool maSlolam_WithinRadiusOfBoggy(f32 position[3], s32 radius);
void maSlalom_end();
void maSlalom_init(void);
void maSlalom_linkActiveFlag(ActorMarker *marker);
void maSlalom_linkBoggy(ActorMarker *marker);
void maSlalom_linkDummyFlag(ActorMarker *marker);
void maSlalom_setBoggyGate(s32 gate_num);
void maSlalom_setPlayerGate(s32 gate_num);
void maSlalom_start(void);
void maSlalom_unlinkBoggy(void);
void maSlalom_update(void);

// --- src/FP/ma/snowbutton.c ---
void maSnowButton_decRemaining(void);
void maSnowButton_end(void);
void maSnowButton_init(void);
void maSnowButton_update(void);

// --- src/FP/ma/snowy.c ---
void maSnowy_decRemaining(void);
void maSnowy_end(void);
void maSnowy_incTotal(void);
void maSnowy_init(void);
void maSnowy_update(void);

// --- src/FP/wozza_fire.c ---
void func_803918C0(void);
void func_8039195C(void);
void func_80391994(void);

// --- src/GV/actor_spawninit.c ---
s32 func_8038F4C0(Actor *arg0, s32 arg1);
void GV_func_8038F154(void);

// --- src/GV/ch/ancientone.c ---
void GV_func_80387118(void);

// --- src/GV/ch/buriedpyramid.c ---
void chBuriedPyramid_setRaisedAmount(ActorMarker *this_marker, s32 arg1);

// --- src/GV/ch/gobi1.c ---
s32 func_80387354(void);
s32 func_80387360(void);

// --- src/GV/ch/gobirock.c ---
bool chGobiRock_isDestroyed(void);

// --- src/GV/ch/grabba.c ---
s32 GV_func_8038C5BC(void);

// --- src/GV/ch/gv_enemy.c ---
void func_8038C748(void);

// --- src/GV/ch/jinxy_head.c ---
int func_8038E344(ActorMarker *this_marker);
void func_8038E2FC(ActorMarker *this_marker);

// --- src/GV/ch/toots.c ---
void func_803865E8(void);
void func_803865F8(void);
void func_80386608(void);

// --- src/GV/crc.c ---
// [port] func_80389F5C removed: defined as (void) but GV/actor_spawninit.c passes Actor* arg (decomp pattern)

// --- src/GV/gv_helpers.c ---
s32 func_8038E178(void);
s32 func_8038E184(void);
void func_8038E140(void);
void func_8038E18C(void);

// --- src/GV/matchinggame.c ---
void gv_matchingGame_init(void);
void gv_matchingGame_reset(void);
void gv_matchingGame_update(void);

// --- src/GV/pyramid_trigger.c ---
int func_8038D388(void);

// --- src/GV/water_pyramidactivate.c ---
void func_803900F8(void);
void func_80390100(void);
void func_80390138(void);

// --- src/GV/water_pyramidrot.c ---
void func_8038FF60(void);
void func_8038FF68(void);
void func_8038FFF4(void);

// --- src/GV/waterctrl.c ---
void gv_waterCtrl_end(void);
void gv_waterCtrl_init(void);
void gv_waterCtrl_update(void);

// --- src/MM/actor_spawninit.c ---
void MM_func_803888B0(void);

// --- src/MM/ch/hut.c ---
void mm_resetHuts(void);

// --- src/MM/ch/juju.c ---
bool __chjuju_isEveryJujuDespawned(ActorMarker **ptr);
bool __chjuju_isEveryJujuStable(ActorMarker **ptr);
void __chjuju_initialize_all(ActorMarker *marker, s32 count);
void __chjuju_updateCount(ActorMarker **ptr);
void func_803892A8(ActorMarker **ptr);

// --- src/MM/ch/jujuhitbox.c ---
void chjujuhitbox_setJuju(Actor *self, s32 slave_id, Actor *slavePtr);

// --- src/MMM/actor_spawninit.c ---
void MMM_func_803890E0(void);

// --- src/MMM/ch/color_puzzle.c ---
void func_8038B5D8(Struct5Fs *arg0, Struct68s *arg1, s32 arg2, s32 arg3);

// --- src/MMM/ch/flowerpot.c ---
bool chFlowerpot_eggCollision(ActorMarker *marker);
void chFlowerpot_reset();

// --- src/MMM/ch/motzhand.c ---
bool func_8038769C(ActorMarker *marker);
void func_80387654(ActorMarker *marker);
void func_803876C8(ActorMarker *marker, s32 arg1);
void func_80387720(ActorMarker *marker);

// --- src/MMM/ch/tumblar.c ---
bool chTumblar_isBanjoAbove(void *arg0, Struct68s *arg1); // [port] arg0 is Struct_MMM_47D0_0* (local to tumblar.c)
bool chTumblar_isDisappeared(void *arg0, s32 arg1); // [port] arg0 is Struct_MMM_47D0_0* (local to tumblar.c)
void chTumblar_congratulate(void *arg0, s32 arg1); // [port] arg0 is Struct_MMM_47D0_0* (local to tumblar.c)
void chTumblar_copyPosition(s32 arg0, Struct68s *arg1, f32 arg2[3]);
void func_8038AC04(void);

// --- src/MMM/minigame_organ.c ---
int func_80389CE8(s32 arg0, s32 arg1, s32 arg2);
s32 func_80389BBC(void);
void MMM_func_80389CD8();
void code3420_handleOrganGame(s32 arg0, s32 arg1);
void func_80389CE0();
void func_80389D9C(s32 key_id);
void maOrgan_update(void);
void organMinigame_getKeyPosition(s32 key_indx, f32 position[3]);

// --- src/MMM/minigame_organinput.c ---
void func_8038A994();
void func_8038A9B4(void);
void func_8038AA30(void *arg0, void *arg1); // [port] polymorphic: called with BKModel*/Struct_MMM_47D0_0* and s32/Struct68s*
void func_8038AA44(void);

// --- src/MMM/napper_room.c ---
s32 func_80389510();
u8 MMM_func_80389530();
u8 func_80389524();
void func_8038953C();
void func_80389544(void);
void func_803895B0(s32 arg0);
void func_8038966C(void);

// --- src/RBB/actor_spawninit.c ---
void RBB_func_80386C48(void);

// --- src/RBB/ch/eggtoll1.c ---
void func_8038685C(ActorMarker *marker);

// --- src/RBB/ch/whistlectrl.c ---
s32 chWhistleCtrl_newEvent(Actor *self, s32 whistle_id, Actor *other);

// --- src/RBB/life_balloontrigger.c ---
void func_8038FB54(void);
void func_8038FB6C(void);

// --- src/RBB/propellorctrl.c ---
void rbb_propellorCtrl_reset(void);
void rbb_propellorCtrl_start(void);
void rbb_propellorCtrl_update(void);

// --- src/SM/ch/quarrie_honeycomb.c ---
bool codeBF0_shouldSpawnQuarrieHoneyComb(ActorMarker *marker);

// --- src/SM/crc.c ---
void SM_resetSpawnableActors();
void codeF0_func_80386540();

// --- src/SM/model_visibility.c ---
void func_80388D48(void);

// --- src/SM/version_compat.c ---
// [port] func_8038AAB0 removed: defined as (void) but gameSelect.c passes args (decomp version mismatch)

// --- src/TTC/actor_spawninit.c ---
void code26D0_resetSpawnableActorsForTTC(void);

// --- src/TTC/ch/leaky.c ---
bool chLeaky_eggCollision(ActorMarker *marker);

// --- src/TTC/ch/nipper.c ---
// [port] chNipper_isInState7 removed: defined with (s16[3]) but cutscene/lair.c calls with no args (decomp pattern)

// --- src/TTC/ch/treasurehunt.c ---
void chTreasurehunt_resetProgress(void);

// --- src/TTC/crc.c ---
void code3040_func_80389468(void);

// --- src/TTC/ma/castle.c ---
bool maCastle_hasBanjoKazooieCodeBeenEntered(void);
bool maCastle_isSecretCheatCodeRelatedValueEqualToScrambledAddressValue();
void maCastle_init(void);
void maCastle_release(void);
void maCastle_update(void);

// --- src/core1/audio_instruments.c ---
s32 func_80250034(s32 track_id);
s32 func_802501A0(u8 arg0, s32 arg1, s32 *arg2);
void func_8024F764(s32 arg0);
void func_8024F7C4(s32 arg0);
void func_8024F83C(void);
void func_8024FB8C(void);
s32 func_8024FEEC(u8 arg0); // [port] MIPS implicit return — wraps alCSeqGetTicks (returns s32)
void func_8024FF34(void);
void func_80250170(u8 arg0, s32 arg1, s32 arg2);
void func_80250650(void);
void musicInstruments_init(void);

// --- src/core1/audio_musicplayer.c ---
int func_8025AEEC(void);
s32 comusic_active_track_count(void);
s32 func_8025ADD4(enum comusic_e id);
void comusicPlayer_free(void);
void comusicPlayer_init(void);
void comusic_defrag(void);
void func_8025A23C(s32 arg0);
void func_8025A2B0(void);
void func_8025A2D8(void);
void func_8025A2FC(s32 arg0, s32 arg1);
void func_8025A388(s32 arg0, s32 arg1);
void func_8025A430(s32 arg0, s32 arg1, s32 arg2);
void func_8025A4C4(s32 arg0, s32 arg1, s32 *arg2);
void func_8025A6CC(enum comusic_e track_id, s32 volume);
void func_8025A7DC(enum comusic_e);
void func_8025A8B8(enum comusic_e track_id, s32 arg1);
void func_8025A904(void);
void func_8025A9D4(void);
void func_8025AB00(void);

// --- src/core1/audio_soundplayer.c ---
bool func_802445C4(void *bank, s16 arg1); // [port] ALBank* -> void* (N64 audio type not in port headers)
s32 func_802445AC(void *arg0); // [port] N_AL_Struct81s* -> void* (N64 audio type not in port headers)
void * func_80244608(void *bank, s16 arg1, struct46s *arg2); // [port] ALBank* -> void* (N64 audio type not in port headers)
void func_80244814(void *arg0); // [port] N_AL_Struct81s* -> void* (N64 audio type not in port headers)
void func_80244978(intptr_t arg0, s16 type, s32 arg2);
void func_80244A98(s32 arg0);

// --- src/core1/ba_motor.c ---
void baMotor_80250C08(void);
void baMotor_80250FC0(void);
void baMotor_init(void);

// --- src/core1/collision.c ---
// [port] removed: func_80245524 — polymorphic callers pass intptr_t*, s32* for arg2
int collisionTri_isHitFromAbove_actor(f32 arg0[3], Actor *arg1, s32 arg2);
int collisionTri_isHitFromAbove_marker(f32 position[3], ActorMarker *marker, s32 verticalOffset);
s32 func_8024559C(f32 arg0[3], intptr_t *arg1, f32 *arg2);
void collisionTri_copy(BKCollisionTri *dst, BKCollisionTri *src);
void func_802450DC(f32 arg0[3], f32 arg1[3], f32 arg2[3], f32 arg3[3], f32 arg4[3]);
void func_802451A4(f32 arg0[3], f32 arg1[3], f32 arg2[3], f32 arg3[3], f32 arg4[3], s32 arg5);

// --- src/core1/debugtext.c ---
s32 func_802485BC(void);
void func_80247F24(s32 arg0, s32 arg1);
void func_80247F9C(s32 arg0);
void func_802483D8(void);

// --- src/core1/defragmanager.c ---
void defragManager_free(void);
void defragManager_init(void);

// --- src/core1/graphics_thread.c ---
void func_80246670(OSMesg arg0);
void func_80247380(void);
void func_80247560(void);
void func_802476DC(void);
void func_802476EC(Gfx **gfx);

// --- src/core1/gu/gu_mtx.c ---
void _guMtxF2L(float mf[4][4], Mtx *m);

// --- src/core1/inflate.c ---
int bk_inflate(void);

// --- src/core1/initthread.c ---
void initThread_create(void);

// --- src/core1/memory.c ---
bool func_802555D0(void);
bool func_802559A0(void);
int func_80254BC4(int arg0);
int func_80255B08(int arg0);
s32 heap_get_size(void);
u32 heap_get_occupied_size(void);
void * func_80254BD0(s32 *size, u32 arg1);
void func_80255170(void **arg0);
void func_80255198(void);
void func_80255524(void);
void func_802555C4(void);
void func_80255A04(void);
void func_80255A14(void);
void func_80255ACC(void);
// [port] deleted memcpy, memmove, and wmemcpy prototypes — conflicts with stdlib/wchar.h

// --- src/core1/mlmtx.c ---
void func_802515D4(f32 arg0[3][3]);

// --- src/core1/overlaymanager.c ---
bool overlayManagerload(enum overlay_e overlay_id);
int overlayManagergetLoadedId(void);
void overlayManagerloadCore2(void);

// --- src/core1/sns.c ---
void snspayload_finalise_outgoing_payload(struct SnsPayload *payload);
void snspayload_rewind_outgoing(void);

// --- src/core1/stopnswop.c ---
bool DEBUG_use_special_bootmap(void);
void sns_backup_items_and_unlock_all(void);
void sns_find_and_parse_payload(void);
void sns_init_base_payloads(void);
void sns_restore_backed_up_items(void);
void sns_save_and_update_global_data(void);
void sns_set_item_and_update_payload(enum StopNSwop_Item item, s32 set, s32 state);
void sns_write_payload_over_heap(void);

// --- src/core1/stub_1D590.c ---
void dummy_func_8025AFB0(void);
void dummy_func_8025AFB8(void);
void dummy_func_8025AFC0(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- src/core2/abilityprogress.c ---
int ability_hasLearned(s32);
int ability_hasUsed(enum ability_e move);
s32 ability_getAllLearned(void);
void ability_clearAll(void);
void ability_debug(void);
void ability_getSizeAndPtr(s32 *size, u8 **addr);
void ability_setAllLearned(s32 val);
void ability_setAllUsed(s32 val);
void ability_setHasUsed(enum ability_e move);
void ability_setLearned(s32 move, s32 val);
void ability_use(s32 arg0);

// --- src/core2/actor_array.c ---
Struct64s* func_8032994C(void);
bool func_80329140(Actor *self, s32 arg1, s32 arg2);
bool func_80329260(Actor *self, f32 p1[3]);
bool func_803292E0(Actor *self);
bool func_8032944C(Actor *self);
bool func_803294B4(Actor *self, s32 arg1);
bool func_803294F0(Actor *self, s32 arg1, s32 arg2);
bool func_803296D8(Actor *self, s32 dist);
bool func_8032A9E4(s32 arg0, s32 arg1, s32 arg2);
bool func_8032BBE8(Actor *self);
bool subaddie_playerIsWithinAsymmetricCylinder(Actor *self, s32 radius, s32 d_upper, s32 d_lower);
bool subaddie_playerIsWithinCylinder(Actor *self, s32 radius, s32 d_y);
int func_80329210(Actor * arg0, f32 (* arg1)[3]); // [port] fixed truncated function pointer param
s32 actorArray_actorCount(enum actor_e actor_id);
s32 func_80326218(void);
s32 func_8032627C(Actor *self);
s32 func_8032970C(Actor *self);
void actorArray_defrag(void);
void actorArray_free(void);
void actor_setOpacity(Actor *self, s32 alpha);
void func_803255FC(Actor *self);
void func_80325F8C(void);
void func_803262B8(Actor *self);
void func_803262E4(Actor *self);
void func_80326894(Actor *self);
void func_803268B4(void);
void func_80326C24(s32 arg0);
void func_803283BC(void);
void func_803283D4(void);
void func_80328CA8(Actor *self, s32 angle);
void func_803297FC(Actor *arg0, f32 *o1, f32 *o2);
s32 func_80329904(ActorMarker *arg0, s32 arg1, f32 *arg2); // [port] was void — definition returns s32
void func_8032A09C(s32 arg0, ActorListSaveState *arg1);
void func_8032A5F8(void);
void func_8032A82C(Actor *arg0, s32 arg1);
void func_8032A95C(Actor *arg0, s32 arg1, s32 arg2);
void func_8032AA9C(void);
void func_8032AABC(void);
void func_8032AB84(Actor *arg0);
void func_8032ACA8(Actor *arg0);
void func_8032AD7C(s32 arg0);
void func_8032AEB4(void);
void func_8032B258(Actor *self, enum collision_e arg1);
void func_8032B4DC(Actor *self, ActorMarker *arg1, s32 arg2);
void func_8032BB88(Actor *self, s32 arg1, s32 arg2);
void func_8032BC18(Actor *self);
void func_8032BC60(Actor *self, s32 arg1, f32 arg2[3]);
void subaddie_set_state_looped(Actor * self, u32 arg1);

// --- src/core2/actor_cubebounds.c ---
bool func_80305248(f32 arg0[3], s32 arg1, f32 *arg2);
bool func_8030526C(f32 arg0[3], s32 arg1, f32 *arg2);
bool func_80305290(bool (* arg0)(NodeProp *), bool (* arg1)(Prop *)); // [port] fixed truncated function pointer params
bool func_80305344(s32 arg0, u32 *arg1);
bool func_80305D14(void);
bool func_80307390(s32 arg0, s32 arg1);
enum actor_e func_803084F0(s32 arg0);
s32 func_803048E0(s32 arg0[3], void *arg1, void *arg2, s32 arg3, s32 arg4);
s32 func_80304FC4(enum actor_e *actor_id_list, NodeProp **node_list, s32 arg2);
s32 func_8030508C(s32 arg0, f32 arg1[3], s32 arg2);
s32 func_80306DBC(s32 arg0);
s32 func_80306DDC(s32 *position);
s32 func_80307164(s32 arg0[3]);
s32 func_80307258(f32 arg0[3], s32 arg1, s32 arg2);
s32 func_803083B0(s32 arg0);
s32 nodeprop_getRadius(NodeProp *arg0);
s32 nodeprop_getScale(NodeProp *nodeProp);
u32 nodeprop_getYaw(NodeProp *nodeProp);
void code7AF80_freeTotalCounts(void);
void cubeList_defrag();
void cubeList_free();
void cubeList_fromFile(File *file_ptr);
void cubeList_init();
void func_80302C94(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_80303F6C(s32 indx, s32 arg1);
void func_803045CC(s32 arg0, s32 arg1);
void func_803045D8();
void func_8030578C(void);
void func_80305990(s32 mode);
void func_80305D38(void);
void func_80305D94(void);
void func_803062D0(void);
void func_803065E4(s32 arg0, s32 position[3], s32 radius, s32 arg3, s32 arg4);
void func_8030688C(s32 arg0, s32 position[3], s32 radius, s32 arg3);
void func_80306AA8(s32 arg0, s32 position[3], s32 radius);
void func_80307CA0(ActorMarker *marker);
void func_80308230(s32 arg0);
void nodeprop_getPosition_s32(NodeProp *nodeProp, s32 dst[3]);
void spawnableActorList_free(void);
void spawnableActorList_new(void);

// --- src/core2/actor_cubepropsystem.c ---
s32 actor_getAnimatedTexturesCacheId(Actor *actor);
s32 codeA5BC0_getNodePropBit6(NodeProp *arg0);
s32 codeA5BC0_getNodePropUnk8(NodeProp *arg0);
s32 codeA5BC0_getNodePropUnkA(NodeProp *arg0);
s32 codeA5BC0_getNodePropUnkC(NodeProp *arg0);
// [port] removed: codeA5BC0_getPositionAndReturnRadius — polymorphic callers pass NodeProp*, struct_core2_DB010*
s32 func_8032E49C(Cube *cube, enum actor_e *actor_id_list, NodeProp **node_list, s32 node_list_capacity);
s32 func_8032E5A8(Cube *cube, s32 arg1, f32 (*arg2)[3], s32 capacity); // [port] fixed truncated function pointer param
s32 func_8032F170(Cube **arg0, void **arg1);
void code7AF80_initCubeFromFile(File *file_ptr, Cube *cube);
void codeA5BC0_getActorPosition(ActorProp *prop, s32 dst[3]);
void codeA5BC0_setNodePropUnkC(NodeProp *arg0, s32 arg1);
void code_A5BC0_initCubePropActorProp(Cube*);
void cube_free(Cube *cube);
void func_8032D120(Cube *cube);
void func_8032D158(Cube *cube);
void func_8032D3A8(void);
void func_8032D3D8(Gfx **gdl, Mtx **mptr, Vtx **vptr);
void func_8032D474(Gfx **gdl, Mtx **mptr, Vtx **vptr);
void func_8032E070(void);
void func_8032EE2C(s32 arg0[3], s32 arg1, s32 arg2);
void func_8032EE80(Cube *cube);
void func_8032F464(bool arg0);
void func_8032F64C(f32 *pos, ActorMarker * marker);
void func_8032F6A4(s32 *pos, ActorMarker * marker, s32 *rot);
void func_8032FFD4(ActorMarker *self, s32 arg1);
void func_80330078(ActorMarker *marker, ActorMarker *other_marker, s16 *arg2);
void func_803300B8(ActorMarker *marker, MarkerCollisionFunc method);
void func_803300C0(ActorMarker *marker, s32 (*method)(ActorMarker *, ActorMarker *)); // [port] fixed truncated function pointer param
void func_80330208(Cube *cube);
void func_803303B8(Cube *cube);
void func_803305AC(void);
void func_803306C8(s32 arg0);
void func_803308A0(void);
void func_80330FCC(ActorMarker *marker, s32 arg1[3]);
void func_80330FF4(void);
void func_80332790(s32 arg0);
void func_80332894(void);
void func_8033297C(void);
void func_803329AC(void);
void func_80332A38(void);
void marker_free(ActorMarker *self);
void marker_setModelId(ActorMarker *self, enum asset_e modelIndex);

// --- src/core2/actor_motioncurve.c ---
s32 func_80296560(void);
void func_802964B8(void);
void func_8029656C(f32 dst[3]);
void func_80296C30(void);
void func_80296CA8(ActorMarker *arg0);
void func_80296CB4(s32 arg0);
void func_80296CC0(f32 arg0[3]);

// --- src/core2/actor_pool48.c ---
bool func_802FCD98(struct8s *arg0);

// --- src/core2/ambient_fish.c ---
void func_803500D8(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_803500E8(void);
void func_80350174(void);
void func_80350250(void);

// --- src/core2/anim/anim_bonetransform.c ---
bool func_8033B338(void **sprite_ptr, BKSpriteDisplayData **arg1);
bool func_8033B388(BKSprite **sprite_ptr, BKSpriteDisplayData **arg1);
s32 code_B3A80_func_8033BDAC(enum asset_e id, void *dst, s32 size);
s32 func_8033B678(void);
void assetCache_free(void *arg0);
void assetCache_init(void);
void assetcache_update_ptr(void * arg0, void* arg1);
void func_8033B5FC(void);
void func_8033B61C(void);
void func_8033B788(void);
// [port] removed: func_8033BD20 — polymorphic callers pass BKModelBin**, BKSprite**, void**
void func_8033BD6C(void);
bool func_8033BD8C(void* arg0); // [port] MIPS implicit return — wraps func_8033B0D0 (returns bool)

// --- src/core2/anim/anim_buffer.c ---
void anim_802897D4(AnimMtxList **this_ptr, BKAnimationList *arg0, Animation *dst); // [port] takes AnimMtxList** — caller passes &marker->unk20
void anim_drawSetup(Animation *self);
void anim_release(Animation *self);
void anim_resetNow(Animation *self);
void anim_resetSmooth(Animation *self);
void anim_setIndex(Animation *self, enum asset_e arg1);
void anim_update(Animation *self);

// --- src/core2/anim/anim_cache.c ---
bool animCache_inUse(s16 index);
// [port] removed: animCache_getBoneTransformList — polymorphic callers pass BoneTransformList**, s32**
s16 animCache_getNew(void);
void animCache_defrag(void);
void animCache_flushAll(void);
void animCache_free(void);
void animCache_init(void);
void animCache_release(s16 index);
void animCache_update(void);

// --- src/core2/anim/anim_sequencehandler.c ---
void func_80361DC4(Actor *self);
void func_80361E10(Actor *self);
void func_80361E9C(Actor *self);
void func_80361EE0(Actor *self);

// --- src/core2/anim/anim_texturecache.c ---
bool AnimTextureListCache_tryGetTextureOffset(s32 list_index, s32 texture_index, s32 *current_frame);
s32 AnimTextureListCache_newList(void);
void AnimTextureListCache_at(s32 arg0, AnimTexture arg1[4]);
void AnimTextureListCache_free(void);
void AnimTextureListCache_freeList(s32 arg0);
void AnimTextureListCache_init(void);
void AnimTextureListCache_update(void);

// --- src/core2/anim_bonetransformlist.c ---
void boneTransformList_getBoneScale(BoneTransformList *self, s32 bone_id, f32 scale[3]);
// [port] removed: boneTransformList_reset — polymorphic callers pass BoneTransformList*, s32*
void boneTransformList_setBoneScale(BoneTransformList *self, s32 bone_id, f32 scale[3]);
void func_8033A57C(BoneTransformList *self, s32 bone_id, f32 arg2[4]);
void func_8033A6B0(BoneTransformList *self, s32 bone_id, f32 arg2[3]);
void func_8033A8F0(BoneTransformList *self, s32 bone_id, f32 arg2[4]);
void func_8033A968(BoneTransformList *self, s32 bone_id, f32 arg2[3]);

// --- src/core2/anseq.c ---
void anSeq_free(void **ppAnSeq); // [port] was bk_vector(AnSeqElement)** — internal type, use void*
void anSeq_setActivationFrameDelay(void **ppAnSeq, s32 arg1); // [port] was bk_vector(AnSeqElement)** — internal type, use void*
void anSeq_update(void **ppAnSeq, AnimCtrl *pAnCtl); // [port] was bk_vector(AnSeqElement)** — internal type, use void*

// --- src/core2/audio_sfxinstruments.c ---
bool func_803354EC(enum sfx_e sfx_id);
bool func_80335520(s32 arg0);
intptr_t func_8033531C(enum sfx_e uid, struct46s *arg1);
intptr_t func_80335354(int uid, struct46s *arg1);
void func_803353BC(intptr_t arg0, u16 arg1);
void func_803353F4(intptr_t arg0, s32 arg1);
void func_80335418(intptr_t arg0, s32 arg1);
// [port] removed: func_8033543C — polymorphic callers pass Struct81s*, N_AL_Struct81s*
void sfxInstruments_init(void);

// --- src/core2/ba/ba_anim.c ---
void baAnim_defrag(void);
void baAnim_free(void);
void baAnim_init(void);
void baAnim_update(void);
void baanim_80289F30(void);
void baanim_setModifyMethod(void (*arg0)(uintptr_t, uintptr_t)); // [port] fixed truncated function pointer param
void baanim_setUpdateType(enum baanim_update_type_e arg0);

// --- src/core2/ba/ba_animcache.c ---
void animBinCache_free(void);
void animBinCache_init(void);
void animBinCache_update(void);

// --- src/core2/ba/ba_animstate.c ---
void func_8029DD6C(void);
void func_8029DFF8(void);
void func_8029E058(bool);
void func_8029E064(bool);
void func_8029E070(bool);
void func_8029E0DC(bool);
void func_8029E0E8(bool);
void func_8029E0F4(bool);
void func_8029E100(void);

// --- src/core2/ba/ba_bounds.c ---
void func_80295DD0(void);
void func_80295E74(void);

// --- src/core2/ba/ba_buzz.c ---
void babuzz_release(void);
void babuzz_reset(void);

// --- src/core2/ba/ba_carriedobj.c ---
void bacarriedobj_dec(enum actor_e actor_id);
void bacarriedobj_displayOnHud(enum actor_e actor_id);
void bacarriedobj_inc(enum actor_e actor_id);
void bacarriedobj_spawn(enum actor_e actor_id);
void func_8028DEEC(enum actor_e actor_id, Actor *actor);

// --- src/core2/ba/ba_carry.c ---
void bacarry_end(void);
void bacarry_init(void);
void bacarry_reset_marker(void);
void bacarry_set_marker(ActorMarker *arg0);
void bacarry_update(void);

// --- src/core2/ba/ba_drone.c ---
enum bs_e badrone_802926E8(void);
enum bs_e badrone_enter(void);
enum bs_e badrone_look(void);
enum bs_e badrone_transform(void);
enum bs_e badrone_vanish(void);
void badrone_get_position_and_duration(f32 position[3], f32 *duration);
void badrone_goto_end(void);
void badrone_init(void);

// --- src/core2/ba/ba_eyeblink.c ---
s32 func_80297C6C(void);
void func_80297C78(void);
void func_80297CA8(void);
void func_80297CF8(void);

// --- src/core2/ba/ba_eyeblink_data.c ---
void func_80290070(void);
void func_802900B4(void);
void func_802900D8(void);
void func_802900FC(void);
void func_80290108(void);

// --- src/core2/ba/ba_eyemouth_data.c ---
void func_802C7318(Actor *actor);
void func_802C7478(Actor *actor);
void func_802C75A0(Actor *actor, s32 arg1);
void func_802C79C4(void);

// --- src/core2/ba/ba_eyes.c ---
void baeyes_close(void);
void baeyes_open(void);
void baeyes_reset(void);
void baeyes_update(void);

// --- src/core2/ba/ba_falldamage.c ---
s32 bafalldamage_get_damage(s32 *damage);
s32 bafalldamage_get_state(void);
void bafalldamage_init(void);
void bafalldamage_set_state(s32 arg0);
void bafalldamage_update(void);

// --- src/core2/ba/ba_falling.c ---
void func_80350818(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_80350BC8(void);
void func_80350BFC(void);
void func_80350CA4(void);

// --- src/core2/ba/ba_groundsurface.c ---
void func_8029CFF8(void);
void func_8029D01C(void);
void func_8029D968(void);

// --- src/core2/ba/ba_health.c ---
s32 func_802903CC(void);
void func_80290220(void);
void func_8029026C(s32 arg0, s32 arg1);
void func_8029028C(bool arg0);
void func_80290298(void);

// --- src/core2/ba/ba_hitbox.c ---
enum hitbox_e hitbox_getHitboxForActor(ActorMarker *arg0);

// --- src/core2/ba/ba_input.c ---
int bainput_isEnabled(s32 arg0);
int bainput_should_beak_barge(void);
int bainput_should_beak_bust(void);
int bainput_should_dive(void);
int bainput_should_feathery_flap(void);
int bainput_should_flap_flip(void);
int bainput_should_look_first_person_camera(void);
int bainput_should_peck(void);
int bainput_should_poop_egg(void);
int bainput_should_rotate_camera_left(void);
int bainput_should_rotate_camera_right(void);
int bainput_should_shoot_egg(void);
int bainput_should_trot(void);
int bainput_should_wonderwing(void);
int bainput_should_zoom_out_camera(void);
void bainput_enable(s32 arg0, int arg1);
void bainput_reset(void);
void bainput_update(void);

// --- src/core2/ba/ba_intensity.c ---
s32 func_80291660(void);
void func_80291590(void);
void func_802915B8(void);

// --- src/core2/ba/ba_key.c ---
int bakey_getAndSetState(s32 button_indx, s32 val);
int bakey_releaseCount(s32 button_indx);
int bakey_released(s32 button_indx);
void bakey_disableAll(s32 arg0);
void bakey_reset(void);
void bakey_update(void);

// --- src/core2/ba/ba_lookat.c ---
int func_80298800(f32 arg0[3]);
s32 func_80298850(void);
void func_8029887C(void);
void func_80298890(void);
void func_802988DC(s32 arg0);
void func_8029892C(f32 arg0[3]);
void func_80298A64(void);
void func_80298A84(void);

// --- src/core2/ba/ba_lookdir.c ---
bool func_8028EFC8(void);
bool func_8028EFEC(void);
bool func_8028F070(void);
bool func_8028F098(void);
bool func_8028F0D4(void);
bool func_8028F150(void);
bool func_8028F170(void);
bool func_8028F1E0(void);
bool func_8028F25C(void);
bool func_8028F2A0(void);
bool func_8028F2DC(void);
bool func_8028F2FC(void);
bool func_8028F428(s32 arg0, ActorMarker *marker);
bool func_8028F45C(s32 arg0, f32 arg1[3]);
bool func_8028F490(f32 arg0[3]);
bool func_8028F504(s32 arg0);
bool func_8028F530(s32 arg0);
bool func_8028F55C(s32 arg0, ActorMarker *marker);
bool func_8028F590(s32 arg0, ActorMarker *marker);
bool func_8028F5F8(f32 arg0[3]);
bool func_8028FB88(enum transformation_e xform_id);
bool func_8028FBD4(f32 arg0[3]);
bool player_is_in_jiggy_jig(void);
bool player_is_present(void);
bool player_throwCarriedObject(void);
enum actor_e carriedObj_getActorId(void);
enum bswatergroup_e player_getWaterState(void);
enum marker_e bacarry_get_markerId(void);
f32 func_8028EC64(f32 arg0[3]);
int ability_isUnlocked(enum ability_e uid);
int func_8028EC04(void);
s32 func_8028E4A4(void);
s32 func_8028F68C(enum bs_interrupt_e arg0, ActorMarker *marker);
s32 func_8028F6B8(enum bs_interrupt_e arg0, enum asset_e model_id);
s32 func_8028F6E4(enum bs_interrupt_e arg0, f32 arg1[3]);
void func_8028E4B0(void);
void func_8028E644(void);
void func_8028E6EC(s32 arg0);
void func_8028E71C(void);
void func_8028E84C(f32 arg0[3]);
void func_8028E964(f32 pos[3]);
void func_8028E9C4(s32 arg0, f32 arg1[3]);
void func_8028EF28(f32 arg0[3]);
void func_8028F010(enum actor_e actor_id);
void func_8028F030(enum actor_e actor_id);
void func_8028F050(enum actor_e actor_id);
void func_8028F784(bool arg0);
void func_8028F7C8(bool arg0);
void func_8028F7F4(s32 arg0, s32 arg1);
void func_8028F85C(f32 arg0[3]);
void func_8028FA34(s32, Actor *);
void func_8028FA74(f32 dst[3]);
void func_8028FAB0(f32 arg0[3]);
u32 func_8028FB48(u32 mask); // [port] MIPS implicit return — wraps func_80294610 (returns u32)
void func_8028FB68(void);
void func_8028FC8C(f32 arg0[3]);
void func_8028FCAC(void);
void func_8028FCBC(void);
void func_8028FCC8(bool arg0);
void func_8028FCE8(void);
void player_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void player_getPosition_s32(s32 arg0[3]);
void player_getVelocity(f32 dst[3]);
void player_setIdealRotation(f32 rotation[3]);
void player_setRotation(f32 rotation[3]);

// --- src/core2/ba/ba_marker.c ---
bool baMarker_isCollidable(void);
enum actor_e baMarker_getCarriedObjectActorId(void);
s32 baMarker_8028D688(void);
s32 baMarker_8028D694(void);
s32 baMarker_8028D6F0(s32 **arg0);
void baMarker_8028D638(s32 arg0, s32 arg1);
void baMarker_collisionOff(void);
void baMarker_collisionOn(void);
void baMarker_free(void);
void baMarker_init(void);
void baMarker_setCarriedObject(enum actor_e actor_id);
void baMarker_update(void);

// --- src/core2/ba/ba_model.c ---
enum asset_e baModel_getModelId(void);
s32 baModel_isVisible(void);
void baModel_8029223C(f32 arg0[3]);
void baModel_80292260(f32 arg0[3]);
void baModel_80292284(f32 arg0[3], s32 arg1);
void baModel_802924B8(f32 arg0[3]);
void baModel_802924E8(f32 arg0[3]);
void baModel_80292554(f32 arg0[3]);
void baModel_80292578(f32 arg0[3]);
void baModel_defrag(void);
void baModel_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void baModel_free(void);
void baModel_getPosition(f32* dst);
void baModel_reset(void);
void baModel_setEnvAlpha(s32 alpha);
void baModel_setPostDraw(void (*draw_func)(Gfx **gfx, Mtx **mtx, Vtx **vtx)); // [port] fixed truncated function pointer param
void baModel_setVisible(s32 arg0);
void baModel_update(void);
void baModel_updateModel(void);

// --- src/core2/ba/ba_modelselect.c ---
s32 func_802985F0(void);
void func_802986D0(void);
void func_80298700(void);

// --- src/core2/ba/ba_momentum.c ---
void func_8029065C(void);
void func_80290664(void);
void func_802906A4(s32 arg0);
void func_802906D8(void);

// --- src/core2/ba/ba_musicstate.c ---
s32 func_80321960(void);
void func_8032196C(void);
void func_803219A8(void);
void func_803219F4(s32 arg0);

// --- src/core2/ba/ba_position.c ---
void func_802983F0(void);
void func_8029842C(void);
void func_80298464(f32 arg0[3]);
void func_80298504(f32 arg0[3]);
void func_80298540(f32 arg0[3]);
void func_80298564(f32 arg0[3]);
void func_8029858C(void);
void player_setPosition(f32 arg0[3]);

// --- src/core2/ba/ba_recoil.c ---
enum asset_e func_80294974(void);
enum transformation_e func_80294A4C(void);
int func_802949C8(void);
void func_80294A58(enum asset_e asset_id);
void func_80294A64(f32 src[3]);
void func_80294A8C(int arg0);
void func_80294AF4(enum transformation_e xform);
void func_80294DD8(void);
void func_80294E54(int arg0);
void func_80294E60(void);
void get_talk_target_position(f32 dst[3]);
void get_throw_target_position(f32 dst[3]);
void set_talk_target_position(f32 src[3]);
void set_throw_target_position(f32 src[3]);

// --- src/core2/ba/ba_sfx.c ---
void basfx_80299AAC(void);
void basfx_80299BD4(void);
void basfx_80299DB8(void);
void basfx_80299E00(void);
void basfx_80299E48(void);
void basfx_80299E6C(void);
void basfx_80299E90(void);
void basfx_debug(void);
void basfx_free(void);
void basfx_reset(void);
void basfx_update(void);

// --- src/core2/ba/ba_sfxintensity.c ---
void cameraMode_update(void);
void func_80290B6C(void);
void func_80291488(s32 arg0);
void func_802914CC(s32 arg0);
void func_8029151C(s32 arg0);
void func_80291548(void);

// --- src/core2/ba/ba_state.c ---
void func_80295914(void);
void func_80295B04(void);
void func_80295C14(void);
void func_80295D74(void);

// --- src/core2/ba/ba_statusflags.c ---
void func_80323100(void);
void func_80323120(void);
void func_80323140(s32 arg0, s32 arg1);
void func_80323170(void);
void func_80323190(void);
void func_803231E8(void);
void func_8032320C(void);
void func_80323230(void);
void func_80323238(void);

// --- src/core2/ba/ba_stick.c ---
// [port] bastick_getZone removed: defined as (void) but bTrot.c/bWhirl.c pass args (decomp pattern)
void bastick_lockAtzero(bool arg0);
void bastick_reset(void);
void bastick_resetZones(void);
void bastick_update(void);

// --- src/core2/ba/ba_underwater.c ---
bool func_8029CF20(s32 arg0);
s32 func_8029CEB0(void);
void func_8029CF6C(void);

// --- src/core2/ba/ba_yaw.c ---
void func_802992F0(void);
void func_802993C8(void);
void func_8029957C(s32 arg0);

// --- src/core2/bs/ant.c ---
int bsant_inSet(s32 move_indx);

// --- src/core2/bs/bBarge.c ---
s32 bsbbarge_hitboxActive(void);

// --- src/core2/bs/bFly.c ---
int bsbfly_bombHitboxActive(void);
int bsbfly_inSet(enum bs_e arg0);

// --- src/core2/bs/bLongLeg.c ---
int bslongleg_inSet(s32 move_indx);

// --- src/core2/bs/bPeck.c ---
s32 bsbpeck_hitboxActive(void);

// --- src/core2/bs/bSwim.c ---
bool bsbswim_inSet(enum bs_e move_id);

// --- src/core2/bs/bTrot.c ---
int bsbtrot_inSet(s32 move_indx);

// --- src/core2/bs/bbuster.c ---
s32 bsbbuster_hitboxActive(void);
s32 func_8029FC4C(void);
void func_802A02B4(s32 arg0);

// --- src/core2/bs/bee.c ---
void func_802A02C0(void);
void func_802A0340(void);

// --- src/core2/bs/beeFly.c ---
int bsBeeFly_inSet(s32);

// --- src/core2/bs/bs_storedstate.c ---
enum transformation_e bsStoredState_getTransformation(void);
s32 bsStoredState_getTrotFlag(void);
void bsStoredState_8029A924(void);
void bsStoredState_clear(void);
void bsStoredState_clearTimers(void);
void bsStoredState_debug(void);
void player_setTransformation(enum transformation_e xform_id);

// --- src/core2/bs/carry.c ---
int bscarry_inSet(enum bs_e state);

// --- src/core2/bs/climb.c ---
int bsclimb_inSet(s32 move_indx);

// --- src/core2/bs/croc.c ---
int bscroc_hitboxActive(void);
int func_802AD3A0(void);
void func_802AD318(void);

// --- src/core2/bs/crouch.c ---
enum bs_e func_802ADCD4(enum bs_e arg0);

// --- src/core2/bs/drone.c ---
void bsdrone_end(void);
void bsdrone_init(void);
void bsdrone_update(void);

// --- src/core2/bs/jig.c ---
int bsjig_inJiggyJig(enum bs_e state);
void func_802B0CD8(void);

// --- src/core2/bs/jump.c ---
bool bsjump_jumpingFromWater(void);

// --- src/core2/bs/pumpkin.c ---
int bspumpkin_inSet(s32 move_indx);

// --- src/core2/bs/rebound.c ---
void func_802B35DC(void);
void func_802B360C(void);

// --- src/core2/bs/rest.c ---
void func_802B3A50(void);

// --- src/core2/bs/swim.c ---
bool bsswim_inset(enum bs_e state_id);

// --- src/core2/bs/walk.c ---
void func_802B6FA8(void);

// --- src/core2/bs/walrus.c ---
int bswalrus_inSledSet(enum bs_e state);

// --- src/core2/camera/camera_ease.c ---
void func_802C2250(void);
void func_802C2258(void);
void func_802C22C0(f32 target_position[3], f32 target_rotation[3]);

// --- src/core2/camera/camera_focus.c ---
void func_802BABD8(void);
void func_802BAC10(void);
void func_802BAC58(void);

// --- src/core2/camera/camera_fog.c ---
void func_802BEE2C(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_802BEF70(void);
void func_802BEF78(void);
void func_802BEFB0(void);

// --- src/core2/camera/camera_interp.c ---
void func_8030A6B0(void);
void func_8030A78C(void);
void func_8030A850(s32 arg0);

// --- src/core2/camera/camera_motor1.c ---
s32 func_802BB294(void);
void func_802BAF20(void);
void func_802BAF40(void);

// --- src/core2/camera/camera_motor2.c ---
void func_802BB2A0(void);
void func_802BB2A8(void);
void func_802BB3C4(s32 arg0);
void func_802BB41C(s32 arg0);
void func_802BB4D8(f32 position[3], f32 rotation[3]);

// --- src/core2/camera/camera_nodemanager.c ---
s32 ncCamera_getType(void);
void camera_setType(enum camera_type_e camera_type);
void func_802BBD0C(Gfx **gdl, Mtx **mptr, Vtx **vptr);
void func_802BBD2C(f32 *arg0, f32 *arg1);
void func_802BC044(void);
void func_802BC10C(void);
void func_802BC21C(s32 arg0, s32 arg1);
void func_802BC280(void);
void func_802BC2CC(s32 arg0);
void func_802BC434(f32 arg0[3], f32 arg1[3], f32 arg2[3]);
void ncCamera_update(void);

// --- src/core2/camera/camera_offset.c ---
int func_80322914(void);
s32 func_803226E8(enum map_e map_id);
s32 func_8032274C(void);
s32 func_80322758(void);
void func_80322764(void);
void func_8032278C(s32 arg0, s32 arg1);

// --- src/core2/camera/camera_position.c ---
void func_802BE940(void);

// --- src/core2/camera/camera_set.c ---
void func_803525A0(f32 arg0[3]);

// --- src/core2/camera/cameranode_type1.c ---
bool code336F0_func_802BA87C(CameraNodeType1 *self);
bool code336F0_func_802BA89C(CameraNodeType1 *self);
bool code336F0_func_802BA8BC(CameraNodeType1 *self);
void code336F0_func_802BA7D8(CameraNodeType1 *self, f32 arg1[3]);

// --- src/core2/camera/cameranode_type3.c ---
bool code33310_func_802BA4D0(CameraNodeType3 *self);
bool code33310_func_802BA4F0(CameraNodeType3 *self);

// --- src/core2/camera/cameranode_type4.c ---
s32 code33250_func_802BA234(CameraNodeType4 *self);

// --- src/core2/camera/cameranodelist.c ---
s32 ncCameraNodeList_getNodeType(int camera_node_index);
s32 ncCameraNodeList_nodeIsValid(int camera_node_index);
void ncCameraNodeList_defrag();
void ncCameraNodeList_free();
void ncCameraNodeList_fromFile(File *file_ptr);
void ncCameraNodeList_init();

// --- src/core2/camera/dynamicCam1.c ---
void ncDynamicCam1_end(void);
void ncDynamicCam1_init(void);
void ncDynamicCam1_update(void);

// --- src/core2/camera/dynamicCam10.c ---
void ncDynamicCam10_end(void);
void ncDynamicCam10_init(void);
void ncDynamicCam10_update(void);

// --- src/core2/camera/dynamicCam11.c ---
void func_802BF798(s32 camera_node_index);
void ncDynamicCam11_end(void);
void ncDynamicCam11_init(void);
void ncDynamicCam11_update(void);

// --- src/core2/camera/dynamicCam12.c ---
void dynamicCam12_init(void);
void func_802C0F4C(void);
void ncDynamicCam12_end(void);
void ncDynamicCam12_update(void);

// --- src/core2/camera/dynamicCam13.c ---
void func_802C095C(void);
void ncDynamicCam13_end(void);
void ncDynamicCam13_init(void);
void ncDynamicCam13_update(void);

// --- src/core2/camera/dynamicCam3.c ---
void ncDynamicCam3_end(void);
void ncDynamicCam3_init(void);
void ncDynamicCam3_update(void);

// --- src/core2/camera/dynamicCam4.c ---
void ncDynamicCam4_end(void);
void ncDynamicCam4_func_802BFE74(bool);
void ncDynamicCam4_init(void);
void ncDynamicCam4_update(void);

// --- src/core2/camera/dynamicCam5.c ---
void ncDynamicCam5_end(void);
void ncDynamicCam5_func_802BF590(f32 arg0[3]);
void ncDynamicCam5_init(void);
void ncDynamicCam5_update(void);

// --- src/core2/camera/dynamicCam8.c ---
void ncDynamicCam8_end(void);
void ncDynamicCam8_func_802BF9B8(s32 arg0);
void ncDynamicCam8_init(void);
void ncDynamicCam8_update(void);

// --- src/core2/camera/dynamicCam9.c ---
void ncDynamicCam9_end(void);
void ncDynamicCam9_init(void);
void ncDynamicCam9_update(void);

// --- src/core2/camera/dynamicCamA.c ---
void ncDynamicCamA_end(void);
void ncDynamicCamA_init(void);
void ncDynamicCamA_update(void);

// --- src/core2/camera/dynamicCamB.c ---
void ncDynamicCamB_end(void);
void ncDynamicCamB_init(void);
void ncDynamicCamB_update(void);

// --- src/core2/camera/dynamicCamC.c ---
void ncDynamicCamC_end(void);
void ncDynamicCamC_init(void);
void ncDynamicCamC_update(void);

// --- src/core2/camera/dynamicCamD.c ---
void ncDynamicCamD_end(void);
void ncDynamicCamD_init(void);
void ncDynamicCamD_update(void);

// --- src/core2/camera/dynamicCamF.c ---
void ncDynamicCamF_end(void);
void ncDynamicCamF_init(void);
void ncDynamicCamF_update(void);

// --- src/core2/camera/dynamicCamera.c ---
int func_802BE60C(void);
int func_802BE834(f32 arg0[3]);
int ncDynamicCamera_getState(void);
s32 func_802BC84C(s32 arg0);
void func_802BCBD4(void);
void func_802BCD30(void);
void func_802BD3CC(f32 arg0[3]);
void func_802BD4C0(f32 arg0[3]);
void func_802BD720(f32 arg0[3]);
void func_802BD840(void);
void func_802BD904(f32 target_rotation[3]);
void func_802BE190(f32 arg0[3]);
void func_802BE6FC(f32 arg0[3], f32 arg1[3]);
void func_802BE720(void);
void func_802BE794(void);
void func_802BE894(f32 position[3], f32 rotation[3]);
void ncDynamicCamera_enterFirstPerson(void);
void ncDynamicCamera_exitFirstPerson(void);
void ncDynamicCamera_getPosition(f32 arg0[3]);
void ncDynamicCamera_getRotation(f32 arg0[3]);
void ncDynamicCamera_setPosition(f32 arg0[3]);
void ncDynamicCamera_setRotation(f32 arg0[3]);
void ncDynamicCamera_setState(s32 state);
void ncDynamicCamera_setUpdateEnabled(bool arg0);
void ncDynamicCamera_update(void);

// --- src/core2/camera/firstpersoncamera.c ---
s32 ncFirstPersonCamera_getState(void);
void ncFirstPersonCamera_getPositionAndRotation(f32 position[3], f32 rotation[3]);
void ncFirstPersonCamera_getZoomedInRotation(f32 dst[3]);
void ncFirstPersonCamera_reset(void);
void ncFirstPersonCamera_setState(enum nc_first_person_state state);
void ncFirstPersonCamera_setZoomedOutPosition(f32 src[3]);
void ncFirstPersonCamera_setZoomedOutRotation(f32 src[3]);

// --- src/core2/camera/randomCamera.c ---
void ncRandomCamera_end(void);
void ncRandomCamera_init(void);
void ncRandomCamera_update(void);

// --- src/core2/camera/staticCamera.c ---
void ncStaticCamera_end(void);
void ncStaticCamera_exit(void);
void ncStaticCamera_getPosition(f32 dst[3]);
void ncStaticCamera_init(void);
void ncStaticCamera_rotateToTarget(f32 target[3]);
void ncStaticCamera_setPositionAndRotation(f32 arg0[3], f32 arg1[3]);
void ncStaticCamera_setPositionAndTarget(f32 arg0[3], f32 arg1[3]);
void ncStaticCamera_setToNode(s32);
void ncStaticCamera_update(void);

// --- src/core2/ch/beeswarm.c ---
void func_802CEB60(Actor *self);

// --- src/core2/ch/bottlesbonus.c ---
f32 * chBottlesBonus_get_piece_distance_vec4f(s32 arg0);
s32 chBottlesBonus_getPuzzleIndex(void);
s32 chBottlesBonus_getState(void);
void chBottlesBonus_func_802DD158(Gfx **gfx, Mtx** mtx);
void chBottlesBonus_func_802DEA74(s32 arg0);
void chBottlesBonus_func_802DEA8C(s32 arg0, s32 arg1);
void chBottlesBonus_func_802DEB80(void);
void chBottlesBonus_lose(u8 *arg0, enum asset_e text_id);
void chBottlesBonus_spawn(s32 arg0, s32 arg1);

// --- src/core2/ch/bottlesbonuscursor.c ---
ActorMarker * chBottlesBonusCursor_spawn(void);
bool chBottlesBonusCursor_isPuzzleCompleted(void);
f32 * chBottlesBonusCursor_func_802E0664(s32 indx);
f32 * chBottlesBonusCursor_func_802E068C(s32 indx);
// [port] chBottlesBonusCursor_func_802E0538 removed: defined with (s32) but bottlesbonus.c:240 calls with no args (decomp pattern)
s32 chBottlesBonusCursor_func_802E0588(s32 indx);
s32 chBottlesBonusCursor_func_802E06B4(void);
s32 chBottlesBonusCursor_getState(void);
void chBottlesBonusCursor_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
// [port] removed: chBottlesBonusCursor_func_802DF460 — polymorphic callers pass f32[3], Struct_core2_560F0_0[]
void chBottlesBonusCursor_lose(void);

// --- src/core2/ch/collectible.c ---
// [port] removed: chCollectible_collectEgg — polymorphic callers pass Prop*, ActorProp*
// [port] removed: chCollectible_collectGoldFeather — polymorphic callers pass Prop*, ActorProp*
// [port] removed: chCollectible_collectRedFeather — polymorphic callers pass Prop*, ActorProp*

// --- src/core2/ch/drips.c ---
void func_80359A40(f32 position[3], void *arg1, s32 cnt); // [port] struct_core2_D2AB0* -> void* (local type in drips.c)

// --- src/core2/ch/goldfeather.c ---
void func_802D8BE4(bool gold_feather);

// --- src/core2/ch/honeycomb.c ---
enum honeycomb_e func_802CA1C4(Actor *self);
void func_802CA1CC(enum honeycomb_e id);

// --- src/core2/ch/jiggy.c ---
enum jiggy_e chjiggy_getJiggyId(Actor *self);
void chjiggy_hide(Actor * self);
void chjiggy_setJiggyId(Actor *self, u32 id);

// --- src/core2/ch/jigsawdance.c ---
void chJigsawDance_setState(Actor * self, u32 arg1);

// --- src/core2/ch/mmm_veggies.c ---
void func_802CDAC4(Actor *);

// --- src/core2/ch/mole.c ---
int chmole_learnedAllLevelAbilities(enum level_e level);
int chmole_learnedAllSpiralMountainAbilities(void);

// --- src/core2/ch/overlaycopyright.c ---
void chOverlayCopyright_func_802DCB0C(s32 arg0, s32 arg1);
void chOverlayCopyright_spawn(s32 arg0, s32 arg1);

// --- src/core2/ch/overlaynocontroller.c ---
void chOverlayNoController_func_802DD040(s32 arg0, s32 arg1);
void chOverlayNoController_spawn(s32 arg0, s32 arg1);

// --- src/core2/ch/overlaypressstart.c ---
void chOverlayPressStart_func_802DCDB0(void);
void chOverlayPressStart_func_802DCDC0(NodeProp *arg0, ActorMarker *arg1);
void chOverlayPressStart_spawn(NodeProp *arg0, ActorMarker *arg1);

// --- src/core2/ch/soundsource.c ---
void func_802D09B8(Actor *self, s32 arg1);

// --- src/core2/ch/trainers.c ---
bool chtrainers_canUse(Actor *self);
void chtrainers_pickup(Actor *self);

// --- src/core2/ch/wadingboots.c ---
bool chwadingboots_802D6E0C(Actor *self);
void chwadingboots_802D6E54(Actor *self);

// --- src/core2/climb.c ---
u8 func_8029825C(void);
void climbClear(void);
void climbRelease(void);
void func_80298344(void);

// --- src/core2/collectible/bundle.c ---
// [port] __bundle_spawnFromFirstActor removed: polymorphic calls (actor_array.c has extern void(s32, Actor*), definition returns Actor*)
bool func_802C9C14(Actor *actor);
void bundle_free(void);
void bundle_reset(void);
void bundle_update(Actor *actor);

// --- src/core2/collectible/printdraw.c ---
// [port] removed: func_802FB0D4 — polymorphic callers pass struct8s*, struct7s*, Struct_core2_79830_0*
void func_802FB020(struct8s *self, s32 arg1);
void func_802FB104(s32, struct8s *);
void func_802FB15C(s32 arg0, struct8s * arg1);
void func_802FB194(s32 arg0, struct8s * arg1);
void func_802FB1CC(void);

// --- src/core2/collectible/printui.c ---
bool func_802FADD4(enum item_e item_id);
s32 func_802FAD9C(enum item_e item_id);
s32 itemPrint_getValue(s32 item_id);
void func_802FA69C(void);
void func_802FAC3C(void);
void func_802FACA4(enum item_e item_id);
void func_802FAFAC(enum item_e item_id, enum comusic_e music_id);
void func_802FAFC0(enum item_e item_id, enum comusic_e music_id);
void func_802FAFD4(enum item_e item_id, enum sfx_e sfx_id);
bool func_802FAFE8(enum item_e item_id); // [port] MIPS implicit return — wraps func_802FCD98 (returns bool)
void itemPrint_draw(Gfx **gdl, Mtx ** mptr, Vtx **vptr);
void itemPrint_free(void);
void itemPrint_init(void);
void itemPrint_reset(void);
void itemPrint_update(void);

// --- src/core2/collision/cachesize.c ---
void func_8033DC04(void);
void func_8033DC10(void);
void func_8033DC18(void);

// --- src/core2/collision/climbsurface.c ---
bool func_80294574(void);
int func_80294560(void);
s32 func_802944F4(void);
s32 func_80294524(void);
s32 func_80294530(void);
s32 func_80294548(void);
s32 func_80294554(void);
u32 func_80294610(u32 mask);
u32 func_80294660(void);
void func_80293D2C(f32 *arg0, f32 * arg1);
void func_80293D74(void);
void func_80293DA4(void);
void func_80293E88(void);
void func_80293F0C(void);
void func_8029436C(s32 arg0);
void func_80294378(s32 arg0);
void func_8029445C(f32 arg0[3]);
void func_80294480(f32 arg0[3]);
void func_802944D0(f32 dst[3]);
s32 func_80294684(void); // [port] MIPS implicit return — wraps func_8031C5A4 (returns s32)
void func_80294750(void);

// --- src/core2/collision/filter.c ---
ActorMarker * func_80351794(Struct68s *arg0);
Struct68s * func_803517E8(s32 arg0);
bool func_803518C0(Struct68s *arg0);
bool func_803518D4(Struct68s *arg0);
s32 func_80351758_getSfxsourceIndex(Struct68s *arg0);
s32 func_80351838(f32 position[3], s32 key_flag, s32 arg2);
void func_803518E8(void);
void func_80351954(Struct68s *arg);
void func_80351998(void);
void func_80351A1C(s32 arg0, s32 arg1);
void func_80351B28(Struct68s *arg0, f32 arg1[3]);
void func_80351C2C(Struct68s *arg0, f32 arg1[3]);
void func_80351C48(void);

// --- src/core2/collision/funcs.c ---
bool func_803209F8(f32 arg0[3], f32 arg1[3], f32 *arg2, f32 arg3[3]);
// func_80320B44: see funcs.c local forward-decl
void func_80320B7C(void);
void func_80320B84(void);

// --- src/core2/collision/hitboxdata.c ---
bool func_8033D410(ActorMarker *arg0, ActorMarker *arg1);
s32 func_8033D564(struct5Cs *arg0);
s32 func_8033D584(struct5Cs *arg0);
s32 func_8033D594(struct5Cs *arg0);
s32 func_8033D5A4(struct5Cs *arg0);
s32 func_8033D5B4(struct5Cs *arg0);
void func_8033D2F4(void);

// --- src/core2/collision/init.c ---
void func_8031B710(void);
void func_8031B718(void);
void func_8031B790(void);
void func_8031B990(s32 red, s32 blue, s32 green);

// --- src/core2/collision/raycast.c ---
s32 func_8031C59C(struct0 *self);
s32 func_8031C5A4(struct0 *self);
u8 func_8031C594(struct0 *self);
void func_8031BA7C(struct0 *self);
void func_8031BA9C(struct0 *self);
void func_8031C44C(struct0 *arg0);
void func_8031C608(struct0 *self);
void func_8031C618(struct0 *self, f32 *arg1);
void func_8031C638(struct0 *self, s32 arg1);

// --- src/core2/collision/spherecast.c ---
s32 func_8032CA80(Actor *actor, s32 arg1);
void func_8032C9E0(f32 arg0[3]);

// --- src/core2/collision/tricheck.c ---
void func_80344C50(void);
void func_80344C80(void);
void func_80344E7C(u8 index, f32 dst[3]);
void func_803451B0(u8 index, f32 arg1[3]);

// --- src/core2/crc_bootvalidation.c ---
void func_80356714(void);
void func_80356734(void);

// --- src/core2/cutscene/animated.c ---
void func_8035D490(ActorMarker *marker);
void func_8035D4F0(ActorMarker *marker, s32 arg1);

// --- src/core2/cutscene/ctrl.c ---
s32 func_802C0190(void);
void func_802C0120(void);
void func_802C0148(void);
void func_802C0150(s32 arg0);
void func_802C02D4(f32 arg0[3]);

// --- src/core2/cutscene/flag.c ---
s32 func_80334ECC(void);
s32 func_80335134();
void func_80334540(Gfx** gdl, Mtx **mptr, Vtx **vptr);
void func_80334910(void);
void func_80334B20(enum map_e arg0, s32 arg1, s32 arg2);
void func_80334E1C(s32);
void func_80335110(s32);
void func_80335128(s32);

// --- src/core2/cutscene/lair.c ---
s32 cutscenetrigger_update(void);
void func_8031CC40(enum map_e map_id, s32 arg1);
void func_8031D04C(enum map_e arg0, s32 exit_id);
void func_8031D06C(s32 arg0, s32 arg1); // [port] s32 — definition is (enum map_e, s32), nodeupdate.c calls as (NodeProp*, ActorMarker*)
void func_8031D0C0(NodeProp *arg0, ActorMarker *arg1);
void func_8031F9E0(void);
void func_8031F9E8();
void func_8031F9F4(s32 arg0);
void func_8031FBA0(void);
void func_8031FBF8(void);
void warp_mmmEnterLoggo(NodeProp *arg0, ActorMarker *arg1);

// --- src/core2/cutscene/nodeupdate.c ---
s32 func_8033451C(s32 arg0);
s32 func_80334524(s32 arg0);
s32 func_8033452C(s32 arg0);
void func_803343AC(void);
void func_803343D0(void);
void func_803343F8(s32 indx);
void func_80334428(void);
void func_80334448(NodeProp *arg0, ActorMarker *arg1);

// --- src/core2/demo_input.c ---
int demo_readInput(OSContPad* arg0, s32* arg1);
void demo_free(void);
void demo_load(enum map_e map, s32 demo_id);

// --- src/core2/dialog/binload.c ---
bool func_8031B604(u8 *arg0);
s32 code94620_func_8031B5B0(void);
s32 func_8031B5BC(void);          // dialog language count
void func_8031B5C4(s32 arg0);     // set dialog language index
void dialogBin_initialize(void);
void dialogBin_release(s32 arg0);
void dialogBin_terminate(void);
void dialogBin_update(void);
void func_8031B62C(void);

// --- src/core2/dialog/code_CF3E0.c ---
void func_8035644C(enum file_progress_e progress_flag);
void volatileFlag_setAndTriggerDialog_4(enum volatile_flags_e arg0);
void volatileFlag_setAndTriggerDialog_E(enum volatile_flags_e arg0);

// --- src/core2/equipped_items.c ---
void func_803246F0(u8* self, s32 indx);
void func_80324700(u8* self);
void func_80324770(u8* self, s32 indx, s32 value);
void func_8032477C(u8 *self);

// --- src/core2/fileselect.c ---
bool gameFile_anyNonEmpty(void);
bool gameFile_isNotEmpty(s32 gamenum);
s32 gameFile_8033CFD4(s32 gamenum);
void gameFile_8033CE40(void);
void gameFile_clear(s32 gamenum);
void gameFile_load(s32 gamenum);
void gameFile_save(s32 gamenum);

// --- src/core2/font/print.c ---
void func_802F4F64(void);
void func_802F5188(void);
void func_802F51B8(void);
void func_802F53D0(void);
void func_802F7B90(s32 arg0, s32 arg1, s32 arg2);
void printbuffer_defrag(void);
void printbuffer_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- src/core2/font/render.c ---
void func_802E5F10(Gfx **gdl);
void func_802E5F38(void);
void func_802E5F68(void);
void func_802E67AC(void);
void func_802E67C4(void);
void func_802E6820(s32 arg0);

// --- src/core2/frame/bufferreadback.c ---
bool func_802E4424(void);
bool func_802E4A08(void);
s32 func_802E4A98(s32 arg0);
s32 func_802E4AAC(s32 arg0);
s32 func_802E4AC0(s32 arg0);
// [port] func_802E4AD4 removed: defined with (s32) but ba_lookdir.c calls with no args (decomp pattern)
s32 func_802E4AE8(s32 arg0);
s32 func_802E4AFC(s32 arg0);
s32 func_802E4B10(s32 arg0);
s32 func_802E4B24(s32 arg0);
s32 game_defrag(void);
u8 func_802E4A8C(void);
void func_802E4048(s32 map, s32 exit, s32 transition);
void func_802E40C4(s32 arg0);
void func_802E40D0(s32 map, s32 exit);
void func_802E40E8(s32 transition);
void func_802E412C(s32 arg0, s32 arg1);
void func_802E4170(void);
void func_802E4214(enum map_e map_id);
void func_802E4384(void);
void func_802E4A70(void);
void func_802E4A80(void);
void game_draw(s32 arg0);

// --- src/core2/frame/rendermem.c ---
void dummy_func_802E35D0(void);
void func_802E3580(void);
void func_802E35D8(void);

// --- src/core2/fx/airscore.c ---
s32 fxairscore_count_to_time(s32 count);
s32 fxairscore_time_to_count(s32 time);

// --- src/core2/fx/common2score.c ---
enum item_e func_802FDD0C(struct8s *arg0);

// --- src/core2/fx/effect_colordata.c ---
bool vec4f_isAlmostZero(f32 arg0[4]);
bool vec4f_isZero(f32 arg0[4]);
void func_80345274(f32 arg0[4], f32 arg1[3][3]);
void func_80345C78(f32 arg0[4], f32 arg1[3]);
void func_80345CD4(f32 arg0[4], f32 arg1[4]);
void func_80345D30(f32 arg0[4], f32 arg1[4], f32 arg2[4]);
// [port] removed: vec4f_clone — polymorphic callers pass f32[4], BoneTransform*

// --- src/core2/fx/effect_eggshatter.c ---
void eggShatter_defrag(void);
void eggShatter_draw(Gfx **gPtr, Mtx **mPtr, Vtx **vPtr);
void eggShatter_free(void);
void eggShatter_init(void);
void eggShatter_new(f32 position[3]);
void eggShatter_update(void);

// --- src/core2/fx/effect_enemyrender.c ---
Actor * func_802DF160(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_802DF270(void);

// --- src/core2/fx/effect_modelparticle.c ---
void func_802DAC84(ParticleEmitter *pCtrl, Actor *self, enum asset_e model_id);
void func_802DAD08(ParticleEmitter *pCtrl, Actor *self, enum asset_e model_id);
void func_802DAD8C(ParticleEmitter *pCtrl, Actor *self, enum asset_e model_id);
void func_802DB4E0(ActorMarker *marker, s32 arg1);
void func_802DB5A0(Actor *self);

// --- src/core2/fx/effect_particle.c ---
void func_802DF090(s32 arg0, s32 arg1);
void func_802DF0C8(void);
void func_802DF11C(s32 arg0, s32 arg1);

// --- src/core2/fx/effect_playerspray.c ---
void func_802929F8(void);

// --- src/core2/fx/effect_simplesprite.c ---
void func_802DC110(f32 *position, enum asset_e sprite_id);

// --- src/core2/fx/effect_spriteanim.c ---
void func_802DC9A4(s32 arg0, s32 arg1);
void func_802DC9DC(s32 arg0, s32 arg1);

// --- src/core2/fx/honeycarrierscore.c ---
void gcpausemenu_80314AC8(int arg0);

// --- src/core2/fx/ripple.c ---
void fxRipple_802F3554(s32 arg0, f32 position[3]);
void fxRipple_802F3584(s32 arg0, f32 position[3], uintptr_t arg2); // [port] was s32 — carries BKCollisionTri*
void fxRipple_free(void);
void fxRipple_init(void);

// --- src/core2/fx/score_honeycomb.c ---
s32 honeycombscore_get_level_total(enum level_e level_id);
s32 honeycombscore_get_total(void);
void honeycombscore_clear(void);
void honeycombscore_debug(void);
void honeycombscore_getSizeAndPtr(s32 *size, u8 **addr);
void honeycombscore_set(enum honeycomb_e indx,  bool val);

// --- src/core2/fx/score_jiggy.c ---
s32 jiggyscore_leveltotal(s32 lvl);
s32 jiggyscore_total(void);
void * jiggyscore_clearAllSpawned(void);
void jiggyscore_clearAll(void);
void jiggyscore_debug(void);
void jiggyscore_getSizeAndPtr(s32 *size, u8 **addr);
void jiggyscore_setCollected(s32 indx,  s32 val);
void jiggyscore_setSpawned(s32, s32);

// --- src/core2/fx/score_jiggylist.c ---
void codeABC00_spawnJiggyAtLocation(enum jiggy_e, f32[3]);
void func_80332E08(void);
void func_8033301C(void);
void func_80333270(enum jiggy_e jiggy_id, f32 position[3], void (*method)(Actor *, ActorMarker *), ActorMarker *other_marker); // [port] fixed truncated function pointer param
void func_80333388(enum jiggy_e jiggy_id);
void func_803333DC(Struct81s *arg0, Actor *arg1);
void jiggylist_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void jiggylist_map_actors(void);
void jiggylist_set_level(enum map_e map_id);

// --- src/core2/fx/score_mumbo.c ---
bool mumboscore_get(enum mumbotoken_e indx);
void mumboscore_clear(void);
void mumboscore_debug(void);
void mumboscore_getSizeAndPtr(s32 *size, u8 **addr);
void mumboscore_set(enum mumbotoken_e indx,  bool val);

// --- src/core2/fx/sparkle.c ---
void fxSparkle_blueEgg(s16 position[3]);
void fxSparkle_brentilda(s16 position[3]);
void fxSparkle_emptyHoneycomb(s16 position[3]);
void fxSparkle_extraLife(s16 position[3]);
void fxSparkle_free(void);
void fxSparkle_giantBlueEgg(s16 position[3]);
void fxSparkle_giantGoldFeather(s16 position[3]);
void fxSparkle_giantRedFeather(s16 position[3]);
void fxSparkle_goldFeather(s16 position[3]);
void fxSparkle_honeycomb(s16 position[3]);
void fxSparkle_init(void);
void fxSparkle_mumboToken(s16 position[3]);
void fxSparkle_musicNote(s16 position[3]);
void fxSparkle_redFeather(s16 position[3]);

// --- src/core2/gameSelect.c ---
s32 func_802C5A30(void);
void func_802C5994(void);
void func_802C5A3C(s32 arg0);
void func_802C5A48(void);

// --- src/core2/game_complete.c ---
s32 bitfield_get_bit(u8 *array, s32 index);
s32 bitfield_get_n_bits(u8 *array, s32 offset, s32 numBits);
s32 dummy_func_80320240(void);
s32 dummy_func_80320248(void);
s32 fileProgressFlag_getAndSet(enum file_progress_e index, s32 set);
s32 func_8032056C(void);
s32 func_80320708(void);
void bitfield_set_bit(u8 *array, s32 index, s32 set);
void bitfield_set_n_bits(u8 *array, s32 startIndex, s32 set, s32 length);
void fileProgressFlag_getSizeAndPtr(s32 *size, u8 **addr);
void fileProgressFlag_set(enum file_progress_e index, s32 set);
void volatileFlag_backupAll(void);
void volatileFlag_clear(void);
void volatileFlag_restoreAll(void);

// --- src/core2/gc/dialog.c ---
int func_803110F8(s32 next_state, s32 arg1, s32 arg2, s32 arg3, void (*arg4)(ActorMarker *, enum asset_e, s32)); // [port] fixed truncated function pointer param
int func_80311174(s32 text_id, s32 arg1, f32 *pos, ActorMarker *marker, void(*callback)(ActorMarker *, enum asset_e, s32), void(*arg5)(ActorMarker *, enum asset_e, s32), s32(*arg6)(ActorMarker *, s32, s32)); // [port] arg6 was s32 — holds function pointer
int func_803114C4(void);
int func_803115C4(s32 next_state);
void func_8030F1D0(void);
void func_80310D2C(void);
void func_80311604(void);
void func_80311650(void);
void func_80311714(int next_state);
void gcdialog_defrag(void);
void gcdialog_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void gcdialog_init(void);

// --- src/core2/gc/parade.c ---
int gcparade_8031B4CC(void);
int gcparade_8031B4F4(void);
void func_8031B010(void);
void gcparade_8031ABA0(void);
void gcparade_8031ABF8(void);
void gcparade_beginFFParade(void);
void gcparade_free(void);
void gcparade_init(void);
void gcparade_update(void);

// --- src/core2/gc/pauseMenu.c ---
int gcpausemenu_80314B00(void);
s32 gcPauseMenu_update(void);
void gcpausemenu_80314B24(void);
void gcpausemenu_defrag(void);
void gcpausemenu_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void gcpausemenu_free(void);
void gcpausemenu_init(void);
void gcpausemenu_returnToLair(void);

// --- src/core2/gc/section.c ---
enum level_e map_getLevel(enum map_e map);
enum map_e level_get_main_map(enum level_e level_id);
s32 level_get_main_exit(enum level_e level_id);
void func_8030AFA0(enum map_e arg0);
void func_8030AFD8(s32 arg0);

// --- src/core2/gc/sky.c ---
void sky_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void sky_free(void);
void sky_reset(void);
void sky_update(void);

// --- src/core2/gfx/displaylistdata.c ---
void func_8030C180(void);
void func_8030C1A0(void);
void func_8030C204(void);
void func_8030C2D4(Gfx **gdl, Mtx **mptr, Vtx **vptr);
void scissorBox_setSmall(void);

// --- src/core2/gfx/displaylistinit.c ---
void func_80315084(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_80315110(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_803151D0(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- src/core2/glspline.c ---
bool func_80344040(Actor *self);
int func_80343D50(Actor *self, s32 arg1, s32 arg2, s32 arg3);
s32 func_80341C78(s32 arg0[3]);
s32 func_80341D5C(s32 arg0[3], s32 arg1[3]);
// [port] removed: func_80341EC4 — polymorphic callers pass f32[3], Actorlocal_Core2_9E370*
s32 func_80341F2C(s32 arg0);
s32 func_80343654(Actor *self);
void func_803411B0(void);
void func_80341A54(void);
void glspline_defrag(void);

// --- src/core2/level/lightconfig.c ---
void func_802BC538(s32 arg0, s32 *arg1, s32 *arg2, s32 *arg3, s32 *arg4, s32 *arg5, s32 *arg6, s32 *arg7, s32 *arg8, s32 *arg9);
void func_802BC5CC(void);

// --- src/core2/level/metadata.c ---
int func_80298760(s32 arg0);
s32 func_802987B4(void);

// --- src/core2/level/specificflags.c ---
s32 levelSpecificFlags_getN(s32 i, s32 n);
s32 levelSpecificFlags_getSet(s32 arg0, s32 arg1);
s32 levelSpecificFlags_validateCRC1(void);
s32 levelSpecificFlags_validateCRC2(void);
void levelSpecificFlags_clear(void);
void levelSpecificFlags_setN(s32 index, s32 val, s32 n);

// --- src/core2/load_zone.c ---
void codeA960_forceLoadzone(s32);
void func_80291910(void);
void func_802919A0(void);

// --- src/core2/map/audioconfig.c ---
s32 func_803246B4(enum map_e map_id, s32 arg1);

// --- src/core2/map/exit.c ---
bool func_8034BB48(void);
s32 func_8034BAFC(void);
s32 func_8034BDA4(enum map_e map_id, s32 exit_id);
void func_8034B8C0(enum map_e map_id, s32 demo_id);
void func_8034B940(void);
void func_8034B968(void);
void func_8034B9BC(s32 arg0);
void func_8034B9E4(void);
void func_8034BA7C(enum map_e map_id, s32 exit_id);

// --- src/core2/map/model.c ---
// [port] mapModel_getModel removed: conflicts with ff_manager.c local extern (void* vs BKModel* return type)
bool func_80309D58(f32 arg0[3], s32 arg1);
bool mapModel_has_xlu_bin(void);
void func_8030A078(void);
void mapModel_defrag(void);
void mapModel_free(void);
// [port] removed: mapModel_getBounds — polymorphic callers pass s32[3], f32[3]
void mapModel_opa_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void mapModel_setEnvColor(s32 r, s32 g, s32 b);
void mapModel_xlu_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- src/core2/map/savestate.c ---
int game_is_frozen(void);
void mapSavestate_apply(enum map_e map_id);
void mapSavestate_defrag_all(void);
void mapSavestate_free_all(void);
void mapSavestate_init(void);
void mapSavestate_save(enum map_e map);

// --- src/core2/map/specificflags.c ---
s32 mapSpecificFlags_validateCRC1(void);
u32 mapSpecificFlags_getAll(void);
u32 mapSpecificFlags_getClear(s32 i);
u32 mapSpecificFlags_getN(s32 idx, s32 n);
void mapSpecificFlags_clearAll(void);
void mapSpecificFlags_setAll(u32 arg0);
void mapSpecificFlags_setN(s32 idx, s32 val, s32 n);

// --- src/core2/misc_updates.c ---
void func_8029A47C(s32 arg0[3]);
void func_8029A4D0(void);
void func_8029A54C(void);
void func_8029A554(void);

// --- src/core2/model/matrixrotate.c ---
// [port] func_8033F7F0 removed: defined with 4 args but displaydata.c calls with 1 arg (decomp pattern)
void func_8033F9C0(void);
void func_8033FA24(void);

// --- src/core2/model/meshbounds.c ---
s32 func_8033F3C0(BKModel *model, f32 position[3]);
s32 func_8033F3E8(BKModel *model, f32 position[3], s32 min_id, s32 max_id);
void func_8033F738(ActorMarker *arg0);
void func_8033F784(ActorMarker *arg0);
void func_8033F7A4(ActorMarker *arg0, BKVertexList *arg1);

// --- src/core2/model/render.c ---
s32 func_8033A170(void);
void func_8033A1FC(void);
void func_8033A25C(bool arg0);
void func_8033A28C(bool arg0);
void modelRender_free(void);
void modelRender_init(void);
void modelRender_setAnimatedTexturesCacheId(s32 arg0);
void modelRender_setBoneTransformList(BoneTransformList *arg0);

// --- src/core2/model/rendernormal.c ---
void func_802E6BD0(BKModelUnk28List *arg0, BKVertexList *arg1, AnimMtxList *mtx_list);

// --- src/core2/mumbo_transforms.c ---
void chmumbo_func_802D1724(void);
void func_802D2CB8(void);
void func_802D2CDC(void);

// --- src/core2/overlay.c ---
enum overlay_e level_to_overlay(enum level_e lvl);
void func_80322E64(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_80322F5C(void);
void func_80322F7C(void);
void func_80322F9C(void);
void func_80322FBC(void);
void func_80322FDC(void);
void func_80323098(s32 arg0, s32 arg1);
void overlay_init(void);
void overlay_update(void);

// --- src/core2/particle/accel.c ---
u8 func_802F0F78(s32 cnt);
void func_802F10A4(void);
void func_802F1104(void);
void func_802F1190(u8 arg0);
void func_802F11E8(void);
void func_802F1320(void);
void func_802F1388(void);
void func_802F13E0(void);

// --- src/core2/particle/bathroom.c ---
void func_8029ADA8(void);
void func_8029ADCC(void);
void func_8029AE1C(void);
void func_8029AE48(void);
void func_8029AE74(s32 arg0);
void func_8029AF1C(void);

// --- src/core2/particle/colordefault.c ---
bool func_802EE5F0(s32 arg0);
s32 func_802EE5E0(s32 arg0);
void func_802EE5E8(void *self); // [port] ParticleEmitter* in definition, void* for sparkleemit.c compat
void func_802EE63C(void);
void func_802EE684(void);

// --- src/core2/particle/emitter1.c ---
void func_802F3CB0(void);
void func_802F3CD4(void);
void func_802F3CF8(f32 arg0[3], s32 arg1, s32 arg2);

// --- src/core2/particle/emitter2.c ---
void func_802F3E50(void);
void func_802F3E74(void);

// --- src/core2/particle/initcallback.c ---
s32 commonParticleType_80352C7C(enum common_particle_e id);
void commonParticleType_init(void);
void commonParticleType_set(enum common_particle_e arg0, GenFunction_0 init_method, GenFunction_0 update_method, GenFunction_0 free_method, s32 arg4, s32 arg5);

// --- src/core2/particle/lifescale.c ---
void func_802F1858(void *arg0, Gfx **gfx, Mtx **mtx, Vtx **vtx); // [port] arg0 is Struct_Core2_6A4B0_2* (local to .c)
void func_802F1884(void *arg0); // [port] arg0 is Struct_Core2_6A4B0_2* (local to .c)
void func_802F1934(void *arg0, s32 arg1); // [port] arg0 is Struct_Core2_6A4B0_2* (local to .c)

// --- src/core2/particle/particle.c ---
int particleEmitter_isDone(ParticleEmitter *self);
void func_802F053C(ParticleEmitter *self, f32 arg1[3]);
void func_802F066C(ParticleEmitter *self, f32 position[3]);
void func_802F0E80(void* arg0, s32 arg1);
void partEmitMgr_defrag(void);
void partEmitMgr_draw(Gfx **gdl, Mtx **mptr, Vtx **vptr);
void partEmitMgr_drawPass0(Gfx **gdl, Mtx **mptr, Vtx **vptr);
void partEmitMgr_drawPass1(Gfx **gdl, Mtx **mptr, Vtx **vptr);
void partEmitMgr_free(void);
void partEmitMgr_freeEmitter(ParticleEmitter *self);
void partEmitMgr_init(void);
void partEmitMgr_update(void);
void particleEmitter_draw(ParticleEmitter *self, Gfx **gdl, Mtx **mPtr, Vtx **vPtr);
void particleEmitter_emitUniformLine(ParticleEmitter *self, f32 start[3], f32 end[3], s32 count);
void particleEmitter_free(ParticleEmitter *self);
void particleEmitter_func_802EFA78(ParticleEmitter *self, s32 arg1);
// [port] particleEmitter_manualFree removed: defined with (ParticleEmitter*) but dronevanish.c calls with no args (decomp pattern)
void particleEmitter_setParticleCallback(ParticleEmitter *self, void (*arg1)(ParticleEmitter *self, f32 pos[3])); // [port] fixed truncated function pointer param
void particleEmitter_update(ParticleEmitter *self);

// --- src/core2/particle/playertrail.c ---
bool func_8029BDE8(void);
enum bs_e bs_getIdleState(void);
enum bs_e bs_getTypeOfJump(void);
enum bs_e func_8029B504(void);
enum bs_e func_8029BA80(void);
enum bs_e func_8029BD90(void);
enum bs_e func_8029BDBC(void);
enum bs_e func_8029BE5C(void);
enum bs_e func_8029BED4(void);
enum bs_e func_8029BF4C(void);
s32 func_8029C9C0(s32 arg0);
s32 func_8029CA94(s32 arg0);
void func_8029B6F0(void);
void func_8029B890(void);
void func_8029B930(void);
// [port] removed: func_8029BC60 — polymorphic callers pass enum asset_e*, s32*
void func_8029BCAC(enum asset_e *anim_id, f32 *anim_duration);
void func_8029BCF8(enum asset_e *anim_id, f32 *anim_duration);
void func_8029BD44(enum asset_e *anim_id, f32 *anim_duration);
void func_8029BE10(enum asset_e *anim_id, f32 *anim_duration);
void func_8029BE88(enum asset_e *anim_id, f32 *anim_duration);
void func_8029BF00(enum asset_e *anim_id, f32 *anim_duration);
void func_8029C0D0(void);
void func_8029C22C(void);
void func_8029C304(s32 arg0);
void func_8029C348(void);
void func_8029C4E4(bool arg0);
void func_8029C5E8(void);
void func_8029C674(void);
void func_8029C6D0(void);
void func_8029C748(void);
void func_8029C7F4(s32 arg0, s32 yaw_state, s32 arg2, s32 arg3); // [port] enum args → s32 to avoid forward-decl scoping issues with clang
void func_8029C834(enum map_e map_id, s32 exit_id);
void func_8029C848(AnimCtrl *arg0);
void func_8029C984(void);
void func_8029CB84(void);
void func_8029CCC4(void);
void func_8029CDA0(void);
void update_void_return_Location(void);

// --- src/core2/particle/positionset.c ---
void func_803541C0(s32 arg0);
void func_803541CC(s32 arg0);

// --- src/core2/particle/samplerate.c ---
void func_802F4798(Struct5Ds *self);
void func_802F487C(Struct5Ds *self, void (*arg1)(Struct5Ds *, s32)); // [port] fixed truncated function pointer param
void func_802F4894(Struct5Ds *self, f32 arg1[3]);
void func_802F48B4(Struct5Ds *self, void (*arg1)(Struct5Ds *, s32)); // [port] fixed truncated function pointer param
void func_802F48BC(Struct5Ds *self);
void func_802F48E0(Struct5Ds *self);
void func_802F4900(Struct5Ds *self, s32 arg1);
void func_802F4924(Struct5Ds *self);
void func_802F4978(Struct5Ds *self);

// --- src/core2/particle/scale1.c ---
void func_802EDD20(void);
void func_802EDD44(void);

// --- src/core2/particle/spawn.c ---
void func_802F3FE4(f32 pos[3]);
void func_802F404C(void);
void func_802F4070(void);

// --- src/core2/particle/typeindex.c ---
void func_802F1E80(void);
void func_802F1EA4(void);

// --- src/core2/particle/velocityrange.c ---
void func_802F3300(void);

// --- src/core2/particle/velocityset.c ---
void func_802F4200(f32 arg0[3]);
void func_802F422C(void);
void func_802F4250(void);

// --- src/core2/physics_force.c ---
void func_803223AC(void);
void func_80322490(void);
void func_803224FC(void);
void func_803225B0(s32 arg0, s32 arg1);

// --- src/core2/pitch.c ---
void pitch_applyIdeal(void);
void pitch_reset(void);
void pitch_update(void);

// --- src/core2/playerutils.c ---
bool func_8028B254(s32 arg0);
bool player_isActive(void);
bool player_isFallTumbling(void);
bool player_isInRBB(void);
bool player_isSwimming(void);
int can_dive(void);
int can_feathery_flap(void);
int can_peck(void);
int can_view_first_person(void);
int func_8028ABB8(void);
int func_8028ADB4(void);
int func_8028B394(void);
int func_8028B4C4(void);
int func_8028B528(void);
int player_isOnDangerousGround(void);
int player_isSliding(void);
int player_isStable(void);
int player_shouldFall(void);
int player_shouldSlideTrot(void);
int wishyWashyFlag_get(void);
s32 can_beak_barge(void);
s32 can_beak_bomb(void);
s32 can_beak_bust(void);
s32 can_claw(void);
s32 can_control_jump_height(void);
s32 can_egg(void);
s32 can_flap_flip(void);
s32 can_roll(void);
s32 can_trot(void);
s32 can_wonderwing(void);
s32 func_8028B120(void);
void func_8028B6FC(void);
void func_8028B71C(void);

// --- src/core2/prop_decoration.c ---
void func_802DC528(NodeProp *arg0, ActorMarker *arg1);
void func_802DC560(NodeProp*, ActorMarker*);
void func_802DC604(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- src/core2/propeller_engine.c ---
int func_8032190C(void);
s32 level_get(void);
void func_803216D0(enum map_e map);
void func_80321854(void);
void func_80321918(int arg0);
void func_80321924(void);

// --- src/core2/quiz/bitfield.c ---
void quizQuestionAskedBitfield_defrag(void);
void quizQuestionAskedBitfield_free(void);
void quizQuestionAskedBitfield_init(void);

// --- src/core2/quiz/game.c ---
bool func_802D4608(void);
enum actor_e func_802D67DC(enum actor_e arg0);
enum map_e func_802D677C(enum map_e arg0);
int func_802D6088(void);
int func_802D60C4(void);
int func_802D686C(void);
int func_802D6A38(enum map_e map_id);
s32 func_802D67AC(s32 arg0);
s32 func_802D680C(s32 arg0);
s32 func_802D683C(s32 arg0);
void func_802D3CE8(Actor *self);
void func_802D3D74(Actor *self);
void func_802D48B8(Actor *self);
void func_802D48F0(void);
void func_802D4928(Actor *self, s32 arg1, s32 arg2, s32 arg3);
void func_802D5058(enum map_e map_id, s32 arg1, bool arg2);
void func_802D520C(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_802D5628(void);
void func_802D6114(void);
void func_802D6344(void);
void func_802D63D4(void);
void func_802D6924(void);
void func_802D6948(void);

// --- src/core2/quiz/questionmanager.c ---
bool gcquiz_func_8031A154(enum ff_question_type_e q_type, s32 q_index, s32 arg2, s32 arg3, s32 arg4, void (*arg5)(s32, s8)); // [port] fixed truncated function pointer param
bool gcquiz_isNotInInitialState();
s32 gcquiz_getLastIndexOfQuestionType(enum ff_question_type_e question_type);
void gcquiz_defrag();
void gcquiz_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void gcquiz_free();
void gcquiz_func_80319EA4(void);
void gcquiz_func_8031A48C(void);
void gcquiz_init();

// --- src/core2/quiz/storage.c ---
void func_80350E00(void);

// --- src/core2/rand.c ---
s32 sfx_randi2(s32 min, s32 max);
void rand_reset(void);
void rand_seed(s32 seed);
void rand_shuffle(void);
void rand_sync_to_sfx_rand(void);
void sfx_rand_sync_to_rand(void);

// --- src/core2/roll.c ---
void roll_applyIdeal(void);
void roll_reset(void);
void roll_update(void);

// --- src/core2/savedata.c ---
// [port] removed: savedata_8033CC98 — polymorphic callers pass u8*, SaveData*
int savedata_8033CCD0(s32 filenum);
// [port] removed: savedata_8033CE40 — polymorphic callers pass GlobalSave*, u8*
s32 savedata_8033CA2C(s32 filenum, void *save_data); // [port] SaveData* -> void* (local type in save.h, not included here)
s32 savedata_8033CA9C(void *savedata); // [port] SaveData* -> void*
void saveData_create(void *savedata); // [port] SaveData* -> void*
void saveData_load(void *savedata); // [port] SaveData* -> void*
// [port] removed: savedata_clear — polymorphic callers pass u8*, SaveData*
void savedata_init(void);
void savedata_update_crc(void *buffer, s32 size);

// --- src/core2/sfx/filedata.c ---
void func_802F8FF0(void);
void func_802F8FFC(void);
void func_802F9054(void);
void func_802F90F4(void);
void func_802F9114(void);
void func_802F919C(void);
void func_802F962C(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- src/core2/sfx/init.c ---
struct3s * func_802F7C7C(void);
struct6s * func_802F7C38(void);
void func_802F7CC0(void);
void func_802F7CE0(void);
void func_802F7D30(void);
void func_802F7D44(void);
void func_802F7DE4(void);
void func_802F7E54(void);

// --- src/core2/sfx/playstate.c ---
void func_802F8A70(struct6s *self);
void func_802F8A90(struct6s *self, Gfx **gdl, Mtx **mptr, Vtx **vptr);
void func_802F8B8C(struct6s *self);
void func_802F8C90(struct6s *self);
void func_802F8CB0(struct6s *self);
void func_802F8CD0(struct6s * self);

// --- src/core2/sfx/randompitch.c ---
int func_802F9C0C(s32 arg0);
void func_802F9C48(void);
void func_802F9CD8(void);
void func_802F9D38(s32 arg0);
void func_802F9EC4(s32 arg0, f32 *arg1, s32 arg2, s32 arg3); // [port] arg1 is f32* position
void func_802F9F48(s32 arg0, s32 arg1);
void func_802FA028(s32 arg0, s32 arg1);
void func_802FA0B0(s32 arg0, s32 arg1);
void func_802FA4E0(void);
void func_802FA508(void);

// --- src/core2/sfx/sequenceindex.c ---
s32 func_8030C77C(void);
void func_8030C740(void);

// --- src/core2/sfx/source.c ---
int func_8030ED70(enum sfx_e uid);
s32 func_8030E1C4(u8 indx);
void func_8030D86C(void);
void func_8030D8A8(s32 arg0, s32 arg);
void func_8030D8DC(void);
void func_8030DCCC(u8, s32);
void func_8030DD54(u8 indx, void (*arg1)(u8)); // [port] fixed truncated function pointer param
void func_8030DFB4(u8 indx, s32 arg1);

// --- src/core2/sfx/streamctrl.c ---
void func_802F80F0(struct3s *self);
void func_802F8110(struct3s *self, Gfx **gdl, Mtx **mptr, u32 arg3);
void func_802F8214(struct3s * self);
void func_802F8338(struct3s *self);
void func_802F8358(struct3s *self);
void func_802F83AC(struct3s *arg0);

// --- src/core2/sfx/volume.c ---
void func_8034F6F0(Gfx **gdl, Mtx **mptr, s32 vptr);
void func_8034F734(void);
void func_8034F774(void);
void func_8034F918(void);

// --- src/core2/snacker_pool1.c ---
bool func_802FBE04(void);
void func_802FBB18(void);
void func_802FBDFC(void);

// --- src/core2/snacker_pool2.c ---
bool func_802FC390(void);

// --- src/core2/spawn_queue.c ---
// [port] spawnQueue_defrag: prototype at end of file uses void* since FunctionQueue is local to spawn_queue.c
void spawnQueue_flush(void);
void spawnQueue_free(void);
void spawnQueue_func_802C3A18(void);
void spawnQueue_lock(void);
void spawnQueue_malloc(void);
void spawnQueue_reset(void);
void spawnQueue_unlock(void);

// --- src/core2/spline_bezier.c ---
void func_8034A130(struct5Bs *self);
void func_8034A1B4(struct5Bs *self, s32 indx, s32 dst[3]);
void func_8034A308(struct5Bs *self, s32 indx, f32 arg2[3]);

// --- src/core2/sprite/displaydata.c ---
s32 func_8033E8AC(void);
u8 func_8033E93C(void);
void func_8033DEA0(void);
void func_8033E184(void);
void func_8033E1E0(void);
void func_8033E7CC(ActorMarker *arg0);
void func_8033E9A8(s32 arg0);
void func_8033E9D4(void);
void func_8033E9F4(void);
void func_8033EA78(s32 arg0, s32 arg1);

// --- src/core2/sprite/render.c ---
s32 func_80344C20(BKSpriteDisplayData *self);
void func_80344138(BKSpriteDisplayData *self, s32 frame, s32 mirrored, f32 position[3], f32 scale[3], Gfx **gfx, Mtx **mtx);
void func_80344720(BKSpriteDisplayData *arg0, s32 frame, bool mirrored, f32 position[3], f32 rotation[3], f32 scale[3], Gfx **gfx, Mtx **mtx);
void func_80344C2C(bool arg0);
void func_80344C38(void (*method)(ActorMarker *), ActorMarker *marker); // [port] fixed truncated function pointer param + missing param

// --- src/core2/sprite/rendertex.c ---
void func_80347FC0(Gfx **gfx, BKSprite *sprite, s32 frame, s32 tmem, s32 rtile, s32 uls, s32 ult, s32 cms, s32 cmt, s32 *width, s32 *height);
void func_80348044(Gfx **gfx, BKSprite* sprite, s32 frame, s32 tmem, s32 rtile, s32 uls, s32 ult, s32 cms, s32 cmt, s32 *width, s32 *height, s32 *frame_width, s32 *frame_height, s32 *texture_x, s32 *texture_y, s32 *textureCount);
void func_80349AD0(void);
void func_80349B1C(Gfx **gfx);

// --- src/core2/sprite/screenoverlay.c ---
void func_80335D30(Gfx **gfx);
void func_8033687C(Gfx **gfx);
void func_803382B4(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_803382E4(s32 arg0);
void func_803382FC(s32 arg0);
void func_80338308(s32 arg0, s32 arg1);
void func_80338338(s32 r, s32 g, s32 b);
void func_80338370(void);
void func_8033837C(s32 arg0);
void spriteRender_draw(Gfx **gfx, Vtx **vtx, BKSprite *sp, u32 frame);

// --- src/core2/terrain_material.c ---
enum item_e carriedobj_actorId2ItemId(enum actor_e actor_id);
s32 item_adjustByDiffWithHud(enum item_e item, s32 diff);
s32 item_getCount(enum item_e item);
s32 itemscore_noteScores_getTotal(void);
s32 itemscore_timeScores_getTotal(void);
u16 itemscore_timeScores_get(enum level_e level_id);
void func_803465BC(void);
void func_803465DC(void);
void func_803465E4(void);
void func_80346CA8(void);
void func_80346CE8(void);
void func_8034789C(void);
void func_80347958(void);
void func_80347984(void);
void func_8034798C(void);
void func_803479C0(u8 *arg0);
void func_80347A70(void);
void func_80347A7C(void);
void func_80347AA8(void);
void item_adjustByDiffWithoutHud(enum item_e item, s32 diff);
void item_dec(enum item_e item);
void item_inc(enum item_e item);
void item_setItemsStartCounts(void);
void item_setMaxCount(s32 item);
void itemscore_highNoteScores_fromSaveData(u8 *savedata);
void itemscore_levelReset(enum level_e level);
// [port] removed: itemscore_timeScores_fromSaveData — polymorphic callers pass u16*, u8*
void notescore_getSizeAndPtr(s32 *size, void **ptr);
void saveditem_getSizeAndPtr(s32 *size, u8 **buffer);
void timeScores_getSizeAndPtr(s32 *size, void **ptr);

// --- src/core2/texture/copy.c ---
void func_802FEF48(BKModelBin *model_bin);

// --- src/core2/timed_funcqueue.c ---
// [port] timedFuncQueue_is_empty removed: defined as (void) but conga.c passes Actor* arg (decomp pattern)
void func_80324C58(void);
void timedFuncQueue_defrag(void);
void timedFuncQueue_flush(void);
void timedFuncQueue_free(void);
void timedFuncQueue_init(void);
void timedFuncQueue_update(void);

// --- src/core2/vtx/alphablend.c ---
s32 func_8034E698(Struct73s *arg0);

// --- src/core2/vtx/colorapply.c ---
void func_8034BF54(ActorMarker *marker);
void func_8034BFF8(ActorMarker *marker);
void func_8034C21C(ActorMarker *marker);

// --- src/core2/vtx/lighting.c ---
void lighting_free();
void lighting_init();
void lightingbk_vectorList_fromFile(File *file_ptr);

// --- src/core2/vtx/listutils.c ---
void func_802E73C8(f32 arg0[3][3]);

// --- src/core2/vtx/meshcount.c ---
bool meshList_meshContainsVtx(BKMeshList * meshList, s32 mesh_id, void *vtx_id);

// --- src/core2/vtx/normalset.c ---
// [port] removed: func_8034CF6C — polymorphic callers pass Struct72s*, f32*, ActorLocal_core2_560F0*
// [port] removed: func_8034CF74 — polymorphic callers pass Struct72s*, ActorLocal_core2_560F0*
// [port] removed: func_8034CF90 — polymorphic callers pass Struct72s*, ActorLocal_core2_560F0*

// --- src/core2/vtx/positionset.c ---
s32 func_8034F560(Struct76s *arg0);

// --- src/core2/vtx/renderstart.c ---
bool func_8034DC80(Struct6Ds *arg0, f32 arg1[3]);
s32 func_8034DC78(Struct6Ds *arg0);
// [port] removed: func_8034E0FC — polymorphic callers pass Struct6Ds*, Struct70s*
void func_8034E174(Struct6Ds *arg0);
void func_8034E254(Struct6Ds *arg0, void (*arg1)(Struct6Ds *)); // [port] fixed truncated function pointer param
void func_8034E25C(Struct6Ds *arg0, void (*arg1)(Struct6Ds *)); // [port] fixed truncated function pointer param
void func_8034E264(Struct6Ds *arg0, s32 arg1);

// --- src/core2/vtx/transform.c ---
int func_8034C4CC(void);
void * func_8034C9D4(void);
void func_8034C8D8(void);
void func_8034C97C(void);
void func_8034C9B0(int arg0);

// --- src/core2/vtxlist.c ---
s32 vtxList_getVtxCount(BKVertexList *self);
void vtxList_getCenterAndNorm(BKVertexList *self, f32 center[3], f32 *norm);
void vtxList_getVtxRange(BKVertexList *self, Vtx **vtx, Vtx **vtx_end);
void vtxList_recolor(BKVertexList *self, s32 arg1[3]);

// --- src/core2/yaw.c ---
void yaw_init(void);
void yaw_setUpdateState(s32 arg0);
void yaw_update(void);

// --- src/cutscenes/actor_spawninit.c ---
void cutscene_func_8038C4E0(void);

// --- src/fight/fight.c ---
void fight_addSpawnableActors(void);

// --- src/lair/actor_spawninit.c ---
void lair_func_8038A0C4(void);

// --- src/lair/ch/brentilda.c ---
void gzquiz_initGruntyQuestions(void);

// --- src/lair/ff_manager.c ---
s32 func_8038E800(void);
void func_8038E7C4(void);
void func_8038E968(s32 idx);
void lair_func_8038CD48(void);
void lair_func_8038CF18(void);
void lair_func_8038E0B0(void);
void lair_func_8038E768(Gfx **dl, Mtx **m, Vtx **v);

// --- src/lair/puzzle_door.c ---
bool func_8038EAE0(s32 arg0);

// --- src/port/stub.c ---
s32 osContSetCh(u8 ch);
u32 __osGetSR(void);
void func_80253010(void* dest, void* src, s32 size);
void osCreateThread(OSThread* thread, OSId id, void* entry, void* arg, void* sp, OSPri p);
void osDestroyThread(OSThread* thread);
void osDpSetStatus(u32 data);
void osSetThreadPri(OSThread* thread, OSPri p);
void osSpTaskYield(void);
void osStartThread(OSThread* thread);
void osStopThread(OSThread* t);

// --- unmapped (definition not found in src/) ---
s32 osPiReadIo(u32, u32 *);

// ============================================================
// MISSING PROTOTYPES (clang -Wimplicit-function-declaration)
// ============================================================

// --- src/core2/ba/ba_stick.c ---
s32 bastick_getZone(void);

// --- src/core2/ba/ba_lookdir.c ---
void func_8028F94C(s32 arg0, f32 arg1[3]);

// --- src/core2/collectible/printdraw.c ---
s32 func_802FB0D4(void *self); // [port] void* — callers pass struct7s*, struct8s*, Struct_core2_79830_0*

// --- src/core2/particle/particle.c ---
void particleEmitter_manualFree(ParticleEmitter *self);

// --- src/core2/anim/anim_bonetransform.c ---
void func_8033BD20(void **arg0); // [port] callers pass BKModelBin**, BKSprite**, void** — use void**

// --- src/core2/timed_funcqueue.c ---
bool timedFuncQueue_is_empty(void);

// --- src/core2/fx/effect_colordata.c ---
void vec4f_clone(f32 dst[4], f32 src[4]);

// --- src/core2/collectible/bundle.c ---
Actor *__bundle_spawnFromFirstActor(enum bundle_e bundle_id, Actor *actor);

// --- src/FP/ch/twinklybox.c ---
bool func_8038DD14(void);

// --- src/SM/version_compat.c ---
int func_8038AAB0(); // [port] PAL takes 2 args, US stub ignores them — empty parens for C compat with 0-arg calls

// --- src/GV/crc.c ---
void func_80389F5C(void);

// --- src/BGS/ch/tanktup.c ---
s32 func_8038F570(); // [port] definition takes s16* but some callers pass no args

// --- src/core1/collision.c ---
bool func_80245524(f32 arg0[3], void *arg1, intptr_t *arg2, f32 *arg3);

// --- src/core2/particle/playertrail.c ---
void func_8029BC60(enum asset_e *anim_id, f32 *anim_duration);

// --- src/core2/frame/bufferreadback.c ---
s32 func_802E4AD4(s32 arg0);

// --- src/core2/audio_sfxinstruments.c ---
void func_8033543C(Struct81s *arg0);

// --- src/core2/model/matrixrotate.c ---
void func_8033F7F0(u8 indx, Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- src/core2/glspline.c ---
s32 func_80341EC4(f32 arg0[3]);

// --- src/core2/vtx/normalset.c ---
void func_8034CF6C(void *arg0); // [port] canonical is Struct72s*, use void* for compat
void func_8034CF74(void *arg0, s32 arg1, BKModel *arg2, s32 arg3); // [port] arg0 is Struct72s*
void func_8034CF90(void *arg0, BKModel *arg1, s32 arg2); // [port] arg0 is Struct72s*

// --- src/core2/vtx/renderstart.c ---
void func_8034E0FC(Struct6Ds *arg0, s32 arg1);

// --- src/core2/map/model.c ---
BKModel *mapModel_getModel(s32 arg0);
void mapModel_getBounds(s32 min[3], s32 max[3]);

// --- src/core2/anim/anim_cache.c ---
int animCache_getBoneTransformList(s16 index, BoneTransformList **arg1);

// --- src/core2/anim_bonetransformlist.c ---
void boneTransformList_reset(BoneTransformList *self);

// --- src/core2/ch/bottlesbonuscursor.c ---
void chBottlesBonusCursor_func_802DF460(s32 indx, ActorMarker *caller, f32 arg2[3]);
s32 chBottlesBonusCursor_func_802E0538(s32 indx);

// --- src/core2/ch/collectible.c ---
s32 chCollectible_collectEgg(ActorProp *arg0);
bool chCollectible_collectGoldFeather(ActorProp *arg0);
bool chCollectible_collectRedFeather(ActorProp *arg0);

// --- src/TTC/ch/nipper.c ---
bool chNipper_isInState7(); // [port] definition takes s16[3] but lair.c calls with no args

// --- src/core2/actor_cubepropsystem.c ---
s32 codeA5BC0_getPositionAndReturnRadius(void *arg0, s32 arg1[3]); // [port] void* — callers pass NodeProp*, struct_core2_DB010*

// --- src/core2/terrain_material.c ---
void itemscore_timeScores_fromSaveData(u16 *savedata);

// --- src/core2/savedata.c ---
int savedata_8033CE40(void *buffer); // [port] void* — callers pass u8*, GlobalSave*, SaveData*
int savedata_8033CC98(s32 filenum, void *buffer); // [port] void*
void savedata_clear(void *savedata); // [port] void*

// --- src/core2/spawn_queue.c ---
void spawnQueue_defrag(); // [port] definition takes FunctionQueue* but some callers pass no args

#endif // PORT_PROTOTYPES_H
