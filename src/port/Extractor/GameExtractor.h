#pragma once

#include "Companion.h"
#include <filesystem>
#include <functional>
#include <optional>
#include <vector>
#include <cstdint>
#include <atomic>

class GameExtractor {
public:
    static bool GenAssetFile();
    std::optional<std::string> ValidateChecksum() const;
    bool RunStandalone(std::string rom);
    bool LoadRomFromPath(const std::string& romPath);
    // Open the file picker for an N64 ROM, then load the choice into this extractor. onComplete
    // reports whether a ROM was both picked and loaded. Treat as async: the extractor must outlive
    // the pick (the boot flow keeps a long-lived instance; the Mod Menu holds a shared_ptr).
    void SelectGameFromUI(std::function<void(bool)> onComplete);
    void SetSearchPath(const std::string& path);
    void GetRoms(std::vector<std::string>& roms);
    std::string GetRomPath();
    std::string GetRegionSlug() const;
    bool GenerateOTR(std::string appShortName = "");
    bool GenerateOTR(std::atomic<size_t>& assetCount, std::string appShortName = "");
    bool GenerateOTR(std::atomic<size_t>& assetCount, std::atomic<size_t>& totalAssets, std::string appShortName = "");
    void WritePortVersion();
    void SetDialogPack(bool enabled) {
        mDialogPack = enabled;
    }
    static std::string sStatusText;
    static std::string sLastError;
    // Full path of the o2r produced by the last successful GenerateOTR (e.g.
    // <dest>/mods/<slug>.o2r). The inline Mod Menu flow reads this to enable
    // exactly the file it just generated, regardless of its name.
    static std::string sLastOutputPath;
    static std::atomic<int> sPhase; // 0=idle, 1=parsing, 2=exporting, 3=done

    // Custom-code prompt: extraction worker raises sCustomCodePromptRequested
    // when it detects an injected MIPS blob in the ROM and pauses, waiting for
    // a user decision. The main loop notices the request, registers a popup on
    // its own thread, and writes the result back to sCustomCodePromptResult
    // (0 = cancel, 1 = continue) which unblocks the worker.
    static std::atomic<bool> sCustomCodePromptRequested;
    static std::atomic<bool> sCustomCodePromptActive;
    static std::atomic<int> sCustomCodePromptResult; // -1 = pending, 0 = cancel, 1 = continue
private:
    fs::path mGamePath;
    std::vector<uint8_t> mGameData;
    std::string mSearchPath;
    bool mDialogPack = false;
};