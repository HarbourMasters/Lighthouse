#pragma once

#include <cstdint>

// Trace the caller of a bad asset id to assist in locating
// the causes of heap corruptions and other subtle issues.
void port_traceBadAssetId(uint32_t assetId);
