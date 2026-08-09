// BanjoDecomp: hut.c
/*!!!DONE!!!*/
#include <ultra64.h>
#include "functions.h"
#include "variables.h"

#include "prop.h"

#include <bk_math.h>

#include "port/Patches/Patches.h"

/* extern function declarations */

void bundle_setYaw(f32);
Actor *spawnQueue_bundle_f32(s32, s32, s32, s32);

/* public function declarations */
Actor *chhut_draw(ActorMarker *, Gfx **, Mtx **, Vtx **);
void chhut_update(Actor *);

/* .bss */
extern s32 mm_hut_smash_count; //mm_hut_smash_count

/* .data */
enum chhut_state_e {
    HUT_STATE_0_INTACT,
    HUT_STATE_1_DAMAGED,
    HUT_STATE_2_DESTROYED
};

ActorAnimationInfo chhutAnimations[3] = {
    {0,                          0.0f},
    {ASSET_4E_ANIM_MUD_HUT_SMASH, 0.25f},
    {ASSET_4E_ANIM_MUD_HUT_SMASH, 1000000.0f}
};

/* .code */
Actor *chhut_draw(ActorMarker *this, Gfx **gfx, Mtx **mtx, Vtx **vtx) {
    Actor *actorPtr = marker_getActor(this);
    s32 is_not_destroyed  = actorPtr->state != HUT_STATE_2_DESTROYED;
    s32 is_intact_or_destroyed  = actorPtr->state == HUT_STATE_0_INTACT || actorPtr->state == HUT_STATE_2_DESTROYED;

    this->propPtr->unk8_3 = is_intact_or_destroyed;
    modelRender_setAppendageVisibility(1, is_not_destroyed);
    return actor_draw(this, gfx, mtx, vtx);
}

void __chhut_spawnExplosion(ActorMarker *this) {
    Actor *actorPtr = marker_getActor(this);
    actorPtr = actor_spawnWithYaw_f32(ACTOR_4B_WOOD_EXPLOSION_2, actorPtr->position, 0);
    actor_spawnWithYaw_f32(ACTOR_4D_STEAM_2, actorPtr->position, 0);

    if (this);
}

static enum bundle_e mm_hut_bundles[6] = {
    BUNDLE_0_MM_HUT_MUSIC_NOTE,
    BUNDLE_1_MM_HUT_BLUE_EGG,
    BUNDLE_2_MM_HUT_GRUBLIN,
    BUNDLE_3_MM_HUT_JINJO_GREEN,
    BUNDLE_6_MM_HUT_EXTRA_LIFE,
    BUNDLE_4_MM_HUT_JIGGY
};

static void chhut_dropRecordedBundle(Actor *this, s32 loot, s32 fullBundle) {
    f32 pos[3];
    if (loot < 0 || loot >= 5) {
        return; // jiggy / out of range
    }
    if (!fullBundle && (loot == 0 || loot == 3)) {
        return; // note / jinjo: skip on reload
    }
    pos[0] = this->position_x;
    pos[1] = this->position_y + 125.0f;
    pos[2] = this->position_z;
    __spawnQueue_add_4((GenFunction_4) spawnQueue_bundle_f32, mm_hut_bundles[loot], *(s32 *)(&pos[0]), *(s32 *)(&pos[1]), *(s32 *)(&pos[2]));
}

static void chhut_replaySmash(Actor *this, s32 loot) {
    sfxsource_playHighPriority(SFX_5B_HEAVY_STUFF_FALLING);
    subaddie_set_state(this, HUT_STATE_1_DAMAGED);
    actor_playAnimationOnce(this);
    __spawnQueue_add_1((GenFunction_1) __chhut_spawnExplosion, (uintptr_t)this->marker);
    bundle_setYaw(this->yaw);
    chhut_dropRecordedBundle(this, loot, 1);
}

void chhut_update(Actor *this) {
    f32 diff_pos[3];
    f32 plyr_pos[3];
    s32 loot;
    s32 smashIndex;

    if (gsworld_getUnk0() != 2) {
        return;
    }

    if (!this->initialized) {
        this->marker->collidable = false;
        this->initialized = true;
        loot = port_hutSmash_get((s32)this->position_x, (s32)this->position_y, (s32)this->position_z);
        if (loot >= 0) {
            chhut_dropRecordedBundle(this, loot, 0);
            subaddie_set_state(this, HUT_STATE_2_DESTROYED);
            this->position_y -= 160.0f;
            return;
        }
    }

    switch (this->state) {
        case HUT_STATE_0_INTACT:
            player_getPosition(plyr_pos);
            diff_pos[0] = plyr_pos[0] - this->position_x;
            diff_pos[1] = plyr_pos[1] - this->position_y;
            diff_pos[2] = plyr_pos[2] - this->position_z;

            if (150.0f < diff_pos[1]
                && player_getActiveHitbox(this->marker) == HITBOX_1_BEAK_BUSTER
                && player_isStableWithExtraSteps()
                && LENGTH_VEC3F(diff_pos) < 350.0f
            ){
                diff_pos[0] = this->position_x;
                diff_pos[1] = this->position_y;
                diff_pos[2] = this->position_z;
                diff_pos[1] += 125.0;

                sfxsource_playHighPriority(SFX_5B_HEAVY_STUFF_FALLING);
                subaddie_set_state(this, HUT_STATE_1_DAMAGED);
                actor_playAnimationOnce(this);
                __spawnQueue_add_1((GenFunction_1) __chhut_spawnExplosion, (uintptr_t)this->marker);
                bundle_setYaw(this->yaw);

                smashIndex = port_hutSmash_countForCurrentLevel();
                if (smashIndex < 5) {
                    __spawnQueue_add_4((GenFunction_4) spawnQueue_bundle_f32, mm_hut_bundles[smashIndex], *(s32 * )(&diff_pos[0]), *(s32 * )(&diff_pos[1]), *(s32 * )(&diff_pos[2]));
                }
                else {
                    jiggy_spawn(JIGGY_5_MM_HUTS, diff_pos);
                }

                port_hutSmash_record((s32)this->position_x, (s32)this->position_y, (s32)this->position_z, smashIndex);
            }
            else {
                loot = port_hutSmash_get((s32)this->position_x, (s32)this->position_y, (s32)this->position_z);
                if (loot >= 0) {
                    chhut_replaySmash(this, loot);
                }
            }
            break;

        case HUT_STATE_1_DAMAGED:
            if (anctrl_getAnimTimer(this->anctrl) > 0.99) {
                anctrl_setTransitionDuration(this->anctrl, 0.0f);
                subaddie_set_state(this, HUT_STATE_2_DESTROYED);
                this->position_y -= 160.0f;
            }
            break;

        case HUT_STATE_2_DESTROYED:
            break;
    }
}

void mm_resetHuts(void) {
    mm_hut_smash_count = 0;
}

ActorInfo chhutInfo = {
    MARKER_51_MM_HUT, ACTOR_9_MM_HUT, ASSET_7D7_MODEL_MM_HUT,
    0, chhutAnimations,
    chhut_update, actor_update_func_80326224, chhut_draw,
    0, 0x100, 0.0f, 0
};
