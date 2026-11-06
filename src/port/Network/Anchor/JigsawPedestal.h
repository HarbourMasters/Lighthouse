#pragma once
#include <stdint.h>

// Per-pedestal interaction lock for the Lair jigsaw podiums (keyed by actorTypeSpecificField).

bool JigsawPedestal_TryClaim(int32_t id);
bool JigsawPedestal_IsSelf(int32_t id);
void JigsawPedestal_Release(int32_t id);

// Network-internal entry points (Anchor packet handlers / lifecycle).
void JigsawPedestal_ApplyRemote(int32_t id, uint32_t clientId, bool claimed);
void JigsawPedestal_ClearClient(uint32_t clientId);
void JigsawPedestal_ReleaseAllSelf();
void JigsawPedestal_ResendSelf();
void JigsawPedestal_Reset();
