#include "port/Network/Anchor/Anchor.h"
#include <nlohmann/json.hpp>
#include <libultraship/libultraship.h>

#include "port/ShipInit.hpp"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Enhancements/Events/Hooks/Events.h"

#include <algorithm>
#include <array>
#include <set>

#include "functions.h"
extern "C" {
extern ActorArray* suBaddieActorArray;
void code_4C020_breakableObjectSfx(ActorMarker* marker, ActorMarker* other_marker);
}

#include "port/Patches/Patches.h"

/**
 * BREAK_OBJECT
 */

std::set<std::array<int32_t, 5>> sBrokenObjects;
static bool sBreakReplay = false;

// fieldSel picks between two markers sharing an id: 0 = any, 1 = actorTypeSpecificField==1, 2 = !=1.
struct LairBreakable {
    s16 flag;
    s16 markerId;
    s8 fieldSel;
};

static const LairBreakable kLairBreakables[] = {
    { FILEPROG_A5_LAIR_CRYPT_GATE_OPEN, 0x17D, 0 },
    { FILEPROG_C2_GRATE_TO_RBB_PUZZLE_OPEN, 0x11A, 0 },
    { FILEPROG_C3_ICE_BALL_TO_CHEATO_BROKEN, 0x11E, 0 },
    { FILEPROG_C4_STATUE_EYE_BROKEN, MARKER_121_GLASS_EYE, 0 },
    { FILEPROG_C5_RAREWARE_BOX_BROKEN, 0x11F, 0 },
    { FILEPROG_C8_LAIR_BRICKWALL_TO_WADINGBOOTS_BROKEN, MARKER_109_BREAKABLE_BRICK_WALL, 1 },
    { FILEPROG_C9_LAIR_BRICKWALL_TO_SHOCKJUMP_PAD_BROKEN, MARKER_109_BREAKABLE_BRICK_WALL, 2 },
    { FILEPROG_CA_COBWEB_BLOCKING_PURPLE_CAULDRON_BROKEN, MARKER_225_BREAKABLE_WALL_COBWEB, 0 },
    { FILEPROG_CB_LAIR_COBWEB_OVER_FLIGHTPAD_BROKEN, MARKER_224_BREAKABLE_FLOOR_COBWEB, 1 },
    { FILEPROG_CC_LAIR_COBWEB_OVER_GREEN_CAULDRON_BROKEN, MARKER_224_BREAKABLE_FLOOR_COBWEB, 2 },
    { FILEPROG_CD_GRATE_TO_WATER_SWITCH_3_OPEN, 0x118, 0 },
    { FILEPROG_CE_GRATE_TO_MMM_PUZZLE_OPEN, 0x119, 0 },
};

static void remoteBreak(Actor* actor) {
    sBreakReplay = true;
    code_4C020_breakableObjectSfx(actor->marker, NULL);
    sBreakReplay = false;
}

extern "C" void port_breakable_remoteBreak(int32_t progressFlag) {
    const LairBreakable* entry = nullptr;
    for (const LairBreakable& b : kLairBreakables) {
        if (b.flag == progressFlag) {
            entry = &b;
            break;
        }
    }
    if (entry == nullptr || suBaddieActorArray == NULL) {
        return;
    }
    for (s32 i = 0; i < suBaddieActorArray->cnt; i++) {
        Actor* actor = &suBaddieActorArray->data[i];
        if (actor->marker == NULL || actor->marker->id != entry->markerId) {
            continue;
        }
        if ((entry->fieldSel == 1 && actor->actorTypeSpecificField != 1) ||
            (entry->fieldSel == 2 && actor->actorTypeSpecificField == 1)) {
            continue;
        }
        remoteBreak(actor);
        return;
    }
}

static void remoteBreakAt(s32 markerId, s32 x, s32 y, s32 z) {
    if (suBaddieActorArray == NULL) {
        return;
    }
    for (s32 i = 0; i < suBaddieActorArray->cnt; i++) {
        Actor* actor = &suBaddieActorArray->data[i];
        if (actor->marker == NULL || actor->marker->id != markerId) {
            continue;
        }
        if ((s32)actor->position[0] != x || (s32)actor->position[1] != y || (s32)actor->position[2] != z) {
            continue;
        }
        remoteBreak(actor);
        return;
    }
}

static bool survivesBreak(s32 markerId) {
    switch (markerId) {
        case MARKER_34_CEMETARY_POT: // flowers
        case MARKER_66_ORANGE_PAD:   // stays, marked hit
            return true;
        default:
            return false;
    }
}

// Iterates backward: despawn swap-removes from the end.
static void despawnBrokenRestores(s32 map) {
    if (suBaddieActorArray == NULL) {
        return;
    }
    for (s32 i = suBaddieActorArray->cnt - 1; i >= 0; i--) {
        Actor* actor = &suBaddieActorArray->data[i];
        if (actor->marker == NULL || actor->despawn_flag || survivesBreak(actor->marker->id)) {
            continue;
        }
        if (port_breakable_isBroken(map, actor->marker->id, (s32)actor->position[0], (s32)actor->position[1],
                                    (s32)actor->position[2])) {
            marker_despawn(actor->marker);
        }
    }
}

extern "C" int32_t port_breakable_isBroken(int32_t map, int32_t markerId, int32_t x, int32_t y, int32_t z) {
    if (!Anchor::GetInstance()->IsWorldSyncActive()) {
        return 0;
    }
    return sBrokenObjects.count({ map, markerId, x, y, z }) != 0 ? 1 : 0;
}

std::vector<int32_t> port_breakable_snapshotBroken() {
    std::vector<int32_t> flat;
    flat.reserve(sBrokenObjects.size() * 5);
    for (const auto& e : sBrokenObjects) {
        flat.insert(flat.end(), e.begin(), e.end());
    }
    return flat;
}

void port_breakable_restoreBroken(const std::vector<int32_t>& flat) {
    sBrokenObjects.clear();
    for (size_t i = 0; i + 5 <= flat.size(); i += 5) {
        sBrokenObjects.insert({ flat[i], flat[i + 1], flat[i + 2], flat[i + 3], flat[i + 4] });
    }
}

void port_breakable_clearForLevel(int32_t levelId) {
    std::erase_if(sBrokenObjects, [levelId](const std::array<int32_t, 5>& e) {
        return (int32_t)map_getLevel((enum map_e)e[0]) == levelId;
    });
}

void Anchor::SendPacket_BreakObject(s16 markerId, s32 x, s32 y, s32 z, s32 map, bool replay) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    nlohmann::json payload;
    payload["type"] = BREAK_OBJECT;
    payload["targetTeamId"] = CVarGetString(CVAR_REMOTE_ANCHOR("TeamId"), "default");
    payload["addToQueue"] = true;
    payload["marker"] = markerId;
    payload["x"] = x;
    payload["y"] = y;
    payload["z"] = z;
    payload["map"] = map;
    payload["replay"] = replay;

    SendJsonToRemote(payload);
}

void Anchor::HandlePacket_BreakObject(nlohmann::json& payload) {
    if (!IsSaveLoaded() || !roomState.syncItemsAndFlags) {
        return;
    }

    s16 markerId = payload.at("marker").get<s16>();
    s32 x = payload.at("x").get<s32>();
    s32 y = payload.at("y").get<s32>();
    s32 z = payload.at("z").get<s32>();
    s32 map = payload.at("map").get<s32>();
    bool replay = payload.contains("replay") ? payload.at("replay").get<bool>() : true;

    sBrokenObjects.insert({ map, markerId, x, y, z });
    if (replay && (s32)gsworld_getMap() == map) {
        remoteBreakAt(markerId, x, y, z);
    }
}

void RegisterBreakObject_Init() {
    // At boot, with the rest of this file: the record does nothing until world sync is live.
    COND_HOOK(OnSaveLoad, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) { sBrokenObjects.clear(); });
    // replay is false for the actors that read the record as they update, so teammates only record those.
    COND_HOOK(OnBreakableBroken, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnBreakableBroken*)event;
        const s32 map = (s32)gsworld_getMap();
        if (!Anchor::GetInstance()->IsWorldSyncActive() ||
            !sBrokenObjects.insert({ map, ev->markerId, ev->x, ev->y, ev->z }).second) {
            return;
        }
        Anchor::GetInstance()->SendPacket_BreakObject((s16)ev->markerId, ev->x, ev->y, ev->z, map, ev->replay);
    });
    COND_VB_SHOULD(VB_PLAY_PUZZLE_JINGLE, EVENT_PRIORITY_NORMAL, true, {
        Actor* actor = va_arg(args, Actor*);
        if (sBreakReplay && !Anchor_InEarshot(actor->position)) {
            *should = false;
        }
    });
    COND_HOOK(OnBreakableSetUp, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        Actor* actor = ((OnBreakableSetUp*)event)->actor;
        if (port_breakable_isBroken((s32)gsworld_getMap(), actor->marker->id, (s32)actor->position[0],
                                    (s32)actor->position[1], (s32)actor->position[2])) {
            marker_despawn(actor->marker);
        }
    });
    COND_HOOK(OnFlowerpotRemaining, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        auto* ev = (OnFlowerpotRemaining*)event;
        if (!Anchor::GetInstance()->IsWorldSyncActive()) {
            return;
        }
        const s32 map = (s32)gsworld_getMap();
        const s32 flowered =
            (s32)std::count_if(sBrokenObjects.begin(), sBrokenObjects.end(), [map](const std::array<int32_t, 5>& e) {
                return e[0] == map && e[1] == MARKER_34_CEMETARY_POT;
            });
        *ev->remaining = ev->total - flowered + 1; // the caller takes this pot off
    });

    COND_HOOK(OnMapLoaded, EVENT_PRIORITY_NORMAL, true, [](IEvent* event) {
        OnMapLoaded* ev = (OnMapLoaded*)event;
        if (!Anchor::GetInstance()->IsWorldSyncActive()) {
            return;
        }
        despawnBrokenRestores((s32)ev->nextMap);
    });
}

static RegisterShipInitFunc initBreakObject(RegisterBreakObject_Init, {});
