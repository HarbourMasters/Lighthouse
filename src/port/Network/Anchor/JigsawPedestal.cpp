#include "port/Network/Anchor/JigsawPedestal.h"
#include "port/Network/Anchor/Anchor.h"
#include "functions.h"
#include <unordered_map>
#include <vector>

static std::unordered_map<int32_t, uint32_t> sPedestalOwner;

bool JigsawPedestal_TryClaim(int32_t id) {
    Anchor* anchor = Anchor::GetInstance();
    if (!Anchor_IsWorldSyncActive()) {
        return true;
    }
    auto it = sPedestalOwner.find(id);
    if (it != sPedestalOwner.end() && it->second != anchor->ownClientId) {
        return false;
    }
    sPedestalOwner[id] = anchor->ownClientId;
    anchor->SendPacket_PedestalOwner(id, true);
    return true;
}

// True if we may drive this podium (unowned/self/offline), false if another owns it.
bool JigsawPedestal_IsSelf(int32_t id) {
    Anchor* anchor = Anchor::GetInstance();
    if (!Anchor_IsWorldSyncActive()) {
        return true;
    }
    auto it = sPedestalOwner.find(id);
    return it == sPedestalOwner.end() || it->second == anchor->ownClientId;
}

void JigsawPedestal_Release(int32_t id) {
    Anchor* anchor = Anchor::GetInstance();
    if (!Anchor_IsWorldSyncActive()) {
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

// A peer who arrives in our map, or reconnected and lost its table, would otherwise see our
// podium as free and take it from us.
void JigsawPedestal_ResendSelf() {
    Anchor* anchor = Anchor::GetInstance();
    if (!Anchor_IsWorldSyncActive()) {
        return;
    }
    for (auto& [id, owner] : sPedestalOwner) {
        if (owner == anchor->ownClientId) {
            anchor->SendPacket_PedestalOwner(id, true);
        }
    }
}

void JigsawPedestal_Reset() {
    sPedestalOwner.clear();
}
