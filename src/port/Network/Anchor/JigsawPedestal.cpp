#include "port/Network/Anchor/JigsawPedestal.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/ShipInit.hpp"
#include "functions.h"
#include <unordered_map>
#include <vector>

static std::unordered_map<int32_t, uint32_t> sPedestalOwner;

static bool JigsawPedestal_SyncActive() {
    Anchor* anchor = Anchor::GetInstance();
    return anchor != nullptr && anchor->IsWorldSyncActive();
}

static int32_t JigsawPedestal_TryClaim(int32_t id) {
    Anchor* anchor = Anchor::GetInstance();
    if (!JigsawPedestal_SyncActive()) {
        return 1;
    }
    auto it = sPedestalOwner.find(id);
    if (it != sPedestalOwner.end() && it->second != anchor->ownClientId) {
        return 0;
    }
    sPedestalOwner[id] = anchor->ownClientId;
    anchor->SendPacket_PedestalOwner(id, true);
    return 1;
}

static int32_t JigsawPedestal_IsSelf(int32_t id) {
    Anchor* anchor = Anchor::GetInstance();
    if (!JigsawPedestal_SyncActive()) {
        return 1;
    }
    auto it = sPedestalOwner.find(id);
    return (it == sPedestalOwner.end() || it->second == anchor->ownClientId) ? 1 : 0;
}

static void JigsawPedestal_Release(int32_t id) {
    Anchor* anchor = Anchor::GetInstance();
    if (!JigsawPedestal_SyncActive()) {
        return;
    }
    // Keep a finished picture's claim until we leave the map: vanilla already refuses to let anyone
    // step back on it, and holding covers a teammate whose completion bits are still in flight.
    if (jigsawPicture_isJigsawPictureComplete(id)) {
        return;
    }
    auto it = sPedestalOwner.find(id);
    if (it != sPedestalOwner.end() && it->second == anchor->ownClientId) {
        sPedestalOwner.erase(it);
        anchor->SendPacket_PedestalOwner(id, false);
    }
}

void JigsawPedestal_ApplyRemote(int32_t id, uint32_t clientId, bool claimed) {
    auto it = sPedestalOwner.find(id);
    if (claimed) {
        // Simultaneous-claim tie-break: lowest clientId wins.
        if (it == sPedestalOwner.end() || clientId < it->second) {
            sPedestalOwner[id] = clientId;
        }
    } else if (it != sPedestalOwner.end() && it->second == clientId) {
        sPedestalOwner.erase(it);
    }
}

void JigsawPedestal_ClearClient(uint32_t clientId) {
    for (auto it = sPedestalOwner.begin(); it != sPedestalOwner.end();) {
        it = (it->second == clientId) ? sPedestalOwner.erase(it) : std::next(it);
    }
}

void JigsawPedestal_ReleaseAllSelf() {
    Anchor* anchor = Anchor::GetInstance();
    if (anchor == nullptr) {
        return;
    }
    std::vector<int32_t> mine;
    for (auto& [id, owner] : sPedestalOwner) {
        if (owner == anchor->ownClientId) {
            mine.push_back(id);
        }
    }
    for (int32_t id : mine) {
        sPedestalOwner.erase(id);
        if (anchor->isConnected) {
            anchor->SendPacket_PedestalOwner(id, false);
        }
    }
}

void JigsawPedestal_Reset() {
    sPedestalOwner.clear();
}

static void RegisterJigsawPedestal_Init() {
    const bool connected = Anchor::GetInstance()->isConnected;

    COND_VB_SHOULD(VB_JIGSAW_PEDESTAL_CLAIM, EVENT_PRIORITY_NORMAL, connected,
                   { *should = JigsawPedestal_TryClaim(va_arg(args, s32)) != 0; });

    COND_VB_SHOULD(VB_JIGSAW_PEDESTAL_OWNED, EVENT_PRIORITY_NORMAL, connected,
                   { *should = JigsawPedestal_IsSelf(va_arg(args, s32)) != 0; });

    COND_HOOK(OnJigsawPedestalLeave, EVENT_PRIORITY_NORMAL, connected,
              [](IEvent* event) { JigsawPedestal_Release(((OnJigsawPedestalLeave*)event)->id); });
}

static RegisterShipInitFunc initJigsawPedestal(RegisterJigsawPedestal_Init, { ANCHOR_CONNECTED_PATH });
