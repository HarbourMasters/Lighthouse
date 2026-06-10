#ifdef _WIN32
#include <Windows.h>
#include <winuser.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif
#include "port/build.h"
#include "GameExtractor.h"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <thread>
#include <unordered_map>

#include "ship/Context.h"
#include "spdlog/spdlog.h"
#include <port/Engine.h>

#ifdef unix
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifndef __SWITCH__
#include "Companion.h"
#include "factories/bk64/ConfigFactory.h"

#if !defined(__IOS__) && !defined(__ANDROID__) && !defined(__SWITCH__)
#include "portable-file-dialogs.h"
#endif

std::string GameExtractor::sStatusText;
std::string GameExtractor::sLastError;
std::string GameExtractor::sLastOutputPath;
std::atomic<int> GameExtractor::sPhase{ 0 };
std::atomic<bool> GameExtractor::sCustomCodePromptRequested{ false };
std::atomic<bool> GameExtractor::sCustomCodePromptActive{ false };
std::atomic<int> GameExtractor::sCustomCodePromptResult{ -1 };

std::unordered_map<std::string, std::string> mGameList = {
    { "1fe1632098865f639e22c11b9a81ee8f29c75d7a", "Banjo-Kazooie (U) (V1.0)" },
    { "ded6ee166e740ad1bc810fd678a84b48e245ab80", "Banjo-Kazooie (U) (V1.1)" },
    { "bb359a75941df74bf7290212c89fbc6e2c5601fe", "Banjo-Kazooie (PAL)" },
    { "90726d7e7cd5bf6cdfd38f45c9acbf4d45bd9fd8", "Banjo-Kazooie (Japan)" },
};

bool GameExtractor::RunStandalone(std::string rom) {
    // Store both path and already-read data
    std::string romPath;
    std::vector<uint8_t> romData;

    if (!std::filesystem::exists(rom)) {
        return false;
    }

    std::ifstream inFile(rom, std::ios::binary);
    if (!inFile.is_open()) {
        SPDLOG_INFO("Failed to open ROM at path: {}, continuing", rom);
        return false;
    }

    inFile.seekg(0, std::ios::end);
    size_t fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(fileSize);
    if (!inFile.read(reinterpret_cast<char*>(data.data()), fileSize)) {
        SPDLOG_INFO("Failed to read ROM at path: {}, continuing", rom);
        return false;
    }

    inFile.close();
    std::string hash = Companion::CalculateHash(data);

    if (mGameList.find(hash) != mGameList.end()) {
        romPath = rom;
        romData = std::move(data);
    }

    // Load file if it is not already open
    if (romData.empty()) {
        std::ifstream inFile(romPath, std::ios::binary);
        if (!inFile.is_open()) {
            return false;
        }

        romData = std::vector<uint8_t>(std::istreambuf_iterator<char>(inFile), {});
        inFile.close();
    }

    this->mGamePath = romPath;
    this->mGameData = std::move(romData);

    return true;
}

bool GameExtractor::SelectGameFromUI() {
    //// Store both path and already-read data
    std::string romPath;
    std::vector<uint8_t> romData;

#if !defined(__IOS__) && !defined(__ANDROID__) && !defined(__SWITCH__)
    // Desktop: fallback to file dialogue if no baserom found
    // if (!foundGame) {
    if (!pfd::settings::available()) {
        SPDLOG_ERROR("portable-file-dialogs is not available on this system.");
        return false;
    }

    auto selection = pfd::open_file("Select a file", ".", { "N64 Roms", "*.z64" }).result();
    if (selection.empty()) {
        return false;
    }

    romPath = selection[0];
    //}
#else
    // Mobile: fallback to baserom.us.z64
    if (/*!foundGame && */ !std::filesystem::exists(Ship::Context::GetPathRelativeToAppDirectory("baserom.us.z64"))) {
        SPDLOG_ERROR("baserom not found");
        return false;
    }

    // if (!foundGame) {
    romPath = Ship::Context::GetPathRelativeToAppDirectory("baserom.us.z64");
    //}
#endif

    // Load file if it is not already open
    if (romData.empty()) {
        if (!std::filesystem::exists(romPath)) {
            SPDLOG_ERROR("Failed to find ROM at path: {}", romPath);
            return false;
        }

        std::ifstream inFile(romPath, std::ios::binary);
        if (!inFile.is_open()) {
            return false;
        }

        romData = std::vector<uint8_t>(std::istreambuf_iterator<char>(inFile), {});
        inFile.close();
    }

    this->mGamePath = romPath;
    this->mGameData = std::move(romData);

    return true;
}

void GameExtractor::SetSearchPath(const std::string& path) {
    mSearchPath = path;
}

void GameExtractor::GetRoms(std::vector<std::string>& roms) {
#ifdef _WIN32
    WIN32_FIND_DATAA ffd;
    std::string search = std::string(mSearchPath + "\\*");
    HANDLE h = FindFirstFileA(search.c_str(), &ffd);

    do {
        if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            char* ext = PathFindExtensionA(ffd.cFileName);

            // Check for any standard N64 rom file extensions.
            if (ext != NULL &&
                (strcmp(ext, ".z64") == 0) /* || (strcmp(ext, ".n64") == 0) || (strcmp(ext, ".v64") == 0)*/)
                roms.push_back(ffd.cFileName);
        }
    } while (FindNextFileA(h, &ffd) != 0);
    FindClose(h);
#elif unix
    // Open the directory of the app.
    DIR* d = opendir(mSearchPath.c_str());
    struct dirent* dir;

    if (d != NULL) {
        // Go through each file in the directory
        while ((dir = readdir(d)) != NULL) {
            struct stat path;

            // Check if current entry is not folder
            stat(dir->d_name, &path);
            if (S_ISREG(path.st_mode)) {

                // Get the position of the extension character.
                char* ext = strrchr(dir->d_name, '.');
                if (ext != NULL &&
                    (strcmp(ext, ".z64") == 0 /* || strcmp(ext, ".n64") == 0 || strcmp(ext, ".v64") == 0*/)) {
                    roms.push_back(dir->d_name);
                }
            }
        }
    }
    closedir(d);
#else
    for (const auto& file : std::filesystem::directory_iterator(mSearchPath)) {
        if (file.is_directory()) {
            continue;
        }
        if (/*(file.path().extension() == ".n64") || */(file.path().extension() == ".z64")/* ||
            (file.path().extension() == ".v64")*/) {
            roms.push_back((file.path().generic_string()));
        }
    }
#endif
}

std::optional<std::string> GameExtractor::ValidateChecksum() const {
    auto rom = std::make_unique<N64::Cartridge>(this->mGameData);
    rom->Initialize();
    auto hash = rom->GetHash();

    if (mGameList.find(hash) == mGameList.end()) {
        return std::nullopt;
    }

    return mGameList[hash];
}

void GameExtractor::WritePortVersion() {
    auto writer = LUS::BinaryWriter();
    writer.SetEndianness(Torch::Endianness::Big);
    writer.Write((uint16_t)gBuildVersionMajor);
    writer.Write((uint16_t)gBuildVersionMinor);
    writer.Write((uint16_t)gBuildVersionPatch);
    writer.Close();

    Companion::Instance->RegisterCompanionFile("portVersion", writer.ToVector());
}

std::string GameExtractor::GetRomPath() {
    return mGamePath.generic_string();
}

std::string GameExtractor::GetRegionSlug() const {
    if (mGameData.empty()) {
        return "";
    }
    auto cart = std::make_unique<N64::Cartridge>(this->mGameData);
    cart->Initialize();
    switch (cart->GetCountry()) {
        case N64::CountryCode::NorthAmerica:
            return "us";
        case N64::CountryCode::Europe:
            return "pal";
        case N64::CountryCode::Japan:
            return "jp";
        default:
            return "";
    }
}

bool GameExtractor::GenerateOTR(std::string appShortName) {
    std::atomic<size_t> assetCount{ 0 };
    return GenerateOTR(assetCount, appShortName);
}

bool GameExtractor::GenerateOTR(std::atomic<size_t>& assetCount, std::string appShortName) {
    std::atomic<size_t> unused{ 0 };
    return GenerateOTR(assetCount, unused, appShortName);
}

bool GameExtractor::GenerateOTR(std::atomic<size_t>& assetCount, std::atomic<size_t>& totalAssets,
                                std::string appShortName) {
    const std::string assets_path =
        fs::path(Ship::Context::LocateFileAcrossAppDirs("assets", appShortName)).parent_path().generic_string();
    const std::string game_path = Ship::Context::GetAppDirectoryPath(appShortName);

    // Count symbol_map entries for parse-phase total
    totalAssets = 0;
    try {
        auto configPath = fs::path(assets_path) / "config.yml";
        if (fs::exists(configPath)) {
            YAML::Node config = YAML::LoadFile(configPath.generic_string());
            std::string hash = Companion::CalculateHash(this->mGameData);
            if (!config[hash]) {
                BK64::TrySynthesizeRomConfig(config, hash, this->mGamePath, this->mGameData);
            }
            auto rom = config[hash];
            if (rom && rom["path"]) {
                auto assetDir = (fs::path(assets_path) / rom["path"].as<std::string>()).generic_string();
                for (const auto& entry : std::filesystem::recursive_directory_iterator(assetDir)) {
                    if (entry.is_directory()) {
                        continue;
                    }
                    const auto path = entry.path().generic_string();
                    if (path.find(".yaml") == std::string::npos && path.find(".yml") == std::string::npos) {
                        continue;
                    }
                    if (path.find("config.yml") != std::string::npos) {
                        continue;
                    }
                    if (path.find("hashes.yaml") != std::string::npos) {
                        continue;
                    }
                    YAML::Node root = YAML::LoadFile(path);
                    for (auto asset = root.begin(); asset != root.end(); ++asset) {
                        auto key = asset->first.as<std::string>();
                        if (key.find(":config") != std::string::npos) {
                            continue;
                        }
                        auto node = asset->second;
                        if (node["type"] && node["type"].as<std::string>() == "BK64:ASSET_TABLE" &&
                            node["symbol_map"]) {
                            totalAssets += node["symbol_map"].size();
                        } else {
                            totalAssets++;
                        }
                    }
                }

                // Adjust progress bar calculation if extracting a romhack
                if (BK64::IsRomhack(this->mGameData)) {
                    auto hashesPath = fs::path(assetDir) / "hashes.yaml";
                    if (fs::exists(hashesPath)) {
                        size_t baselineCount = 0;
                        try {
                            YAML::Node hashesRoot = YAML::LoadFile(hashesPath.generic_string());
                            YAML::Node hashesMap = hashesRoot["hashes"];
                            if (hashesMap && hashesMap.IsMap()) {
                                baselineCount = hashesMap.size();
                            }
                        } catch (const std::exception& e) {
                            SPDLOG_WARN("Failed to read hashes.yaml for progress count: {}", e.what());
                        }
                        if (baselineCount > 0 && baselineCount <= totalAssets) {
                            size_t modified = BK64::CountModifiedSlots(this->mGameData, hashesPath);
                            totalAssets = totalAssets - baselineCount + modified;
                            SPDLOG_INFO("Romhack progress sizing: {} baseline entries replaced with {} modified slots",
                                        baselineCount, modified);
                        }
                    }
                }
            }
        }
    } catch (const std::exception& e) { SPDLOG_WARN("Failed to count assets: {}", e.what()); }

    // Detect non-BB custom MIPS code injection
    if (BK64::HasCustomCodeBlob(this->mGameData)) {
        SPDLOG_WARN("[GameExtractor] Custom MIPS code detected in ROM; prompting user before extraction.");
        sCustomCodePromptResult = -1;
        sCustomCodePromptActive = true;
        sCustomCodePromptRequested = true;
        while (sCustomCodePromptResult.load() == -1) {
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
        if (sCustomCodePromptResult.load() == 0) {
            SPDLOG_INFO("[GameExtractor] User cancelled extraction at custom-code prompt.");
            sLastError = "Extraction cancelled by user (custom-code romhack warning).";
            sStatusText.clear();
            sPhase = 0;
            return false;
        }
    }

    sPhase = 1; // Parsing phase
    delete Companion::Instance;
    Companion::Instance = new Companion(this->mGameData, ArchiveType::O2R, false, assets_path, game_path);
    Companion::Instance->SetRomPath(this->mGamePath);
    Companion::Instance->SetAssetTotal(&totalAssets);
    Companion::Instance->SetPhaseCallback([](int phase) { sPhase = phase; });
    Companion::Instance->GetConfig().dialogPack = this->mDialogPack;
    if (!BK64::IsRomhack(this->mGameData)) {
        this->WritePortVersion();
    }
    try {
        Companion::Instance->Init(ExportType::Binary, assetCount, true);
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Failed to process O2R: {}", e.what());
        sLastError = e.what();
        sStatusText.clear();
        sPhase = 0;
        delete Companion::Instance;
        Companion::Instance = nullptr;
        return false;
    }

    // Record the produced archive path before tearing Companion down, so the
    // inline Mod Menu flow can enable exactly this file by name.
    sLastOutputPath = Companion::Instance->GetOutputPath();

    sPhase = 3;
    sStatusText = "Cleaning up...";
    delete Companion::Instance;
    Companion::Instance = nullptr;
    sStatusText.clear();
    sPhase = 0;
    return true;
}
#else
static bool GameExtractor::GenAssetFile() {
    return false;
}

std::optional<std::string> GameExtractor::ValidateChecksum() const {
    return std::nullopt;
}

bool GameExtractor::SelectGameFromUI() {
    return false;
}

void GameExtractor::GetRoms(std::vector<std::string>& roms) {
    // None
}

bool GameExtractor::GenerateOTR() {
    return false;
}

void GameExtractor::WritePortVersion() {
    // None
}
#endif
