#include "FightSync.h"
#include "port/Network/Anchor/Authority.h"

#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "prop.h"
#include "FINALE/fight.h"

// chfinalboss.c globals the sync layer mirrors/replays. D_803927xx are the spell-spawn
// scratch vectors; the pad/barrier globals feed the latecomer world snapshot.
extern f32 D_80392758[3];
extern f32 D_80392768[3];
extern f32 D_80392778[3];
extern f32 __chFinalBossFireballFlightTime;
extern ActorMarker *__chFinalBossFlightPadMarker;
extern u8 __chFinalBossSpellBarrierActive;
extern ActorArray *suBaddieActorArray;
// Non-zero while the first-statue spawn cutscene is running; vanilla freezes the local
// player through it, so a statue can never be fed until it ends.
extern u8 sFinalBossJinjoStatueActivated;
// Collision radius the boss brain uses when testing for a jinjo that has flown into Grunty.
extern f32 func_8033229C(ActorMarker *marker);

// Set once the ending script takes over (chfinalboss_setBossDefeated); every client then
// plays the fixed ending locally and stream/world packets are ignored.
static u8 sFightNetCinematic = 0;
// True once we've mirrored a remote authority; on promotion, re-enter the current phase
// so the resumed brain starts clean instead of from the mirror's partial internals.
static u8 sFightNetWasFollower = 0;
// Bit per jinjo statue id (1-4): its jinjo already slammed Grunty. Feeds the world
// snapshot so a latecomer doesn't spawn a jinjo that circles forever.
static u8 sFightNetJinjoSlammed = 0;
// FIGHT_STATE snapshot staged by FightSync_ApplyWorld, applied incrementally by the
// follower tick since the replayed actors take frames to come into existence.
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

static Actor *FightSync_FindBoss(void) {
    Actor *boss = actorArray_findActorFromActorId(ACTOR_38B_GRUNTILDA_FINAL_BOSS);

    if (boss == NULL || !boss->volatile_initialized || boss->despawn_flag) {
        return NULL;
    }
    return boss;
}

// The jinjo statue bases share one actor id, so find by their statue id (1-4).
static Actor *FightSync_FindStatueBase(s32 statue_id) {
    s32 i;

    if (suBaddieActorArray == NULL) {
        return NULL;
    }
    for (i = 0; i < suBaddieActorArray->cnt; i++) {
        Actor *actor = &suBaddieActorArray->data[i];
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

// Apply a networked egg via getHitByEgg(other == NULL), the vanilla counting path.
static void FightSync_ApplyJinjoStatueEgg(s32 statue_id) {
    Actor *base = FightSync_FindStatueBase(statue_id);

    if (base != NULL) {
        chBossJinjoBase_getHitByEgg(base->marker, NULL);
    }
}

// The first jinjo statue's break plays a static-camera cutscene on EVERY client
// (chstonejinjo_breakOpen, gated by FILEPROG_D1), but the vanilla release runs only in the
// authority's phase-4 brain when that first jinjo slams Grunty (chfinalboss_phase4_update). A
// follower skips that brain, so its camera would stay locked on the cutscene node until it warped
// out. Mirror the release here when we process the first slam, freeing the follower's camera the
// same way — sFinalBossJinjoStatueActivated (set by the break, cleared here) makes it fire once.
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
    // The one-shot "already spawned" latches are cleared here (fresh fight / re-entry respawns the
    // boss) rather than in FightSync_ApplyWorld, so a second world snapshot for the *same* boss (the
    // authority re-sends on every peer map-load) can't re-run the catch-up and spawn a duplicate
    // flight pad or spell barrier — a second barrier would orphan the first, which then stops
    // following Grunty and lingers at her old position.
    sFightNetCatchupPadDone = 0;
    sFightNetCatchupBarrierDone = 0;
    for (i = 0; i < 6; i++) {
        sFightNetCatchupSpawned[i] = 0;
    }
}

void FightSync_OnBossDefeated(void) {
    sFightNetCinematic = 1;
}

// --- authority replication hooks (each a no-op unless we are the live authority) --------------

void FightSync_OnSpellSpawned(s32 kind) {
    if (FightSync_IsLiveAuthority()) {
        // Ballistic fireball (kind 0) carries lead time so each follower re-aims at its own player.
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

bool FightSync_GatherUpdate(f32 pos[3], f32 *yaw, s32 *state, s32 *phase, s32 *mirror, s32 *vuln) {
    Actor *boss;
    ActorLocal_FinalBoss *local;

    if (sFightNetCinematic) {
        return false;
    }
    boss = FightSync_FindBoss();
    if (boss == NULL) {
        return false;
    }
    local = (ActorLocal_FinalBoss *)&boss->local;
    pos[0] = boss->position[0];
    pos[1] = boss->position[1];
    pos[2] = boss->position[2];
    *yaw = boss->yaw;
    *state = boss->state;
    *phase = local->phase;
    *mirror = local->mirror_phase5;
    // Phase-2 vulnerability toggle (unkA); brain-internal, so it must be streamed explicitly
    // or a follower picks the wrong hittable/invulnerable marker id and eggs pass through.
    *vuln = local->unkA;
    return true;
}

// Mirror a streamed state/phase change, including the model-part toggles setState does,
// so the follower's Grunty looks right in every state.
static void FightSync_ApplyBossState(Actor *this, s32 phase, s32 state) {
    ActorLocal_FinalBoss *local = (ActorLocal_FinalBoss *)&this->local;

    // Leaving the broomstick phase drops the motor sfx source our own intro path created.
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
        // The broom shatters at the end of phase 4 — replay the burst for the follower.
        chfinalboss_createBroomstickParticles(this->position, ASSET_552_MODEL_BROOMSTICK_PIECE_HEAD, 1);
        chfinalboss_createBroomstickParticles(this->position, ASSET_553_MODEL_BROOMSTICK_PIECE_SHORT, 12);
        chfinalboss_createBroomstickParticles(this->position, ASSET_554_MODEL_BROOMSTICK_PIECE_LONG, 20);
        chfinalboss_createBroomstickParticles(this->position, ASSET_555_MODEL_BROOMSTICK_PIECE_EYE, 2);
    }
}

void FightSync_ApplyUpdate(const f32 pos[3], f32 yaw, s32 state, s32 phase, s32 mirror, s32 vuln) {
    Actor *boss;
    ActorLocal_FinalBoss *local;

    if (!FightSync_IsFollower() || sFightNetCinematic) {
        return;
    }
    boss = FightSync_FindBoss();
    if (boss == NULL) {
        return;
    }
    sFightNetWasFollower = 1;
    local = (ActorLocal_FinalBoss *)&boss->local;
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

bool FightSync_GatherWorld(FightWorldSnapshot *snap) {
    Actor *base;
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

void FightSync_ApplyWorld(const FightWorldSnapshot *snap) {
    s32 i;

    if (!FightSync_IsFollower() || sFightNetCinematic) {
        return;
    }
    sFightNetSnap = *snap;
    sFightNetCatchupActive = 1;
    // The "already spawned" latches (pad/barrier/statues) are NOT reset here — they clear on a fresh
    // boss spawn (FightSync_OnBossSpawned) instead, so re-receiving a snapshot for the same boss
    // re-drives egg/statue catch-up without spawning a duplicate flight pad or spell barrier.
    // Seed the slammed set from the snapshot up front: a statue whose jinjo already slammed before
    // we arrived is rebuilt by the catch-up (which despawns the re-hatched jinjo when it appears),
    // but should the jinjo reach Grunty before that, the phase-4 slam guard (BossFollowerTick) uses
    // this bit to remove it quietly rather than replaying the attack.
    for (i = 0; i < 4; i++) {
        if (snap->jinjoGone[i]) {
            sFightNetJinjoSlammed |= 1 << (i + 1);
        }
    }
}

// Rebuild the snapshot piece by piece: queue missing spawns, replay eggs once bases exist,
// clear jinjos whose slam we missed. Runs from the follower tick until caught up.
static void FightSync_CatchupTick(Actor *boss) {
    Actor *base;
    Actor *jinjo;
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
                // Its slam happened before we arrived — remove it quietly.
                sFightNetJinjoSlammed |= 1 << (i + 1);
                marker_despawn(jinjo->marker);
                sFightNetSnap.jinjoGone[i] = 0;
            } else if (base->state == CHBOSSJINJOBASE_STATE_3_SPAWNED_BOSS_JINJO) {
                done = 0; // the stone jinjo is still cracking open; wait for the spawn
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

// Follower gate + per-frame cosmetics (broom glow/trail) plus latecomer catch-up. Returns
// true while a remote authority drives the boss, so chfinalboss_update skips the local brain.
bool FightSync_BossFollowerTick(void *bossPtr) {
    Actor *boss = (Actor *)bossPtr;

    if (!FightSync_IsFollower() || sFightNetCinematic) {
        return false;
    }
    FightSync_CatchupTick(boss);

    // Detect a jinjo-into-Grunty collision locally too (display-only), so it pops immediately
    // instead of waiting on the authority's slam event, which can arrive after ours has spawned.
    if (((ActorLocal_FinalBoss *)&boss->local)->phase == FINALBOSS_PHASE_4_JINJOS) {
        ActorMarker *jinjoMarker = chfinalboss_findCollidingJinjo(boss, func_8033229C(boss->marker));
        if (jinjoMarker != NULL) {
            Actor *jinjo = marker_getActor(jinjoMarker);
            s32 sid = (jinjo != NULL) ? jinjo->actorTypeSpecificField : 0;
            if (sid >= BOSSJINJO_1_ORANGE && sid <= BOSSJINJO_4_YELLOW &&
                (sFightNetJinjoSlammed & (1 << sid))) {
                // This jinjo's slam already happened before we got here (a re-entry catch-up rebuilt
                // its statue and hatched it again) — remove the rebuilt jinjo quietly instead of
                // replaying its attack particles + Grunty reaction.
                marker_despawn(jinjoMarker);
            } else {
                if (sid >= BOSSJINJO_1_ORANGE && sid <= BOSSJINJO_4_YELLOW) {
                    sFightNetJinjoSlammed |= 1 << sid;
                }
                chbossjinjo_attack(jinjoMarker);
                // First real slam releases the follower's first-statue cutscene camera.
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
    Actor *boss = FightSync_FindBoss();
    s32 i;

    switch (ev) {
        case FIGHT_EV_SPELL:
            if (!FightSync_IsFollower() || v0 == NULL) {
                return;
            }
            // Ballistic fireball (kind 0) is re-aimed at this client's own player, not replayed
            // from the authority's arc, so every player has to dodge one.
            if (a == 0) {
                if (boss != NULL) {
                    f32 src[3];
                    f32 leadTime = (b > 0) ? (b / 1000.0f) : 1.3f;
                    src[0] = boss->position[0];
                    src[1] = boss->position[1];
                    src[2] = boss->position[2];
                    chfinalboss_func_80387110(boss->marker, src, leadTime, 0);
                }
                break;
            }
            // Spawn helpers read these globals; replay through the same queue path as the authority.
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
            Actor *jinjo;
            if (!FightSync_IsFollower() || a < BOSSJINJO_1_ORANGE || a > BOSSJINJO_4_YELLOW) {
                return;
            }
            sFightNetJinjoSlammed |= 1 << a;
            jinjo = actorArray_findActorFromActorId(ACTOR_3A4_BOSS_JINJO_BASE_IDX + a);
            if (jinjo != NULL) {
                chbossjinjo_attack(jinjo->marker);
            }
            // First slam from the authority releases the follower's first-statue cutscene camera
            // (the vanilla release lives in the authority-only phase-4 brain).
            FightSync_ReleaseFirstStatueCutscene();
            break;
        }

        case FIGHT_EV_BOSS_HIT:
            if (!FightSync_IsLiveAuthority() || boss == NULL) {
                return;
            }
            // Deter pause abuse: discard hits landed while our game is paused.
            if (getGameMode() == GAME_MODE_4_PAUSED) {
                return;
            }
            // Drop hits from a client whose view of the fight lagged past a phase change.
            if (((ActorLocal_FinalBoss *)&boss->local)->phase != (u8)a) {
                return;
            }
            chfinalboss_collisionPassive(boss->marker, NULL);
            break;

        case FIGHT_EV_EGG:
            if (!FightSync_IsLiveAuthority()) {
                return;
            }
            // Same pause guard as BOSS_HIT: no statue activations against a frozen fight.
            if (getGameMode() == GAME_MODE_4_PAUSED) {
                return;
            }
            if (a == BOSSJINJO_5_JINJONATOR) {
                chjinjonatorbase_netApplyEgg(b);
            } else {
                // Drop eggs that arrive during the first-statue spawn cutscene: a follower isn't
                // frozen like vanilla and could release a statue mid-cutscene and softlock the
                // authority. The follower re-feeds once the statues are up.
                if (sFinalBossJinjoStatueActivated != 0) {
                    return;
                }
                FightSync_ApplyJinjoStatueEgg(a);
            }
            break;
    }
}

void FightSync_OnAuthorityChanged(void) {
    Actor *boss;
    ActorLocal_FinalBoss *local;

    FightSyncSeq_Reset();
    if (sFightNetCinematic) {
        return;
    }
    boss = FightSync_FindBoss();
    if (boss == NULL) {
        return;
    }
    // Promoted mid-fight: re-enter the mirrored phase so the brain starts clean.
    if (sFightNetWasFollower && FightSync_IsLiveAuthority()) {
        sFightNetWasFollower = 0;
        local = (ActorLocal_FinalBoss *)&boss->local;
        if (local->phase != FINALBOSS_PHASE_0_INTRO) {
            chfinalboss_setPhase(boss->marker, local->phase);
        }
    }
}
