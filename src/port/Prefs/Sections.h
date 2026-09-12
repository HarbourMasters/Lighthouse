#pragma once

#include <stdint.h>

// A setting's section does double duty: it is the preset section the setting belongs to, and
// it names the root JSON object the value is stored under. Block names match the existing
// preset block keys in Presets.cpp so preset files keep their current shape.
enum PrefSection : uint8_t {
    SECTION_SETTINGS,
    SECTION_ENHANCEMENTS,
    SECTION_RANDO,
    SECTION_TRACKERS,
    SECTION_NETWORK,
    // SECTION_COSMETICS,
    // SECTION_AUDIO,
    SECTION_COUNT,
};

namespace Prefs {

const char* SectionBlock(PrefSection section);

} // namespace Prefs
