#include "RomhackCompat.h"

#include "port/Romhack/RomhackConfig.h"

namespace Lighthouse {

static constexpr const char* kVanillaLabel = "Vanilla";

std::string CurrentRomhackLabel() {
    return port_isRomhack() ? port_getRomhackName() : kVanillaLabel;
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
