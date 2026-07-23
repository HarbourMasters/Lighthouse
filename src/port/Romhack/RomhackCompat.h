#pragma once

#include <string>

/*
    Shared helpers for reasoning about romhack identity across features that
    must agree on "which romhack is loaded".
*/
namespace Lighthouse {

// Canonical identity label for the locally loaded romhack, or "Vanilla" when
// none is active. This is the string compared between netplay peers.
std::string CurrentRomhackLabel();

// One-line description of how the local romhack state differs from a peer's,
// suitable for a mismatch popup. Returns an empty string when the two match.
std::string DescribeRomhackMismatch(bool localIsRomhack, const std::string& localLabel, bool remoteIsRomhack,
                                    const std::string& remoteLabel);

} // namespace Lighthouse
