#pragma once
#include <stdint.h>

// Per-pedestal interaction lock for the Lair jigsaw podiums (keyed by actorTypeSpecificField).

#ifdef __cplusplus
extern "C" {
#endif

int32_t port_jigsawPedestal_tryClaim(int32_t id);
// 1 if we may drive this podium (unowned/self/offline), 0 if another owns it.
int32_t port_jigsawPedestal_isSelf(int32_t id);
void port_jigsawPedestal_release(int32_t id);

#ifdef __cplusplus
}

// Network-internal entry points (Anchor packet handlers / lifecycle).
void JigsawPedestal_ApplyRemote(int32_t id, uint32_t clientId, bool claimed);
void JigsawPedestal_ClearClient(uint32_t clientId);
void JigsawPedestal_ReleaseAllSelf();
void JigsawPedestal_Reset();
#endif
