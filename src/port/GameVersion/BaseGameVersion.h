#pragma once

#include <string>

#include "../Engine.h" // BKVersion

namespace Lighthouse {

// The ROM CRC that Torch inserts into the o2r version file
BKVersion GetBaseVersion();

// Romhacks are generally only supported by US 1.0
bool BaseGameSupportsRomhacks();

// Region slug of the loaded base game
std::string BaseRegionSlug();

} // namespace Lighthouse
