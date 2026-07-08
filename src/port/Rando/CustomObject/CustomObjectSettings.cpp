#include "CustomObject.h"
#include "port/Rando/Rando.h"
#include "port/ShipUtils.h"
#include "port/Save/Types.h"
#include <map>

#include "save.h"

#define BUNDLE_STATE_DEFAULT 1

typedef struct {
    s32 index;
    s16 flags;
    s16 unk6;
    f32 position[3];
    f32 velocity[3];
    f32 yaw;
    f32 yaw_speed;
    f32 elapsed_time;
    u8 unk2C;
    u8 unk2D;
    u8 unk2E;
    u8 state;
} Bundle;

typedef struct {
    f32 velocity_x;
    f32 velocity_y;
    f32 velocity_z;
    f32 randomVelocity_x;
    f32 randomVelocity_y;
    f32 randomVelocity_z;
    f32 yaw;
    int32_t flags;
} BundlePhysics;

extern "C" {
extern SaveData gameFile_saveData[4];
extern u8 D_80385FF0[0xE];
void ml_vec3f_copy(f32 dst[3], f32 src[3]);
extern f32 gBundle_randomVelocity;
extern f32 gBundle_yaw;
f32 randf2(f32 min, f32 max);
f32 randf(void);
void ml_vec3f_yaw_rotate_copy(f32 dst[3], f32 src[3], f32 yaw);
}

static f32 baseSpeed = 400.0f;

// clang-format off
std::map<RandoCheckId, BundlePhysics> customActorPhysicsMap = {
    { RC_UNKNOWN,                                   { 0, 0, 0, 0, 0, 0, 0, 0 } },
    { RC_BGS_JIGGY_TANKTUP,                         { 0, 100.0f, 300.0f, 0, 0, 0, 0, 0x1 } },
    { RC_GL_JIGGY_WITCH_SWITCH_TREASURE_TROVE_COVE, { 0, 100.0f, 2800.0f, 0, 0, 0, 0, 0x1 } },
    { RC_MM_JIGGY_CHIMPY,                           { 0, 800.0f, 0, 0, 10.0f, 0, 0, 0x1 } },
    { RC_MM_JIGGY_CONGA,                            { 350.0f, 275.0f, 0, 0, 0, 0, 0, 0x1 } },
    { RC_MM_JIGGY_JUJU,                             { 0, 300.0f, 0, 0, 10.0f, 0, 0, 0x1 } },
    { RC_MM_NOTE_HUT_BUNDLE_1,                      { 125.0f, 725.0f, 125.0f, 0, 0, 0, 0, 0x1 } },
};
// clang-format on

BundlePhysics jinjoJiggySpawnPhysics = { 0, 300.0f, 0, 0, 10.0f, 0, 0, 0x1 };

BundlePhysics GetPhysicsByCheckId(RandoCheckId randoCheckId) {
    for (auto& [check, physics] : customActorPhysicsMap) {
        if (check == randoCheckId) {
            return physics;
        }
    }

    return customActorPhysicsMap.at(RC_UNKNOWN);
}

void UpdateSaveDataNoteScores() {
    u64* noteData = (u64*)&gameFile_saveData[selectedFileNum].data[NOTE_OFFSET];
    u64 currentSaveData;
    u64 packed_notes = 0;
    int8_t level_id = (int8_t)LEVEL_A_MAD_MONSTER_MANSION;

    for (int i = 0; i <= LEVEL_A_MAD_MONSTER_MANSION; i++) {
        packed_notes <<= 7;
        packed_notes |= (D_80385FF0[level_id] & 0x7F);

        level_id--;
        if (level_id == LEVEL_6_LAIR) {
            level_id--;
        }
    }

    currentSaveData = *noteData;
    currentSaveData &= ~((u64)0x7FFFFFFFFFFFFFFF);
    currentSaveData |= (packed_notes & 0x7FFFFFFFFFFFFFFF);

    *noteData = currentSaveData;
}

void ApplyBundleActorPhysics(Actor* actor, int32_t bundle_id, BundleInfo* bundle_info, f32 bundleYaw) {
    actor->is_bundle = true;
    Bundle* bundle = (Bundle*)&actor->unkBC;

    bundle->index = bundle_id;
    bundle->state = BUNDLE_STATE_DEFAULT;
    bundle->unk6 = 1;

    ml_vec3f_copy(bundle->position, actor->position);
    ml_vec3f_copy(actor->position, bundle->position);

    if (gBundle_randomVelocity != 1.0f) {
        bundle->velocity[0] = bundle_info->velocity_x * gBundle_randomVelocity;
        bundle->velocity[1] = bundle_info->velocity_y + randf2(0.0f, bundle_info->randomVelocity_y);
        bundle->velocity[2] = bundle_info->velocity_z * gBundle_randomVelocity;
        gBundle_randomVelocity = 1.0f;
    } else {
        bundle->velocity[0] = bundle_info->velocity_x + randf2(0.0f, bundle_info->randomVelocity_x);
        bundle->velocity[1] = bundle_info->velocity_y + randf2(0.0f, bundle_info->randomVelocity_y);
        bundle->velocity[2] = bundle_info->velocity_z + randf2(0.0f, bundle_info->randomVelocity_z);
    }

    ml_vec3f_yaw_rotate_copy(bundle->velocity, bundle->velocity, bundleYaw);

    bundle->yaw_speed = (baseSpeed *= -1.0f);

    float targetYaw = (bundle_info->flags & 0x20) ? bundle_info->yaw : randf2(0.0f, 360.0f);
    actor->yaw = bundle->yaw = targetYaw;

    bundle->elapsed_time = 0.0f;
    bundle->unk2C = 0;
    bundle->unk2D = 1;
    bundle->flags = bundle_info->flags;
    bundle->unk2E = (bundle_info->flags & 0x1) ? (randf() > 0.5f) : 0;

    if (bundle_info->flags & 0x200) {
        actor->unk5C = bundle->position[1];
    }
}

void ApplyCustomActorPhysics(RandoCheckId randoCheckId, Actor* actor, bool isJinjoJiggy) {
    if (actor == NULL) {
        return;
    }

    float bundleYaw = gBundle_yaw;
    BundlePhysics physicsData;

    if ((randoCheckId >= RC_MM_NOTE_HUT_BUNDLE_1 && randoCheckId <= RC_MM_NOTE_HUT_BUNDLE_5) ||
        randoCheckId == RC_MM_JINJO_GREEN || randoCheckId == RC_MM_JIGGY_HUTS ||
        (randoCheckId >= RC_MM_BLUE_EGG_HUT_BUNDLE_1 && randoCheckId <= RC_MM_BLUE_EGG_HUT_BUNDLE_5) ||
        randoCheckId == RC_MM_EXTRA_LIFE_HUT) {
        physicsData = GetPhysicsByCheckId(RC_MM_NOTE_HUT_BUNDLE_1);
    } else {
        physicsData = isJinjoJiggy ? jinjoJiggySpawnPhysics : GetPhysicsByCheckId(randoCheckId);
    }

    actor->is_bundle = true;
    Bundle* bundle = (Bundle*)&actor->unkBC;

    bundle->index = 0;
    bundle->state = BUNDLE_STATE_DEFAULT;
    bundle->unk6 = 1;

    ml_vec3f_copy(bundle->position, actor->position);
    ml_vec3f_copy(actor->position, bundle->position);

    bundle->velocity[0] = physicsData.velocity_x + randf2(0.0f, physicsData.randomVelocity_x);
    bundle->velocity[1] = physicsData.velocity_y + randf2(0.0f, physicsData.randomVelocity_y);
    bundle->velocity[2] = physicsData.velocity_z + randf2(0.0f, physicsData.randomVelocity_z);

    ml_vec3f_yaw_rotate_copy(bundle->velocity, bundle->velocity, bundleYaw);
    bundle->yaw_speed = (baseSpeed *= -1.0f);
    float targetYaw = (physicsData.flags & 0x20) ? physicsData.yaw : randf2(0.0f, 360.0f);
    actor->yaw = bundle->yaw = targetYaw;

    bundle->elapsed_time = 0.0f;
    bundle->unk2C = 0;
    bundle->unk2D = 1;
    bundle->flags = physicsData.flags;
    bundle->unk2E = (physicsData.flags & 0x1) ? (randf() > 0.5f) : 0;
}
