#include "port/Network/Anchor/Authority.h"
#include "port/Network/Anchor/Anchor.h"
#include "port/Network/Anchor/JigsawPedestal.h"
#include "port/Network/Anchor/VileSync.h"
#include "port/Network/Anchor/FightSync.h"
#include <libultraship/libultraship.h>

#include "enums.h"

struct ActivityState {
    bool claimed = false;
    uint32_t owner = 0;
};

static ActivityState sActivities[NET_ACTIVITY_COUNT];

// The map each activity lives in. A claim is only valid while its owner is in this map.
static const int32_t sActivityMap[NET_ACTIVITY_COUNT] = {
    MAP_10_BGS_MR_VILE,       // NET_ACTIVITY_VILE_MINIGAME
    MAP_90_GL_BATTLEMENTS,    // NET_ACTIVITY_FINAL_BOSS
    MAP_27_FP_FREEZEEZY_PEAK, // NET_ACTIVITY_FP_TWINKLY
    MAP_1_SM_SPIRAL_MOUNTAIN, // NET_ACTIVITY_SM_TUTORIAL
};

static bool Authority_IsValidActivity(NetworkActivityId activity) {
    return activity > NET_ACTIVITY_NONE && activity < NET_ACTIVITY_COUNT;
}

// Activity-specific cleanup when an activity's owner changes or its claim drops.
static void Authority_OnOwnerChanged(NetworkActivityId activity) {
    switch (activity) {
        case NET_ACTIVITY_VILE_MINIGAME:
            VileSync_OnAuthorityChanged();
            break;
        case NET_ACTIVITY_FINAL_BOSS:
            FightSync_OnAuthorityChanged();
            break;
        default:
            break;
    }
}

static void Authority_ClearClaim(NetworkActivityId activity) {
    if (sActivities[activity].claimed) {
        sActivities[activity].claimed = false;
        sActivities[activity].owner = 0;
        Authority_OnOwnerChanged(activity);
    }
}

static bool Authority_SyncActive() {
    Anchor* anchor = Anchor::GetInstance();
    return anchor != nullptr && anchor->IsWorldSyncActive();
}

bool NetAuthority_IsSelf(NetworkActivityId activity) {
    Anchor* anchor = Anchor::GetInstance();
    if (!Authority_SyncActive() || !Authority_IsValidActivity(activity)) {
        return true;
    }
    const ActivityState& state = sActivities[activity];
    return !state.claimed || state.owner == anchor->ownClientId;
}

bool NetAuthority_IsClaimed(NetworkActivityId activity) {
    return Authority_SyncActive() && Authority_IsValidActivity(activity) && sActivities[activity].claimed;
}

uint32_t NetAuthority_GetOwner(NetworkActivityId activity) {
    if (!Authority_IsValidActivity(activity) || !sActivities[activity].claimed) {
        return 0;
    }
    return sActivities[activity].owner;
}

void NetAuthority_Claim(NetworkActivityId activity) {
    Anchor* anchor = Anchor::GetInstance();
    if (!Authority_SyncActive() || !Authority_IsValidActivity(activity)) {
        return;
    }
    ActivityState& state = sActivities[activity];
    if (state.claimed && state.owner != anchor->ownClientId) {
        return;
    }
    if (!state.claimed || state.owner != anchor->ownClientId) {
        state.claimed = true;
        state.owner = anchor->ownClientId;
        Authority_OnOwnerChanged(activity);
    }
    anchor->SendPacket_AuthorityState((uint8_t)activity, true);
}

void NetAuthority_Release(NetworkActivityId activity) {
    Anchor* anchor = Anchor::GetInstance();
    if (!Authority_SyncActive() || !Authority_IsValidActivity(activity)) {
        return;
    }
    ActivityState& state = sActivities[activity];
    if (!state.claimed || state.owner != anchor->ownClientId) {
        return;
    }
    Authority_ClearClaim(activity);
    anchor->SendPacket_AuthorityState((uint8_t)activity, false);
}

void Authority_ApplyRemote(NetworkActivityId activity, uint32_t clientId, bool claimed) {
    if (!Authority_IsValidActivity(activity)) {
        return;
    }
    ActivityState& state = sActivities[activity];
    if (claimed) {
        if (!state.claimed) {
            state.claimed = true;
            state.owner = clientId;
            Authority_OnOwnerChanged(activity);
        } else if (clientId < state.owner) {
            // Simultaneous-claim tie-break: lowest clientId wins. Every client applies
            // this same rule, so all converge; a losing local claimant observes
            // NetAuthority_IsSelf() turning false.
            state.owner = clientId;
            Authority_OnOwnerChanged(activity);
        }
    } else if (state.claimed && state.owner == clientId) {
        Authority_ClearClaim(activity);
    }
}

void Authority_OnClientStateChanged(uint32_t clientId, bool online, int32_t map) {
    if (!online) {
        JigsawPedestal_ClearClient(clientId);
    }
    for (int32_t i = 0; i < NET_ACTIVITY_COUNT; i++) {
        ActivityState& state = sActivities[i];
        if (state.claimed && state.owner == clientId && (!online || map != sActivityMap[i])) {
            Authority_ClearClaim((NetworkActivityId)i);
        }
    }
}

void Authority_OnPeerMapLoad(uint32_t clientId, int32_t map) {
    JigsawPedestal_ClearClient(clientId);
    Anchor* anchor = Anchor::GetInstance();
    for (int32_t i = 0; i < NET_ACTIVITY_COUNT; i++) {
        ActivityState& state = sActivities[i];
        if (!state.claimed) {
            continue;
        }
        if (state.owner == clientId && map != sActivityMap[i]) {
            // Owner left the activity's map; everyone drops the claim by the same rule.
            Authority_ClearClaim((NetworkActivityId)i);
        } else if (anchor != nullptr && state.owner == anchor->ownClientId && map == sActivityMap[i]) {
            // A peer just entered the map of an activity we own; make sure they know.
            anchor->SendPacket_AuthorityState((uint8_t)i, true);
            if (i == NET_ACTIVITY_FINAL_BOSS) {
                FightSync_SendSnapshot(clientId);
            }
        }
    }
}

void Authority_OnSelfMapChanged(int32_t map) {
    JigsawPedestal_ReleaseAllSelf();
    Anchor* anchor = Anchor::GetInstance();
    if (anchor == nullptr) {
        return;
    }
    for (int32_t i = 0; i < NET_ACTIVITY_COUNT; i++) {
        ActivityState& state = sActivities[i];
        if (state.claimed && state.owner == anchor->ownClientId && map != sActivityMap[i]) {
            NetAuthority_Release((NetworkActivityId)i);
        }
    }
}

void Authority_Reset() {
    JigsawPedestal_Reset();
    for (int32_t i = 0; i < NET_ACTIVITY_COUNT; i++) {
        Authority_ClearClaim((NetworkActivityId)i);
    }
}
