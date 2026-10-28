#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void CarriedSync_ApplyRemoteCollect(int32_t kind, int32_t mapId, int32_t id, int32_t sameMap);
#ifdef __cplusplus
}
#include <vector>
std::vector<int32_t> CarriedSync_SnapshotCollected();
void CarriedSync_RestoreCollected(const std::vector<int32_t>& flat);
void CarriedSync_ClearForLevel(int32_t levelId);
#endif
