#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Fast {
class Fast3dWindow;
}

inline const std::vector<std::string> kRomArchives = { "bk.o2r" };
extern std::string assets_path;
extern bool portArchiveVersionMatch;
extern std::shared_ptr<Fast::Fast3dWindow> lhFast3dWindow;
