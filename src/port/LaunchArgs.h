#pragma once

#include <string>

namespace Lighthouse {

void ParseLaunchArgs(int argc, char* argv[]);
bool IsLaunchHackFlag(const std::string& arg, std::string& outInlineValue, bool& outTakesValue);
void ApplyLaunchHack();
void RestoreModSelectionAfterLaunchHack();
void FlushLaunchHackLog();

} // namespace Lighthouse
