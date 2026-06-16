#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <ultra64.h>

#include "enums.h"
#include "structs.h"
#include "string.h"
#include "rand.h"

#include "prop.h"
#include "model.h"

#include "core1/core1.h"
#include "core2/core2.h"
#include "core2/camera.h"
#include "core2/abilityprogress.h"
#include "core2/commonParticle.h"
#include "core2/dustemitter.h"
#include "core2/particleemittermanager.h"
#include "core2/staticcamera.h"
#include "core2/anim/sprite.h"
#include "core2/ba/anim.h"
#include "core2/ba/model.h"
#include "core2/ba/timer.h"
#include "core2/nc/camera.h"

#include "port/ShipUtils.h"
#include "port/UI/cvar_prefixes.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include "math.h"
#include "bk_time.h"
#include "bs_funcs.h"
#include "bsint.h"
#include "generic.h"

#include <libultraship/bridge.h>

// FuncUnk40 defined in core2/commonParticle.h — forward-typedef here to avoid circular include
#ifndef FUNCUNK40_DEFINED
#define FUNCUNK40_DEFINED
typedef s32 (*FuncUnk40)(ActorMarker *, s32, f32[3]);
#endif

extern f32 fabsf(f32);

#define FUNC_8030E624(sfx_e, vol, sample_rate) func_8030E624(\
    _SHIFTL((vol*1023), 21, 11) + _SHIFTL(sample_rate >> 5, 11, 10) + _SHIFTL(sfx_e, 0, 11)\
)

#define sfx_playFadeShorthandDefault(sfx_e, vol, sample_rate, position, e, f) sfx_playFadeShorthand(\
    _SHIFTL((vol*1023), 21, 11) + _SHIFTL(sample_rate >> 5, 11, 10) + _SHIFTL(sfx_e, 0, 11), \
    position, \
    _SHIFTL(e, 0, 16) + _SHIFTL(f, 16, 16)\
)
#define bzero(pointer, size) memset(pointer, 0, size)
#define bcopy(src, dest, size) memcpy(dest, src, size)

f32 cosf(f32);


// --- core2/prop_assetcache.c ---
BKModelBin *propModelList_getModelIfActive(s32 arg0);
BKSpriteDisplayData *propModelList_getSpriteDisplayList(s32 arg0);
BKSprite *propModelList_getSprite(s32 arg0);
BKModelBin *propModelList_getModel(s32);

// --- core2/actor_cubepropsystem.c ---
ActorMarker *func_8032DCAC(void);
BKVertexList *func_80330CFC(Actor *self, s32 arg1);
BKModelBin *func_80330E28(Actor *self);
BKSpriteDisplayData *func_80330E54(ActorMarker *marker, BKSprite **sprite_ptr);
BKSpriteDisplayData *func_80330F30(ActorMarker *marker);
BKSprite *func_80330F50(ActorMarker *marker);
BKSprite *codeB3A80_getSprite(enum asset_e sprite_id, BKSpriteDisplayData **arg1);
NodeProp *codeA5BC0_getPropNodeAtIndex(Cube *cube, s32 prop_index);
ActorMarker *func_8032FBE4(f32 *pos, MarkerDrawFunc arg1, int arg2, enum asset_e model_id);
ActorMarker *marker_init(s32 *pos, MarkerDrawFunc draw_func, int arg2, int marker_id, int arg4);
BKModelBin *  func_80330DE4(ActorMarker *marker);
BKModelBin *marker_loadModelBin(ActorMarker *marker);
BKVertexList *func_80330C74(Actor *actor);
Prop *func_8032F528(void);
void marker_callCollisionFunc(ActorMarker *, ActorMarker *, enum marker_collision_func_type_e);
void marker_setActorUpdate2Func(ActorMarker *marker, ActorUpdateFunc method);
void marker_setActorUpdateFunc(ActorMarker *marker, ActorUpdateFunc method);
void marker_setCollisionScripts(ActorMarker *marker, MarkerCollisionFunc ow_func, MarkerCollisionFunc arg2, MarkerCollisionFunc die_func);
void marker_setFreeMethod(ActorMarker *, void (*)(Actor *));

// --- core2/actor_cubebounds.c ---
NodeProp *cubeList_findNodePropByActorIdAndPosition_s32(enum actor_e actor_id, s32 position[3]);
NodeProp *func_80305510(s32 arg0);
BKCollisionTriangle *func_80303800(f32 volume_p1[3], f32 volume_p2[3], f32 arg2[3], u32 arg3);
NodeProp *nodeprop_findByActorIdAndPosition_s16(enum actor_e actor_id, s16 *position);
Actor * func_803055E0(enum actor_e id, s32 pos[3], s32 arg2, s32 arg3, s32 arg4);
NodeProp *nodeprop_findByActorIdAndActorPosition(enum actor_e actor_id, Actor *actor_ptr);
NodeProp *nodeprop_findByActorIdAndPosition_f32(enum actor_e actor_id, f32 position[3]);
void nodeprop_getPosition(NodeProp *, f32[3]);
void spawnableActorList_add(ActorInfo *arg0, Actor *(*arg1)(s32[3], s32, ActorInfo *, u32), u32 arg2);
void spawnableActorList_addIfMapVisited(ActorInfo *arg0, Actor *(*arg1)(s32[3], s32, ActorInfo *, u32), u32 arg2, enum map_e arg3);

// --- core2/actor_cubepropsystem.c ---
BKCollisionTriangle *func_803311D4(Cube *cube, f32 arg1[3], f32 arg2[3], f32 arg3[3], u32 arg4);

// --- core2/gameloop.c ---
u8 GetCurrentMap();
s32 getGameMode(void);
void transitionToMap(enum map_e map, s32 exit, s32 transition);

// --- core2/time.c ---
f32 time_getDelta(void);

// --- core2/map_list.c ---
BKCollisionTriangle *func_8029463C(void);
BKModelBin *func_802946A8(void);
BKCollisionTriangle *func_802946CC(void);

// --- core2/collision/raycast.c ---
BKCollisionTriangle *func_8031BABC(f32 *arg0, f32 arg1, f32 arg2, u32 arg3, struct86s *arg4);
BKCollisionTriangle *func_8031BBA0(f32 *self, f32 arg1, f32 arg2, u32 arg3, struct86s *arg4);
BKCollisionTriangle *func_8031C5EC(struct0 *self);
BKCollisionTriangle *func_8031C5F4(struct0 *self);
BKModelBin *func_8031C5DC(struct0 *self);
struct0 *func_8031B9D8(void);

// --- core2/collision/funcs.c ---
BKCollisionTriangle *func_80320B98(f32 arg0[3], f32 arg1[3], f32 arg2[3], u32 arg3);
// func_80320C94, func_80320DB0 return BKCollisionTriangle* but
// have conflicting local externs (bool/s32/int) in decomp source files.
void *func_803209EC(void);

// --- core1/collision.c ---
BKCollisionTriangle *func_80244E54(f32 arg0[3], f32 arg1[3], f32 arg2[3], u32 arg3, f32 arg4, f32 arg5);
BKCollisionTriangle *func_8024575C(f32 arg0[3], f32 arg1[3], f32 arg2, f32 arg3[3], s32 arg4, u32 arg5);
BKCollisionTriangle *func_802457C4(f32 arg0[3], f32 arg1[3], f32 arg2, f32 arg3, f32 arg4[3], s32 arg5, u32 arg6);

// --- core2/actor_array.c ---
BKModelBin *func_803257B4(ActorMarker *marker);
Actor *actorArray_findActorFromMarkerId(enum marker_e marker_id);
void *actors_appendToSavestate(void *begin, void *end);
Actor * __actor_spawnWithYaw_s32(enum actor_e id, s32 pos[3], s32 yaw);
Actor * spawn_child_actor(enum actor_e id, Actor ** parent);
Actor *actorArray_findActorFromActorId(enum actor_e);
Actor *actorArray_findClosestActorFromActorId(f32 position[3], enum actor_e actor_id, s32 arg2, f32 *min_distance_ptr);
Actor *actor_draw(ActorMarker *, Gfx**, Mtx**, Vtx **);
Actor *actor_drawFullDepth(ActorMarker *marker, Gfx **gfx, Mtx **mtx, Vtx **vtx);
Actor *actor_new(s32 position[3], s32 yaw, ActorInfo *actorInfo, u32 flags);
Actor *actor_spawnWithYaw_f32(enum actor_e actor_id, f32 position[3], s32 yaw);
Actor *actor_spawnWithYaw_s16(enum actor_e id, s16 (*pos)[3], s32 yaw);
Actor *func_80325340(ActorMarker *, Gfx **, Mtx **, Vtx **);
Actor *fxTouchSparkle_draw(ActorMarker *marker, Gfx **gfx, Mtx **mtx, Vtx **vtx);
Actor *func_8032A7AC(Actor *);
Actor *marker_getActor(ActorMarker *);
Actor *marker_getActorAndRotation(ActorMarker *marker, f32 rotation[3]);
Actor *subaddie_getLinkedActor(Actor *);
ActorMarker *func_8032B16C(enum jiggy_e jiggy_id);
bool  func_80329078(Actor *, s32, s32);
bool  func_80329480(Actor *);
bool subaddie_maybe_set_state_position_direction(Actor *, s32, f32, s32, f32 );
int  func_8032863C(AnimCtrl *, f32, f32);
int  func_80328A2C(Actor *, f32, s32, f32);
int  func_80329030(Actor *, s32);
int  subaddie_maybe_set_state(Actor *, s32, f32);
int actor_animationIsAt(Actor*, f32);
s32  subaddie_getYawToPlayer(Actor *);
s32 asset_getFlag(enum asset_e arg0);
Vec3fArray *func_80329934(void);
void actor_collisionOff(Actor *);
void actor_collisionOn(Actor *);
void actor_loopAnimation(Actor *);
void actor_playAnimationOnce(Actor *);
void actor_predrawMethod(Actor *);
void actor_update_func_80326224(Actor *actor);
void func_80326244(Actor *);
void func_80326310(Actor *actor); // actor_setBlendStateFadeOut ??
void func_80328CEC(Actor *, s32, s32, s32);
void subaddie_turnToYaw(Actor *, f32);
void func_80329878(Actor *, f32);
void suSetSpriteScale(Actor *, f32);
void marker_despawn(ActorMarker *marker);
void subaddie_set_state(Actor *, u32);
void subaddie_set_state_forward(Actor *, s32);
void subaddie_set_state_with_direction(Actor * actor, s32 myAnimId, f32 anim_start_position, s32 direction);

// --- core2/spline_pathfollow.c ---
struct56s *func_80342038(s32 indx);
struct56s *func_80341EF0(f32 arg0[3]);
struct56s *func_80341F64(s32 arg0);
struct56s *func_80343F00(s32 indx, f32 arg1[3]);

// --- core2/collision/polydetect.c ---
Struct83s *func_803406B0(void);
Struct83s *func_803406D4(Struct83s *self);

// --- core2/vtx/list.c ---

// --- core2/spline_bezier.c ---

// --- core2/model/render.c ---

// --- core2/vtx/colorapply.c ---
Struct70s *func_8034C344(s32 arg0);
Struct70s *func_8034C448(s32 arg0);
Struct70s *func_8034C630(void *arg0);
Struct70s *func_8034C2C4(ActorMarker *marker, s32 arg1);

// --- core2/particle/samplerate.c ---
Struct5Ds *func_802F47D0(void);
Struct5Ds *func_802F499C(Struct5Ds *self);

// --- core2/font/render.c ---
BKSpriteTextureBlock *func_802E4D5C(s32 arg0, char arg1);

// --- core2/font/print.c ---
BKSpriteTextureBlock *print_getBoldFontLetterSprite(s32 letterId, s32 *fontType);

// --- core2/anim/anim_spriteframe.c / particle/typeindex.c etc. ---
ParticleEmitter *func_802EDD8C(f32 pos[3], f32 xz_range, f32 arg2);
ParticleEmitter *func_802F1EC8(f32 *position);
ParticleEmitter *func_802F3E98(f32 pos[3], enum asset_e sprite_id);
ParticleEmitter *func_802F4274(f32 arg0[3]);

// --- core2/bundle.c ---
Actor *bundle_spawn_f32(enum bundle_e bundle_id, f32 position[3]);
Actor *bundle_spawn_s32(enum bundle_e bundle_id, s32 position[3]);
void bundle_setYaw(f32);

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

// FLOAT-RETURNING FUNCTIONS

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
f32 bsbtrot_getMaxTargetVelocity(void);
f32 bsbtrot_getMinTargetVelocity(void);
f32 bsbtrot_getFastestDuration(void);
f32 bsbtrot_getSlowestDuration(void);
f32 func_802A716C(void);
f32 func_802B051C(s32 arg0, f32 arg1, f32 arg2, f32 arg3);
f32 __bscroc_getMaxVelocity(void);
f32 func_802D7038(Actor *self);
f32 player_getYaw(void);


// These return pointers but are commonly listed with utility functions.
// Only functions that return pointers or have no conflicting local externs.
NodeProp *cube_findNodePropByActorId(Cube *cube, enum actor_e actor_id);

// --- core2/camera/cameranodelist.c ---
PivotCameraNode *ncCameraNodeList_getPivotCameraNode(int camera_node_index);
StaticCameraNode *ncCameraNodeList_getStaticCameraNode(int camera_node_index);
ZoomCameraNode *ncCameraNodeList_getZoomCameraNode(int camera_node_index);
RandomCameraNode *ncCameraNodeList_getRandomCameraNode(int camera_node_index);

// MISC POINTER-RETURNING + OS + GBI PROTOTYPES

s16 *func_8030C704(void);

// COMMONLY MISSING PROTOTYPES

// --- core2/fx/score_jiggy.c ---
u32 jiggyscore_isCollected(enum jiggy_e jiggy_id);
int jiggyscore_isSpawned(enum jiggy_e jiggy_id);

// --- core2/fx/score_honeycomb.c ---
bool honeycombscore_get(enum honeycomb_e indx);

// --- core2/map/specificflags.c ---
s32 mapSpecificFlags_get(s32 i);
void mapSpecificFlags_set(s32, s32);

// --- core2/level/specificflags.c ---
s32 levelSpecificFlags_get(s32 i);
void levelSpecificFlags_set(s32, s32);

// --- core2/gameloop.c ---
s32 getGameMode(void);
void transitionToMap(enum map_e map, s32 exit, s32 transition);

// --- core2/gamestate.c (item system) ---
s32 item_empty(enum item_e item);
void item_set(s32 item, s32 val);

// --- core2/game_complete.c ---
bool fileProgressFlag_get(enum file_progress_e index);
s32 fileProgressFlag_getN(enum file_progress_e offset, s32 numBits);
s32 volatileFlag_get(enum volatile_flags_e index);
s32 volatileFlag_getAndSet(enum volatile_flags_e index, s32 arg1);
s32 volatileFlag_getN(enum volatile_flags_e index, s32 numBits);
void fileProgressFlag_setN(enum file_progress_e, s32, s32);
void volatileFlag_set(enum volatile_flags_e index, s32 set);
void volatileFlag_setN(enum volatile_flags_e startIndex, s32 set, s32 length);

// --- core2/dialog/progress_dialogs.c ---
void volatileFlag_setAndTriggerDialog_0(enum volatile_flags_e arg0);

// --- core1/init.c ---
s32 globalTimer_getTime(void);

// --- core2/actor_array.c ---
bool subaddie_playerIsWithinSphereAndActive(Actor *self, s32 dist);
s32 subaddie_getYawToPosition(Actor *arg0, f32 arg1[3]);
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
void code_7060_setVoidOutLocation(enum map_e map_id, s32 exit_id);
bool player_setCarryObjectPoseInCylinder(f32[3], f32, f32, enum actor_e actor_id, Actor**);
u32 player_getTransformation(void);
void ability_unlock(enum ability_e);
void func_8028E668(f32[3], f32, f32, f32);
void func_8028E7EC(f32 arg0[3]);
void player_getPosition(f32 dst[3]);
void player_getRotation(f32 *dst);
void player_setThrowTargetPosition(f32[3]);

// --- core2/sfx/source.c ---
void func_8030DD90(u8 indx, s32 arg1);
int func_8030E3FC(u8 indx);
f32  func_8030E200(u8);
u8   sfxsource_createSfxsourceAndReturnIndex(void);
u8 func_8030ED2C(enum sfx_e uid, s32 arg1);
void func_8030DB04(u8, s32, f32 position[3], f32, f32);
void sfxSource_setunk43_7ByIndex(u8, int);
void func_8030DFF0(u8, s32);
void func_8030E04C(u8, f32, f32, f32);
void func_8030E0FC(u8, f32, f32, f32);
void sfxSource_func_8030E2C4(u8);
void sfxSource_func_8030E2C4(u8 indx);
void func_8030E4E4(enum sfx_e uid);
void gcsfx_playAtSampleRate(enum sfx_e uid);
void func_8030E560(enum sfx_e uid, s32 arg1);
void func_8030E58C(enum sfx_e uid, f32 arg1);
void func_8030E5F4(enum sfx_e uid, f32 arg1);
void func_8030E624(u32);
void gcsfx_playWithPitch(enum sfx_e uid, f32 arg1, s32 arg2);
void func_8030E6D4(enum sfx_e uid);
void func_8030E704(enum sfx_e uid);
void func_8030E760(enum sfx_e uid, f32 arg1, s32 arg2);
void func_8030E878(enum sfx_e uid, f32 arg1, u32 arg2, f32 arg3[3], f32 arg4, f32 arg5);
void sfx_playFadeShorthand(u32,f32 [3], u32);
void func_8030E988(enum sfx_e uid, f32 arg1, u32 arg2, f32 arg3[3], f32 arg4, f32 arg5);
void func_8030E9C4(enum sfx_e uid, f32 arg1, u32 arg2, f32 arg3[3], f32 arg4, f32 arg5);
void func_8030EAAC(enum sfx_e uid, f32 arg1, s32 arg2, s32 arg3);
void func_8030EB00(enum sfx_e uid, f32, f32);
void func_8030EB88(enum sfx_e uid, f32 arg1, f32 arg2);
void func_8030EBC8(enum sfx_e uid, f32 arg1, f32 arg2, s32 arg3, s32 arg4);
void func_8030EC20(enum sfx_e uid, f32 arg1, f32 arg2, u32 arg3, u32 arg4);
void sfxsource_freeSfxsourceByIndex(u8);
void sfxsource_play(enum sfx_e uid, s32 sample_rate);
void sfxsource_playHighPriority(enum sfx_e uid);
void sfxsource_playSfxAtVolume(u8, f32);
void sfxsource_setSfxId(u8 indx, enum sfx_e uid);
void sfxsource_set_fade_distances(u8, f32, f32);
void sfxsource_set_position(u8, f32[3]);

// --- core2/camera/camera_motor1.c ---
void gcStaticCamera_activate(s32 arg0);
bool func_802BB270(void);

// --- core2/gc/dialog.c ---
void func_803114D0(void);
int gcdialog_hasCurrentTextId(void);
bool gcdialog_showDialog(s32 text_id, s32 arg1, f32 *pos, ActorMarker *marker, void(*callback)(ActorMarker *, enum asset_e, s32), void(*arg5)(ActorMarker *, enum asset_e, s32));

// --- core2/cutscene/lair.c ---
void func_8031CD20(NodeProp *arg0, s32 arg1, s32 arg2);

// --- core2/collision/polydetect.c ---
void func_80340690(Struct83s *self);

// --- core2/actor_cubebounds.c ---
bool func_803077FC(f32 arg0[3], s32 *arg1, s32 *arg2, s32 arg3, u32 arg4);
bool func_80305C30(s32 arg0);
bool func_80308F54(s32 cube_index);
bool nodeprop_findPositionFromActorId(enum actor_e actor_id, s32 *position);
s32 func_80306D40(s32 arg0);
s32 func_80306EF4(s32 arg0[3], s32 arg1, s32 arg2);
s32 func_80307504(f32 arg0[3], s32 arg1, s32 arg2, s32 arg3, s32 arg4);

// --- core2/actor_cubepropsystem.c ---
bool func_8032E398(Cube *cube, bool (*arg1)(NodeProp *), bool (*arg2)(Prop *));
bool func_80330534(Actor *actor);
bool func_8033056C(Actor *actor);
bool func_80331158(ActorMarker *arg0, f32 *arg1, f32 *arg2);

// --- core2/ba/ba_lookdir.c ---
bool func_8028F280(void);

// --- core2/ba/ba_anim.c ---
bool baanim_isStopped(void);
bool baanim_isAnimID(enum asset_e anim_id);
AnimCtrl *baanim_getAnimCtrlPtr(void);
bool  baanim_isAt(f32);
void baanim_playForDuration_loopSmooth(enum asset_e anim_id, f32 duration);
void baanim_playForDuration_loopStartingAt(enum asset_e anim_id, f32, f32);
void baanim_playForDuration_once(enum asset_e anim_id, f32 duration);
void baanim_playForDuration_onceSmooth(enum asset_e, f32);
void baanim_playForDuration_onceStartingAt(enum asset_e anim_id, f32 duration, f32 arg2);
void baanim_scaleDuration(f32);
void baanim_setDurationRange(f32, f32);
void baanim_setEnd(f32);
void baanim_setEndAndDuration(f32, f32);
void baanim_setVelocityMapRanges(f32, f32, f32, f32);

// --- core2/ba/ba_groundsurface.c ---
bool isOnFloor(void);
bool isPlayerInHazard(void);
bool canTakeGroundDamage(void);

// --- core2/ba/ba_animstate.c ---
bool func_8029DFE0(void);
f32  func_8029DFC8(void);
f32  func_8029DFD4(void);
void func_8029E090(bool, f32);
void func_8029E0C4(f32);
void func_8029E0D0(f32);
bool func_8029DFBC(void);
bool func_8029DFA4(void);
bool func_8029DFB0(void);
bool func_8029DFEC(void);

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
f32  func_802BD8D4(void);
void func_802BD8A4(f32, f32, f32);
void func_802BE230(f32, f32);
void func_802BE244(f32, f32);

// --- core2/camera/dynamicCam12.c ---
bool __is_flying_in_FP(void);
bool func_802C189C(void);

// --- core2/camera/dynamicCam13.c ---
bool func_802C0640(void);

// --- core2/camera/dynamicCamA.c ---
bool ncDynamicCamA_func_802C1EE0(void);
bool  ncDynamicCamA_func_802C1DB0(f32);

// --- core2/camera/firstpersoncamera.c ---
bool __ncFirstPersonCamera_fullyZoomedIn(void);

// --- core2/collision/funcs.c ---
void func_80320B24(void *arg0, void *arg1, void *arg2);
void func_80320B44(void *arg0, void *arg1, void *arg2, void *arg3);

// --- core2/collision/climbsurface.c ---
s32 func_8029453C(void);
BKCollisionTriangle *func_802946F0(void);
f32  func_80294438(void);
f32  floor_getCurrentFloorYPosition(void);
void func_80293D48(f32, f32);

// --- core2/spline_bezier.c ---

// --- core2/vtx/list.c ---

// --- core2/spline_pathfollow.c ---
void func_80343DEC(Actor *self);

// --- core2/model/render.c ---

// --- core2/spawn_queue.c ---
void __spawnQueue_add_0(void (*arg0)(void));
void __spawnQueue_add_2(void (*arg0)(void), uintptr_t arg1, uintptr_t arg2);
Actor *spawnQueue_actor_f32(enum actor_e actor_id, uintptr_t x, uintptr_t y, uintptr_t z);
void __spawnQueue_add_1(GenFunction_1, uintptr_t);
#define SPAWNQUEUE_ADD_1(method, arg0) __spawnQueue_add_1((GenFunction_1) (method), reinterpret_cast(uintptr_t, (arg0)))
void __spawnQueue_add_4(GenFunction_4, uintptr_t, uintptr_t, uintptr_t, uintptr_t);

// --- core2/sprite/displaydata.c ---
void commonParticle_add(ActorMarker *arg0, s32 arg1, FuncUnk40 arg2);
int commonParticle_new(enum common_particle_e particle_id, int arg1);

// --- core2/particle/particle.c ---
void particleEmitter_setModel(ParticleEmitter *self, enum asset_e model_id);
void particleEmitter_setSfx(ParticleEmitter *self, enum sfx_e sfx_id, s32 arg2);
void particleEmitter_setAlpha(ParticleEmitter *self, s32 arg1);
void particleEmitter_setVelocityAccelerationAndPositionRanges(ParticleEmitter *self, ParticleSettingsVelocityAccelerationPosition *settings);
void func_802EFC28(ParticleEmitter *self, ParticleSettingsScaleAndLifetimeDrawModeEmitCount *settings);
ParticleEmitter *partEmitMgr_defragEmitter(ParticleEmitter *);
ParticleEmitter *partEmitMgr_newEmitter(u32);
ParticleEmitter *particleEmitter_new(u32 capacity);
void particleEmitter_func_802EFA20(ParticleEmitter *, f32, f32);
void func_802EFF50(ParticleEmitter *, f32);
void particleEmitter_emitInVolume(ParticleEmitter *, f32[3], f32[3], s32);
void particleEmitter_emitN(ParticleEmitter *, int);
void particleEmitter_func_802EF9F8(ParticleEmitter *, f32);
void particleEmitter_func_802EFA18(ParticleEmitter *, s32);
void particleEmitter_setAccelerationRange(ParticleEmitter *, f32, f32, f32, f32, f32, f32);
void particleEmitter_setAngularVelocityRange(ParticleEmitter *, f32, f32, f32, f32, f32, f32);
void particleEmitter_setDrawMode(ParticleEmitter *, s32);
void particleEmitter_setFade(ParticleEmitter *, f32, f32);
void particleEmitter_setFinalScaleRange(ParticleEmitter *, f32, f32);
void particleEmitter_setParticleFramerateRange(ParticleEmitter *, f32, f32);
void particleEmitter_setParticleLifeTimeRange(ParticleEmitter *, f32, f32);
void particleEmitter_setParticleVelocityRange(ParticleEmitter *, f32, f32, f32, f32, f32, f32);
void particleEmitter_setPosition(ParticleEmitter *, f32[3]);
void particleEmitter_setPositionAndVelocityRanges(ParticleEmitter *emitter, ParticleSettingsVelocityPosition *arg1);
void particleEmitter_setRGB(ParticleEmitter *emitter, s32 arg1[3]);
void particleEmitter_setScaleAndLifetimeRanges(ParticleEmitter *, ParticleScaleAndLifetimeRanges *);
void particleEmitter_setSpawnInterval(ParticleEmitter *, f32);
void particleEmitter_setSpawnIntervalRange(ParticleEmitter *, f32, f32);
void particleEmitter_setSpawnPositionRange(ParticleEmitter *, f32, f32, f32, f32, f32, f32);
void particleEmitter_setSprite(ParticleEmitter *, enum asset_e);
void particleEmitter_setStartingFrameRange(ParticleEmitter *emitter, s32 arg1, s32 arg2);
void particleEmitter_setStartingScaleRange(ParticleEmitter *, f32, f32);
void particleEmitter_update(ParticleEmitter *self);
void particleEmitter_setVelocityAndAccelerationRanges(ParticleEmitter *, ParticleSettingsVelocityAcceleration *);

// --- core2/quiz/game.c ---
void func_802D68F0(s32 seconds);
void func_802D4A9C(Actor *self, s32 arg1);
void func_802D4AC0(Actor *self, s32 arg1, enum file_progress_e arg2);
void func_802D6264(f32, enum map_e, s32, s32, s32, enum file_progress_e);

// --- core2/fx/projectile_blueegg.c ---
void func_80353580(ActorMarker *marker);

// --- core1/musicplayer.c ---
void func_8025A58C(u32 arg0, u32 arg1);
void func_8025AABC(enum comusic_e track_id);
void func_8025AEA0(enum comusic_e track_id, s32 arg1);
void comusic_8025AB44(enum comusic_e comusic_id, s32 arg1, s32 arg2);
void comusic_playTrack(enum comusic_e);
void coMusicPlayer_playMusic(enum comusic_e, s32);

// --- core1/stopnswop.c ---
bool sns_get_item_state(enum StopNSwop_Item item, s32 set);

// --- FP/mapstate/slalom.c ---
bool maSlalom_isActive(void);

// --- FP/ch/boggy2.c ---
bool func_8038A1A0(ActorMarker *marker);

// code3B10_checkGVChecksums omitted: decomp defines as (void) but callers pass Actor*

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

// --- BGS/actor_spawninit.c ---
void BGS_func_8038F1E0(void);

// --- BGS/ch/croctus.c ---
void BGS_func_803885DC(void);

// --- BGS/ch/frogminigame.c ---
void BGS_func_8038CED0(void);
void func_8038CE88(void);
void func_8038CEA0(void);
void func_8038CEB8(void);

// --- BGS/ch/mrvile.c ---
bool BGS_func_8038C338(ActorMarker *marker);
bool func_8038C2A8(ActorMarker *marker);
void BGS_func_8038C434(ActorMarker *marker);
void BGS_func_8038C460(ActorMarker *arg0);
void func_8038C384(ActorMarker *marker);
void func_8038C3B0(ActorMarker *marker);
void func_8038C3DC(ActorMarker *marker);
void func_8038C408(ActorMarker *marker);

// --- BGS/ch/mudhut.c ---
void chMudHut_checkBGSChecksums(void);

// --- BGS/ch/tanktup.c ---
void func_8038F51C(Actor *self);

// --- BGS/ch/tiptup.c ---
s32 func_80388E70(ActorMarker *self);
void chTiptup_choirHitReaction(ActorMarker *self, s32 arg1);
void func_80388FFC(ActorMarker *self, s32 *arg1, f32* arg2);

// --- BGS/ch/yumblie.c ---
bool chyumblie_is_edible(ActorMarker * arg0);
bool func_8038B684(ActorMarker * arg0);

// --- BGS/minigame_vile.c ---
bool chvilegame_cpu_consume_piece(ActorMarker *marker, f32 position[3]);
s32 chvilegame_get_piece_count(ActorMarker *marker);
s32 chvilegame_get_score_difference(ActorMarker *marker);
s32 func_8038A9E0(ActorMarker *marker);
void chvilegame_new_piece(ActorMarker *game_marker, ActorMarker *piece_marker, f32 position[3], u32 yumblie_type);
void chvilegame_remove_piece(ActorMarker *game_marker, ActorMarker *piece_marker);

// --- CC/actor_spawninit.c ---
void CC_func_80387DA0(void);

// --- CC/ch/clankertoothext.c ---
void CC_func_803870E0(void);
void func_803870EC(s32 arg0);

// --- CC/minigame_rings.c ---
void func_803880D4(void);
void func_80388104(void);
void func_8038817C(void);

// --- CC/platform_physics.c ---
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

// --- CCW/actor_spawninit.c ---
void CCW_func_8038DB6C(void);

// --- CCW/ch/nabnut_winter.c ---
void func_8038BC50(f32 dst[3]);

// --- CCW/ch/eyrie_baby.c ---
void func_80389BD8(f32 dst[3]);

// --- CCW/flower.c ---
bool func_80388438();
void func_803883F4();

// --- CCW/minigame_zubba.c ---
void func_803867C8(ActorMarker *marker);
void func_80386814(ActorMarker *marker);
void func_80386840(ActorMarker *marker, s32 *score, s32 *total);

// --- FP/actor_spawninit.c ---
void FP_func_80391324(void);

// --- FP/ch/boggy2.c ---
void func_8038A09C(f32 arg0[3]);

// --- FP/ch/twinkly.c ---
void chTwinkly_decideShatterColor(f32 position[3], enum marker_e marker_id);

// --- FP/ch/twinklybox.c ---
bool preventSnowmanAttack(ActorMarker *marker);

// --- FP/mapstate/slalom.c ---
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

// --- FP/mapstate/snowbutton.c ---
void maSnowButton_decRemaining(void);
void maSnowButton_end(void);
void maSnowButton_init(void);
void maSnowButton_update(void);

// --- FP/mapstate/snowy.c ---
void maSnowy_decRemaining(void);
void maSnowy_end(void);
void maSnowy_incTotal(void);
void maSnowy_init(void);
void maSnowy_update(void);

// --- FP/ch/wozza_fire.c ---
void func_803918C0(void);
void func_8039195C(void);
void func_80391994(void);

// --- GV/actor_spawninit.c ---
s32 code7FF0_getMagicCarpetState(Actor *arg0, s32 arg1);
void GV_func_8038F154(void);

// --- GV/ch/ancientone.c ---
void GV_func_80387118(void);

// --- GV/ch/buriedpyramid.c ---
void chBuriedPyramid_setRaisedAmount(ActorMarker *this_marker, s32 arg1);

// --- GV/ch/gobi1.c ---
s32 func_80387354(void);
s32 func_80387360(void);

// --- GV/ch/gobirock.c ---
bool chGobiRock_isDestroyed(void);

// --- GV/ch/grabba.c ---
s32 GV_func_8038C5BC(void);

// --- GV/ch/handshadow.c ---
void func_8038C748(void);

// --- GV/ch/jinxy_head.c ---
int func_8038E344(ActorMarker *this_marker);
void func_8038E2FC(ActorMarker *this_marker);

// --- GV/ch/toots.c ---
void func_803865E8(void);
void func_803865F8(void);
void func_80386608(void);

// --- GV/gv_helpers.c ---
s32 func_8038E178(void);
s32 func_8038E184(void);
void func_8038E140(void);
void func_8038E18C(void);

// --- GV/matchinggame.c ---
void gv_matchingGame_init(void);
void gv_matchingGame_reset(void);
void gv_matchingGame_update(void);

// --- GV/pyramid_trigger.c ---
int func_8038D388(void);

// --- GV/water_pyramidactivate.c ---
void func_803900F8(void);
void func_80390100(void);
void func_80390138(void);

// --- GV/water_pyramidrot.c ---
void func_8038FF60(void);
void func_8038FF68(void);
void func_8038FFF4(void);

// --- GV/waterctrl.c ---
void gv_waterCtrl_end(void);
void gv_waterCtrl_init(void);
void gv_waterCtrl_update(void);

// --- MM/actor_spawninit.c ---
void MM_func_803888B0(void);

// --- MM/ch/hut.c ---
void mm_resetHuts(void);

// --- MM/ch/juju.c ---
bool __chjuju_isEveryJujuDespawned(ActorMarker **ptr);
bool __chjuju_isEveryJujuStable(ActorMarker **ptr);
void __chjuju_initialize_all(ActorMarker *marker, s32 count);
void __chjuju_updateCount(ActorMarker **ptr);
void func_803892A8(ActorMarker **ptr);

// --- MM/ch/jujuhitbox.c ---
void chjujuhitbox_setJuju(Actor *self, s32 slave_id, Actor *slavePtr);

// --- MMM/actor_spawninit.c ---
void MMM_func_803890E0(void);

// --- MMM/organ_tiles.c ---
void func_8038B5D8(Struct5Fs *arg0, Struct68s *arg1, s32 arg2, s32 arg3);

// --- MMM/ch/flowerpot.c ---
bool chFlowerpot_eggCollision(ActorMarker *marker);
void chFlowerpot_reset();

// --- MMM/ch/motzhand.c ---
bool func_8038769C(ActorMarker *marker);
void func_80387654(ActorMarker *marker);
void func_803876C8(ActorMarker *marker, s32 arg1);
void func_80387720(ActorMarker *marker);

// --- MMM/ch/tumblar.c ---
bool chTumblar_isBanjoAbove(void *arg0, Struct68s *arg1);
bool chTumblar_isDisappeared(void *arg0, s32 arg1);
void chTumblar_congratulate(void *arg0, s32 arg1);
void chTumblar_copyPosition(s32 arg0, Struct68s *arg1, f32 arg2[3]);
void chTumblar_checkMMMChecksums(void);

// --- MMM/minigame_organ.c ---
int func_80389CE8(s32 arg0, s32 arg1, s32 arg2);
s32 func_80389BBC(void);
void MMM_func_80389CD8();
void code3420_handleOrganGame(s32 arg0, s32 arg1);
void func_80389CE0();
void func_80389D9C(s32 key_id);
void maOrgan_update(void);
void organMinigame_getKeyPosition(s32 key_indx, f32 position[3]);

// --- MMM/minigame_shed.c ---
void func_8038A994();
void func_8038A9B4(void);
void func_8038AA30(void *arg0, void *arg1);
void func_8038AA44(void);

// --- MMM/napper_room.c ---
s32 func_80389510();
u8 MMM_func_80389530();
u8 func_80389524();
void func_8038953C();
void func_80389544(void);
void func_803895B0(s32 arg0);
void func_8038966C(void);

// --- RBB/actor_spawninit.c ---
void RBB_func_80386C48(void);

// --- RBB/ch/eggtoll1.c ---
void func_8038685C(ActorMarker *marker);

// --- RBB/ch/whistlectrl.c ---
s32 chWhistleCtrl_newEvent(Actor *self, s32 whistle_id, Actor *other);

// --- RBB/miniboombox_container.c ---
void func_8038FB54(void);
void func_8038FB6C(void);

// --- RBB/propellorctrl.c ---
void rbb_propellorCtrl_reset(void);
void rbb_propellorCtrl_start(void);
void rbb_propellorCtrl_update(void);

// --- SM/quarrie_honeycomb.c ---
bool codeBF0_shouldSpawnQuarrieHoneyComb(ActorMarker *marker);

// --- SM/crc.c ---
void SM_resetSpawnableActors();
void codeF0_breakAbilitiesIfChecksumsFail();

// --- SM/model_visibility.c ---
void code2900_checkSMChecksums(void);

// --- TTC/actor_spawninit.c ---
void code26D0_resetSpawnableActorsForTTC(void);

// --- TTC/ch/leaky.c ---
bool chLeaky_eggCollision(ActorMarker *marker);

// --- TTC/ch/treasurehunt.c ---
void chTreasurehunt_resetProgress(void);

// --- TTC/crc.c ---
void code3040_checkTTCChecksums(void);

// --- TTC/castle.c ---
bool maCastle_hasBanjoKazooieCodeBeenEntered(void);
bool maCastle_isSecretCheatCodeRelatedValueEqualToScrambledAddressValue();
void maCastle_init(void);
void maCastle_release(void);
void maCastle_update(void);

// --- core1/audio_instruments.c ---
s32 gcMusic_getDefaultVolumeForTrack(s32 track_id);
s32 func_802501A0(u8 arg0, s32 arg1, s32 *arg2);
void func_8024F764(s32 arg0);
void func_8024F7C4(s32 arg0);
void func_8024F83C(void);
void func_8024FB8C(void);
s32 func_8024FEEC(u8 arg0);
void func_8024FF34(void);
void func_80250170(u8 arg0, s32 arg1, s32 arg2);
void func_80250650(void);
void musicInstruments_init(void);

// --- core1/musicplayer.c ---
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
void coMusicPlayer_playMusicWeak(enum comusic_e track_id, s32 volume);
void func_8025A7DC(enum comusic_e);
void func_8025A8B8(enum comusic_e track_id, s32 arg1);
void func_8025A904(void);
void func_8025A9D4(void);
void func_8025AB00(void);

// --- core1/audio_soundplayer.c ---
bool func_802445C4(void *bank, s16 arg1);
s32 func_802445AC(void *arg0);
void * func_80244608(void *bank, s16 arg1, struct46s *arg2);
void func_80244814(void *arg0);
void func_80244978(intptr_t arg0, s16 type, s32 arg2);
void func_80244A98(s32 arg0);

// --- core1/io/ba_motor.c ---
void baMotor_80250C08(void);
void baMotor_80250FC0(void);
void baMotor_init(void);

// --- core1/collision.c ---
int collisionTri_isHitFromAbove_actor(f32 arg0[3], Actor *arg1, s32 arg2);
int collisionTri_isHitFromAbove_marker(f32 position[3], ActorMarker *marker, s32 verticalOffset);
s32 func_8024559C(f32 arg0[3], intptr_t *arg1, f32 *arg2);
void collisionTri_copy(BKCollisionTriangle *dst, BKCollisionTriangle *src);
void func_802450DC(f32 arg0[3], f32 arg1[3], f32 arg2[3], f32 arg3[3], f32 arg4[3]);
void func_802451A4(f32 arg0[3], f32 arg1[3], f32 arg2[3], f32 arg3[3], f32 arg4[3], s32 arg5);

// --- core1/debugtext.c ---
s32 gcdebugText_isThreadLocked(void);
void func_80247F24(s32 arg0, s32 arg1);
void func_80247F9C(s32 arg0);
void gcdebugText_pauseThread(void);

// --- core1/defragmanager.c ---
void defragManager_free(void);
void defragManager_init(void);

// --- core1/math/mlmtx.c ---
void _guMtxF2L(float mf[4][4], Mtx *m);

// --- core1/inflate.c ---
int bk_inflate(void);

// --- core1/initthread.c ---
void initThread_create(void);

// --- core1/memory.c ---
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

// --- core1/ml.c ---
void func_802596AC(f32 a0[3], f32 a1[3], f32 a2[3], f32 a3[3]);

// --- core1/mlmtx.c ---
void func_802515D4(f32 arg0[3][3]);

// --- core1/overlaymanager.c ---
// overlayManager_* functions are declared in core1/core1.h

// --- core1/sns.c ---
void snspayload_finalise_outgoing_payload(struct SnsPayload *payload);
void snspayload_rewind_outgoing(void);

// --- core1/stopnswop.c ---
bool DEBUG_use_special_bootmap(void);
void sns_backup_items_and_unlock_all(void);
void sns_find_and_parse_payload(void);
void sns_init_base_payloads(void);
void sns_restore_backed_up_items(void);
void sns_save_and_update_global_data(void);
void sns_set_item_and_update_payload(enum StopNSwop_Item item, s32 set, s32 state);
void sns_write_payload_over_heap(void);

// --- unused/dummy_overlay_callbacks.c ---
void dummy_func_8025AFB0(void);
void dummy_func_8025AFB8(void);
void dummy_func_8025AFC0(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- core2/abilityprogress.c ---
int ability_hasLearned(enum ability_e);
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

// --- core2/actor_array.c ---
void func_8032728C(f32[3], f32, s32, int(*)(Actor *));
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
int func_80329210(Actor * arg0, f32 (* arg1)[3]);
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
s32 func_80329904(ActorMarker *arg0, s32 arg1, f32 *arg2);
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

// --- core2/actor_cubebounds.c ---
bool func_80305248(f32 arg0[3], s32 arg1, f32 *arg2);
bool func_8030526C(f32 arg0[3], s32 arg1, f32 *arg2);
bool func_80305290(bool (* arg0)(NodeProp *), bool (* arg1)(Prop *));
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
void nodeprop_getPosition_s32(NodeProp *nodeProp, s32 dst[3]);
void spawnableActorList_free(void);
void spawnableActorList_new(void);

// --- core2/actor_cubepropsystem.c ---
s32 actor_getAnimatedTexturesCacheId(Actor *actor);
s32 codeA5BC0_getNodePropBit6(NodeProp *arg0);
s32 codeA5BC0_getNodePropUnk8(NodeProp *arg0);
s32 codeA5BC0_getNodePropUnkA(NodeProp *arg0);
s32 codeA5BC0_getNodePropUnkC(NodeProp *arg0);
s32 func_8032E49C(Cube *cube, enum actor_e *actor_id_list, NodeProp **node_list, s32 node_list_capacity);
s32 func_8032E5A8(Cube *cube, s32 arg1, f32 (*arg2)[3], s32 capacity);
s32 func_8032F170(Cube **arg0, void **arg1);
void code7AF80_initCubeFromFile(File *file_ptr, Cube *cube);
void codeA5BC0_getActorPosition(ActorProp *prop, s32 dst[3]);
void codeA5BC0_setNodePropUnkC(NodeProp *arg0, s32 arg1);
void code_A5BC0_initCubePropActorProp(Cube*);
void cube_free(Cube *cube);
void cube_sortAbsolute(Cube *cube);
void cube_sortRelative(Cube *cube);
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
void func_803300C0(ActorMarker *marker, s32 (*method)(ActorMarker *, ActorMarker *));
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

// --- core2/actor_motioncurve.c ---
s32 func_80296560(void);
void func_802964B8(void);
void func_8029656C(f32 dst[3]);
void func_80296C30(void);
void func_80296CA8(ActorMarker *arg0);
void func_80296CB4(s32 arg0);
void func_80296CC0(f32 arg0[3]);

// --- core2/scorequeue/queue_timers.c ---
bool func_802FCD98(struct8s *arg0);

// --- core2/ambient_fish.c ---
void func_803500D8(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_803500E8(void);
void func_80350174(void);
void func_80350250(void);

// --- core2/anim/anim_bonetransform.c ---
bool codeB3A80_releaseSprite(void **sprite_ptr, BKSpriteDisplayData **arg1);
bool func_8033B388(BKSprite **sprite_ptr, BKSpriteDisplayData **arg1);
s32 code_B3A80_func_8033BDAC(enum asset_e id, void *dst, s32 size);
s32 func_8033B678(void);
void assetCache_free(void *arg0);
void assetCache_init(void);
void assetcache_update_ptr(void * arg0, void* arg1);
void func_8033B5FC(void);
void func_8033B61C(void);
void func_8033B788(void);
void func_8033BD6C(void);
bool func_8033BD8C(void* arg0);

// --- core2/anim/anim_buffer.c ---
void anim_802897D4(AnimMtxList **this_ptr, BKAnimationList *arg0, Animation *dst);
void anim_drawSetup(Animation *self);
void anim_release(Animation *self);
void anim_resetNow(Animation *self);
void anim_resetSmooth(Animation *self);
void anim_setIndex(Animation *self, enum asset_e arg1);
void anim_update(Animation *self);

// --- core2/anim/anim_cache.c ---
bool animCache_inUse(s16 index);
s16 animCache_getNew(void);
void animCache_defrag(void);
void animCache_flushAll(void);
void animCache_free(void);
void animCache_init(void);
void animCache_release(s16 index);
void animCache_update(void);

// --- core2/anim/anim_sequencehandler.c ---
void func_80361DC4(Actor *self);
void func_80361E10(Actor *self);
void func_80361E9C(Actor *self);
void func_80361EE0(Actor *self);

// --- core2/anim/anim_texturecache.c ---
bool AnimTextureListCache_tryGetTextureOffset(s32 list_index, s32 texture_index, s32 *current_frame);
s32 AnimTextureListCache_newList(void);
void AnimTextureListCache_setAnimTextureList(s32 arg0, BKAnimTextureList *bk_anim_texture_list);
void AnimTextureListCache_free(void);
void AnimTextureListCache_freeList(s32 arg0);
void AnimTextureListCache_init(void);
void AnimTextureListCache_update(void);

// --- core2/anim/bonetransformlist.c ---
void boneTransformList_getBoneScale(BoneTransformList *self, s32 bone_id, f32 scale[3]);
void boneTransformList_setBoneScale(BoneTransformList *self, s32 bone_id, f32 scale[3]);
void func_8033A57C(BoneTransformList *self, s32 bone_id, f32 arg2[4]);
void func_8033A6B0(BoneTransformList *self, s32 bone_id, f32 arg2[3]);
void func_8033A8F0(BoneTransformList *self, s32 bone_id, f32 arg2[4]);
void func_8033A968(BoneTransformList *self, s32 bone_id, f32 arg2[3]);

// --- core2/anim_sequence.c ---
void anSeq_free(void **ppAnSeq);
void anSeq_setActivationFrameDelay(void **ppAnSeq, s32 arg1);
void anSeq_update(void **ppAnSeq, AnimCtrl *pAnCtl);

// --- core2/sfx/instruments.c ---
bool func_803354EC(enum sfx_e sfx_id);
bool func_80335520(s32 arg0);
intptr_t func_8033531C(enum sfx_e uid, struct46s *arg1);
intptr_t func_80335354(int uid, struct46s *arg1);
void func_803353BC(intptr_t arg0, u16 arg1);
void func_803353F4(intptr_t arg0, s32 arg1);
void func_80335418(intptr_t arg0, s32 arg1);
void sfxInstruments_init(void);

// --- core2/ba/ba_anim.c ---
void baAnim_defrag(void);
void baAnim_free(void);
void baAnim_init(void);
void baAnim_update(void);
void baanim_80289F30(void);
enum baanim_update_type_e baanim_getUpdateType(void);
void baanim_setModifyMethod(void (*arg0)(uintptr_t, uintptr_t));
void baanim_setUpdateType(enum baanim_update_type_e arg0);

// --- core2/ba/ba_animcache.c ---
void animBinCache_free(void);
void animBinCache_init(void);
void animBinCache_update(void);

// --- core2/ba/ba_animstate.c ---
void func_8029DD6C(void);
void func_8029DFF8(void);
void func_8029E058(bool);
void func_8029E064(bool);
void func_8029E070(bool);
void func_8029E0DC(bool);
void func_8029E0E8(bool);
void func_8029E0F4(bool);
void func_8029E100(void);

// --- core2/ba/ba_bounds.c ---
void babounds_init(void);
void babounds_update(void);

// --- core2/ba/ba_buzz.c ---
void babuzz_release(void);
void babuzz_reset(void);

// --- core2/ba/ba_carriedobj.c ---
void bacarriedobj_dec(enum actor_e actor_id);
void bacarriedobj_displayOnHud(enum actor_e actor_id);
void bacarriedobj_inc(enum actor_e actor_id);
void bacarriedobj_spawn(enum actor_e actor_id);
void func_8028DEEC(enum actor_e actor_id, Actor *actor);

// --- core2/ba/ba_carry.c ---
void bacarry_end(void);
void bacarry_init(void);
void bacarry_reset_marker(void);
void bacarry_set_marker(ActorMarker *arg0);
void bacarry_update(void);

// --- core2/ba/ba_scripted.c ---
enum bs_e badrone_802926E8(void);
enum bs_e badrone_enter(void);
enum bs_e badrone_look(void);
enum bs_e badrone_transform(void);
enum bs_e badrone_vanish(void);
void badrone_get_position_and_duration(f32 position[3], f32 *duration);
void badrone_goto_end(void);
void badrone_init(void);

// --- core2/ba/ba_eyeblink.c ---
s32 baiFrame_getState(void);
void baiFrame_reset(void);
void baiFrame_start(void);
void baiFrame_update(void);

// --- core2/ba/ba_eyeblink_data.c ---
void func_80290070(void);
void func_802900B4(void);
void func_802900D8(void);
void func_802900FC(void);
void func_80290108(void);

// --- core2/ba/ba_eyemouth_data.c ---
void func_802C7318(Actor *actor);
void func_802C7478(Actor *actor);
void func_802C75A0(Actor *actor, s32 arg1);
void func_802C79C4(void);

// --- core2/ba/ba_eyes.c ---
void baeyes_close(void);
void baeyes_open(void);
void baeyes_reset(void);
void baeyes_update(void);

// --- core2/ba/ba_falldamage.c ---
s32 bafalldamage_get_damage(s32 *damage);
s32 bafalldamage_get_state(void);
void bafalldamage_init(void);
void bafalldamage_set_state(s32 arg0);
void bafalldamage_update(void);

// --- core2/ba/ba_falling.c ---
void func_80350818(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_80350BC8(void);
void func_80350BFC(void);
void func_80350CA4(void);

// --- core2/ba/ba_groundsurface.c ---
void freeHazardSfxId(void);
void hazards_reset(void);
void hazards_update(void);

// --- core2/ba/ba_health.c ---
s32 func_802903CC(void);
void func_80290220(void);
void func_8029026C(s32 arg0, s32 arg1);
void func_8029028C(bool arg0);
void func_80290298(void);

// --- core2/ba/ba_hitbox.c ---
enum hitbox_e hitbox_getHitboxForActor(ActorMarker *arg0);

// --- core2/ba/ba_input.c ---
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

// --- core2/ba/ba_intensity.c ---
s32 func_80291660(void);
void func_80291590(void);
void func_802915B8(void);

// --- core2/ba/ba_key.c ---
int bakey_getAndSetState(s32 button_indx, s32 val);
int bakey_releaseCount(s32 button_indx);
int bakey_released(s32 button_indx);
void bakey_disableAll(s32 arg0);
void bakey_reset(void);
void bakey_update(void);

// --- core2/ba/ba_lookat.c ---
int balookat_try_get_position(f32 arg0[3]);
s32 balookat_getState(void);
void balookat_init(void);
void balookat_pop(void);
void balookat_push(s32 arg0);
void balookat_set_position(f32 arg0[3]);
void balookat_end(void);
void balookat_update(void);

// --- core2/ba/ba_lookdir.c ---
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
bool player_checkHazardInterrupt(s32 arg0);
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
void func_8028FA34(enum actor_e, Actor *);
void func_8028FA74(f32 dst[3]);
void func_8028FAB0(f32 arg0[3]);
u32 func_8028FB48(u32 mask);
void func_8028FB68(void);
void func_8028FC8C(f32 arg0[3]);
void func_8028FCAC(void);
void func_8028FCBC(void);
void player_setModelVisible(bool arg0);
void func_8028FCE8(void);
void player_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void player_getPosition_s32(s32 arg0[3]);
void player_getVelocity(f32 dst[3]);
void player_setIdealRotation(f32 rotation[3]);
void player_setRotation(f32 rotation[3]);

// --- core2/ba/ba_marker.c ---
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

// --- core2/ba/ba_model.c ---
void assetcache_release(void *);
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
enum player_model_direction_e baModel_getDirection(void);
void baModel_getPosition(f32* dst);
void baModel_reset(void);
void baModel_setEnvAlpha(s32 alpha);
void baModel_setPostDraw(void (*draw_func)(Gfx **gfx, Mtx **mtx, Vtx **vtx));
void baModel_setVisible(s32 arg0);
void baModel_update(void);
void baModel_updateModel(void);

// --- core2/ba/ba_modelselect.c ---
s32 func_802985F0(void);
void func_802986D0(void);
void func_80298700(void);

// --- core2/ba/ba_momentum.c ---
void func_8029065C(void);
void func_80290664(void);
void func_802906A4(s32 arg0);
void func_802906D8(void);

// --- core2/ba/ba_musicstate.c ---
s32 func_80321960(void);
void func_8032196C(void);
void func_803219A8(void);
void func_803219F4(s32 arg0);

// --- core2/ba/ba_position.c ---
void playerPosition_init(void);
void playerPosition_func_8029842C(void);
void playerPosition_func_80298464(f32 arg0[3]);
void playerPosition_func_80298504(f32 arg0[3]);
void playerPosition_getOffset(f32 arg0[3]);
void playerPosition_setOffset(f32 arg0[3]);
void playerPosition_applyOffset(void);
void player_setPosition(f32 arg0[3]);

// --- core2/ba/ba_recoil.c ---
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

// --- core2/ba/ba_sfx.c ---
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

// --- core2/ba/ba_sfxintensity.c ---
void cameraMode_update(void);
void func_80290B6C(void);
void func_80291488(s32 arg0);
void func_802914CC(s32 arg0);
void func_8029151C(s32 arg0);
void func_80291548(void);

// --- core2/ba/ba_state.c ---
void func_80295914(void);
void func_80295B04(void);
void func_80295C08(void (* arg0)(void));
void func_80295C14(void);
void func_80295D74(void);

// --- core2/ba/ba_statusflags.c ---
void func_80323100(void);
void func_80323120(void);
void func_80323140(s32 arg0, s32 arg1);
void func_80323170(void);
void func_80323190(void);
void func_803231E8(void);
void func_8032320C(void);
void func_80323230(void);
void func_80323238(void);

// --- core2/ba/ba_stick.c ---
void bastick_lockAtzero(bool arg0);
void bastick_reset(void);
void bastick_resetZones(void);
void bastick_update(void);

// --- core2/ba/ba_underwater.c ---
bool func_8029CF20(s32 arg0);
s32 func_8029CEB0(void);
void func_8029CF6C(void);

// --- core2/ba/ba_yaw.c ---
void func_802992F0(void);
void func_802993C8(void);
void func_8029957C(s32 arg0);

// --- core2/ba/ba_timer.c ---
s32 batimer_decrement(s32 timer_id);
void batimer_incrementBy(s32 id, f32 inc_value_sec);
void batimer_set(s32 timer_id, f32 duration);

// --- core2/bs/ant.c ---
int bsant_inSet(s32 move_indx);

// --- core2/bs/beak_barge.c ---
s32 bsbbarge_hitboxActive(void);

// --- core2/bs/beak_bomb.c ---
int bsbfly_bombHitboxActive(void);
int bsbfly_inSet(enum bs_e arg0);

// --- core2/bs/stilt_stride.c ---
int bslongleg_inSet(s32 move_indx);

// --- core2/bs/beak_peck.c ---
s32 bsbpeck_hitboxActive(void);

// --- core2/bs/beak_swim.c ---
bool bsbswim_inSet(enum bs_e move_id);

// --- core2/bs/talon_trot.c ---
int bsbtrot_inSet(s32 move_indx);

// --- core2/bs/beak_buster.c ---
s32 bsbbuster_hitboxActive(void);
s32 func_8029FC4C(void);
void func_802A02B4(s32 arg0);

// --- core2/bs/bee.c ---
void func_802A02C0(void);
void func_802A0340(void);

// --- core2/bs/bee_fly.c ---
int bsBeeFly_inSet(s32);

// --- core2/bs/bs_storedstate.c ---
enum transformation_e bsStoredState_getTransformation(void);
s32 bsStoredState_getTrotFlag(void);
void bsStoredState_8029A924(void);
void bsStoredState_clear(void);
void bsStoredState_clearTimers(void);
void bsStoredState_debug(void);
void player_setTransformation(enum transformation_e xform_id);

// --- core2/bs/carry.c ---
int bscarry_inSet(enum bs_e state);

// --- core2/bs/climb.c ---
int bsclimb_inSet(s32 move_indx);

// --- core2/bs/croc.c ---
int bscroc_hitboxActive(void);
int bscroc_ate_wrong_thing(void);
void bscroc_set_ate_wrong_thing(void);

// --- core2/bs/crouch.c ---
enum bs_e func_802ADCD4(enum bs_e arg0);

// --- core2/bs/scripted.c ---
void bsdrone_end(void);
void bsdrone_init(void);
void bsdrone_update(void);

// --- core2/bs/jiggy_dance.c ---
int bsjig_inJiggyJig(enum bs_e state);
void func_802B0CD8(void);

// --- core2/bs/jump.c ---
bool bsjump_jumpingFromWater(void);

// --- core2/bs/pumpkin.c ---
int bspumpkin_inSet(s32 move_indx);

// --- core2/bs/rebound.c ---
void func_802B35DC(void);
void func_802B360C(void);

// --- core2/bs/rest.c ---
void func_802B3A50(void);

// --- core2/bs/swim.c ---
bool bsswim_inset(enum bs_e state_id);

// --- core2/bs/walk.c ---
void func_802B6FA8(void);

// --- core2/bs/walrus.c ---
int bswalrus_inSledSet(enum bs_e state);

// --- core2/camera/camera_ease.c ---
void func_802C2250(void);
void func_802C2258(void);
void func_802C22C0(f32 target_position[3], f32 target_rotation[3]);

// --- core2/camera/camera_focus.c ---
void func_802BABD8(void);
void func_802BAC10(void);
void func_802BAC58(void);

// --- core2/camera/camera_fog.c ---
void func_802BEE2C(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_802BEF70(void);
void func_802BEF78(void);
void func_802BEFB0(void);

// --- core2/prop_assetcache.c ---
void propModelList_free(void);
void propModelList_init(void);
void propModelList_flush(s32 arg0);

// --- core2/camera/camera_motor1.c ---
s32 func_802BB294(void);
void func_802BAF20(void);
void func_802BAF40(void);

// --- core2/camera/camera_motor2.c ---
void func_802BB2A0(void);
void func_802BB2A8(void);
void func_802BB3C4(s32 arg0);
void func_802BB41C(s32 arg0);
void func_802BB4D8(f32 position[3], f32 rotation[3]);

// --- core2/camera/camera_nodemanager.c ---
s32 ncCamera_getType(void);
void camera_setType(enum camera_type_e camera_type);
void func_802BBD0C(Gfx **gdl, Mtx **mptr, Vtx **vptr);
void func_802BBD2C(f32 *arg0, f32 *arg1);
void nccamera_init(void);
void func_802BC10C(void);
void func_802BC21C(s32 arg0, s32 arg1);
void func_802BC280(void);
void func_802BC2CC(s32 arg0);
void func_802BC434(f32 arg0[3], f32 arg1[3], f32 arg2[3]);
void ncCamera_update(void);

// --- core2/sfx/trackmanager.c ---
int func_80322914(void);
s32 func_803226E8(enum map_e map_id);
s32 func_8032274C(void);
s32 func_80322758(void);
void func_80322764(void);
void func_8032278C(s32 arg0, s32 arg1);

// --- core2/camera/camera_position.c ---
void func_802BE940(void);

// --- core2/camera/camera_set.c ---
void func_803525A0(f32 arg0[3]);

// --- core2/camera/cameranode_type1.c ---
bool code336F0_func_802BA87C(PivotCameraNode *self);
bool code336F0_func_802BA89C(PivotCameraNode *self);
bool code336F0_func_802BA8BC(PivotCameraNode *self);
void code336F0_func_802BA7D8(PivotCameraNode *self, f32 arg1[3]);

// --- core2/camera/cameranode_type3.c ---
bool code33310_func_802BA4D0(ZoomCameraNode *self);
bool code33310_func_802BA4F0(ZoomCameraNode *self);

// --- core2/camera/cameranode_type4.c ---
s32 code33250_func_802BA234(RandomCameraNode *self);

// --- core2/camera/cameranodelist.c ---
s32 ncCameraNodeList_getNodeType(int camera_node_index);
s32 ncCameraNodeList_nodeIsValid(int camera_node_index);
void ncCameraNodeList_defrag();
void ncCameraNodeList_free();
void ncCameraNodeList_fromFile(File *file_ptr);
void ncCameraNodeList_init();

// --- core2/camera/dynamicCam1.c ---
void ncDynamicCam1_end(void);
void ncDynamicCam1_init(void);
void ncDynamicCam1_update(void);

// --- core2/camera/dynamicCam10.c ---
void ncDynamicCam10_end(void);
void ncDynamicCam10_init(void);
void ncDynamicCam10_update(void);

// --- core2/camera/dynamicCam11.c ---
void func_802BF798(s32 camera_node_index);
void ncDynamicCam11_end(void);
void ncDynamicCam11_init(void);
void ncDynamicCam11_update(void);

// --- core2/camera/dynamicCam12.c ---
void dynamicCam12_init(void);
void func_802C0F4C(void);
void ncDynamicCam12_end(void);
void ncDynamicCam12_update(void);

// --- core2/camera/dynamicCam13.c ---
void func_802C095C(void);
void ncDynamicCam13_end(void);
void ncDynamicCam13_init(void);
void ncDynamicCam13_update(void);

// --- core2/camera/dynamicCam3.c ---
void ncDynamicCam3_end(void);
void ncDynamicCam3_init(void);
void ncDynamicCam3_update(void);

// --- core2/camera/dynamicCam4.c ---
void ncDynamicCam4_end(void);
void ncDynamicCam4_func_802BFE74(bool);
void ncDynamicCam4_init(void);
void ncDynamicCam4_update(void);

// --- core2/camera/dynamicCam5.c ---
void ncDynamicCam5_end(void);
void ncDynamicCam5_func_802BF590(f32 arg0[3]);
void ncDynamicCam5_init(void);
void ncDynamicCam5_update(void);

// --- core2/camera/dynamicCam8.c ---
void ncDynamicCam8_end(void);
void ncDynamicCam8_func_802BF9B8(s32 arg0);
void ncDynamicCam8_init(void);
void ncDynamicCam8_update(void);

// --- core2/camera/dynamicCam9.c ---
void ncDynamicCam9_end(void);
void ncDynamicCam9_init(void);
void ncDynamicCam9_update(void);

// --- core2/camera/dynamicCamA.c ---
void ncDynamicCamA_end(void);
void ncDynamicCamA_init(void);
void ncDynamicCamA_update(void);

// --- core2/camera/dynamicCamB.c ---
void ncDynamicCamB_end(void);
void ncDynamicCamB_init(void);
void ncDynamicCamB_update(void);

// --- core2/camera/dynamicCamC.c ---
void ncDynamicCamC_end(void);
void ncDynamicCamC_init(void);
void ncDynamicCamC_update(void);

// --- core2/camera/dynamicCamD.c ---
void ncDynamicCamD_end(void);
void ncDynamicCamD_init(void);
void ncDynamicCamD_update(void);

// --- core2/camera/dynamicCamF.c ---
void ncDynamicCamF_end(void);
void ncDynamicCamF_init(void);
void ncDynamicCamF_update(void);

// --- core2/camera/dynamicCamera.c ---
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

// --- core2/camera/firstpersoncamera.c ---
s32 ncFirstPersonCamera_getState(void);
void ncFirstPersonCamera_getPositionAndRotation(f32 position[3], f32 rotation[3]);
void ncFirstPersonCamera_getZoomedInRotation(f32 dst[3]);
void ncFirstPersonCamera_reset(void);
void ncFirstPersonCamera_setState(enum nc_first_person_state state);
void ncFirstPersonCamera_setZoomedOutPosition(f32 src[3]);
void ncFirstPersonCamera_setZoomedOutRotation(f32 src[3]);

// --- core2/camera/randomCamera.c ---
void ncRandomCamera_end(void);
void ncRandomCamera_init(void);
void ncRandomCamera_update(void);

// --- core2/camera/staticCamera.c ---
void ncStaticCamera_end(void);
void ncStaticCamera_exit(void);
void ncStaticCamera_getPosition(f32 dst[3]);
void ncStaticCamera_init(void);
void ncStaticCamera_rotateToTarget(f32 target[3]);
void ncStaticCamera_setPositionAndRotation(f32 arg0[3], f32 arg1[3]);
void ncStaticCamera_setPositionAndTarget(f32 arg0[3], f32 arg1[3]);
void ncStaticCamera_setToNode(s32);
void ncStaticCamera_update(void);

// --- core2/ch/beeswarm.c ---
void func_802CEB60(Actor *self);

// --- core2/ch/bottlesbonus.c ---
f32 * chBottlesBonus_get_piece_distance_vec4f(s32 arg0);
s32 chBottlesBonus_getPuzzleIndex(void);
s32 chBottlesBonus_getState(void);
void chBottlesBonus_func_802DD158(Gfx **gfx, Mtx** mtx);
void chBottlesBonus_func_802DEA74(s32 arg0);
void chBottlesBonus_func_802DEA8C(s32 arg0, s32 arg1);
void chBottlesBonus_resetCompleted(void);
void chBottlesBonus_lose(u8 *arg0, enum asset_e text_id);
void chBottlesBonus_spawn(s32 arg0, s32 arg1);

// --- core2/ch/bottlesbonuscursor.c ---
ActorMarker * chBottlesBonusCursor_spawn(void);
bool chBottlesBonusCursor_isPuzzleCompleted(void);
f32 * chBottlesBonusCursor_func_802E0664(s32 indx);
f32 * chBottlesBonusCursor_func_802E068C(s32 indx);
s32 chBottlesBonusCursor_func_802E0588(s32 indx);
s32 chBottlesBonusCursor_func_802E06B4(void);
s32 chBottlesBonusCursor_getState(void);
void chBottlesBonusCursor_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void chBottlesBonusCursor_lose(void);

// --- core2/ch/drips.c ---
void func_80359A40(f32 position[3], void *arg1, s32 cnt);

// --- core2/ch/goldfeather.c ---
void func_802D8BE4(bool gold_feather);

// --- core2/ch/honeycomb.c ---
enum honeycomb_e func_802CA1C4(Actor *self);
void func_802CA1CC(enum honeycomb_e id);

// --- core2/ch/jiggy.c ---
enum jiggy_e chjiggy_getJiggyId(Actor *self);
void chjiggy_hide(Actor * self);
void chjiggy_setJiggyId(Actor *self, u32 id);

// --- core2/ch/jigsawdance.c ---
void chJigsawDance_setState(Actor * self, u32 arg1);

// --- core2/ch/mmm_veggies.c ---
void func_802CDAC4(Actor *);

// --- core2/ch/mole.c ---
int chmole_learnedAllLevelAbilities(enum level_e level);
int chmole_learnedAllSpiralMountainAbilities(void);

// --- core2/ch/overlaycopyright.c ---
void chOverlayCopyright_func_802DCB0C(s32 arg0, s32 arg1);
void chOverlayCopyright_spawn(s32 arg0, s32 arg1);

// --- core2/ch/overlaynocontroller.c ---
void chOverlayNoController_func_802DD040(s32 arg0, s32 arg1);
void chOverlayNoController_spawn(s32 arg0, s32 arg1);

// --- core2/ch/overlaypressstart.c ---
void chOverlayPressStart_func_802DCDB0(void);
void chOverlayPressStart_func_802DCDC0(NodeProp *arg0, ActorMarker *arg1);
void chOverlayPressStart_spawn(NodeProp *arg0, ActorMarker *arg1);

// --- core2/ch/soundsource.c ---
void func_802D09B8(Actor *self, s32 arg1);

// --- core2/ch/trainers.c ---
bool chtrainers_canUse(Actor *self);
void chtrainers_pickup(Actor *self);

// --- core2/ch/wadingboots.c ---
bool chwadingboots_802D6E0C(Actor *self);
void chwadingboots_802D6E54(Actor *self);

// --- core2/climb.c ---
u8 func_8029825C(void);
void climbClear(void);
void climbRelease(void);
void func_80298344(void);

// --- core2/bundle.c ---
bool func_802C9C14(Actor *actor);
void bundle_free(void);
void bundle_reset(void);
void bundle_update(Actor *actor);

// --- core2/scorequeue/dispatch.c ---
void func_802FB020(struct8s *self, s32 arg1);
void func_802FB104(s32, struct8s *);
void func_802FB15C(s32 arg0, struct8s * arg1);
void func_802FB194(s32 arg0, struct8s * arg1);
void func_802FB1CC(void);

// --- core2/scorequeue/manager.c ---
bool func_802FADD4(enum item_e item_id);
s32 func_802FAD9C(enum item_e item_id);
s32 itemPrint_getValue(s32 item_id);
void func_802FA69C(void);
void func_802FAC3C(void);
void code_73640_printItemCount(enum item_e item_id);
void func_802FAFAC(enum item_e item_id, enum comusic_e music_id);
void func_802FAFC0(enum item_e item_id, enum comusic_e music_id);
void func_802FAFD4(enum item_e item_id, enum sfx_e sfx_id);
bool func_802FAFE8(enum item_e item_id);
void itemPrint_draw(Gfx **gdl, Mtx ** mptr, Vtx **vptr);
void itemPrint_free(void);
void itemPrint_init(void);
void itemPrint_reset(void);
void itemPrint_update(void);

// --- core2/collision/cachesize.c ---
void func_8033DC04(void);
void func_8033DC10(void);
void func_8033DC18(void);

// --- core2/collision/climbsurface.c ---
bool floor_isCurrentFloorunk59(void);
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
s32 func_80294684(void);
void func_80294750(void);

// --- core2/collision/filter.c ---
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

// --- core2/collision/funcs.c ---
bool func_803209F8(f32 arg0[3], f32 arg1[3], f32 *arg2, f32 arg3[3]);
void func_80320B7C(void);
void func_80320B84(void);

// --- core2/collision/hitboxdata.c ---
bool func_8033D410(ActorMarker *arg0, ActorMarker *arg1);
s32 collision_getPlayerInteraction(CollisionParams *arg0);
s32 collision_getUnkBit7(CollisionParams *arg0);
s32 collision_getDamageToPlayer(CollisionParams *arg0);
s32 collision_getHitsToTrigger(CollisionParams *arg0);
s32 collision_getDropBundleNum(CollisionParams *arg0);
void func_8033D2F4(void);

// --- core2/collision/init.c ---
void func_8031B710(void);
void func_8031B718(void);
void func_8031B790(void);
void func_8031B990(s32 red, s32 blue, s32 green);

// --- core2/collision/raycast.c ---
s32 func_8031C59C(struct0 *self);
s32 func_8031C5A4(struct0 *self);
u8 func_8031C594(struct0 *self);
void func_8031BA7C(struct0 *self);
void func_8031BA9C(struct0 *self);
void func_8031C44C(struct0 *arg0);
void func_8031C608(struct0 *self);
void func_8031C618(struct0 *self, f32 *arg1);
void func_8031C638(struct0 *self, s32 arg1);

// --- core2/collision/spherecast.c ---
s32 func_8032CA80(Actor *actor, s32 arg1);
void func_8032C9E0(f32 arg0[3]);

// --- core2/collision/tricheck.c ---
void func_80344C50(void);
void func_80344C80(void);
void func_80344E7C(u8 index, f32 dst[3]);
void func_803451B0(u8 index, f32 arg1[3]);

// --- core2/crc_bootvalidation.c ---
void func_80356714(void);
void codeCF5F0_forgetAllAbilitiesExceptClawSwipeIfChecksumsFail(void);

// --- core2/cutscene/animated.c ---
void func_8035D490(ActorMarker *marker);
void func_8035D4F0(ActorMarker *marker, s32 arg1);

// --- core2/cutscene/ctrl.c ---
s32 func_802C0190(void);
void func_802C0120(void);
void func_802C0148(void);
void func_802C0150(s32 arg0);
void func_802C02D4(f32 arg0[3]);

// --- core2/cutscene/lair.c ---
s32 cutscenetrigger_update(void);
void func_8031CC8C(NodeProp *arg0, s32 arg1);
void func_8031CC40(enum map_e map_id, s32 arg1);
void func_8031D04C(enum map_e arg0, s32 exit_id);
void func_8031D06C(enum map_e arg0, s32 arg1);
void func_8031D0C0(NodeProp *arg0, ActorMarker *arg1);
void func_8031F9E0(void);
void func_8031F9E8();
void func_8031F9F4(s32 arg0);
void clearScoreStates(void);
void debugScoreStates(void);
void warp_mmmEnterLoggo(NodeProp *arg0, ActorMarker *arg1);

// --- core2/cutscene/nodeupdate.c ---
s32 func_8033451C(s32 arg0);
s32 func_80334524(s32 arg0);
s32 func_8033452C(s32 arg0);
void func_803343AC(void);
void func_803343D0(void);
void func_803343F8(s32 indx);
void func_80334428(void);
void func_80334448(NodeProp *arg0, ActorMarker *arg1);

// --- core2/demo_input.c ---
int demo_readInput(OSContPad* arg0, s32* arg1);
void demo_free(void);
void demo_load(enum map_e map, s32 demo_id);

// --- core2/dialog/binload.c ---
bool func_8031B604(u8 *arg0);
s32 code94620_func_8031B5B0(void);
s32 func_8031B5BC(void);
void func_8031B5C4(s32 arg0);
void dialogBin_initialize(void);
void dialogBin_release(s32 arg0);
void dialogBin_terminate(void);
void dialogBin_update(void);
void func_8031B62C(void);

// --- core2/dialog/progress_dialogs.c ---
void progressDialog_showDialogMaskZero(enum file_progress_e progress_flag);
void volatileFlag_setAndTriggerDialog_4(enum volatile_flags_e arg0);
void volatileFlag_setAndTriggerDialog_E(enum volatile_flags_e arg0);

// --- core2/inventory_slots.c ---
void func_803246F0(u8* self, s32 indx);
void func_80324700(u8* self);
void func_80324770(u8* self, s32 indx, s32 value);
void func_8032477C(u8 *self);

// --- core2/fileselect.c ---
bool gameFile_anyNonEmpty(void);
bool gameFile_isNotEmpty(s32 gamenum);
s32 gameFile_8033CFD4(s32 gamenum);
void gameFile_8033CE40(void);
void gameFile_clear(s32 gamenum);
void gameFile_load(s32 gamenum);
void gameFile_save(s32 gamenum);

// --- core2/font/print.c ---
void print_free(void);
void print_resetBoldFontTexture(void);
void print_init(void);
void print_freeBoldLetterFont(void);
void text_setNormalTextColor(s32 arg0, s32 arg1, s32 arg2);
void printbuffer_defrag(void);
void printbuffer_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- core2/font/render.c ---
void func_802E5F10(Gfx **gdl);
void func_802E5F38(void);
void func_802E5F68(void);
void func_802E67AC(void);
void func_802E67C4(void);
void func_802E6820(s32 arg0);

// --- core2/frame/bufferreadback.c ---
bool func_802E4424(void);
bool func_802E4A08(void);
s32 func_802E4A98(s32 arg0);
s32 func_802E4AAC(s32 arg0);
s32 func_802E4AC0(s32 arg0);
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

// --- core2/frame/rendermem.c ---
void dummy_func_802E35D0(void);
void func_802E3580(void);
void func_802E35D8(void);

// --- core2/fx/airscore.c ---
s32 fxairscore_count_to_time(s32 count);
s32 fxairscore_time_to_count(s32 time);

// --- core2/fx/common2score.c ---
enum item_e func_802FDD0C(struct8s *arg0);

// --- core2/fx/effect_colordata.c ---
bool vec4f_isAlmostZero(f32 arg0[4]);
bool vec4f_isZero(f32 arg0[4]);
void func_80345274(f32 arg0[4], f32 arg1[3][3]);
void func_80345C78(f32 arg0[4], f32 arg1[3]);
void func_80345CD4(f32 arg0[4], f32 arg1[4]);
void func_80345D30(f32 arg0[4], f32 arg1[4], f32 arg2[4]);

// --- core2/fx/effect_eggshatter.c ---
void eggShatter_defrag(void);
void eggShatter_draw(Gfx **gPtr, Mtx **mPtr, Vtx **vPtr);
void eggShatter_free(void);
void eggShatter_init(void);
void eggShatter_new(f32 position[3]);
void eggShatter_update(void);

// --- core2/fx/effect_enemyrender.c ---
Actor * func_802DF160(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_802DF270(void);

// --- core2/fx/effect_modelparticle.c ---
void func_802DAC84(ParticleEmitter *pCtrl, Actor *self, enum asset_e model_id);
void func_802DAD08(ParticleEmitter *pCtrl, Actor *self, enum asset_e model_id);
void func_802DAD8C(ParticleEmitter *pCtrl, Actor *self, enum asset_e model_id);
void humanoidBaddie_enterInvulnerableState(ActorMarker *marker, s32 arg1);
void humanoidBaddie_update(Actor *self);

// --- core2/fx/effect_particle.c ---
void func_802DF090(s32 arg0, s32 arg1);
void func_802DF0C8(void);
void func_802DF11C(s32 arg0, s32 arg1);

// --- core2/fx/effect_playerspray.c ---
void func_802929F8(void);

// --- core2/fx/effect_simplesprite.c ---
void func_802DC110(f32 *position, enum asset_e sprite_id);

// --- core2/fx/effect_spriteanim.c ---
void func_802DC9A4(s32 arg0, s32 arg1);
void func_802DC9DC(s32 arg0, s32 arg1);

// --- core2/fx/honeycarrierscore.c ---
void gcpausemenu_80314AC8(int arg0);

// --- core2/fx/ripple.c ---
void fxRipple_802F3554(s32 arg0, f32 position[3]);
void fxRipple_802F3584(s32 arg0, f32 position[3], uintptr_t arg2);
void fxRipple_free(void);
void fxRipple_init(void);

// --- core2/fx/score_honeycomb.c ---
s32 honeycombscore_get_level_total(enum level_e level_id);
s32 honeycombscore_get_total(void);
void honeycombscore_clear(void);
void honeycombscore_debug(void);
void honeycombscore_getSizeAndPtr(s32 *size, u8 **addr);
void honeycombscore_set(enum honeycomb_e indx,  bool val);

// --- core2/fx/score_jiggy.c ---
s32 jiggyscore_leveltotal(s32 lvl);
s32 jiggyscore_total(void);
void * jiggyscore_clearAllSpawned(void);
void jiggyscore_clearAll(void);
void jiggyscore_debug(void);
void jiggyscore_getSizeAndPtr(s32 *size, u8 **addr);
void jiggyscore_setCollected(s32 indx,  s32 val);
void jiggyscore_setSpawned(s32, s32);

// --- core2/fx/score_jiggylist.c ---
void codeABC00_spawnJiggyAtLocation(enum jiggy_e, f32[3]);
void func_80332E08(void);
void func_8033301C(void);
void func_80333270(enum jiggy_e jiggy_id, f32 position[3], void (*method)(Actor *, ActorMarker *), ActorMarker *other_marker);
void func_80333388(enum jiggy_e jiggy_id);
void func_803333DC(Struct81s *arg0, Actor *arg1);
void jiggylist_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void jiggylist_map_actors(void);
void jiggylist_set_level(enum map_e map_id);

// --- core2/fx/score_mumbo.c ---
bool mumboscore_get(enum mumbotoken_e indx);
void mumboscore_clear(void);
void mumboscore_debug(void);
void mumboscore_getSizeAndPtr(s32 *size, u8 **addr);
void mumboscore_set(enum mumbotoken_e indx,  bool val);

// --- core2/fx/sparkle.c ---
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

// --- core2/gameSelect.c ---
s32 gameSelect_getGameNumber(void);
void gameSelect_saveAndExit(void);
void gameSelect_setGameNumber(s32 arg0);
void gameSelect_resetGameNumber(void);

// --- core2/game_complete.c ---
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

// --- core2/gc/dialog.c ---
int func_803110F8(s32 next_state, s32 arg1, s32 arg2, s32 arg3, void (*arg4)(ActorMarker *, enum asset_e, s32));
int gcdialog_showDialogConditional(s32 text_id, s32 arg1, f32 *pos, ActorMarker *marker, void(*callback)(ActorMarker *, enum asset_e, s32), void(*arg5)(ActorMarker *, enum asset_e, s32), s32(*arg6)(ActorMarker *, s32, s32));
int gcdialog_getCurrentTextId(void);
int func_803115C4(s32 next_state);
void func_8030F1D0(void);
void func_80310D2C(void);
void gcdialog_incrementYPositionModifier(void);
void gcdialog_decrementYPositionModifier(void);
void func_80311714(int next_state);
void gcdialog_defrag(void);
void gcdialog_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void gcdialog_init(void);

// --- core2/gc/parade.c ---
int gcparade_8031B4CC(void);
int gcparade_8031B4F4(void);
void func_8031B010(void);
void gcparade_8031ABA0(void);
void gcparade_8031ABF8(void);
void gcparade_beginFFParade(void);
void gcparade_free(void);
void gcparade_init(void);
void gcparade_update(void);

// --- core2/gc/pauseMenu.c ---
int gcpausemenu_80314B00(void);
s32 gcPauseMenu_update(void);
void gcpausemenu_80314B24(void);
void gcpausemenu_defrag(void);
void gcpausemenu_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void gcpausemenu_free(void);
void gcpausemenu_init(void);
void gcpausemenu_returnToLair(void);

// --- core2/gc/section.c ---
enum level_e map_getLevel(enum map_e map);
enum map_e level_get_main_map(enum level_e level_id);
s32 level_get_main_exit(enum level_e level_id);
void func_8030AFA0(enum map_e arg0);
void func_8030AFD8(s32 arg0);

// --- core2/gc/sky.c ---
void sky_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void sky_free(void);
void sky_reset(void);
void sky_update(void);

// --- core2/gfx/displaylistdata.c ---
void func_8030C180(void);
void func_8030C1A0(void);
void func_8030C204(void);
void func_8030C2D4(Gfx **gdl, Mtx **mptr, Vtx **vptr);
void scissorBox_setSmall(void);

// --- core2/gfx/displaylistinit.c ---
void func_80315084(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_80315110(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_803151D0(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- core2/spline_pathfollow.c ---
void func_80344040(Actor *self);
int func_80343D50(Actor *self, s32 arg1, s32 arg2, s32 arg3);
s32 func_80341C78(s32 arg0[3]);
s32 func_80341D5C(s32 arg0[3], s32 arg1[3]);
s32 func_80341F2C(s32 arg0);
s32 func_80343654(Actor *self);
void func_803411B0(void);
void func_80341A54(void);
void glspline_defrag(void);

// --- core2/level/lightconfig.c ---
// code35520_getDistanceVectors and code35520_selectTable are declared in core2/core2.h

// --- core2/level/metadata.c ---
int barebound_set_active(s32 arg0);
s32 barebound_802987B4(void);

// --- core2/level/specificflags.c ---
s32 levelSpecificFlags_getN(s32 i, s32 n);
s32 levelSpecificFlags_getSet(s32 arg0, s32 arg1);
s32 levelSpecificFlags_validateCRC1(void);
s32 levelSpecificFlags_validateCRC2(void);
void levelSpecificFlags_clear(void);
void levelSpecificFlags_setN(s32 index, s32 val, s32 n);

// --- core2/load_zone.c ---
void codeA960_forceLoadzone(s32);
void func_80291910(void);
void func_802919A0(void);

// --- core2/map/audioconfig.c ---
s32 func_803246B4(enum map_e map_id, s32 arg1);

// --- core2/map/exit.c ---
bool func_8034BB48(void);
s32 func_8034BAFC(void);
s32 func_8034BDA4(enum map_e map_id, s32 exit_id);
void func_8034B8C0(enum map_e map_id, s32 demo_id);
void func_8034B940(void);
void func_8034B968(void);
void func_8034B9BC(s32 arg0);
void func_8034B9E4(void);
void func_8034BA7C(enum map_e map_id, s32 exit_id);

// --- core2/map/model.c ---
// NOTE: func_802E76B0, collisionList_func_802E805C, func_802E8E88, func_802E9118,
// func_802E92AC, func_802E9DD8 return BKCollisionTriangle* but have conflicting
// local externs (bool/s32/void) in decomp source files. Files that need
// the pointer type already have correct local externs.
BKCollisionTriangle *func_802E76B0(BKCollisionList *collisionList, BKVertexList *vertexList, f32 startPoint[3], f32 endPoint[3], f32 arg4[3], u32 flagFilter);
Vec3fArray *func_803097A0(void);
BKCollisionTriangle *func_80309B48(f32 startPoint[3], f32 endPoint[3], f32 arg2[3], u32 flagFilter);
bool func_80309D58(f32 arg0[3], s32 arg1);
bool mapModel_has_xlu_bin(void);
void func_8030A078(void);
void mapModel_defrag(void);
void mapModel_free(void);
void mapModel_getBounds(s32 min[3], s32 max[3]);
f32  mapModel_getFloorY(f32[3]);
BKModel *mapModel_getModel(s32 arg0);
BKModelBin *mapModel_getModelBin(s32 arg0);
void mapModel_opa_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void mapModel_setEnvColor(s32 r, s32 g, s32 b);
void mapModel_xlu_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- core2/map/savestate.c ---
int game_is_frozen(void);

// --- core2/map/gsworld.c ---
enum map_e gsworld_getMap(void);

// --- core2/map/specificflags.c ---
s32 mapSpecificFlags_validateCRC1(void);
u32 mapSpecificFlags_getClear(s32 i);
u32 mapSpecificFlags_getN(s32 idx, s32 n);
void mapSpecificFlags_clearAll(void);
void mapSpecificFlags_setN(s32 idx, s32 val, s32 n);

// --- core2/map_list.c ---
void func_8029A47C(s32 arg0[3]);
void func_8029A4D0(void);
void func_8029A54C(void);
void func_8029A554(void);

// --- core2/model/matrixrotate.c ---
void func_8033F9C0(void);
void func_8033FA24(void);

// --- core2/model/meshbounds.c ---
void func_8033F738(ActorMarker *arg0);
void func_8033F784(ActorMarker *arg0);
void func_8033F7A4(ActorMarker *arg0, BKVertexList *arg1);

// --- core2/model/render.c ---
void modelRender_setBoneTransformList(BoneTransformList *arg0);

// --- core2/model/rendernormal.c ---
void animVerticesList_transform(BKAnimVerticesList *arg0, BKVertexList *arg1, AnimMtxList *mtx_list);

// --- core2/mumbo_transforms.c ---
void chmumbo_func_802D1724(void);
void func_802D2CB8(void);
void func_802D2CDC(void);

// --- core2/overlay.c ---
// leveloverlay_* functions are declared in core2/core2.h

// --- core2/particle/bathroom.c ---
void func_8029ADA8(void);
void func_8029ADCC(void);
void func_8029AE1C(void);
void func_8029AE48(void);
void func_8029AE74(s32 arg0);
void func_8029AF1C(void);

// --- core2/particle/colordefault.c ---
bool dustEmitter_isActive(s32 arg0);
s32 dustEmitter_returnGiven(s32 arg0);
void dustEmitter_init(void);
void dustEmitter_free(void);

// --- core2/particle/emitter1.c ---
void func_802F3CB0(void);
void func_802F3CD4(void);
void func_802F3CF8(f32 arg0[3], s32 arg1, s32 arg2);

// --- core2/particle/emitter2.c ---
void func_802F3E50(void);
void func_802F3E74(void);

// --- core2/particle/initcallback.c ---
s32 commonParticleType_80352C7C(enum common_particle_e id);
void commonParticleType_init(void);
void commonParticleType_set(enum common_particle_e arg0, GenFunction_0 init_method, GenFunction_0 update_method, GenFunction_0 free_method, s32 arg4, s32 arg5);

// --- core2/particle/lifescale.c ---
void func_802F1858(void *arg0, Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_802F1884(void *arg0);
void func_802F1934(void *arg0, s32 arg1);

// --- core2/particle/particle.c ---
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
void particleEmitter_setParticleCallback(ParticleEmitter *self, void (*arg1)(ParticleEmitter *self, f32 pos[3]));

// --- core2/bs/player_spawn.c ---
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
void func_8029C7F4(s32 arg0, s32 yaw_state, s32 arg2, s32 arg3);
void code_14420_setVoidOutLocation(enum map_e map_id, s32 exit_id);
void func_8029C848(AnimCtrl *arg0);
void func_8029C984(void);
void func_8029CB84(void);
void func_8029CCC4(void);
void func_8029CDA0(void);
void update_void_return_Location(void);

// --- core2/particle/positionset.c ---
void func_803541C0(s32 arg0);
void func_803541CC(s32 arg0);

// --- core2/particle/samplerate.c ---
void func_802F4798(Struct5Ds *self);
void func_802F487C(Struct5Ds *self, void (*arg1)(Struct5Ds *, s32));
void func_802F4894(Struct5Ds *self, f32 arg1[3]);
void func_802F48B4(Struct5Ds *self, void (*arg1)(Struct5Ds *, s32));
void func_802F48BC(Struct5Ds *self);
void func_802F48E0(Struct5Ds *self);
void func_802F4900(Struct5Ds *self, s32 arg1);
void func_802F4924(Struct5Ds *self);
void func_802F4978(Struct5Ds *self);

// --- core2/particle/scale1.c ---
void func_802EDD20(void);
void func_802EDD44(void);

// --- core2/particle/spawn.c ---
void func_802F3FE4(f32 pos[3]);
void func_802F404C(void);
void func_802F4070(void);

// --- core2/particle/typeindex.c ---
void func_802F1E80(void);
void func_802F1EA4(void);

// --- core2/particle/velocityrange.c ---
void func_802F3300(void);

// --- core2/particle/velocityset.c ---
void func_802F4200(f32 arg0[3]);
void func_802F422C(void);
void func_802F4250(void);

// --- core2/cutscene_triggers.c ---
void func_803223AC(void);
void func_80322490(void);
void func_803224FC(void);
void func_803225B0(s32 arg0, s32 arg1);

// --- core2/pitch.c ---
void pitch_applyIdeal(void);
void pitch_reset(void);
void pitch_update(void);

// --- core2/playerutils.c ---
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

// --- core2/actor_gameover.c ---
void func_802DC528(NodeProp *arg0, ActorMarker *arg1);
void func_802DC560(NodeProp*, ActorMarker*);
void func_802DC604(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- core2/level_transition.c ---
int func_8032190C(void);
enum level_e level_get(void);
void func_803216D0(enum map_e map);
void func_80321854(void);
void func_80321918(int arg0);
void func_80321924(void);

// --- core2/quiz/bitfield.c ---
void quizQuestionAskedBitfield_defrag(void);
void quizQuestionAskedBitfield_free(void);
void quizQuestionAskedBitfield_init(void);

// --- core2/quiz/game.c ---
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
void func_getCameraViewFromLevel(enum map_e map_id, s32 arg1, bool arg2);
void func_802D520C(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void func_802D5628(void);
void func_802D6114(void);
void func_802D6344(void);
void func_802D63D4(void);
void func_802D6924(void);
void func_802D6948(void);

// --- core2/quiz/questionmanager.c ---
bool gcquiz_showQuestion(enum ff_question_type_e q_type, s32 q_index, s32 arg2, s32 arg3, s32 arg4, void (*arg5)(s32, s8));
bool gcquiz_isNotInInitialState();
s32 gcquiz_getLastIndexOfQuestionType(enum ff_question_type_e question_type);
void gcquiz_defrag();
void gcquiz_draw(Gfx **gfx, Mtx **mtx, Vtx **vtx);
void gcquiz_free();
void gcquiz_func_80319EA4(void);
void gcquiz_func_8031A48C(void);
void gcquiz_init();

// --- core2/quiz/storage.c ---
void code_C9E70_defrag(void);

// --- core2/rand.c ---
s32 sfx_randi2(s32 min, s32 max);
void rand_reset(void);
void rand_seed(s32 seed);
void rand_shuffle(void);
void rand_sync_to_sfx_rand(void);
void sfx_rand_sync_to_rand(void);

// --- core2/roll.c ---
void roll_applyIdeal(void);
void roll_reset(void);
void roll_update(void);

// --- core2/savedata.c ---
int savedata_8033CCD0(s32 filenum);
s32 savedata_8033CA2C(s32 filenum, void *save_data);
s32 savedata_8033CA9C(void *savedata);
void saveData_create(void *savedata);
void saveData_load(void *savedata);
void savedata_init(void);
void savedata_update_crc(void *buffer, s32 size);

// --- core2/sfx/filedata.c ---
void func_802F8FF0(void);
void func_802F8FFC(void);
void func_802F9054(void);
void func_802F90F4(void);
void func_802F9114(void);
void func_802F919C(void);
void func_802F962C(Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- core2/sfx/init.c ---
struct3s * func_802F7C7C(void);
struct6s * func_802F7C38(void);
void func_802F7CC0(void);
void func_802F7CE0(void);
void func_802F7D30(void);
void func_802F7D44(void);
void func_802F7DE4(void);
void func_802F7E54(void);

// --- core2/sfx/playstate.c ---
void func_802F8A70(struct6s *self);
void func_802F8A90(struct6s *self, Gfx **gdl, Mtx **mptr, Vtx **vptr);
void func_802F8B8C(struct6s *self);
void func_802F8C90(struct6s *self);
void func_802F8CB0(struct6s *self);
void func_802F8CD0(struct6s * self);

// --- core2/sfx/randompitch.c ---
int func_802F9C0C(s32 arg0);
void func_802F9C48(void);
void func_802F9CD8(void);
void func_802F9D38(s32 arg0);
void func_802F9EC4(s32 arg0, f32 *arg1, s32 arg2, s32 arg3);
void func_802F9F48(s32 arg0, s32 arg1);
void func_802FA028(s32 arg0, s32 arg1);
void func_802FA0B0(s32 arg0, s32 arg1);
void func_802FA4E0(void);
void func_802FA508(void);

// --- core2/sfx/sequenceindex.c ---
s32 func_8030C77C(void);
void func_8030C740(void);

// --- core2/sfx/source.c ---
int func_8030ED70(enum sfx_e uid);
s32 sfxSource_getSampleRate(u8 indx);
void func_8030D86C(void);
void func_8030D8A8(s32 arg0, s32 arg);
void func_8030D8DC(void);
void func_8030DCCC(u8, s32);
void sfxSource_setCallbackByIndex(u8 indx, void (*arg1)(u8));
void sfxSource_triggerCallbackByIndex(u8 indx);
void func_8030DFB4(u8 indx, s32 arg1);

// --- core2/sfx/streamctrl.c ---
void func_802F80F0(struct3s *self);
void func_802F8110(struct3s *self, Gfx **gdl, Mtx **mptr, u32 arg3);
void func_802F8214(struct3s * self);
void func_802F8338(struct3s *self);
void func_802F8358(struct3s *self);
void func_802F83AC(struct3s *arg0);

// --- core2/sfx/volume.c ---
void func_8034F6F0(Gfx **gdl, Mtx **mptr, s32 vptr);
void func_8034F734(void);
void func_8034F774(void);
void func_8034F918(void);

// --- core2/snacker_pool1.c ---
bool func_802FBE04(void);
void func_802FBB18(void);
void func_802FBDFC(void);

// --- core2/snacker_pool2.c ---
bool func_802FC390(void);

// --- core2/spawn_queue.c ---
void spawnQueue_flush(void);
void spawnQueue_free(void);
void spawnQueue_func_802C3A18(void);
void spawnQueue_lock(void);
void spawnQueue_malloc(void);
void spawnQueue_reset(void);
void spawnQueue_unlock(void);

// --- core2/spline_bezier.c ---

// --- core2/sprite/displaydata.c ---
s32 func_8033E8AC(void);
u8 func_8033E93C(void);
void commonParticle_init(void);
void commonParticle_freeAllParticles(void);
void commonParticle_update(void);
void commonParticle_freeParticleByActorMarker(ActorMarker *arg0);
void commonParticle_freeParticleByIndex(s32 arg0);
void commonParticle_stashCurrentIndex(void);
void commonParticle_applyIndexStash(void);
void commonParticle_setActive(s32 arg0, s32 arg1);

// --- core2/sprite/render.c ---
s32 codeBD100_getSpriteType(BKSpriteDisplayData *self);
void func_80344138(BKSpriteDisplayData *self, s32 frame, s32 mirrored, f32 position[3], f32 scale[3], Gfx **gfx, Mtx **mtx);
void func_80344720(BKSpriteDisplayData *arg0, s32 frame, bool mirrored, f32 position[3], f32 rotation[3], f32 scale[3], Gfx **gfx, Mtx **mtx);
void func_80344C2C(bool arg0);
void func_80344C38(void (*method)(ActorMarker *), ActorMarker *marker);

// --- core2/sprite/rendertex.c ---
void func_80347FC0(Gfx **gfx, BKSprite *sprite, s32 frame, s32 tmem, s32 rtile, s32 uls, s32 ult, s32 cms, s32 cmt, s32 *width, s32 *height);
void func_80348044(Gfx **gfx, BKSprite* sprite, s32 frame, s32 tmem, s32 rtile, s32 uls, s32 ult, s32 cms, s32 cmt, s32 *width, s32 *height, s32 *frame_width, s32 *frame_height, s32 *texture_x, s32 *texture_y, s32 *textureCount);
void func_80349AD0(void);
void func_80349B1C(Gfx **gfx);

// --- core2/sprite/screenoverlay.c ---
void codeAEDA0_postDrawSprite(Gfx **gfx);
void codeAEDA0_drawSprite(Gfx **gfx);
void func_803382B4(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void codeAEDA0_setSpriteDrawMode(s32 arg0);
void func_803382FC(s32 arg0);
void func_80338308(s32 arg0, s32 arg1);
void codeAEDA0_setPrimaryColorRGB(s32 r, s32 g, s32 b);
void func_80338370(void);
void func_8033837C(s32 arg0);
void spriteRender_draw(Gfx **gfx, Vtx **vtx, BKSprite *sp, u32 frame);

// --- core2/gamestate.c ---
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
void notescore_getSizeAndPtr(s32 *size, void **ptr);
void saveditem_getSizeAndPtr(s32 *size, u8 **buffer);
void timeScores_getSizeAndPtr(s32 *size, void **ptr);

// --- core2/texture/copy.c ---

// --- core2/timed_funcqueue.c ---
void func_80324C58(void);
void func_80324DBC(f32 time, enum asset_e text_id, s32 arg2, f32 position[3], ActorMarker *caller, void (*callback_method_1)(ActorMarker *, enum asset_e, s32), void (*callback_method_2)(ActorMarker *, enum asset_e, s32));
void timedFuncQueue_defrag(void);
void timedFuncQueue_flush(void);
void timedFuncQueue_free(void);
void timedFuncQueue_init(void);
void timedFuncQueue_update(void);

// --- core2/vtx/alphablend.c ---
s32 func_8034E698(Struct73s *arg0);

// --- core2/vtx/colorapply.c ---
void func_8034BF54(ActorMarker *marker);
void func_8034BFF8(ActorMarker *marker);
void func_8034C21C(ActorMarker *marker);

// --- core2/vtx/lighting.c ---
void lighting_free();
void lighting_init();
void lightingVectorList_fromFile(File *file_ptr);

// --- core2/vtx/listutils.c ---
void func_802E73C8(f32 arg0[3][3]);

// --- core2/vtx/positionset.c ---
s32 func_8034F560(Struct76s *arg0);

// --- core2/vtx/renderstart.c ---
bool func_8034DC80(Struct6Ds *arg0, f32 arg1[3]);
s32 func_8034DC78(Struct6Ds *arg0);
void func_8034E174(Struct6Ds *arg0);
void func_8034E254(Struct6Ds *arg0, void (*arg1)(Struct6Ds *));
void func_8034E25C(Struct6Ds *arg0, void (*arg1)(Struct6Ds *));
void func_8034E264(Struct6Ds *arg0, s32 arg1);

// --- core2/vtx/transform.c ---
int func_8034C4CC(void);
void * func_8034C9D4(void);
void func_8034C8D8(void);
void func_8034C97C(void);
void func_8034C9B0(int arg0);

// --- core2/vtx/list.c ---

// --- core2/yaw.c ---
void yaw_init(void);
void yaw_setUpdateState(s32 arg0);
void yaw_update(void);

// --- cutscenes/actor_spawninit.c ---
void cutscene_func_8038C4E0(void);

// --- FINALE/fight.c ---
void fight_addSpawnableActors(void);

// --- lair/actor_spawninit.c ---
void lair_func_8038A0C4(void);

// --- lair/ch/brentilda.c ---
void gzquiz_initGruntyQuestions(void);

// --- lair/ff_manager.c ---
s32 func_8038E800(void);
void func_8038E7C4(void);
void func_8038E968(s32 idx);
void lair_func_8038CD48(void);
void ff_init(void);
void lair_func_8038E0B0(void);
void lair_func_8038E768(Gfx **dl, Mtx **m, Vtx **v);

// --- LAIR/jigsawpicture.c ---
bool jigsawPicture_isJigsawPictureComplete(s32 arg0);

// --- port/stub.c ---
s32 osContSetCh(u8 ch);
u32 __osGetSR(void);
void osCreateThread(OSThread* thread, OSId id, void* entry, void* arg, void* sp, OSPri p);
void osDestroyThread(OSThread* thread);
void osDpSetStatus(u32 data);
void osSetThreadPri(OSThread* thread, OSPri p);
void osSpTaskYield(void);
void osStartThread(OSThread* thread);
void osStopThread(OSThread* t);

// --- unmapped (definition not found in src/) ---
s32 osPiReadIo(u32, u32 *);

// --- core2/ba/ba_stick.c ---
s32 bastick_getZone(void);

// --- core2/ba/ba_lookdir.c ---
void func_8028F94C(s32 arg0, f32 arg1[3]);

// --- core2/scorequeue/dispatch.c ---
s32 func_802FB0D4(void *self);

// --- core2/particle/particle.c ---
void particleEmitter_manualFree(ParticleEmitter *self);

// --- core2/anim/anim_bonetransform.c ---
void func_8033BD20(void **arg0);

// --- core2/timed_funcqueue.c ---
bool timedFuncQueue_is_empty(void);

// --- core2/fx/effect_colordata.c ---
void vec4f_clone(f32 dst[4], f32 src[4]);

// --- core2/bundle.c ---
Actor *__bundle_spawnFromFirstActor(enum bundle_e bundle_id, Actor *actor);

// --- FP/ch/twinklybox.c ---
bool func_8038DD14(void);

// --- SM/version_compat.c ---
int func_8038AAB0(void);

// --- GV/crc.c ---
void code3B10_checkGVChecksums(void);

// --- BGS/ch/tanktup.c ---
s32 func_8038F570(s16 *arg0);

// --- core1/collision.c ---
bool func_80245524(f32 arg0[3], void *arg1, intptr_t *arg2, f32 *arg3);

// --- core2/bs/player_spawn.c ---
void func_8029BC60(enum asset_e *anim_id, f32 *anim_duration);

// --- core2/frame/bufferreadback.c ---
s32 func_802E4AD4(s32 arg0);

// --- core2/sfx/instruments.c ---
void func_8033543C(Struct81s *arg0);

// --- core2/model/matrixrotate.c ---
void func_8033F7F0(u8 indx, Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- core2/spline_pathfollow.c ---
s32 func_80341EC4(f32 arg0[3]);

// --- core2/vtx/normalset.c ---
void func_8034CF6C(void *arg0);
void func_8034CF74(void *arg0, s32 arg1, BKModel *arg2, s32 arg3);
void func_8034CF90(void *arg0, BKModel *arg1, s32 arg2);

// --- core2/vtx/renderstart.c ---
void setStruct6DsOpacity(Struct6Ds *arg0, s32 arg1);

// --- core2/anim/anim_cache.c ---
int animCache_getBoneTransformList(s16 index, BoneTransformList **arg1);

// --- core2/anim/bonetransformlist.c ---
void boneTransformList_reset(BoneTransformList *self);

// --- core2/ch/bottlesbonuscursor.c ---
void chBottlesBonusCursor_func_802DF460(s32 indx, ActorMarker *caller, f32 arg2[3]);
s32 chBottlesBonusCursor_func_802E0538(s32 indx);

// --- core2/ch/collectible.c ---
s32 chCollectible_collectEgg(ActorProp *arg0);
bool chCollectible_collectGoldFeather(ActorProp *arg0);
bool chCollectible_collectRedFeather(ActorProp *arg0);

// --- TTC/ch/nipper.c ---
bool chNipper_isInState7(s16 arg0[3]);

// --- core2/actor_cubepropsystem.c ---
s32 codeA5BC0_getPositionAndReturnRadius(void *arg0, s32 arg1[3]);

// --- core2/gamestate.c ---
void itemscore_timeScores_fromSaveData(u16 *savedata);

// --- core2/savedata.c ---
int savedata_8033CE40(void *buffer);
int savedata_8033CC98(s32 filenum, void *buffer);
void savedata_clear(void *savedata);

// --- core2/spawn_queue.c ---
void spawnQueue_defrag(void);

void bkmemcpy64(void *dest, void *src, s32 size); // handwritten assembly code that performs an optimized 8 byte memcpy
void bkmemset64(void *dest, s32 value, s32 size); // handwritten assembly code that performs an optimized 8 byte memset
u32 bkGetSR(void); // handwritten assembly code that replicates the __osGetSR function

// --- RBB/ch/engineparts.c ---
f32 func_8038A6B8(ActorMarker *);

// --- core1/io/ba_motor.c ---
void baMotor_80250D94(f32, f32, f32);
void baMotor_80250E94(f32, f32, f32, f32, f32, f32);

// --- core1/io/pfsmanager.c ---
OSContPad *func_8024F3F4(void);
OSMesgQueue *pfsManager_getFrameReplyQ(void);
void controller_copyFaceButtons(s32, s32 [6]);

// --- core1/memory.c ---
void * bk_malloc(size_t size);
void *bk_realloc(void* ptr, size_t size);
void bk_free(void*);
void *defrag(void *);
void *defrag_asset(void *);

// --- core2/anim/anim_bonetransform.c ---
void *assetcache_get(enum asset_e assetId);
void *assetcache_reload(enum asset_e assetId);

// --- core2/sfx/trackmanager.c ---
void musicKeepsPlaying(void);

// --- core2/ba/ba_eyeblink.c ---
void baiFrame_startWithValue(f32);

// --- core2/ba/ba_eyes.c ---
void baeyes_openSingleEye(s32, f32);

// --- core2/ba/ba_falldamage.c ---
void bafalldamage_start(void);

// --- core2/ba/ba_flag.c ---
bool baflag_isFalse(enum misc_flag_e arg0);
bool baflag_isTrue(enum misc_flag_e arg0);
void baflag_clear(enum misc_flag_e arg0);
void baflag_clearAll(void);
void baflag_set(enum misc_flag_e arg0);
void baflag_toggle(enum misc_flag_e arg0);

// --- core2/ba/ba_intensity.c ---
f32  func_802915D8(void);
f32  func_80291604(void);

// --- core2/ba/ba_key.c ---
int bakey_pressed(s32);
u32 bakey_held(s32);

// --- core2/ba/ba_marker.c ---
ActorMarker *baMarker_get(void);

// --- core2/ba/ba_model.c ---
f32  baModel_80292230(void);
void baModel_80291A50(s32 arg0, f32* arg1);
void baModel_80292078(s32, f32);
void baModel_80292158(f32);

// --- core2/ba/ba_physics.c ---
f32  get_slope_timer(void);
void baphysics_get_velocity(f32 dst[3]);
void baphysics_reset_gravity(void);
void baphysics_set_gravity(f32 gravity);

// --- core2/ba/ba_position.c ---
f32 player_getYPosition(void);
void _player_getPosition(f32 dst[3]);
void playerPosition_addY(f32);
void player_setYPosition(f32);

// --- core2/ba/ba_recoil.c ---
f32  get_turbo_duration(void);
void func_80294980(f32 arg0[3]);

// --- core2/ba/ba_sfx.c ---
void basfx_80299CF4(enum sfx_e, f32, s32);
void basfx_80299D2C(enum sfx_e, f32, s32);
void basfx_playJumpSfx(f32, f32);
void basfx_playOwSfx(f32);
void basfx_updateClockSfxSource(f32, f32);

// --- core2/ba/ba_stick.c ---
f32  bastick_distance(void);
f32  bastick_getAngleRelativeToBanjo(void);
f32  bastick_getY(void);
f32  bastick_getZonePosition(void);
void bastick_setZoneMax(s32, f32);

// --- core2/ba/ba_underwater.c ---
void func_8029CF48(s32, s32, f32);

// --- core2/ba/ba_yaw.c ---
void func_8029932C(f32);
void func_80299594(s32, f32);
void func_80299628(s32);

// --- core2/bs/bs_statemachine.c ---
enum bs_interrupt_e bs_getInterruptType(void);
s32 bs_checkInterrupt(enum bs_interrupt_e arg0);
s32 bs_getNextState(void);
s32 bs_getPrevState(void);
s32 bs_getState(void);
void bs_clearState(void);
void bs_setState(s32 state_id);
void bs_updateState(void);
void bs_setInterruptResponse(s32 arg0);

// --- core2/bs/bs_storedstate.c ---
f32  bsStoredState_getLongLegTimer(void);
f32  bsStoredState_getTurboTimer(void);
void bsStoredState_setLongLegTimer(f32);
void bsStoredState_setTrot(bool);
void bsStoredState_setTurboTimer(f32);

// --- core2/bs/walk.c ---
f32  func_802B6F9C(void);

// --- core2/camera/camera_motor2.c ---
void func_802BB3DC(s32, f32, f32);

// --- core2/camera/dynamicCam3.c ---
void func_802C1B20(f32);

// --- core2/camera/dynamicCamD.c ---
void ncDynamicCamD_func_802BF2C0(f32);

// --- core2/climb.c ---
f32 climbGetBottomY(void);
f32 climbGetTopY(void);
void climbGetBottom(f32 dst[3]);

// --- core2/scorequeue/manager.c ---
void func_802FAD64(enum item_e);

// --- core2/collision/hitboxdata.c ---
enum marker_collision_func_type_e collision_getNextState(CollisionParams *arg0);

// --- core2/fx/airscore.c ---
struct7s *fxairscore_new(s32);
void fxairscore_draw(enum item_e, struct8s *, Gfx**, Mtx**, Vtx **);
void fxairscore_free(s32, struct7s *);
void fxairscore_update(enum item_e, struct7s *);

// --- core2/fx/common1score.c ---
struct7s *fxcommon1score_new(enum asset_e item_id);
void fxcommon1score_draw(enum item_e arg0, struct8s *arg1, Gfx **arg2, Mtx **arg3, Vtx **arg4);
void fxcommon1score_free(enum item_e item_id, struct8s *);
void fxcommon1score_update(enum item_e, struct8s *);

// --- core2/fx/common2score.c ---
struct8s *fxcommon2score_new(enum item_e);
void fxcommon2score_draw(enum item_e, struct8s *, Gfx**, Mtx**, Vtx **);
void fxcommon2score_free(enum item_e, struct8s *);
void fxcommon2score_update(s32, struct8s *);

// --- core2/fx/common3score.c ---
void *fxcommon3score_new(enum item_e);
void fxcommon3score_draw(enum item_e, void *, Gfx**, Mtx**, Vtx **);
void fxcommon3score_free(enum item_e item_id, void *);
void fxcommon3score_update(enum item_e, void *);

// --- core2/fx/effect_debris.c ---
Actor *func_802C8A54(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags);
Actor *func_802C8AA8(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags);
Actor *func_802C8AF8(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags);
Actor *func_802C8B4C(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags);
Actor *func_802C8BA8(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags);
Actor *func_802C8C04(s32 position[3], s32 yaw, ActorInfo* actorInfo, u32 flags);

// --- core2/fx/effect_playerspray.c ---
void func_802927E0(f32, f32);
void func_80292900(f32, f32);
void func_80292974(f32, f32, f32);

// --- core2/fx/effect_spriteanim.c ---
Actor *func_802DC7E0(ActorMarker *marker, Gfx **gfx, Mtx **mtx, Vtx **vtx);

// --- core2/fx/healthscore.c ---
struct7s *fxhealthscore_new(enum item_e);
void fxhealthscore_draw(enum item_e item_id, struct8s *arg1, Gfx **gfx, Mtx **mtx, Vtx **vtx);
void fxhealthscore_free(enum item_e, struct8s *);
void fxhealthscore_update(enum item_e, struct8s *);

// --- core2/fx/honeycarrierscore.c ---
struct7s *fxhoneycarrierscore_new(s32);
void fxhoneycarrierscore_draw(s32, struct8s *, Gfx**, Mtx**, Vtx **);
void fxhoneycarrierscore_free(s32, struct8s *);
void fxhoneycarrierscore_update(s32, struct8s *);

// --- core2/fx/jinjoscore.c ---
struct8s *fxjinjoscore_new(enum item_e);
void fxjinjoscore_draw(s32, struct8s *, Gfx**, Mtx**, Vtx **);
void fxjinjoscore_free(enum item_e, struct8s *);
void fxjinjoscore_update(enum item_e, struct8s *);

// --- core2/fx/lifescore.c ---
struct8s *fxlifescore_new(s32);
void fxlifescore_draw(enum item_e, struct8s *, Gfx**, Mtx**, Vtx **);
void fxlifescore_free(s32, struct8s *);
void fxlifescore_update(enum item_e, struct8s *);

// --- core2/fx/score_jiggylist.c ---
void jiggy_spawn(enum jiggy_e jiggy_id, f32 pos[3]);

// --- core2/fx/sparkle.c ---
void fxSparkle_chTreasure(s16[3]);

// --- core2/gamestate.c ---
s32 itemscore_noteScores_get(enum level_e lvl_id);
void func_80346C10(enum bs_e *retVal, enum bs_e fail_state, enum bs_e success_state, enum item_e item_id, int use_item);
void func_80347A14(s32);
void itemscore_noteScores_clear(void);
void itemscore_timeScores_clear(void);

// --- core2/level/metadata.c ---
f32  barebound_get_vertical_velocity(void);
f32  barebound_get_horizontal_velocity(void);
f32  barebound_get_gravity(void);

// --- core2/particle/bathroom.c ---
void func_8029AD28(f32, s32);

// --- core2/bs/player_spawn.c ---
ParticleEmitter *func_8029B950(f32[3],f32);
f32  func_8029B41C(void);
void func_8029C3E8(f32, f32);

// --- core2/particle/scale2.c ---
void func_802EE278(Actor *, s32, s32, s32, f32, f32);

// --- core2/particle/spawn.c ---
ParticleEmitter *func_802F4094(f32[3], f32);

// --- core2/pitch.c ---
f32  pitch_getIdeal(void);
f32 pitch_get(void);
void pitch_setAngVel(f32, f32);
void pitch_setIdeal(f32);

// --- core2/playerutils.c ---
bool  func_8028AED4(f32*, f32);
int player_inWater(void);

// --- core2/roll.c ---
f32 roll_get(void);
void roll_setAngularVelocity(f32, f32);
void roll_setIdeal(f32);

// --- core2/sfx/playstate.c ---
struct6s *func_802F8BE0(s32 arg0);

// --- core2/sfx/randompitch.c ---
s32 func_802F9AA8(enum sfx_e);
void func_802F9DB8(s32, f32, f32, f32);
void func_802F9F80(s32, f32, f32, f32);
void func_802F9FD0(s32, f32, f32, f32);
void func_802FA060(s32, s32, s32, f32);

// --- core2/sfx/streamctrl.c ---
struct3s *func_802F8264(s32 arg0);

// --- core2/fx/projectile_anim.c ---
void func_80352CF4(f32 *, f32 *, f32, f32);

// --- core2/timed_funcqueue.c ---
void func_80324D2C(f32, enum comusic_e);
void func_80324D54(f32, enum sfx_e, f32, s32, f32 [3], f32, f32);
void func_80324E38(f32, s32);
void timed_exitStaticCamera(f32 time);
void timed_playSfx(f32, enum sfx_e, f32, s32);
void timed_setStaticCameraToNode(f32, s32);

// --- core2/vector.c ---
VLA *   bk_vector_defrag(VLA *vla);
VLA *   bk_vector_new(u32 elemSize, u32 cnt);
s32     bk_vector_getIndex(VLA *vla, void *element);
s32     bk_vector_size(VLA *vla);
void    bk_vector_assign(VLA *vla, s32 indx, void* value);
void    bk_vector_clear(VLA *vla);
void    bk_vector_free(VLA *vla);
void    bk_vector_popBack_n(VLA *vla, u32 n);
void    bk_vector_remove(VLA *vla, u32 indx);
void *  bk_vector_at(VLA *vla, u32 n);
void *  bk_vector_getBegin(VLA *vla);
void *  bk_vector_getEnd(VLA *vla);
void *  bk_vector_insertNew(VLA **vlaPtr, s32 indx);
void *  bk_vector_pushBackNew(VLA **vlaPtr);

// --- core2/vtx/alphablend.c ---
void func_8034E71C(Struct73s *arg0, s32 arg1, f32 arg2);
void func_8034E78C(Struct73s *arg0, s32 arg1, f32 arg2);
void func_8034E7B8(Struct73s *, s32, f32, s32, f32);

// --- core2/vtx/renderstart.c ---
void func_8034DC08(Struct6Ds *, f32[3], f32[3], f32, s32);
void func_8034DDF0(Struct6Ds *arg0, f32 arg1[3], f32 arg2[3], f32 arg3, s32 arg4);
void subaddie_positionMoveVertical(Struct6Ds *, f32, f32, f32, s32);
void func_8034DEB4(Struct6Ds *, f32);
void func_8034DFB0(Struct6Ds *arg0, s32 arg1[4], s32 arg2[4], f32 arg3);
void func_8034E1A4(Struct6Ds *arg0, enum sfx_e, f32, f32);

// --- core2/vtx/transform.c ---
BKModel *func_8034C4F0(Struct70s *arg0);
Struct70s *func_8034C528(s32);
Struct70s *func_8034C5AC(s32);
s16 func_8034C50C(Struct70s *arg0);

// --- core2/yaw.c ---
f32 yaw_get(void);
f32 yaw_getIdeal(void);
void yaw_applyIdeal(void);
void yaw_rotateTimed(f32);
void yaw_set(f32);
void yaw_setIdeal(f32);
void yaw_setVelocityBounded(f32, f32);

// --- port/stub.c ---
float gu_sqrtf(float val);

// --- unknown source (no .c definition found) ---
void _guMtxIdentF_80245D44(float mf[4][4]); //static should NOT be here
void func_80241304(Mtx *m, float x, float y, float z);
BKModelBin *  marker_loadModelBin(ActorMarker *marker);

#endif // FUNCTIONS_H
