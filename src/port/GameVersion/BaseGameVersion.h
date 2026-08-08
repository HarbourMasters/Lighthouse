#pragma once

#include <string>

#include "../Engine.h" // BKVersion

namespace Lighthouse {

// The ROM CRC that Torch inserts into the o2r version file
BKVersion GetBaseVersion();

// Classify a version stamp taken from an archive
bool ClassifyArchiveVersion(uint32_t rawVersion, BKVersion& out);

// Filesystem path of the base bk.o2r, or empty if it hasn't been extracted yet
std::string BaseArchivePath();

// Romhacks are generally only supported by US 1.0
bool BaseGameSupportsRomhacks();

// Region slug of the loaded base game
std::string BaseRegionSlug();

} // namespace Lighthouse
