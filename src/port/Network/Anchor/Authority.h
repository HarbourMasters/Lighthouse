#pragma once
#include <stdint.h>
#include <stdbool.h>

// Generic network authority registry.
//
// An "activity" is anything where one client must run the real logic and the others
// follow (minigames, battles, boss fights). The client that starts the activity claims
// authority; everyone else gates their local simulation on NetAuthority_IsSelf().
//
// Semantics:
//  - Offline (or before Anchor init), NetAuthority_IsSelf() is always true, so vanilla
//    single-player code paths run unmodified.
//  - While connected but unclaimed, IsSelf() is also true: every client is locally
//    authoritative for ambient behavior until someone actually starts the activity.
//  - Simultaneous claims resolve deterministically: the lowest clientId wins. Both
//    sides apply the same rule on receive, so they converge without extra round trips;
//    the loser observes NetAuthority_IsSelf() turning false.
//  - Authority is auto-released when the owner disconnects or leaves the activity's
//    map (each activity is tied to one map in Authority.cpp).
//
// To add a new gated activity: add an enum entry below and its map to sActivityMap in
// Authority.cpp.

typedef enum NetworkActivityId {
    NET_ACTIVITY_NONE = -1,
    NET_ACTIVITY_VILE_MINIGAME,
    NET_ACTIVITY_COUNT
} NetworkActivityId;

#ifdef __cplusplus
extern "C" {
#endif

// True if the local client should run the real logic for this activity
// (owns the claim, nobody has claimed, or we are offline).
bool NetAuthority_IsSelf(NetworkActivityId activity);
bool NetAuthority_IsClaimed(NetworkActivityId activity);
// Owning clientId, or 0 when unclaimed.
uint32_t NetAuthority_GetOwner(NetworkActivityId activity);
// Claim authority for the local client and broadcast it. No-op offline, or if another
// client already holds the claim.
void NetAuthority_Claim(NetworkActivityId activity);
// Release our own claim and broadcast it. No-op unless we are the owner.
void NetAuthority_Release(NetworkActivityId activity);

#ifdef __cplusplus
}

// Network-internal entry points (called from Anchor packet handlers / lifecycle):
void Authority_ApplyRemote(NetworkActivityId activity, uint32_t clientId, bool claimed);
// React to a client's online/map state changing (offline or wrong map drops its claims).
void Authority_OnClientStateChanged(uint32_t clientId, bool online, int32_t map);
// A peer finished loading a map: drop their stale claims, and rebroadcast any claim of
// ours that lives on the map they just entered so late joiners learn it.
void Authority_OnPeerMapLoad(uint32_t clientId, int32_t map);
// The local player changed map: release any claim of ours that lives elsewhere.
void Authority_OnSelfMapChanged(int32_t map);
// Clear everything (disconnect).
void Authority_Reset();
#endif
