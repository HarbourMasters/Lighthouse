#include "port/Network/Anchor/JigsawPedestal.h"
#include "port/Network/Anchor/Anchor.h"
#include <unordered_map>
#include <vector>

static std::unordered_map<int32_t, uint32_t> sPedestalOwner;

int32_t port_jigsawPedestal_tryClaim(int32_t id) {
    Anchor* anchor = Anchor::GetInstance();
    if (anchor == nullptr || !anchor->isConnected) {
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

int32_t port_jigsawPedestal_isSelf(int32_t id) {
    Anchor* anchor = Anchor::GetInstance();
    if (anchor == nullptr || !anchor->isConnected) {
        return 1;
    }
    auto it = sPedestalOwner.find(id);
    return (it == sPedestalOwner.end() || it->second == anchor->ownClientId) ? 1 : 0;
}

void port_jigsawPedestal_release(int32_t id) {
    Anchor* anchor = Anchor::GetInstance();
    if (anchor == nullptr || !anchor->isConnected) {
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
