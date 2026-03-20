#pragma once

#include "Companion.h"
#include <filesystem>
#include <vector>
#include <cstdint>
#include <atomic>

class GameExtractor {
public:
    static bool GenAssetFile();
    std::optional<std::string> ValidateChecksum() const;
    bool RunStandalone(std::string rom);
    bool SelectGameFromUI();
    void SetSearchPath(const std::string& path);
    void GetRoms(std::vector<std::string>& roms);
    std::string GetRomPath();
    bool GenerateOTR(std::string appShortName = "");
    bool GenerateOTR(std::atomic<size_t>& assetCount, std::string appShortName = "");
    bool GenerateOTR(std::atomic<size_t>& assetCount, std::atomic<size_t>& totalAssets, std::string appShortName = "");
    void WritePortVersion();
    static std::string sStatusText;
    static std::atomic<int> sPhase; // 0=idle, 1=parsing, 2=exporting, 3=done
private:
    fs::path mGamePath;
    std::vector<uint8_t> mGameData;
    std::string mSearchPath;
};