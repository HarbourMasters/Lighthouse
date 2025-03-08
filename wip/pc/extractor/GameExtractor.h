#pragma once

#include <filesystem>
#include <vector>
#include <cstdint>
#include <optional>
#include <fstream>

namespace fs = std::filesystem;

class GameExtractor {
public:
    std::optional<std::string> ValidateChecksum() const;
    bool SelectGameFromUI();
    bool GenerateOTR() const;
private:
    fs::path mGamePath;
    std::vector<uint8_t> mGameData;
};