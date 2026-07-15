#include "FightSync.h"
#include "port/Network/Anchor/Authority.h"

#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "prop.h"
#include "FINALE/fight.h"

// chfinalboss.c data the sync layer mirrors/replays (see the FINALE entry points in fight.h
// for the functions). D_803927xx are the spell-spawn scratch vectors the queued spawn helpers
// read; the pad/barrier globals feed the latecomer world snapshot.
extern f32 D_80392758[3];
extern f32 D_80392768[3];
extern f32 D_80392778[3];
extern f32 __chFinalBossFireballFlightTime;
extern ActorMarker *__chFinalBossFlightPadMarker;
extern u8 __chFinalBossSpellBarrierActive;
extern ActorArray *suBaddieActorArray;
// Non-zero while the first-statue spawn cutscene is running (set when the four jinjo statues begin
// rising, cleared when it ends / the first jinjo slams). Vanilla freezes the local player through
// this cutscene, so a statue can never be fed until it is over.
extern u8 sFinalBossJinjoStatueActivated;
// Collision radius the boss brain uses when testing for a jinjo that has flown into Grunty.
extern f32 func_8033229C(ActorMarker *marker);

// From the jinjonator release (chfinalboss_setBossDefeated) the fight is a fixed script, so
// every client — authority and followers alike — plays the ending on its own local simulation:
// the stream stops, and incoming stream/world packets are ignored.
static u8 sFightNetCinematic = 0;
// True while we mirrored a remote authority at least once this fight; a promotion (the old
// owner left) then re-enters the current phase so the resumed brain starts from clean
// per-phase state instead of the mirror's partial internals.
static u8 sFightNetWasFollower = 0;
// Bit per jinjo statue id (1-4): its jinjo already slammed into Grunty. Feeds the world
// snapshot so a latecomer's replayed statue doesn't hatch a jinjo that circles forever.
static u8 sFightNetJinjoSlammed = 0;
// Latecomer catch-up: a FIGHT_STATE snapshot staged by FightSync_ApplyWorld and applied
// incrementally by the follower tick, since the replayed actors (statue bases, stone jinjos)
// take frames — or whole rise animations — to come into existence.
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

// Apply a networked egg to a jinjo statue base: getHitByEgg with other == NULL takes the
// vanilla counting path on every client (see chbossjinjobase.c).
static void FightSync_ApplyJinjoStatueEgg(s32 statue_id) {
    Actor *base = FightSync_FindStatueBase(statue_id);

    if (base != NULL) {
        chBossJinjoBase_getHitByEgg(base->marker, NULL);
    }
}

// --- boss lifecycle hooks (called from chfinalboss.c) -----------------------------------------

void FightSync_OnBossSpawned(void) {
    sFightNetCinematic = 0;
    sFightNetWasFollower = 0;
    sFightNetJinjoSlammed = 0;
    sFightNetCatchupActive = 0;
}

void FightSync_OnBossDefeated(void) {
    sFightNetCinematic = 1;
}

// --- authority replication hooks (each a no-op unless we are the live authority) --------------

void FightSync_OnSpellSpawned(s32 kind) {
    if (FightSync_IsLiveAuthority()) {
        // For the aimed ballistic fireball (kind 0) carry the flight/lead time (ms) so each follower
        // re-aims it at its own player. The fixed-target final fireball (1) and the homing green
        // blast (2) are reproduced from the exact vectors, so they need no lead value.
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
    // Phase-2 vulnerability toggle. The collision-id callback (chfinalboss_func_8038B834) reads
    // unkA to pick Grunty's hittable vs. moving-invulnerable marker id every frame, but unkA is a
    // brain-internal the follower never runs — so without streaming it a follower computes the
    // wrong id in phase 2 and its eggs pass straight through her. Other phases key off state/phase.
    *vuln = local->unkA;
    return true;
}

// Mirror a streamed state/phase change: the anim comes from the shared state table, plus the
// model-part toggles the authority's setState paths do (ram-attack overlay in phase 1's dive,
// the broom vanishing once it breaks) so the follower's Grunty looks right in every state.
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
    // Track the authority's phase-2 vulnerability toggle so our collision-id callback matches hers
    // (see FightSync_GatherUpdate) — applied every frame, not just on state change.
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
    sFightNetCatchupPadDone = 0;
    sFightNetCatchupBarrierDone = 0;
    for (i = 0; i < 6; i++) {
        sFightNetCatchupSpawned[i] = 0;
    }
}

// Rebuild the snapshot's world piece by piece: queue missing spawns, then replay eggs one per
// frame once each base exists, and finally clear out jinjos whose slam we missed. Runs from the
// follower tick until everything has caught up.
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

// Follower gate + per-frame cosmetics for the mirrored boss: the broom glow/sparkle trail the
// authority's phase updates emit in the moving states, plus the latecomer catch-up. Returns
// true while a remote authority drives the boss, so chfinalboss_update skips the local brain.
bool FightSync_BossFollowerTick(void *bossPtr) {
    Actor *boss = (Actor *)bossPtr;

    if (!FightSync_IsFollower() || sFightNetCinematic) {
        return false;
    }
    FightSync_CatchupTick(boss);

    // A jinjo flies into Grunty at each client's own pace, but only the authority runs the brain
    // that detects the impact and clears it — so a follower would leave the flown-in jinjo drawn on
    // top of Grunty until the authority's one-shot slam event arrives (and that can land before this
    // client's copy has even spawned, stranding it forever). Detect the impact locally too, so the
    // jinjo pops here the moment it reaches our Grunty, exactly as it does on the authority. Grunty's
    // own reaction still rides the FIGHT_UPDATE stream; this is display-only and phase-4-scoped so it
    // can't touch the phase-5 jinjonator.
    if (((ActorLocal_FinalBoss *)&boss->local)->phase == FINALBOSS_PHASE_4_JINJOS) {
        ActorMarker *jinjoMarker = chfinalboss_findCollidingJinjo(boss, func_8033229C(boss->marker));
        if (jinjoMarker != NULL) {
            Actor *jinjo = marker_getActor(jinjoMarker);
            if (jinjo != NULL && jinjo->actorTypeSpecificField >= BOSSJINJO_1_ORANGE &&
                jinjo->actorTypeSpecificField <= BOSSJINJO_4_YELLOW) {
                sFightNetJinjoSlammed |= 1 << jinjo->actorTypeSpecificField;
            }
            chbossjinjo_attack(jinjoMarker);
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
            // The aimed ballistic fireball (kind 0) is re-aimed at THIS client's own player rather
            // than replayed from the authority's arc, so every player has to dodge one and the fight
            // keeps its difficulty. func_80387110 runs the same vanilla lead + spawn against our
            // streamed Grunty and local player; its follower spawn won't re-broadcast (authority-
            // gated). The fixed-target final fireball (1) and homing green blast (2) already threaten
            // everyone from the exact vectors, so those replay unchanged.
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
            // The spawn helpers read these globals; replay through the same queue path the
            // authority used. Their own broadcast is authority-gated, so this can't echo.
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
            break;
        }

        case FIGHT_EV_BOSS_HIT:
            if (!FightSync_IsLiveAuthority() || boss == NULL) {
                return;
            }
            // Deter pause abuse: while our game is paused the boss can't react, so hits landed
            // on the frozen simulation are discarded rather than banked for the unpause.
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
                // Drop a follower's statue egg that arrives while the first-statue spawn cutscene is
                // still playing here. Vanilla freezes the local player through it, so a statue can
                // never be fed this early — but a follower isn't frozen and could feed (and release)
                // the first statue mid-cutscene, whose early jinjo slam then collides with the
                // cutscene's own camera/lock teardown and softlocks the authority. Ignoring the egg
                // restores the vanilla ordering; the follower re-feeds once the statues are up.
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
    // Promoted mid-fight (the previous owner left or disconnected): re-enter the phase we
    // were mirroring so the resumed vanilla brain starts from clean per-phase state.
    if (sFightNetWasFollower && FightSync_IsLiveAuthority()) {
        sFightNetWasFollower = 0;
        local = (ActorLocal_FinalBoss *)&boss->local;
        if (local->phase != FINALBOSS_PHASE_0_INTRO) {
            chfinalboss_setPhase(boss->marker, local->phase);
        }
    }
}
