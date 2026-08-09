#include "FightSync.h"
#include "port/Network/Anchor/Authority.h"

#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "prop.h"
#include "fight/fight.h"

// chfinalboss.c globals. D_803927xx = spell-spawn scratch vectors.
extern f32 D_80392758[3];
extern f32 D_80392768[3];
extern f32 D_80392778[3];
extern f32 __chFinalBossFireballFlightTime;
extern ActorMarker* __chFinalBossFlightPadMarker;
extern u8 __chFinalBossSpellBarrierActive;
extern ActorArray* suBaddieActorArray;
extern u8 sFinalBossJinjoStatueActivated;
// Collision radius for the jinjo-into-Grunty test.
extern f32 func_8033229C(ActorMarker* marker);

// Set once the ending script takes over (chfinalboss_setBossDefeated).
static u8 sFightNetCinematic = 0;
static u8 sFightNetWasFollower = 0;
// Bit per jinjo statue id (1-4): its jinjo already slammed Grunty.
static u8 sFightNetJinjoSlammed = 0;
static FightWorldSnapshot sFightNetSnap;
static u8 sFightNetCatchupActive = 0;
static u8 sFightNetCatchupSpawned[6]; // statue spawn already queued, indexed by statue id
static u8 sFightNetCatchupPadDone = 0;
static u8 sFightNetCatchupBarrierDone = 0;

static bool FightSync_IsFollower(void) {
    return NetAuthority_IsClaimed(NET_ACTIVITY_FINAL_BOSS) && !NetAuthority_IsSelf(NET_ACTIVITY_FINAL_BOSS);
}

static bool FightSync_IsLiveAuthority(void) {
    return NetAuthority_IsClaimed(NET_ACTIVITY_FINAL_BOSS) && NetAuthority_IsSelf(NET_ACTIVITY_FINAL_BOSS);
}

static Actor* FightSync_FindBoss(void) {
    Actor* boss = actorArray_findActorFromActorId(ACTOR_38B_GRUNTILDA_FINAL_BOSS);

    if (boss == NULL || !boss->volatile_initialized || boss->despawn_flag) {
        return NULL;
    }
    return boss;
}

// The jinjo statue bases share one actor id, so find by their statue id (1-4).
static Actor* FightSync_FindStatueBase(s32 statue_id) {
    s32 i;

    if (suBaddieActorArray == NULL) {
        return NULL;
    }
    for (i = 0; i < suBaddieActorArray->cnt; i++) {
        Actor* actor = &suBaddieActorArray->data[i];
        if (actor->marker == NULL || actor->marker->id != MARKER_27A_JINJO_STATUE_BASE) {
            continue;
        }
        if (actor->actorTypeSpecificField != statue_id || actor->despawn_flag) {
            continue;
        }
        return actor;
    }
    return NULL;
}

// Apply a networked egg via getHitByEgg(other == NULL), the counting path.
static void FightSync_ApplyJinjoStatueEgg(s32 statue_id) {
    Actor* base = FightSync_FindStatueBase(statue_id);

    if (base != NULL) {
        chBossJinjoBase_getHitByEgg(base->marker, NULL);
    }
}

static void FightSync_ReleaseFirstStatueCutscene(void) {
    if (sFinalBossJinjoStatueActivated) {
        sFinalBossJinjoStatueActivated = 0;
        timed_exitStaticCamera(1.0f);
        func_80324E38(1.0f, 0);
    }
}

// --- boss lifecycle hooks (called from chfinalboss.c) -----------------------------------------

void FightSync_OnBossSpawned(void) {
    s32 i;
    sFightNetCinematic = 0;
    sFightNetWasFollower = 0;
    sFightNetJinjoSlammed = 0;
    sFightNetCatchupActive = 0;
    // Cleared on a fresh boss spawn, not in ApplyWorld, so a repeat snapshot can't re-run catch-up.
    sFightNetCatchupPadDone = 0;
    sFightNetCatchupBarrierDone = 0;
    for (i = 0; i < 6; i++) {
        sFightNetCatchupSpawned[i] = 0;
    }
}

void FightSync_OnBossDefeated(void) {
    sFightNetCinematic = 1;
}

// --- authority replication hooks -------------------------------------------------------------

void FightSync_OnSpellSpawned(s32 kind) {
    if (FightSync_IsLiveAuthority()) {
        // Kind 0 (ballistic fireball) carries lead time; each follower re-aims at its own player.
        s32 leadMs = (kind == 0) ? (s32)(__chFinalBossFireballFlightTime * 1000.0f) : 0;
        FightSync_SendEvent(FIGHT_EV_SPELL, kind, leadMs, D_80392758, D_80392768, D_80392778);
    }
}

void FightSync_OnFlightPadSpawned(void) {
    if (FightSync_IsLiveAuthority()) {
        FightSync_SendEvent(FIGHT_EV_PAD_SPAWN, 0, 0, NULL, NULL, NULL);
    }
}

void FightSync_OnFlightPadDespawned(void) {
    if (FightSync_IsLiveAuthority()) {
        FightSync_SendEvent(FIGHT_EV_PAD_DESPAWN, 0, 0, NULL, NULL, NULL);
    }
}

void FightSync_OnBarrierSpawned(void) {
    if (FightSync_IsLiveAuthority()) {
        FightSync_SendEvent(FIGHT_EV_BARRIER, 0, 0, NULL, NULL, NULL);
    }
}

void FightSync_OnStatueSpawned(s32 statue_id) {
    if (FightSync_IsLiveAuthority()) {
        FightSync_SendEvent(FIGHT_EV_STATUE_SPAWN, statue_id, 0, NULL, NULL, NULL);
    }
}

void FightSync_OnJinjoSlam(s32 statue_id) {
    sFightNetJinjoSlammed |= 1 << statue_id;
    if (FightSync_IsLiveAuthority()) {
        FightSync_SendEvent(FIGHT_EV_JINJO_ATTACK, statue_id, 0, NULL, NULL, NULL);
    }
}

void FightSync_ReplicateEgg(s32 statue_id, s32 pad_index) {
    if (FightSync_IsLiveAuthority()) {
        FightSync_SendEvent(FIGHT_EV_EGG_FED, statue_id, pad_index, NULL, NULL, NULL);
    }
}

// --- follower input forwarding ----------------------------------------------------------------

bool FightSync_ForwardBossHit(s32 phase) {
    if (!FightSync_IsFollower()) {
        return false;
    }
    FightSync_SendEvent(FIGHT_EV_BOSS_HIT, phase, 0, NULL, NULL, NULL);
    return true;
}

bool FightSync_ForwardEgg(s32 statue_id, s32 pad_index) {
    if (!FightSync_IsFollower()) {
        return false;
    }
    FightSync_SendEvent(FIGHT_EV_EGG, statue_id, pad_index, NULL, NULL, NULL);
    return true;
}

// --- stream: gather (authority) / apply (follower) ---------------------------------------------

bool FightSync_GatherUpdate(f32 pos[3], f32* yaw, s32* state, s32* phase, s32* mirror, s32* vuln) {
    Actor* boss;
    ActorLocal_FinalBoss* local;

    if (sFightNetCinematic) {
        return false;
    }
    boss = FightSync_FindBoss();
    if (boss == NULL) {
        return false;
    }
    local = (ActorLocal_FinalBoss*)&boss->local;
    pos[0] = boss->position[0];
    pos[1] = boss->position[1];
    pos[2] = boss->position[2];
    *yaw = boss->yaw;
    *state = boss->state;
    *phase = local->phase;
    *mirror = local->mirror_phase5;
    // Phase-2 vulnerability toggle (unkA).
    *vuln = local->unkA;
    return true;
}

static void FightSync_ApplyBossState(Actor* this, s32 phase, s32 state) {
    ActorLocal_FinalBoss* local = (ActorLocal_FinalBoss*)&this->local;

    // unk44_31 = broomstick motor sfx source.
    if (this->unk44_31 != 0 && phase != FINALBOSS_PHASE_1_BROOMSTICK) {
        sfxsource_freeSfxsourceByIndex(this->unk44_31);
        this->unk44_31 = 0;
    }
    local->phase = phase;
    subaddie_set_state_with_direction(this, state, 0.0001f, 1);
    if (state == 9 || state == 0x21 || state == 0x22 || state == 0x2B) {
        actor_playAnimationOnce(this);
    } else {
        actor_loopAnimation(this);
    }
    chfinalboss_func_80386600(this->marker, (state == 6 || state == 7 || state == 8) ? 1 : 0);
    if (phase == FINALBOSS_PHASE_5_JINJONATOR || state == 0x21 || state == 0x22) {
        chfinalboss_func_80386628(this->marker, 0);
    } else {
        chfinalboss_func_80386628(this->marker, 1);
    }
    if (state == 0x21) {
        // State 0x21 = broom shatters (end of phase 4); replay the burst.
        chfinalboss_createBroomstickParticles(this->position, ASSET_552_MODEL_BROOMSTICK_PIECE_HEAD, 1);
        chfinalboss_createBroomstickParticles(this->position, ASSET_553_MODEL_BROOMSTICK_PIECE_SHORT, 12);
        chfinalboss_createBroomstickParticles(this->position, ASSET_554_MODEL_BROOMSTICK_PIECE_LONG, 20);
        chfinalboss_createBroomstickParticles(this->position, ASSET_555_MODEL_BROOMSTICK_PIECE_EYE, 2);
    }
}

void FightSync_ApplyUpdate(const f32 pos[3], f32 yaw, s32 state, s32 phase, s32 mirror, s32 vuln) {
    Actor* boss;
    ActorLocal_FinalBoss* local;

    if (!FightSync_IsFollower() || sFightNetCinematic) {
        return;
    }
    boss = FightSync_FindBoss();
    if (boss == NULL) {
        return;
    }
    sFightNetWasFollower = 1;
    local = (ActorLocal_FinalBoss*)&boss->local;
    boss->position[0] = pos[0];
    boss->position[1] = pos[1];
    boss->position[2] = pos[2];
    boss->yaw = yaw;
    boss->yaw_ideal = yaw;
    local->mirror_phase5 = mirror;
    // Applied every frame, not just on state change (see FightSync_GatherUpdate).
    local->unkA = (u8)vuln;
    if (local->phase != (u8)phase || boss->state != state) {
        FightSync_ApplyBossState(boss, phase, state);
    }
}

// --- latecomer world snapshot -------------------------------------------------------------------

bool FightSync_GatherWorld(FightWorldSnapshot* snap) {
    Actor* base;
    s32 i;

    if (sFightNetCinematic || !FightSync_IsLiveAuthority() || FightSync_FindBoss() == NULL) {
        return false;
    }
    snap->pad = (__chFinalBossFlightPadMarker != NULL) ? 1 : 0;
    snap->barrier = __chFinalBossSpellBarrierActive ? 1 : 0;
    for (i = 0; i < 4; i++) {
        base = FightSync_FindStatueBase(i + 1);
        if (base == NULL) {
            snap->statue[i] = 0xFF;
            snap->jinjoGone[i] = 0;
        } else if (base->state == CHBOSSJINJOBASE_STATE_3_SPAWNED_BOSS_JINJO) {
            snap->statue[i] = 3;
            snap->jinjoGone[i] = (sFightNetJinjoSlammed & (1 << (i + 1))) ? 1 : 0;
        } else {
            snap->statue[i] = base->unk38_31;
            snap->jinjoGone[i] = 0;
        }
    }
    snap->jbase = chjinjonatorbase_netGetPads(snap->jpads) ? 1 : 0;
    return true;
}

void FightSync_ApplyWorld(const FightWorldSnapshot* snap) {
    s32 i;

    if (!FightSync_IsFollower() || sFightNetCinematic) {
        return;
    }
    sFightNetSnap = *snap;
    sFightNetCatchupActive = 1;
    // Seed the slammed set from the snapshot so a jinjo that reaches Grunty before catch-up
    // rebuilds its statue is removed by the phase-4 guard rather than replaying the attack.
    for (i = 0; i < 4; i++) {
        if (snap->jinjoGone[i]) {
            sFightNetJinjoSlammed |= 1 << (i + 1);
        }
    }
}

static void FightSync_CatchupTick(Actor* boss) {
    Actor* base;
    Actor* jinjo;
    u8 pads[4];
    s32 i;
    s32 done = 1;

    if (!sFightNetCatchupActive) {
        return;
    }

    if (sFightNetSnap.pad && !sFightNetCatchupPadDone) {
        sFightNetCatchupPadDone = 1;
        __spawnQueue_add_1((GenFunction_1)chfinalboss_spawnFlightPad, 0);
    }
    if (sFightNetSnap.barrier && !sFightNetCatchupBarrierDone) {
        sFightNetCatchupBarrierDone = 1;
        chfinalboss_spawnSpellBarrier(boss->marker);
    }

    for (i = 0; i < 4; i++) {
        if (sFightNetSnap.statue[i] == 0xFF) {
            continue;
        }
        base = FightSync_FindStatueBase(i + 1);
        if (base == NULL) {
            if (!sFightNetCatchupSpawned[i + 1]) {
                sFightNetCatchupSpawned[i + 1] = 1;
                chfinalboss_spawnStatue(i + 1);
            }
            done = 0;
            continue;
        }
        if (base->state != CHBOSSJINJOBASE_STATE_3_SPAWNED_BOSS_JINJO && base->unk38_31 < sFightNetSnap.statue[i]) {
            chBossJinjoBase_getHitByEgg(base->marker, NULL);
            done = 0;
            continue;
        }
        if (sFightNetSnap.jinjoGone[i]) {
            jinjo = actorArray_findActorFromActorId(ACTOR_3A4_BOSS_JINJO_BASE_IDX + i + 1);
            if (jinjo != NULL) {
                sFightNetJinjoSlammed |= 1 << (i + 1);
                marker_despawn(jinjo->marker);
                sFightNetSnap.jinjoGone[i] = 0;
            } else if (base->state == CHBOSSJINJOBASE_STATE_3_SPAWNED_BOSS_JINJO) {
                done = 0;
            }
        }
    }

    if (sFightNetSnap.jbase) {
        if (!chjinjonatorbase_netGetPads(pads)) {
            if (!sFightNetCatchupSpawned[5]) {
                sFightNetCatchupSpawned[5] = 1;
                chfinalboss_spawnStatue(BOSSJINJO_5_JINJONATOR);
            }
            done = 0;
        } else {
            for (i = 0; i < 4; i++) {
                if (pads[i] > sFightNetSnap.jpads[i]) {
                    chjinjonatorbase_netApplyEgg(i);
                    done = 0;
                    break;
                }
            }
        }
    }

    if (done) {
        sFightNetCatchupActive = 0;
    }
}

bool FightSync_BossFollowerTick(void* bossPtr) {
    Actor* boss = (Actor*)bossPtr;

    if (!FightSync_IsFollower() || sFightNetCinematic) {
        return false;
    }
    FightSync_CatchupTick(boss);

    // Detect a jinjo-into-Grunty collision locally (display-only) ahead of the authority's slam event.
    if (((ActorLocal_FinalBoss*)&boss->local)->phase == FINALBOSS_PHASE_4_JINJOS) {
        ActorMarker* jinjoMarker = chfinalboss_findCollidingJinjo(boss, func_8033229C(boss->marker));
        if (jinjoMarker != NULL) {
            Actor* jinjo = marker_getActor(jinjoMarker);
            s32 sid = (jinjo != NULL) ? jinjo->actorTypeSpecificField : 0;
            if (sid >= BOSSJINJO_1_ORANGE && sid <= BOSSJINJO_4_YELLOW && (sFightNetJinjoSlammed & (1 << sid))) {
                marker_despawn(jinjoMarker);
            } else {
                if (sid >= BOSSJINJO_1_ORANGE && sid <= BOSSJINJO_4_YELLOW) {
                    sFightNetJinjoSlammed |= 1 << sid;
                }
                chbossjinjo_attack(jinjoMarker);
                FightSync_ReleaseFirstStatueCutscene();
            }
        }
    }

    switch (boss->state) {
        case 2:
        case 3:
        case 4:
        case 6:
        case 7:
        case 8:
        case 14:
        case 21:
        case 28:
            chfinalboss_spawnBroomstickGlowParticles(boss);
            break;
    }
    return true;
}

// --- one-shot event dispatch --------------------------------------------------------------------

void FightSync_ApplyEvent(s32 ev, s32 a, s32 b, const f32 v0[3], const f32 v1[3], const f32 v2[3]) {
    Actor* boss = FightSync_FindBoss();
    s32 i;

    switch (ev) {
        case FIGHT_EV_SPELL:
            if (!FightSync_IsFollower() || v0 == NULL) {
                return;
            }
            // Kind 0 (ballistic fireball) is re-aimed at this client's own player.
            if (a == 0) {
                if (boss != NULL) {
                    f32 src[3];
                    f32 leadTime = (b > 0) ? (b / 1000.0f) : 1.3f;
                    src[0] = boss->position[0];
                    src[1] = boss->position[1];
                    src[2] = boss->position[2];
                    chfinalboss_throwObject(boss->marker, src, leadTime, 0);
                }
                break;
            }
            // Spawn helpers read these globals; set them before queueing.
            for (i = 0; i < 3; i++) {
                D_80392758[i] = v0[i];
                D_80392768[i] = v1[i];
                D_80392778[i] = v2[i];
            }
            if (a == 2) {
                __spawnQueue_add_1((GenFunction_1)chfinalboss_func_80386EC0, 0);
            } else if (a == 1) {
                __spawnQueue_add_1((GenFunction_1)chfinalboss_func_80387074, 0);
            }
            break;

        case FIGHT_EV_PAD_SPAWN:
            if (!FightSync_IsFollower()) {
                return;
            }
            __spawnQueue_add_1((GenFunction_1)chfinalboss_spawnFlightPad, 0);
            break;

        case FIGHT_EV_PAD_DESPAWN:
            if (!FightSync_IsFollower()) {
                return;
            }
            chfinalboss_despawnFlightPad();
            break;

        case FIGHT_EV_BARRIER:
            if (!FightSync_IsFollower() || boss == NULL) {
                return;
            }
            chfinalboss_spawnSpellBarrier(boss->marker);
            break;

        case FIGHT_EV_STATUE_SPAWN:
            if (!FightSync_IsFollower() || a < BOSSJINJO_1_ORANGE || a > BOSSJINJO_5_JINJONATOR) {
                return;
            }
            chfinalboss_spawnStatue(a);
            break;

        case FIGHT_EV_EGG_FED:
            if (!FightSync_IsFollower()) {
                return;
            }
            if (a == BOSSJINJO_5_JINJONATOR) {
                chjinjonatorbase_netApplyEgg(b);
            } else {
                FightSync_ApplyJinjoStatueEgg(a);
            }
            break;

        case FIGHT_EV_JINJO_ATTACK: {
            Actor* jinjo;
            if (!FightSync_IsFollower() || a < BOSSJINJO_1_ORANGE || a > BOSSJINJO_4_YELLOW) {
                return;
            }
            sFightNetJinjoSlammed |= 1 << a;
            jinjo = actorArray_findActorFromActorId(ACTOR_3A4_BOSS_JINJO_BASE_IDX + a);
            if (jinjo != NULL) {
                chbossjinjo_attack(jinjo->marker);
            }
            FightSync_ReleaseFirstStatueCutscene();
            break;
        }

        case FIGHT_EV_BOSS_HIT:
            if (!FightSync_IsLiveAuthority() || boss == NULL) {
                return;
            }
            if (getGameMode() == GAME_MODE_4_PAUSED) {
                return;
            }
            if (((ActorLocal_FinalBoss*)&boss->local)->phase != (u8)a) {
                return;
            }
            chfinalboss_collisionPassive(boss->marker, NULL);
            break;

        case FIGHT_EV_EGG:
            if (!FightSync_IsLiveAuthority()) {
                return;
            }
            if (getGameMode() == GAME_MODE_4_PAUSED) {
                return;
            }
            if (a == BOSSJINJO_5_JINJONATOR) {
                chjinjonatorbase_netApplyEgg(b);
            } else {
                if (sFinalBossJinjoStatueActivated != 0) {
                    return;
                }
                FightSync_ApplyJinjoStatueEgg(a);
            }
            break;
    }
}

void FightSync_OnAuthorityChanged(void) {
    Actor* boss;
    ActorLocal_FinalBoss* local;

    FightSyncSeq_Reset();
    if (sFightNetCinematic) {
        return;
    }
    boss = FightSync_FindBoss();
    if (boss == NULL) {
        return;
    }
    if (sFightNetWasFollower && FightSync_IsLiveAuthority()) {
        sFightNetWasFollower = 0;
        local = (ActorLocal_FinalBoss*)&boss->local;
        if (local->phase != FINALBOSS_PHASE_0_INTRO) {
            chfinalboss_setPhase(boss->marker, local->phase);
        }
    }
}
