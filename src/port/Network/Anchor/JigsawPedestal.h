#pragma once
#include <stdint.h>

// Per-pedestal interaction lock for the Lair jigsaw podiums (keyed by actorTypeSpecificField).
// Offline or unclaimed always succeeds, so single-player behaves exactly as vanilla.

#ifdef __cplusplus
extern "C" {
#endif

// Claim for the local client if available and broadcast; 0 if another client owns it.
int32_t port_jigsawPedestal_tryClaim(int32_t id);
// 1 if we may drive this podium (unowned/self/offline), 0 if another owns it.
int32_t port_jigsawPedestal_isSelf(int32_t id);
// Release our own claim (no-op unless we own it).
void port_jigsawPedestal_release(int32_t id);

#ifdef __cplusplus
}

// Network-internal entry points (Anchor packet handlers / lifecycle).
void JigsawPedestal_ApplyRemote(int32_t id, uint32_t clientId, bool claimed);
void JigsawPedestal_ClearClient(uint32_t clientId); // a peer left/went offline
void JigsawPedestal_ReleaseAllSelf();               // we changed map
void JigsawPedestal_Reset();                          // disconnect
#endif
