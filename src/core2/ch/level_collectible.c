#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "port/Enhancements/Retention/Retention.h"

extern void func_8028F7D4(f32, f32);
void chLevelCollectible_update(Actor *this);
extern ActorMarker *func_8028E86C(void);

// [port] Anchor: FP presents and TTC gold are carried collectibles sharing this actor, so they ride
// the same shared-pool framework as CCW worms/acorns (carriedSync): a shared count synced via the
// COLLECT_ITEM packet (so anyone can return what anyone collects), and per-object live despawn keyed
// by spawn position. Maps the world-collectible marker to its ANCHOR_COLLECTIBLE_* kind, or -1 for
// the ones not synced this way (e.g. MM's orange).
static s32 levelCollectible_syncKind(s32 markerId) {
    switch (markerId) {
        case MARKER_36_ORANGE_COLLECTIBLE:         return ANCHOR_COLLECTIBLE_ORANGE;
        case MARKER_37_GOLD_BULLION:               return ANCHOR_COLLECTIBLE_GOLD;
        case MARKER_1FD_BLUE_PRESENT_COLLECTIBLE:  return ANCHOR_COLLECTIBLE_PRESENT_BLUE;
        case MARKER_1FE_GREEN_PRESENT_COLLECTIBLE: return ANCHOR_COLLECTIBLE_PRESENT_GREEN;
        case MARKER_1FF_RED_PRESENT_COLLECTIBLE:   return ANCHOR_COLLECTIBLE_PRESENT_RED;
        default:                                   return -1;
    }
}

// ---------------------------------------------------------------------------
// [port] Anchor: remote teammates' carried/thrown collectibles (display only).
//
// A teammate's PLAYER_UPDATE announces which collectible they're holding
// (port_remoteCarry_setCarried); we spawn a local display copy that tracks
// their dummy player every frame (state 6 — unused by the vanilla actor).
// Their throw arrives as a CARRY_THROW packet and replays the same ballistic
// arc here (state 7), landing with audio/sparkle feedback only — the delivery
// flags, count spends, and quest progress all ride their own sync paths, so
// the display copy must never touch game state.
// ---------------------------------------------------------------------------

// Anchor-side helpers (CarryThrow.cpp): dummy transform for a client, or 0 if that
// client/dummy isn't present in our map; broadcast of our own throw for teammates.
extern s32 port_anchor_getDummyTransform(u32 clientId, f32 pos[3], f32* yaw);
extern void port_anchor_onCarryThrow(s32 markerId, f32 start[3], f32 target[3]);

void __chLevelCollectible_presentReturnEmitSparkles(f32 position[3], enum asset_e sprite_id);

#define REMOTE_CARRY_MAX 16
static struct {
    u32 clientId;        // 0 = free slot
    ActorMarker *marker; // NULL while the spawn is still queued
    s32 markerId;        // which collectible (MARKER_* enum)
    u8 throwPending;     // throw arrived before the queued spawn completed
    f32 throwStart[3];
    f32 throwTarget[3];
} sRemoteCarry[REMOTE_CARRY_MAX];

// Only the known carryables are spawnable as display copies; anything else in a carry packet
// (stale/foreign data) is ignored. The CCW worm/acorn actors delegate their display copies to
// this module too (port_remoteCarry_displayUpdate) even though their real update funcs live in
// caterpillar.c/acorn.c.
static s32 __remoteCarry_actorIdForMarker(s32 markerId) {
    switch (markerId) {
        case MARKER_36_ORANGE_COLLECTIBLE:         return ACTOR_29_ORANGE_COLLECTIBLE;
        case MARKER_37_GOLD_BULLION:               return ACTOR_2A_GOLD_BULLION;
        case MARKER_1FD_BLUE_PRESENT_COLLECTIBLE:  return ACTOR_1ED_BLUE_PRESENT_COLLECTIBLE;
        case MARKER_1FE_GREEN_PRESENT_COLLECTIBLE: return ACTOR_1EF_GREEN_PRESENT_COLLECTIBLE;
        case MARKER_1FF_RED_PRESENT_COLLECTIBLE:   return ACTOR_1F1_RED_PRESENT_COLLECTIBLE;
        case MARKER_1B5_CATERPILLAR:               return ACTOR_2A2_CATERPILLAR;
        case 0x1BC /* acorn marker (unnamed) */:   return ACTOR_2A9_ACORN;
        default:                                   return 0;
    }
}

static s32 __remoteCarry_findByClient(u32 clientId) {
    s32 i;
    for (i = 0; i < REMOTE_CARRY_MAX; i++) {
        if (sRemoteCarry[i].clientId == clientId) {
            return i;
        }
    }
    return -1;
}

static s32 __remoteCarry_findByMarker(ActorMarker *marker) {
    s32 i;
    for (i = 0; i < REMOTE_CARRY_MAX; i++) {
        if (sRemoteCarry[i].clientId != 0 && sRemoteCarry[i].marker == marker) {
            return i;
        }
    }
    return -1;
}

static void __remoteCarry_clearSlot(s32 slot) {
    sRemoteCarry[slot].clientId = 0;
    sRemoteCarry[slot].marker = NULL;
    sRemoteCarry[slot].markerId = 0;
    sRemoteCarry[slot].throwPending = 0;
}

// Mirror of func_802D7DE8's arc math (velocity toward the truncated target, gravity 5/frame)
// WITHOUT its side effects — no map flags, no spend broadcast. State 7 = remote flight.
static void __remoteCarry_launch(Actor *actor, f32 start[3], f32 target[3]) {
    s32 tgt[3];
    f32 var_f12;
    f32 var_f14;
    f32 var_f18;

    actor->position[0] = start[0];
    actor->position[1] = start[1];
    actor->position[2] = start[2];
    ml_vec3f_to_vec3w(tgt, target);
    actor->unk1C[0] = target[0];
    actor->unk1C[1] = target[1];
    actor->unk1C[2] = target[2];
    actor->velocity[0] = (f32)tgt[0] - actor->position[0];
    actor->velocity[1] = 28.0f;
    actor->velocity[2] = (f32)tgt[2] - actor->position[2];
    var_f12 = actor->position[1];
    var_f14 = 28.0f;
    var_f18 = 0.0f;
    while (!(var_f12 < tgt[1]) || !(var_f14 < 0.0f)) {
        var_f18 += 1.0f;
        var_f12 += (var_f14 -= 5.0);
    }
    actor->velocity[0] /= var_f18;
    actor->velocity[2] /= var_f18;
    actor->unk38_31 = (u32)var_f18;
    subaddie_set_state(actor, 7);
}

// Spawn-queue callback: materialize the display copy for (markerId, clientId). Re-finds the
// registry slot — a map change may have reset the registry while this sat in the queue.
static void __remoteCarry_spawnMethod(s32 markerId, s32 clientId) {
    f32 pos[3];
    f32 yaw = 0.0f;
    Actor *actor;
    s32 slot = __remoteCarry_findByClient((u32)clientId);
    s32 actorId = __remoteCarry_actorIdForMarker(markerId);

    if (slot < 0 || sRemoteCarry[slot].markerId != markerId || actorId == 0) {
        return; // stale queue entry
    }
    if (!port_anchor_getDummyTransform((u32)clientId, pos, &yaw)) {
        __remoteCarry_clearSlot(slot);
        return;
    }
    actor = actor_spawnWithYaw_f32(actorId, pos, (s32)yaw);
    // Display copy: skip init entirely (no carriedSync registration/suppression, no collide
    // func — it can never be collected locally) and never collide with the local player.
    actor->volatile_initialized = true;
    actor->marker->propPtr->unk8_3 = false;
    if (sRemoteCarry[slot].throwPending) {
        // The throw beat the spawn: launch straight into flight and release the slot.
        __remoteCarry_launch(actor, sRemoteCarry[slot].throwStart, sRemoteCarry[slot].throwTarget);
        __remoteCarry_clearSlot(slot);
    } else {
        sRemoteCarry[slot].marker = actor->marker;
        subaddie_set_state(actor, 6);
    }
}

void port_remoteCarry_setCarried(u32 clientId, s32 markerId) {
    s32 slot;
    if (clientId == 0) {
        return;
    }
    if (markerId != 0 && __remoteCarry_actorIdForMarker(markerId) == 0) {
        markerId = 0; // not a synced carryable
    }
    slot = __remoteCarry_findByClient(clientId);
    if (markerId == 0) {
        if (slot >= 0 && !sRemoteCarry[slot].throwPending) {
            if (sRemoteCarry[slot].marker != NULL) {
                marker_despawn(sRemoteCarry[slot].marker);
            }
            __remoteCarry_clearSlot(slot);
        }
        return;
    }
    if (slot >= 0) {
        if (sRemoteCarry[slot].markerId == markerId) {
            return; // steady state
        }
        // Switched to a different carryable: drop the old display copy first.
        if (sRemoteCarry[slot].marker != NULL) {
            marker_despawn(sRemoteCarry[slot].marker);
        }
        __remoteCarry_clearSlot(slot);
    }
    slot = __remoteCarry_findByClient(0);
    if (slot < 0) {
        return; // registry full — display-only, safe to drop
    }
    sRemoteCarry[slot].clientId = clientId;
    sRemoteCarry[slot].markerId = markerId;
    sRemoteCarry[slot].marker = NULL;
    sRemoteCarry[slot].throwPending = 0;
    __spawnQueue_add_2((void (*)(void))__remoteCarry_spawnMethod, markerId, clientId);
}

void port_remoteCarry_throw(u32 clientId, s32 markerId, f32 start[3], f32 target[3]) {
    s32 slot;
    if (clientId == 0 || __remoteCarry_actorIdForMarker(markerId) == 0) {
        return;
    }
    slot = __remoteCarry_findByClient(clientId);
    if (slot >= 0 && sRemoteCarry[slot].marker != NULL) {
        // Carried copy exists: launch it (the flight is autonomous, so free the slot).
        Actor *actor = marker_getActor(sRemoteCarry[slot].marker);
        __remoteCarry_launch(actor, start, target);
        __remoteCarry_clearSlot(slot);
        return;
    }
    if (slot < 0) {
        // The carry update never arrived (or was already cleared): spawn straight into flight.
        slot = __remoteCarry_findByClient(0);
        if (slot < 0) {
            return;
        }
        sRemoteCarry[slot].clientId = clientId;
        __spawnQueue_add_2((void (*)(void))__remoteCarry_spawnMethod, markerId, clientId);
    }
    sRemoteCarry[slot].markerId = markerId;
    sRemoteCarry[slot].throwPending = 1;
    sRemoteCarry[slot].throwStart[0] = start[0];
    sRemoteCarry[slot].throwStart[1] = start[1];
    sRemoteCarry[slot].throwStart[2] = start[2];
    sRemoteCarry[slot].throwTarget[0] = target[0];
    sRemoteCarry[slot].throwTarget[1] = target[1];
    sRemoteCarry[slot].throwTarget[2] = target[2];
}

// All actors are being (or were just) freed — drop every tracked display copy without
// touching the now-dangling markers. Called from actorArray_free.
void port_remoteCarry_reset(void) {
    s32 i;
    for (i = 0; i < REMOTE_CARRY_MAX; i++) {
        __remoteCarry_clearSlot(i);
    }
}

// Per-frame behavior for the display copies. Returns nonzero if this actor is one (the
// caller must then skip ALL vanilla logic — collect, flags, despawn checks — for it).
// Non-static as port_remoteCarry_displayUpdate: the CCW worm/acorn actors (caterpillar.c,
// acorn.c) run their display copies through this too, from their own update funcs.
s32 port_remoteCarry_displayUpdate(Actor *this) {
    s32 slot = __remoteCarry_findByMarker(this->marker);

    if (this->state == 6 || slot >= 0) { // carried: glued to the owner's dummy
        f32 pos[3];
        f32 yaw = 0.0f;
        if (slot < 0 || !port_anchor_getDummyTransform(sRemoteCarry[slot].clientId, pos, &yaw)) {
            // Owner left our map / disconnected — the display copy goes with them.
            if (slot >= 0) {
                __remoteCarry_clearSlot(slot);
            }
            marker_despawn(this->marker);
            return 1;
        }
        this->position[0] = pos[0];
        this->position[1] = pos[1];
        this->position[2] = pos[2];
        this->yaw = yaw;
        return 1;
    }

    if (this->state == 7) { // thrown: replay the arc, land with feedback only
        f32 landY;
        this->position[0] += this->velocity[0];
        this->position[1] += (this->velocity[1] -= 5.0);
        this->position[2] += this->velocity[2];
        landY = (--this->unk38_31 < 4) ? this->unk1C[1] : this->position[1];
        if (this->position[1] < landY) {
            // Worms/acorns are consumed at delivery (eaten by Eyrie / stashed by Nabnut) — the
            // thrower's real object despawns with just a ding (state 4 there), so the display
            // copy does too.
            if (this->modelCacheIndex == ACTOR_2A2_CATERPILLAR || this->modelCacheIndex == ACTOR_2A9_ACORN) {
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 28000);
                marker_despawn(this->marker);
                return 1;
            }
            if (this->modelCacheIndex == ACTOR_2A_GOLD_BULLION) {
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 32000);
            }
            if (this->marker->id != MARKER_36_ORANGE_COLLECTIBLE) {
                sfx_playFadeShorthandDefault(SFX_21_EGG_BOUNCE_1, 0.76f, 25000, this->position, 1000, 2000);
            } else {
                sfx_playFadeShorthandDefault(SFX_B3_ORANGE_TALKING, 1.0f, 25000, this->position, 1000, 2000);
            }
            // Land like the thrower's real object does (state 4 -> snap to target height -> sits
            // at the delivery spot): stay as an inert landed display (state 8). Despawning here
            // made the delivered object vanish on teammates' screens the moment it landed.
            this->position[1] = landY;
            subaddie_set_state(this, 8);
            return 1;
        }
        switch (this->marker->id) {
            case MARKER_1FD_BLUE_PRESENT_COLLECTIBLE:
                __chLevelCollectible_presentReturnEmitSparkles(this->position, ASSET_711_SPRITE_SPARKLE_DARK_BLUE);
                break;
            case MARKER_1FE_GREEN_PRESENT_COLLECTIBLE:
                __chLevelCollectible_presentReturnEmitSparkles(this->position, 0x712);
                break;
            case MARKER_1FF_RED_PRESENT_COLLECTIBLE:
                __chLevelCollectible_presentReturnEmitSparkles(this->position, ASSET_715_SPRITE_SPARKLE_RED);
                break;
        }
        return 1;
    }

    if (this->state == 8) { // landed display copy: sits where it landed, display only
        if (this->modelCacheIndex == ACTOR_2A_GOLD_BULLION) {
            // Match the real landed gold's idle spin (func_802D83EC), minus its flag-driven
            // despawn and particles — this copy must never touch game state.
            this->yaw += time_getDelta() * 25.0f;
            if (360.0 < this->yaw) {
                this->yaw -= 360.0;
            }
        }
        return 1;
    }

    return 0;
}
extern void timed_mapSpecificFlags_setTrue(f32, s32);
extern void progressDialog_showDialogMaskFour(s32);

ActorAnimationInfo D_80367B50[] = {
    {0, 0.0f},
    {ASSET_18A_ANIM_XMAS_GIFT, 1.5f},
    {ASSET_18A_ANIM_XMAS_GIFT, 1.5f},
    {ASSET_18A_ANIM_XMAS_GIFT, 1.5f},
    {ASSET_18A_ANIM_XMAS_GIFT, 1.5f},
    {ASSET_18A_ANIM_XMAS_GIFT, 1.5f}
};

ActorInfo D_80367B80 = {
    MARKER_36_ORANGE_COLLECTIBLE, ACTOR_29_ORANGE_COLLECTIBLE, ASSET_2D2_MODEL_ORANGE,
    0x5, NULL,
    chLevelCollectible_update, actor_update_func_80326224, actor_draw,
    0, 0, 0.6f,0
};

ActorInfo D_80367BA4 = {
    MARKER_37_GOLD_BULLION, ACTOR_2A_GOLD_BULLION, ASSET_3C7_MODEL_GOLD_BULLION,
    0x5, NULL,
    chLevelCollectible_update, actor_update_func_80326224, actor_draw,
    0, 0, 0.6f, 0
};

ActorInfo D_80367BC8 = {
    MARKER_1FD_BLUE_PRESENT_COLLECTIBLE, ACTOR_1ED_BLUE_PRESENT_COLLECTIBLE, ASSET_47F_MODEL_XMAS_GIFT_BLUE,
    0x5, D_80367B50,
    chLevelCollectible_update, actor_update_func_80326224, actor_draw,
    0, 0, 1.8f, 0
};

ActorInfo D_80367BEC = {
    MARKER_1FE_GREEN_PRESENT_COLLECTIBLE, ACTOR_1EF_GREEN_PRESENT_COLLECTIBLE, ASSET_480_MODEL_XMAS_GIFT_GREEN,
    0x5, D_80367B50,
    chLevelCollectible_update, actor_update_func_80326224, actor_draw,
    0, 0, 1.4f, 0
};

ActorInfo D_80367C10 = {
    MARKER_1FF_RED_PRESENT_COLLECTIBLE, ACTOR_1F1_RED_PRESENT_COLLECTIBLE, ASSET_481_MODEL_XMAS_GIFT_RED,
    0x5, D_80367B50,
    chLevelCollectible_update, actor_update_func_80326224, actor_draw,
    0, 0, 1.4f, 0
};

/* .code */
void __chLevelCollectible_presentReturnEmitSparkles(f32 position[3], enum asset_e sprite_id) {
    static ParticleScaleAndLifetimeRanges D_80367C34 = {{0.2f, 0.4f}, {0.1f, 0.1f}, {0.0f, 0.01f}, {3.0f, 3.5f}, 0.1f, 0.1f};
    ParticleEmitter *p_emitter;

    p_emitter = partEmitMgr_newEmitter(1);
    particleEmitter_setSprite(p_emitter, sprite_id);
    particleEmitter_setStartingFrameRange(p_emitter, 0, 7);
    particleEmitter_setPosition(p_emitter, position);
    particleEmitter_setSpawnPositionRange(p_emitter, -40.0f, 0.0f, -40.0f, 40.0f, 60.0f, 40.0f);
    particleEmitter_setAccelerationRange(p_emitter, 0.0f, -1000.0f, 0.0f, 0.0f, -1000.0f, 0.0f);
    particleEmitter_setScaleAndLifetimeRanges(p_emitter, &D_80367C34);
    particleEmitter_emitN(p_emitter, 1);
}

void __chLevelCollectible_presentCollectEmitSparkles(f32 position[3], enum asset_e sprite_id) {
    ParticleEmitter *p_emitter;

    p_emitter = partEmitMgr_newEmitter(8);
    particleEmitter_setSprite(p_emitter, sprite_id);
    particleEmitter_setPosition(p_emitter, position);
    particleEmitter_setAccelerationRange(p_emitter, 0.0f, -250.0f, 0.0f, 0.0f, -250.0f, 0.0f);
    particleEmitter_setParticleVelocityRange(p_emitter, -100.0f, 200.0f, -100.0f, 100.0f, 350.0f, 100.0f);
    particleEmitter_setAngularVelocityRange(p_emitter, 0.0f, 0.0f, 200.0f, 0.0f, 0.0f, 240.0f);
    particleEmitter_setStartingScaleRange(p_emitter, 0.47f, 0.47f);
    particleEmitter_setFinalScaleRange(p_emitter, 0.03f, 0.03f);
    particleEmitter_setFade(p_emitter, 0.4f, 0.8f);
    particleEmitter_setParticleLifeTimeRange(p_emitter, 0.9f, 0.9f);
    particleEmitter_emitN(p_emitter, 8);
}

s32 __chLevelCollectible_dialogCallback(ActorMarker *marker, enum asset_e text_id, s32 arg2) {
    return -(levelSpecificFlags_get(LEVEL_FLAG_2A_FP_UNKNOWN) + levelSpecificFlags_get(LEVEL_FLAG_2B_FP_UNKNOWN) + levelSpecificFlags_get(LEVEL_FLAG_2C_FP_UNKNOWN));
}


void __chLevelCollectible_callDialog(enum asset_e text_id){
    gcdialog_showDialogConditional(text_id, 0, NULL, NULL, NULL, NULL, (s32(*)(ActorMarker*,s32,s32))__chLevelCollectible_dialogCallback);
}

void __chLevelCollectible_collide(ActorMarker *marker, ActorMarker *other_marker) {
    Actor *this;
    f32 pad28;
    s32 dialog_id;
    f32 sp18[3];

    this = marker_getActor(marker);
    dialog_id = 0;
    if ((this->state == 1) || (this->state == 2)) {
        sp18[0] = this->position[0];
        sp18[1] = this->position[1];
        sp18[2] = this->position[2];
        switch (marker->id) {
            case MARKER_36_ORANGE_COLLECTIBLE:
                if (mapSpecificFlags_get(MM_SPECIFIC_FLAG_1_ORANGE_HAS_BEEN_COLLECTED)) {
                    return;
                }

                progressDialog_showDialogMaskFour(FILEPROG_8_ORANGE_TEXT);
                func_8030E6D4(SFX_B3_ORANGE_TALKING);
                dialog_id = 0;
                break;
                
            case MARKER_37_GOLD_BULLION:
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 0x7FFF);
                timedFunc_set_1(0.5f, (GenFunction_1)progressDialog_showDialogMaskFour, FILEPROG_9_GOLD_BULLION_TEXT);
                dialog_id = 0;
                break;

            case MARKER_1FD_BLUE_PRESENT_COLLECTIBLE:
                levelSpecificFlags_set(LEVEL_FLAG_2A_FP_UNKNOWN, true);
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 0x7FFF);
                __chLevelCollectible_presentCollectEmitSparkles(this->position, ASSET_711_SPRITE_SPARKLE_DARK_BLUE);
                dialog_id = ASSET_C20_DIALOG_PRESENT_COLLECTIBLE_MEET_BLUE;
                break;

            case MARKER_1FE_GREEN_PRESENT_COLLECTIBLE:
                levelSpecificFlags_set(LEVEL_FLAG_2B_FP_UNKNOWN, true);
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 0x7FFF);
                __chLevelCollectible_presentCollectEmitSparkles(this->position, ASSET_712_SPRITE_SPARKLE_GREEN);
                dialog_id = ASSET_C21_DIALOG_PRESENT_COLLECTIBLE_MEET_GREEN;
                break;

            case MARKER_1FF_RED_PRESENT_COLLECTIBLE:
                levelSpecificFlags_set(LEVEL_FLAG_2C_FP_UNKNOWN, true);
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 0x7FFF);
                __chLevelCollectible_presentCollectEmitSparkles(this->position, ASSET_715_SPRITE_SPARKLE_RED);
                dialog_id = ASSET_C22_DIALOG_PRESENT_COLLECTIBLE_MEET_RED;
                break;

            default:
                break;
        }
        if (dialog_id != 0) {
            timedFunc_set_1(0.5f, (GenFunction_1)__chLevelCollectible_callDialog, dialog_id);
        }
        func_8028F030(this->modelCacheIndex);
        {
            // [port] Broadcast the pickup so this world object despawns on teammates too; the shared
            // count rides the ITEM_* item-count delta via func_8028F030's item_inc.
            s32 kind = levelCollectible_syncKind(marker->id);
            if (kind >= 0) {
                port_carriedSync_onLocalCollect(kind, marker);
            }
        }
        marker_despawn(marker);
    }
}

void func_802D7DE8(ActorMarker *marker, f32 arg1[3]) {
    Actor *this;
    s32 sp50[3];
    s32 sp4C;
    f32 var_f12;
    f32 var_f14;
    f32 var_f18;

    sp4C = marker->id;
    this = marker_getActor(marker);
    ml_vec3f_to_vec3w(sp50, arg1);
    if (sp4C == MARKER_37_GOLD_BULLION) {
        if (mapSpecificFlags_get(0)) {
            mapSpecificFlags_set(1, true);
        } else {
            mapSpecificFlags_set(0, true);
        }
    }
    func_8028F010(this->modelCacheIndex);
    {
        // [port] Spending it (returning to Blubber / the snowman) — sync the -1 to the shared pool.
        s32 kind = levelCollectible_syncKind(marker->id);
        if (kind >= 0) {
            port_carriedSync_onLocalSpend(kind);
        }
    }
    subaddie_set_state(this, 4);
    var_f12 = this->position[1];
    var_f14 = 28.0f;
    var_f18 = 0.0f;
    this->unk1C[0] = arg1[0];
    this->unk1C[1] = arg1[1];
    this->unk1C[2] = arg1[2];
    this->velocity[0] = (f32) sp50[0] - this->position[0];
    this->velocity[1] = 28.0f;
    this->velocity[2] = (f32) sp50[2] - this->position[2];
    while (!(var_f12 < sp50[1]) || !(var_f14 < 0.0f)) {
        var_f18 += 1.0f;
        var_f12 += (var_f14 -= 5.0);
    }
    this->velocity[0] /= var_f18;
    this->velocity[2] /= var_f18;
    this->unk38_31 = (u32)var_f18;
}

void func_802D8030(Actor *this){
    s32 *local;

    local = (s32*)&this->local;
    *local = 1;
    this->marker->collisionFunc = __chLevelCollectible_collide;
    subaddie_set_state(this, 2);
}

void __chLevelCollectible_returnObj(Actor *this) {
    s32 *local;
    f32 sp20;

    local = (s32*)&this->local;
    if( (this->marker->id != MARKER_36_ORANGE_COLLECTIBLE) 
        || (this->secondaryId == 0)
    ) {
        this->position[0] += this->velocity[0];
        this->position[1] += (this->velocity[1] -= 5.0);
        this->position[2] += this->velocity[2];
    }
    if (--this->unk38_31 < 4) {
        sp20 = this->unk1C[1];
    } else {
        sp20 = this->position[1];
    }
    if (this->position[1] < sp20) {
        if (this->modelCacheIndex == ACTOR_2A_GOLD_BULLION) {
            coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 32000);
            if (mapSpecificFlags_get(1)) {
                coMusicPlayer_playMusic(COMUSIC_2D_PUZZLE_SOLVED_FANFARE, 32000);
            }
        }
        this->position[1] = sp20;
        if (this->marker->id != MARKER_36_ORANGE_COLLECTIBLE) {
            sfx_playFadeShorthandDefault(SFX_21_EGG_BOUNCE_1, 0.76f, 25000, this->position, 1000, 2000);
        } else {
            sfx_playFadeShorthandDefault(SFX_B3_ORANGE_TALKING, 1.0f, 25000, this->position, 1000, 2000);
        }
        if (this->state == 4) {
            switch (this->marker->id) {
            case MARKER_37_GOLD_BULLION:
                break;
            case MARKER_36_ORANGE_COLLECTIBLE:
                timed_mapSpecificFlags_setTrue(1.7f, MM_SPECIFIC_FLAG_2_ORANGE_HAS_BEEN_RETURNED);
                coMusicPlayer_playMusic(COMUSIC_2D_PUZZLE_SOLVED_FANFARE, 0x7FFF);
                break;
            case MARKER_1FD_BLUE_PRESENT_COLLECTIBLE:
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 32000);
                levelSpecificFlags_set(LEVEL_FLAG_11_FP_UNKNOWN, true);
                break;
            case MARKER_1FE_GREEN_PRESENT_COLLECTIBLE:
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 32000);
                levelSpecificFlags_set(LEVEL_FLAG_12_FP_UNKNOWN, true);
                break;
            case MARKER_1FF_RED_PRESENT_COLLECTIBLE:
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 32000);
                levelSpecificFlags_set(LEVEL_FLAG_13_FP_UNKNOWN, true);
                break;
            }
        }
        this->unk138_22 = this->unk138_21 = 0;
        subaddie_set_state(this, 2);
    }
    switch (this->marker->id) {
        case MARKER_1FD_BLUE_PRESENT_COLLECTIBLE:
            __chLevelCollectible_presentReturnEmitSparkles(this->position, ASSET_711_SPRITE_SPARKLE_DARK_BLUE);
            break;
        case MARKER_1FE_GREEN_PRESENT_COLLECTIBLE:
            __chLevelCollectible_presentReturnEmitSparkles(this->position, 0x712);
            break;
        case MARKER_1FF_RED_PRESENT_COLLECTIBLE:
            __chLevelCollectible_presentReturnEmitSparkles(this->position, ASSET_715_SPRITE_SPARKLE_RED);
            break;
    }

    if (*local != 0) {
        *local = 0;
    }
}

void func_802D8374(Actor *this){
    s32 pad2C;
    f32 sp20[3];

    if(func_8028E86C() != this->marker){
        func_8028F050(this->modelCacheIndex);
        marker_despawn(this->marker);
    }
    else{
        if(this->unk138_21){
            func_8028EF28(sp20);
            // [port] Anchor: replay this throw on teammates' clients (same start + target =>
            // same arc). Sent before func_802D7DE8 so the position is still the launch point.
            port_anchor_onCarryThrow(this->marker->id, this->position, sp20);
            func_802D7DE8(this->marker, sp20);
        }
    }
}

void func_802D83EC(Actor *this) {
    s32 var_s0;

    // temp_f20 = D_80376D70;
    for(var_s0 = 0; var_s0 < 10; var_s0++){
        if (randf() < 0.03) {
            commonParticle_add(this->marker, var_s0 + 5, func_80329904);
            commonParticle_new(8, this->marker->unk14_21);
        }
    }
    this->yaw = this->yaw + time_getDelta() * 25.0f;
    if (360.0 < this->yaw) {
        this->yaw -= 360.0;
    }
    if (mapSpecificFlags_get(0x1F)) {
        marker_despawn(this->marker);
    }
}

void func_802D84F4(Actor *this){
    this->marker->propPtr->unk8_3 = ( this->state == 2 );
}

void chLevelCollectible_update(Actor *this){
    s32 marker_id;
    if(this->despawn_flag) return;

    // [port] Anchor: a teammate's carried/thrown display copy bypasses ALL vanilla logic —
    // it must never collect, set flags, register with carriedSync, or despawn-check.
    if (port_remoteCarry_displayUpdate(this)) {
        return;
    }

    if(!this->volatile_initialized){
        this->volatile_initialized = true;
        if( this->marker->id == MARKER_1FD_BLUE_PRESENT_COLLECTIBLE
            || this->marker->id == MARKER_1FE_GREEN_PRESENT_COLLECTIBLE
            || this->marker->id == MARKER_1FF_RED_PRESENT_COLLECTIBLE
        ){
            if(jiggyscore_isCollected(JIGGY_2E_FP_PRESENTS)){
                marker_despawn(this->marker);
                return;
            }
        }
        if(this->unk138_22){
            func_8028F7D4(0.0f, 0.0f);
            subaddie_set_state(this, 3);
        } else {
            // [port] Register the world collectible (not the carried instance) for networked live
            // despawn, keyed by its fixed spawn position. If a teammate already grabbed it this
            // session, don't present it.
            s32 kind = levelCollectible_syncKind(this->marker->id);
            if (kind >= 0) {
                s32 suppress;
                port_carriedSync_register(kind, this->marker, (s32)this->position[0],
                                          (s32)this->position[1], (s32)this->position[2], &suppress);
                if (suppress) {
                    marker_despawn(this->marker);
                    return;
                }
            }
        }
    }//L802D85DC

    // [port] A teammate grabbed this world collectible — despawn it here so it vanishes on every
    // client. Only the world instance (not the carried one) was registered, so this is a no-op for
    // carried/thrown copies.
    if (!this->unk138_22) {
        s32 kind = levelCollectible_syncKind(this->marker->id);
        if (kind >= 0 && port_carriedSync_consumeRemoteDespawn(kind, this->marker)) {
            marker_despawn(this->marker);
            return;
        }
    }

    switch(this->state){
        case 5:// 802D8604
            func_802D8030(this);
            break;

        case 1:// 802D8620
            __chLevelCollectible_returnObj(this);
            break;

        case 2:// 802D863C
            break;

        case 3:// 802D8650
            func_802D8374(this);
            break;

        case 4:// 802D866C
            __chLevelCollectible_returnObj(this);
            break;

        default:
            break;
    }

    marker_id = this->marker->id;

    switch(this->marker->id){

        case MARKER_37_GOLD_BULLION: //L802D86CC
            func_802D83EC(this);
            break;
        case MARKER_36_ORANGE_COLLECTIBLE: //L802D86DC
            if (mapSpecificFlags_get(MM_SPECIFIC_FLAG_3_CHIMPY_HAS_LEFT) && gsworld_getMap() == MAP_2_MM_MUMBOS_MOUNTAIN) {
                marker_despawn(this->marker);
            }
            break;
        
        case MARKER_1FD_BLUE_PRESENT_COLLECTIBLE:
        case MARKER_1FE_GREEN_PRESENT_COLLECTIBLE:
        case MARKER_1FF_RED_PRESENT_COLLECTIBLE:
            func_802D84F4(this);
            break;
    }
}
