#pragma once

#include <cstring>

namespace Lighthouse {

struct RomhackTableEntry {
    const char* sha1;
    const char* identifier;
    bool isPorted;
};

/*
    This romhack table contains the sha1 hashes for detected code changes
    injected into romhacks. Torch inserts these hashes into aGameConfig
    per romhack generated into an o2r. Having a table of such hashes allows
    Lighthouse to eventually port the code modifications that allow these
    romhacks to function properly, and gate such changes behind matching
    hashes per o2r.

    Known romhacks hashed against the latest at patch.wedarobi.com
    as of July 2026.
*/
static constexpr RomhackTableEntry kRomhackTable[] = {
    // These hacks ship a separate code blob; the hash covers its decompressed
    // payload.

    // Cut-Throat Coast
    { "13f4fa8a180fe5775a606486effbafeb58862d26", "CutThroatCoast", true },

    // The Gruntch, Santa's Village
    { "bed22dd8ef931228fbc94f006dfc718a4d4f6f8c", "Gruntch", true },

    // Snow Glow Village
    { "23596c2858283b847e9e0ff44785e35110002fc7", "SnowGlowVillage", false },

    // The Corrupted Jiggies
    { "9e20be78496d66f2e5f7930022a0fee769753488", "CorruptedJiggies", false },

    // Bubbling Bog Brewery Redone
    { "af7c71b034b2d7af867cd0aec8751c49aca1f0bc", "BubblingBog", false },

    // Cheatos Challenges
    { "017bf33d80b22d7926a9839f09fd1c52f83d3b97", "CheatosChallenges", true },

    // These hacks ship no blob of their own; they edit the globalized overlay
    // in place, which aGameConfig has no schema for. Mostly actor table
    // rebinds, though Nostalgia 64 also fills spare overlay space with its own
    // functions and repoints update funcs at them.

    // Banjo-Dreamie
    { "3972b2a3aca230dde1cbf66ffa6f052327b7d3fa", "Dreamie", true },

    // Jiggies of Time
    { "ad7ef5ba9051c6a360c1d60546d5ca8cf61f0d92", "JiggiesOfTime", true },

    // New Horizons
    { "663dd77a9e3a1ed1f25399380665511a04b91a54", "NewHorizons", true },

    // Nostalgia 64
    { "7a7a07c26d77530dc45caf8ab96e9056ea2877e1", "Nostalgia64", true },

    { nullptr, nullptr, false }, // terminator
};

inline const RomhackTableEntry* LookupRomhackEntry(const char* sha1Hex) {
    if (sha1Hex == nullptr || sha1Hex[0] == '\0') {
        return nullptr;
    }
    for (const auto* entry = kRomhackTable; entry->sha1 != nullptr; entry++) {
        if (std::strcmp(entry->sha1, sha1Hex) == 0) {
            return entry;
        }
    }
    return nullptr;
}

inline const char* LookupRomhackIdentifier(const char* sha1Hex) {
    const RomhackTableEntry* entry = LookupRomhackEntry(sha1Hex);
    return entry ? entry->identifier : nullptr;
}

} // namespace Lighthouse
