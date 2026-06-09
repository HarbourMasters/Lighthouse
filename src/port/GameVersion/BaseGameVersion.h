#pragma once

#include <cstdint>

namespace Lighthouse {

/*
    Single source of truth for detecting which base game an o2r was built from.
    The version is derived from the aBKAssetTable entry count: US v1.0 is the
    romhack-compatible baseline, and every other version remaps its asset IDs
    onto v1.0 at load time (see ResourceHelpers.cpp). PAL and JP share an
    entry-count range, so the Localized class cannot tell them apart on count
    alone — callers that need the distinction probe the loaded symbol map.
*/
enum class BaseGameVersion {
    Unknown = 0,
    USV10,
    USV11,
    Localized, // PAL (3059) or JP (3065)
};

// Classify a base game from its aBKAssetTable entry count. This owns the
// version thresholds; everything else routes through it.
BaseGameVersion ClassifyAssetCount(uint32_t assetCount);

// Version of the base bk.o2r on disk, read directly from the archive so the
// result is independent of which mod overlays are currently active. Cached
// after the first successful read (bk.o2r is immutable for the session).
BaseGameVersion GetBaseGameVersion();

// True unless the base is positively a non-v1.0 build (v1.1 / PAL / JP).
// Romhack overlays target v1.0 asset IDs, so they may only be applied on a
// v1.0 base. Conservative: an unreadable or unrecognized base is treated as
// compatible.
bool BaseGameSupportsRomhacks();

} // namespace Lighthouse
