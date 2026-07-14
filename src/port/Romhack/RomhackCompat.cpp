#include "RomhackCompat.h"

#include "port/Romhack/RomhackConfig.h"

namespace Lighthouse {

static constexpr const char* kVanillaLabel = "Vanilla";

std::string CurrentRomhackLabel() {
    if (!port_isRomhack()) {
        return kVanillaLabel;
    }
    // Prefer the canonical identifier from the romhack table so two peers running
    // the same hack agree regardless of how each named their generated o2r file.
    if (const char* id = port_getRomhackIdentifier()) {
        return id;
    }
    // Unrecognized hack (not in the table): fall back to the filename-derived name
    // so peers running the same unlisted hack still match each other.
    return port_getRomhackName();
}

std::string DescribeRomhackMismatch(bool localIsRomhack, const std::string& localLabel, bool remoteIsRomhack,
                                    const std::string& remoteLabel) {
    if (localLabel == remoteLabel) {
        return "";
    }
    if (localIsRomhack && !remoteIsRomhack) {
        return " - You have a romhack enabled, but the server is vanilla.";
    }
    if (!localIsRomhack && remoteIsRomhack) {
        return " - The server has a romhack enabled, but your game is vanilla.";
    }
    return " - You have the \"" + localLabel + "\" hack enabled,\n    but the server is using \"" + remoteLabel + "\"";
}

} // namespace Lighthouse
