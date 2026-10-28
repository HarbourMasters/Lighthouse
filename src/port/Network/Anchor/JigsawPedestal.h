#pragma once
#include <stdint.h>

// Per-pedestal interaction lock for the Lair jigsaw podiums (keyed by actorTypeSpecificField).

#ifdef __cplusplus
// Network-internal entry points (Anchor packet handlers / lifecycle).
void JigsawPedestal_ApplyRemote(int32_t id, uint32_t clientId, bool claimed);
void JigsawPedestal_ClearClient(uint32_t clientId);
void JigsawPedestal_ReleaseAllSelf();
void JigsawPedestal_Reset();
#endif
