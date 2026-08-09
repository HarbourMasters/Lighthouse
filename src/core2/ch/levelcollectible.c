// BanjoDecomp: core2/levelcollectible.c
#include <ultra64.h>
#include "functions.h"
#include "variables.h"
#include "port/Enhancements/Retention/Retention.h"

extern void func_8028F7D4(f32, f32);
void chLevelCollectible_update(Actor *this);
extern ActorMarker *bacarry_getMarkerWithExtraSteps(void);

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

extern s32 port_anchor_getDummyTransform(u32 clientId, f32 pos[3], f32* yaw);
extern void port_anchor_onCarryThrow(s32 markerId, f32 start[3], f32 target[3]);

void __chLevelCollectible_presentReturnEmitSparkles(f32 position[3], enum asset_e sprite_id);

#define REMOTE_CARRY_MAX 16
static struct {
    u32 clientId;        // 0 = free slot
    ActorMarker *marker; // NULL while the spawn is still queued
    s32 markerId;
    u8 throwPending;
    f32 throwStart[3];
    f32 throwTarget[3];
    f32 carryOff[3];     // held pose: object position relative to the owner
    f32 carryYawOff;     // held pose: object yaw relative to the owner's yaw
} sRemoteCarry[REMOTE_CARRY_MAX];

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
    sRemoteCarry[slot].carryOff[0] = sRemoteCarry[slot].carryOff[1] = sRemoteCarry[slot].carryOff[2] = 0.0f;
    sRemoteCarry[slot].carryYawOff = 0.0f;
}

static void __remoteCarry_setPose(s32 slot, f32 offset[3], f32 yawOffset) {
    sRemoteCarry[slot].carryOff[0] = offset[0];
    sRemoteCarry[slot].carryOff[1] = offset[1];
    sRemoteCarry[slot].carryOff[2] = offset[2];
    sRemoteCarry[slot].carryYawOff = yawOffset;
}

// Mirror of func_802D7DE8's arc math without its side effects. State 7 = remote flight.
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
    // Display copy: skip init, no collision with the local player.
    actor->volatile_initialized = true;
    actor->marker->propPtr->unk8_3 = false;
    if (sRemoteCarry[slot].throwPending) {
        __remoteCarry_launch(actor, sRemoteCarry[slot].throwStart, sRemoteCarry[slot].throwTarget);
        __remoteCarry_clearSlot(slot);
    } else {
        sRemoteCarry[slot].marker = actor->marker;
        subaddie_set_state(actor, 6);
    }
}

void port_remoteCarry_setCarried(u32 clientId, s32 markerId, f32 offset[3], f32 yawOffset) {
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
            __remoteCarry_setPose(slot, offset, yawOffset);
            return;
        }
        if (sRemoteCarry[slot].marker != NULL) {
            marker_despawn(sRemoteCarry[slot].marker);
        }
        __remoteCarry_clearSlot(slot);
    }
    slot = __remoteCarry_findByClient(0);
    if (slot < 0) {
        return; // registry full
    }
    sRemoteCarry[slot].clientId = clientId;
    sRemoteCarry[slot].markerId = markerId;
    sRemoteCarry[slot].marker = NULL;
    sRemoteCarry[slot].throwPending = 0;
    __remoteCarry_setPose(slot, offset, yawOffset);
    __spawnQueue_add_2((GenFunction_2)__remoteCarry_spawnMethod, markerId, clientId);
}

void port_remoteCarry_throw(u32 clientId, s32 markerId, f32 start[3], f32 target[3]) {
    s32 slot;
    if (clientId == 0 || __remoteCarry_actorIdForMarker(markerId) == 0) {
        return;
    }
    slot = __remoteCarry_findByClient(clientId);
    if (slot >= 0 && sRemoteCarry[slot].marker != NULL) {
        Actor *actor = marker_getActor(sRemoteCarry[slot].marker);
        __remoteCarry_launch(actor, start, target);
        __remoteCarry_clearSlot(slot);
        return;
    }
    if (slot < 0) {
        slot = __remoteCarry_findByClient(0);
        if (slot < 0) {
            return;
        }
        sRemoteCarry[slot].clientId = clientId;
        __spawnQueue_add_2((GenFunction_2)__remoteCarry_spawnMethod, markerId, clientId);
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

void port_remoteCarry_reset(void) {
    s32 i;
    for (i = 0; i < REMOTE_CARRY_MAX; i++) {
        __remoteCarry_clearSlot(i);
    }
}

s32 port_remoteCarry_displayUpdate(Actor *this) {
    s32 slot = __remoteCarry_findByMarker(this->marker);

    if (this->state == 6 || slot >= 0) { // carried: glued to the owner's dummy
        f32 pos[3];
        f32 yaw = 0.0f;
        if (slot < 0 || !port_anchor_getDummyTransform(sRemoteCarry[slot].clientId, pos, &yaw)) {
            if (slot >= 0) {
                __remoteCarry_clearSlot(slot);
            }
            marker_despawn(this->marker);
            return 1;
        }
        this->position[0] = pos[0] + sRemoteCarry[slot].carryOff[0];
        this->position[1] = pos[1] + sRemoteCarry[slot].carryOff[1];
        this->position[2] = pos[2] + sRemoteCarry[slot].carryOff[2];
        this->yaw = mlNormalizeAngle(yaw + sRemoteCarry[slot].carryYawOff);
        return 1;
    }

    if (this->state == 7) { // thrown: replay the arc, land with feedback only
        f32 landY;
        this->position[0] += this->velocity[0];
        this->position[1] += (this->velocity[1] -= 5.0);
        this->position[2] += this->velocity[2];
        landY = (--this->unk38_31 < 4) ? this->unk1C[1] : this->position[1];
        if (this->position[1] < landY) {
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
            // State 8: landed display copy.
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

    // State 8 (landed display copy) falls through to the vanilla lifecycle below.

    return 0;
}
extern void timed_mapSpecificFlags_setTrue(f32, s32);
extern void progressDialog_showDialogMaskFour(s32);

ActorAnimationInfo chPresentCollectibleAnimations[] = {
    {0, 0.0f},
    {ASSET_18A_ANIM_XMAS_GIFT, 1.5f},
    {ASSET_18A_ANIM_XMAS_GIFT, 1.5f},
    {ASSET_18A_ANIM_XMAS_GIFT, 1.5f},
    {ASSET_18A_ANIM_XMAS_GIFT, 1.5f},
    {ASSET_18A_ANIM_XMAS_GIFT, 1.5f}
};

ActorInfo chOrangeCollectible = {
    MARKER_36_ORANGE_COLLECTIBLE, ACTOR_29_ORANGE_COLLECTIBLE, ASSET_2D2_MODEL_ORANGE,
    0x5, NULL,
    chLevelCollectible_update, actor_update_func_80326224, actor_draw,
    0, 0, 0.6f,0
};

ActorInfo chGoldBullion = {
    MARKER_37_GOLD_BULLION, ACTOR_2A_GOLD_BULLION, ASSET_3C7_MODEL_GOLD_BULLION,
    0x5, NULL,
    chLevelCollectible_update, actor_update_func_80326224, actor_draw,
    0, 0, 0.6f, 0
};

ActorInfo chBluePresentCollectible = {
    MARKER_1FD_BLUE_PRESENT_COLLECTIBLE, ACTOR_1ED_BLUE_PRESENT_COLLECTIBLE, ASSET_47F_MODEL_XMAS_GIFT_BLUE,
    0x5, chPresentCollectibleAnimations,
    chLevelCollectible_update, actor_update_func_80326224, actor_draw,
    0, 0, 1.8f, 0
};

ActorInfo chGreenPresentCollectible = {
    MARKER_1FE_GREEN_PRESENT_COLLECTIBLE, ACTOR_1EF_GREEN_PRESENT_COLLECTIBLE, ASSET_480_MODEL_XMAS_GIFT_GREEN,
    0x5, chPresentCollectibleAnimations,
    chLevelCollectible_update, actor_update_func_80326224, actor_draw,
    0, 0, 1.4f, 0
};

ActorInfo chRedPresentCollectible = {
    MARKER_1FF_RED_PRESENT_COLLECTIBLE, ACTOR_1F1_RED_PRESENT_COLLECTIBLE, ASSET_481_MODEL_XMAS_GIFT_RED,
    0x5, chPresentCollectibleAnimations,
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
                dialog_id = VER_SELECT(ASSET_C20_DIALOG_PRESENT_COLLECTIBLE_MEET_BLUE, 0x99A, 0, 0);
                break;

            case MARKER_1FE_GREEN_PRESENT_COLLECTIBLE:
                levelSpecificFlags_set(LEVEL_FLAG_2B_FP_UNKNOWN, true);
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 0x7FFF);
                __chLevelCollectible_presentCollectEmitSparkles(this->position, ASSET_712_SPRITE_SPARKLE_GREEN);
                dialog_id = VER_SELECT(ASSET_C21_DIALOG_PRESENT_COLLECTIBLE_MEET_GREEN, 0x99B, 0, 0);
                break;

            case MARKER_1FF_RED_PRESENT_COLLECTIBLE:
                levelSpecificFlags_set(LEVEL_FLAG_2C_FP_UNKNOWN, true);
                coMusicPlayer_playMusic(COMUSIC_2B_DING_B, 0x7FFF);
                __chLevelCollectible_presentCollectEmitSparkles(this->position, ASSET_715_SPRITE_SPARKLE_RED);
                dialog_id = VER_SELECT(ASSET_C22_DIALOG_PRESENT_COLLECTIBLE_MEET_RED, 0x99C, 0, 0);
                break;

            default:
                break;
        }
        if (dialog_id != 0) {
            timedFunc_set_1(0.5f, (GenFunction_1)__chLevelCollectible_callDialog, dialog_id);
        }
        bacarriedobj_incWithExtraSteps(this->modelCacheIndex);
        {
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
    bacarriedobj_decWithExtraSteps(this->modelCacheIndex);
    {
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

    if(bacarry_getMarkerWithExtraSteps() != this->marker){
        bacarriedobj_displayOnHudWithExtraSteps(this->modelCacheIndex);
        marker_despawn(this->marker);
    }
    else{
        if(this->unk138_21){
            func_8028EF28(sp20);
            // [port] Anchor: replay this throw on teammates; sent before func_802D7DE8 moves the actor.
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
