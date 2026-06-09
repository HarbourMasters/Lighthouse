#pragma once

#include <cstring>

namespace Lighthouse {

struct RomhackTableEntry {
    const char* sha1;
    const char* identifier;
};

/*
    This romhack table contains the sha1 hashes for custom code
    blobs injected into romhacks. Torch inserts these hashes into
    aGameConfig per romhack generated into an o2r. Having a table
    of such hashes allows Lighthouse to eventually port custom code
    that allows these romhacks to function properly, and gate custom
    code paths behind matching hashes per o2r.
*/
static constexpr RomhackTableEntry kRomhackTable[] = {
    // Cut-Throat Coast
    { "13f4fa8a180fe5775a606486effbafeb58862d26", "CutThroatCoast" },

    // How the Gruntch Stole Christmas — also shipped, byte-identical, in
    // Santa's Village. Single hand-port covers both.
    { "bed22dd8ef931228fbc94f006dfc718a4d4f6f8c", "Gruntch" },

    // Snow Glow Village — derivative of Gruntch's blob with diverged code.
    { "23596c2858283b847e9e0ff44785e35110002fc7", "SnowGlowVillage" },

    // The Corrupted Jiggies
    { "9e20be78496d66f2e5f7930022a0fee769753488", "CorruptedJiggies" },

    // Banjo-Dreamie
    { "3972b2a3aca230dde1cbf66ffa6f052327b7d3fa", "Dreamie" },

    { nullptr, nullptr }, // terminator — keep last
};

inline const char* LookupRomhackIdentifier(const char* sha1Hex) {
    if (sha1Hex == nullptr || sha1Hex[0] == '\0') {
        return nullptr;
    }
    for (const auto* entry = kRomhackTable; entry->sha1 != nullptr; entry++) {
        if (std::strcmp(entry->sha1, sha1Hex) == 0) {
            return entry->identifier;
        }
    }
    return nullptr;
}

} // namespace Lighthouse
