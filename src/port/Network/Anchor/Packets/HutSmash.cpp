#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <array>
#include <cstring>
#include <map>
#include <vector>

#include "functions.h"
extern "C" {
#include "BGS/ch/mudhut.h"
#include "MM/ch/hut.h"

extern enum bundle_e chBGSMudHutDrops[6];
s32 gsworld_getUnk0();
void __chhut_spawnExplosion(ActorMarker* self);
void chMudHut_spawnExplosion(ActorMarker* self);
Actor* spawnQueue_bundle_f32(u32 bundle, f32 x, f32 y, f32 z);
}

/**
 * HUT_SMASH
 */

std::map<std::array<int32_t, 4>, int32_t> sHuts;

// hut.c's D_803898D8 (local to chhut_update): the MM huts' drops by smash count, short of the jiggy.
static const enum bundle_e kMmHutDrops[5] = {
    BUNDLE_0_MM_HUT_MUSIC_NOTE,  BUNDLE_1_MM_HUT_BLUE_EGG,   BUNDLE_2_MM_HUT_GRUBLIN,
    BUNDLE_3_MM_HUT_JINJO_GREEN, BUNDLE_6_MM_HUT_EXTRA_LIFE,
};

static int32_t recordedLoot(Actor* hut) {
    if (!Anchor::GetInstance()->IsWorldSyncActive()) {
        return -1;
    }
    auto it = sHuts.find(
        { (int32_t)gsworld_getMap(), (int32_t)hut->position_x, (int32_t)hut->position_y, (int32_t)hut->position_z });
    return it != sHuts.end() ? it->second : -1;
}

static int32_t teamSmashCount() {
    int32_t level = (int32_t)map_getLevel(gsworld_getMap());
    int32_t count = 0;
    for (const auto& [key, loot] : sHuts) {
        if ((int32_t)map_getLevel((enum map_e)key[0]) == level) {
            count++;
        }
    }
    return count;
}

static void dropRecordedBundle(Actor* hut, int32_t loot, bool fullBundle) {
    const bool mm = hut->modelCacheIndex == ACTOR_9_MM_HUT;
    if (loot < 0 || loot >= 5) {
        return; // jiggy / out of range: handled by JIGGY_SPAWN, not the hut
    }
    if (!fullBundle && (mm ? (loot == 0 || loot == 3) : loot == 2)) {
        return; // note / jinjo: spawn live but skip on reload
    }
    f32 pos[3] = { hut->position_x, hut->position_y + (mm ? 125.0f : 130.0f), hut->position_z };
    u32 bits[3];
    memcpy(bits, pos, sizeof(bits));
    __spawnQueue_add_4((GenFunction_4)spawnQueue_bundle_f32, mm ? kMmHutDrops[loot] : chBGSMudHutDrops[loot], bits[0],
                       bits[1], bits[2]);
}

static void replaySmash(Actor* hut, int32_t loot) {
    if (hut->modelCacheIndex == ACTOR_9_MM_HUT) {
        if (Anchor_InEarshot(hut->position)) {
            sfxsource_playHighPriority(SFX_5B_HEAVY_STUFF_FALLING);
        }
        subaddie_set_state(hut, HUT_STATE_1_DAMAGED);
        actor_playAnimationOnce(hut);
        if (EventSystem_Should(VB_MM_HUT_SPAWN_EXPLOSION, true, hut)) {
            __spawnQueue_add_1((GenFunction_1)__chhut_spawnExplosion, (uintptr_t)hut->marker);
        }
        bundle_setYaw(hut->yaw);
    } else {
        sfx_playFadeShorthandDefault(SFX_5B_HEAVY_STUFF_FALLING, 1.0f, 28000, hut->position, 0x12C, 0xBB8);
        subaddie_set_state(hut, CH_BGS_MUDHUT_STATES_2_BREAKING);
        hut->marker->propPtr->unk8_3 = 0;
        actor_playAnimationOnce(hut);
        if (loot == 5 && Anchor_InEarshot(hut->position)) {
            coMusicPlayer_playMusic(COMUSIC_2D_PUZZLE_SOLVED_FANFARE, 28000);
        }
        __spawnQueue_add_1((GenFunction_1)chMudHut_spawnExplosion, (uintptr_t)hut->marker);
    }
    dropRecordedBundle(hut, loot, true);
}

static void HutUpdate(IEvent* event) {
    auto* ev = (ShouldActorUpdate*)event;
    Actor* hut = ev->actor;
    const s32 intact = hut->modelCacheIndex == ACTOR_9_MM_HUT ? HUT_STATE_0_INTACT : CH_BGS_MUDHUT_STATES_1_IDLE;
    if (gsworld_getUnk0() != 2 || !hut->initialized || hut->state != intact) {
        return;
    }
    int32_t loot = recordedLoot(hut);
    if (loot >= 0) {
        replaySmash(hut, loot);
        *ev->should = false;
    }
}

// The MM huts drop by the team's smash count.
static void HutSmashed(IEvent* event) {
    auto* ev = (OnHutSmashed*)event;
    Actor* hut = ev->actor;
    if (!Anchor::GetInstance()->IsWorldSyncActive()) {
        return;
    }
    if (hut->modelCacheIndex == ACTOR_9_MM_HUT) {
        *ev->loot = teamSmashCount();
    }
    int32_t map = (int32_t)gsworld_getMap();
    std::array<int32_t, 4> key = { map, (int32_t)hut->position_x, (int32_t)hut->position_y, (int32_t)hut->position_z };
    if (!sHuts.emplace(key, *ev->loot).second) {
        return;
    }
    Anchor::GetInstance()->SendPacket_HutSmash(key[1], key[2], key[3], *ev->loot, map);
}

void Anchor::SendPacket_HutSmash(s32 x, s32 y, s32 z, s32 loot, s32 map) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = HUT_SMASH;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["x"] = x;
    payload["y"] = y;
    payload["z"] = z;
    payload["loot"] = loot;
    payload["map"] = map;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_HutSmash(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s32 x = payload.at("x").get<s32>();
    s32 y = payload.at("y").get<s32>();
    s32 z = payload.at("z").get<s32>();
    s32 loot = payload.at("loot").get<s32>();
    s32 map = payload.at("map").get<s32>();

    sHuts.emplace(std::array<int32_t, 4>{ map, x, y, z }, loot);
}

std::vector<int32_t> port_hutSmash_snapshot() {
    std::vector<int32_t> flat;
    flat.reserve(sHuts.size() * 5);
    for (const auto& [key, loot] : sHuts) {
        flat.push_back(key[0]);
        flat.push_back(key[1]);
        flat.push_back(key[2]);
        flat.push_back(key[3]);
        flat.push_back(loot);
    }
    return flat;
}

void port_hutSmash_restore(const std::vector<int32_t>& flat) {
    sHuts.clear();
    for (size_t i = 0; i + 5 <= flat.size(); i += 5) {
        sHuts[{ flat[i], flat[i + 1], flat[i + 2], flat[i + 3] }] = flat[i + 4];
    }
}

void port_hutSmash_clearForLevel(int32_t levelId) {
    std::erase_if(sHuts,
                  [levelId](const auto& kv) { return (int32_t)map_getLevel((enum map_e)kv.first[0]) == levelId; });
}

void Anchor_RegisterHutHooks(bool connected) {
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_9_MM_HUT, EVENT_PRIORITY_NORMAL, connected, HutUpdate);
    COND_ID_HOOK(ShouldActorUpdate, ACTOR_C_MUD_HUT, EVENT_PRIORITY_NORMAL, connected, HutUpdate);
}

void RegisterHutSmash_Init() {
    // At boot, with the rest of this file: the smashes do nothing until world sync is live.
    COND_HOOK(OnSaveLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) { sHuts.clear(); });

    COND_HOOK(OnHutSmashed, EVENT_PRIORITY_NORMAL, true, HutSmashed);

    // One a teammate smashed sets up broken, dropping again what doesn't last past a reload.
    COND_HOOK(OnHutSetUp, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        Actor* hut = ((OnHutSetUp*)event)->actor;
        int32_t loot = recordedLoot(hut);
        if (loot < 0) {
            return;
        }
        dropRecordedBundle(hut, loot, false);
        if (hut->modelCacheIndex == ACTOR_9_MM_HUT) {
            subaddie_set_state(hut, HUT_STATE_2_DESTROYED);
            hut->position_y -= 160.0f;
        } else {
            hut->state = CH_BGS_MUDHUT_STATES_3_BROKEN;
            hut->marker->propPtr->unk8_3 = 0;
        }
    });
}

static RegisterShipInitFunc initHutSmash(RegisterHutSmash_Init, {});
