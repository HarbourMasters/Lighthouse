#include "port/Engine.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <cerrno>
#include <cstring>
#endif

#include <libultraship/libultraship.h>
#include <SDL2/SDL.h>
#include <fast/Fast3dWindow.h>
#include <ship/utils/StringHelper.h>
#include "ship/window/gui/FileBrowserWindow.h"

#include "port/build.h"
#include "port/Extractor/ExtractFlow.h"
#include "port/Extractor/GameExtractor.h"
#include "port/LaunchArgs.h"
#include "port/UI/cvar_prefixes.h"
#include "port/UI/LighthouseGui.hpp"
#include "port/UI/LighthouseModMenuWindow.h"

#ifdef __SWITCH__
#include <port/switch/SwitchImpl.h>
#endif

namespace fs = std::filesystem;

namespace {

typedef struct {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
} OTRVersion;

OTRVersion ReadPortVersionFromOTR(std::string otrPath) {
    OTRVersion version = {};

    // Use a temporary archive instance to load the otr and read the version file
    auto archive = std::make_shared<Ship::O2rArchive>(otrPath);
    if (archive->Open()) {
        auto t = archive->LoadFile("portVersion");
        if (t != nullptr && t->IsLoaded) {
            auto stream = std::make_shared<Ship::MemoryStream>(t->Buffer->data(), t->Buffer->size());
            auto reader = std::make_shared<Ship::BinaryReader>(stream);
            reader->SetEndianness(Ship::Endianness::Big);
            version.major = reader->ReadUInt16();
            version.minor = reader->ReadUInt16();
            version.patch = reader->ReadUInt16();
        } else {
            SPDLOG_WARN("Failed to read portVersion file from O2R: {}", otrPath);
        }
    } else {
        SPDLOG_WARN("Failed to open O2R for version reading: {}", otrPath);
    }

    return version;
}

OTRVersion DetectOTRVersion(std::string fileName) {
    std::string otrPath = Ship::Context::LocateFileAcrossAppDirs(fileName);

    if (!std::filesystem::exists(otrPath)) {
        SPDLOG_WARN("O2R file not found at path: {}", otrPath);
        return { INT16_MAX, INT16_MAX, INT16_MAX };
    }

    return ReadPortVersionFromOTR(otrPath);
}

bool VerifyArchiveVersion(OTRVersion version) {
    return version.major == gBuildVersionMajor && version.minor == gBuildVersionMinor;
}

typedef enum ExtractSteps {
    ES_PORT_ARCHIVE,
    ES_WINDOWS,
    ES_EXTRACT_ARGS,
    ES_EXTRACT,
    ES_VERIFY,
} ExtractSteps;

typedef enum PromptSteps {
    PS_FILE_CHECK,
    PS_LOCAL,
    PS_FIRST,
    PS_FIRST_WAIT,
    PS_WAIT,
    PS_NONE,
} PromptSteps;

typedef enum WindowsSteps {
    WS_TEMP,
    WS_PERMS,
    WS_ONEDRIVE,
    WS_DONE,
} WindowsSteps;

bool IsSubpath(const std::filesystem::path& path, const std::filesystem::path& base) {
    auto rel = std::filesystem::relative(path, base);
    return !rel.empty() && rel.native()[0] != '.';
}

bool PathTestCleanup() {
    try {
        if (std::filesystem::exists("./text.txt"))
            std::filesystem::remove("./text.txt");
        if (std::filesystem::exists("./test/"))
            std::filesystem::remove("./test/");
    } catch (std::filesystem::filesystem_error const&) { return false; }
    return true;
}

void CheckAndCreateModFolder() {
    try {
        std::string modsPath = Ship::Context::LocateFileAcrossAppDirs("mods", "bk");
        if (!std::filesystem::exists(modsPath)) {
            // Create mods folder and subdirs relative to app dir
            modsPath = Ship::Context::GetPathRelativeToAppDirectory("mods", "bk");
            std::string filePath = modsPath + "/custom_mod_files_go_here.txt";
            if (std::filesystem::create_directories(modsPath)) {
                std::ofstream(filePath).close();
                std::filesystem::create_directories(modsPath + "/~romhacks"); // BK romhacks go here
                std::filesystem::create_directories(modsPath + "/~lang");     // Language packs go here
                std::filesystem::create_directories(modsPath + "/~shared");   // Mods usable by everything go here
            }
        }
    } catch (std::filesystem::filesystem_error const&) { return; }
}

bool AnyRomArchiveExists() {
    for (const auto& archive : kRomArchives) {
        if (std::filesystem::exists(Ship::Context::LocateFileAcrossAppDirs(archive, "bk"))) {
            return true;
        }
    }
    return false;
}

} // namespace

void GameEngine::RunExtract(int argc, char* argv[]) {
    bool extractDone = false;
    ExtractSteps extractStep = ES_PORT_ARCHIVE;
    WindowsSteps windowsStep = WS_TEMP;
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(context->GetWindow());
    auto gui = wnd->GetGui();
    bool menuWasVisible = false;
    if (gui->GetMenu()->IsVisible()) {
        menuWasVisible = true;
        gui->GetMenu()->Hide();
    }

    OTRVersion romArchiveVersion = { INT16_MAX, 0, 0 };
    for (const auto& archive : kRomArchives) {
        OTRVersion ver = DetectOTRVersion(archive);
        if (ver.major != INT16_MAX) {
            romArchiveVersion = ver;
            break;
        }
    }

    bool shouldRegen = !VerifyArchiveVersion(romArchiveVersion) && romArchiveVersion.major != INT16_MAX;

    std::filesystem::path ownPath;
    std::vector<std::string> args;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        std::string inlineValue;
        bool takesValue = false;
        if (Lighthouse::IsLaunchHackFlag(arg, inlineValue, takesValue)) {
            if (takesValue) {
                i++;
            }
            continue;
        }
        args.push_back(arg);
    }
    GameExtractor extract;
    PromptSteps promptStep = PS_FILE_CHECK;
    std::atomic<bool> extracting = false;
    bool extractStarted = false;
    std::atomic<size_t> extractCount{ 0 }, totalExtract{ 0 };
    bool romLoaded = false;
    bool romResultReady = false;

    std::string installPath = Ship::Context::GetAppBundlePath();
    std::string file;

#if defined(__SWITCH__)
    LighthouseGui::RegisterPopup("Outdated ROM Archives",
                                 "\x1b[2;2HYou've launched Lighthouse with an old ROM O2R file."
                                 "\x1b[4;2HPlease regenerate a new ROM O2R and relaunch."
                                 "\x1b[6;2HPress the Home button to exit...",
                                 "OK", "", [&]() { exit(1); });
#elif defined(__WIIU__)
    LighthouseGui::RegisterPopup("Outdated ROM Archives",
                                 "You've launched Lighthouse with an old a ROM O2R file.\n\n"
                                 "Please generate a ROM O2R and relaunch.\n\n"
                                 "Press and hold the Power button to shutdown...",
                                 "OK", "", [&]() { exit(1); });
    OSFatal();
#endif

    if (!std::filesystem::exists(Ship::Context::LocateFileAcrossAppDirs("/assets"))) {
        LighthouseGui::RegisterPopup(
            "Extractor assets not found",
            "No O2R files found. Missing 'assets/' folder needed to generate O2R file.\nPlease "
            "re-extract them from the download or.\n\nExiting...",
            "OK", "", [&]() {
                lhFast3dWindow = nullptr;
                context = nullptr;
                exit(1);
            });
    } else if (shouldRegen) {
        LighthouseGui::RegisterPopup("Outdated ROM Archives",
                                     "Your ROM archives were created with incompatible versions of Lighthouse.\n"
                                     "You will now be redirected to re-extract them.");
        for (const auto& archive : kRomArchives) {
            std::filesystem::remove(archive);
        }
    }

    std::shared_ptr<BS::thread_pool> threadPool = std::make_shared<BS::thread_pool>(1);
    while (!extractDone) {
        if (GameExtractor::sCustomCodePromptRequested.load()) {
            GameExtractor::sCustomCodePromptRequested = false;
            LighthouseGui::RegisterPopup(
                "Code-Modified Romhack Detected",
                "This romhack contains modified code.\n"
                "Lighthouse cannot extract this code, so expected\n"
                "behavior will be missing or broken when playing.\n"
                "\n"
                "Continue extraction anyway?",
                "Continue", "Cancel",
                []() {
                    GameExtractor::sCustomCodePromptResult = 1;
                    GameExtractor::sCustomCodePromptActive = false;
                },
                []() {
                    GameExtractor::sCustomCodePromptResult = 0;
                    GameExtractor::sCustomCodePromptActive = false;
                });
        }
        if (LighthouseGui::PopupsQueued() > 0 || extracting || Ship::FileBrowserWindow::IsOpen()) {
            goto render;
        }

        if (extractStep == ES_EXTRACT && promptStep == PS_FIRST && extractStarted && !extracting) {
            extractStep = ES_VERIFY;
            extractStarted = false;
            extractCount = 0;
            totalExtract = 0;
        }
        switch (extractStep) {
            case ES_PORT_ARCHIVE: {
                if (portArchiveVersionMatch) {
#ifdef _WIN32
                    extractStep = ES_WINDOWS;
#elif (defined(__WIIU__) || defined(__SWITCH__))
                    extractStep = ES_VERIFY;
#else
                    extractStep = ES_EXTRACT;
#endif
                } else {
                    std::string msg;

#if defined(__SWITCH__)
                    msg = "\x1b[4;2HPlease re-extract it from the download.\n"
                          "\x1b[6;2HPress the Home button to exit...";
#elif defined(__WIIU__)
                    msg = "Please extract the lighthouse.o2r from the Lighthouse download\nto your "
                          "folder.\n\nPress "
                          "and hold the power\n"
                          "button to shutdown...";
#else
                    msg = "Please extract the lighthouse.o2r from the Lighthouse download to your "
                          "folder.\n\nExiting...";
#endif
                    std::string title =
                        !std::filesystem::exists(assets_path) ? "Missing lighthouse.o2r" : "lighthouse.o2r is outdated";
                    LighthouseGui::RegisterPopup(title, msg, "OK", "", [&]() { exit(1); });
                }
                continue;
            }
            case ES_WINDOWS: {
                switch (windowsStep) {
                    case WS_TEMP: {
#ifdef _WIN32
                        char* tempVar = getenv("TEMP");
                        std::filesystem::path tempPath;
                        try {
                            tempPath = std::filesystem::canonical(tempVar);
                        } catch (std::filesystem::filesystem_error const&) {
                            std::string userPath = getenv("USERPROFILE");
                            userPath.append("\\AppData\\Local\\Temp");
                            tempPath = std::filesystem::canonical(userPath);
                        }
                        wchar_t buffer[MAX_PATH];
                        GetModuleFileName(NULL, buffer, _countof(buffer));
                        ownPath = std::filesystem::canonical(buffer).parent_path();
                        if (IsSubpath(ownPath, tempPath)) {
                            LighthouseGui::RegisterPopup(
                                "Lighthouse Path Error",
                                "Lighthouse is running in a temp folder.\nExtract the .zip and run again.", "OK", "",
                                [&]() {
                                    threadPool = nullptr;
                                    lhFast3dWindow = nullptr;
                                    context = nullptr;
                                    exit(0);
                                });
                        } else {
                            windowsStep = WS_PERMS;
                        }
#endif
                        continue;
                    }
                    case WS_PERMS: {
                        FILE* tfile = fopen("./text.txt", "w");
                        std::filesystem::path tfolder = std::filesystem::path("./test/");
                        bool error = false;
                        try {
                            create_directories(tfolder);
                        } catch (std::filesystem::filesystem_error const&) { error = true; }
                        if (tfile == NULL || error) {
                            LighthouseGui::RegisterPopup(
                                "Lighthouse Permissions Error",
                                "Lighthouse does not have proper file permissions.\nPlease move it to a "
                                "folder that does and run again.",
                                "OK", "", [&]() {
                                    if (tfile != NULL) {
                                        fclose(tfile);
                                    }
                                    PathTestCleanup();
                                    threadPool = nullptr;
                                    lhFast3dWindow = nullptr;
                                    context = nullptr;
                                    exit(0);
                                });
                        } else {
                            fclose(tfile);
                            if (!PathTestCleanup()) {
                                LighthouseGui::RegisterPopup(
                                    "Lighthouse Permissions Error",
                                    "Lighthouse does not have proper file permissions.\nPlease move it to a "
                                    "folder that does and run again.",
                                    "OK", "", [&]() {
                                        threadPool = nullptr;
                                        lhFast3dWindow = nullptr;
                                        context = nullptr;
                                        exit(0);
                                    });
                            }
                            windowsStep = WS_ONEDRIVE;
                        }
                        continue;
                    }
                    case WS_ONEDRIVE: {
                        if (ownPath.string().find("OneDrive") != std::string::npos) {
                            LighthouseGui::RegisterPopup(
                                "Lighthouse Path Error",
                                "Lighthouse appears to be in a OneDrive folder, which will cause issues.\n"
                                "Please move it to a folder outside of OneDrive, like the root of a\n"
                                "drive (e.g. \"C:\\Games\\Lighthouse\").",
                                "OK", "", [&]() {
                                    threadPool = nullptr;
                                    lhFast3dWindow = nullptr;
                                    context = nullptr;
                                    exit(0);
                                });
                        } else {
                            windowsStep = WS_DONE;
                            if (args.size() > 0) {
                                extractStep = ES_EXTRACT_ARGS;
                            } else {
                                extractStep = ES_EXTRACT;
                            }
                        }
                        continue;
                    }
                    default:
                        continue;
                }
                break;
            }
            case ES_EXTRACT_ARGS: {
#if !defined(__SWITCH__) && !defined(__WIIU__)
                if (args.size() == 0) {
                    LighthouseGui::RegisterPopup(
                        "Run Lighthouse", "All files have been processed. Run Lighthouse?", "Yes", "No",
                        [&]() {
                            if (!AnyRomArchiveExists()) {
                                extractStep = ES_EXTRACT;
                                promptStep = PS_FILE_CHECK;
                            } else {
                                extractStep = ES_VERIFY;
                            }
                        },
                        [&]() {
                            threadPool = nullptr;
                            lhFast3dWindow = nullptr;
                            context = nullptr;
                            exit(0);
                        });
                    break;
                }
                file = args.at(0);
                args.erase(args.begin());
                extract = GameExtractor();
                if (extract.RunStandalone(file)) {
                    bool doExtract = true;
                    std::string archive = "bk.o2r";
                    if (std::filesystem::exists(Ship::Context::GetAppDirectoryPath("bk") + "/" + archive)) {
                        std::string msg = "Archive for current ROM, " + archive + ", already exists.\nExtract again?";
                        LighthouseGui::RegisterPopup("Confirm Re-extract", msg.c_str(), "Yes", "No", [&]() {
                            extracting = true;
                            (void)threadPool->submit_task([&]() -> void {
                                extract.GenerateOTR(extractCount, totalExtract, "bk");
                                extracting = false;
                            });
                        });
                    } else {
                        extracting = true;
                        (void)threadPool->submit_task([&]() -> void {
                            extract.GenerateOTR(extractCount, totalExtract, "bk");
                            extracting = false;
                        });
                    }
                } else {
                    bool open = true;
                    std::string msg = "File\n" + std::string(file) + "\nis not a ROM or does not match supported ROMs.";
                    LighthouseGui::RegisterPopup("Lighthouse ROM Error", msg.c_str());
                }
#else
                extractStep = ES_VERIFY;
#endif
                break;
            }
            case ES_EXTRACT: {
                switch (promptStep) {
                    case PS_FILE_CHECK: {
                        const bool romO2RExists = AnyRomArchiveExists();

                        if (!romO2RExists) {
                            LighthouseGui::RegisterPopup(
                                "No O2R Files", "No O2R files found. Generate one now?", "Yes", "No",
                                [&]() { promptStep = PS_LOCAL; },
                                [&]() {
                                    threadPool = nullptr;
                                    lhFast3dWindow = nullptr;
                                    context = nullptr;
                                    exit(0);
                                });
                        } else {
                            extractStep = ES_VERIFY;
                        }
                        continue;
                    }
                    case PS_LOCAL: {
                        extract = GameExtractor();
                        const std::string appDir = Ship::Context::GetAppDirectoryPath("bk");
                        std::error_code sameDirEc;
                        const bool sameDir = std::filesystem::equivalent(installPath, appDir, sameDirEc);
                        extract.SetSearchPath(installPath);
                        extract.GetRoms(args);
                        if (sameDirEc || !sameDir) {
                            extract.SetSearchPath(appDir);
                            extract.GetRoms(args);
                        }
                        std::sort(args.begin(), args.end());
                        args.erase(std::unique(args.begin(), args.end()), args.end());
                        if (!args.empty()) {
                            promptStep = PS_WAIT;
                            LighthouseGui::RegisterPopup(
                                "ROMs found", "ROMs found in application directory. Would you like to process them?",
                                "Yes", "No", [&]() { extractStep = ES_EXTRACT_ARGS; },
                                [&]() {
                                    args.clear();
                                    promptStep = PS_FIRST;
                                });
                        } else {
                            promptStep = PS_FIRST;
                        }
                        continue;
                    }
                    case PS_FIRST: {
                        if (args.empty()) {
                            std::string baserom = Ship::Context::GetPathRelativeToAppDirectory("baserom.us.z64");
                            if (std::filesystem::exists(baserom) && extract.LoadRomFromPath(baserom)) {
                                extracting = true;
                                extractStarted = true;
                                file = extract.GetRomPath();
                                (void)threadPool->submit_task([&]() -> void {
                                    extract.GenerateOTR(extractCount, totalExtract, "bk");
                                    extracting = false;
                                });
                                continue;
                            }
                            romResultReady = false;
                            romLoaded = false;
                            extract.SelectGameFromUI([&](bool ok) {
                                romLoaded = ok;
                                romResultReady = true;
                            });
                            promptStep = PS_FIRST_WAIT;
                            continue;
                        }
                        extracting = true;
                        extractStarted = true;
                        file = extract.GetRomPath();
                        (void)threadPool->submit_task([&]() -> void {
                            extract.GenerateOTR(extractCount, totalExtract, "bk");
                            extracting = false;
                        });
                        continue;
                    }
                    case PS_FIRST_WAIT: {
                        if (!romResultReady) {
                            goto render;
                        }
                        romResultReady = false;
                        if (!romLoaded) {
                            promptStep = PS_FILE_CHECK;
                            continue;
                        }
                        extracting = true;
                        extractStarted = true;
                        file = extract.GetRomPath();
                        promptStep = PS_FIRST;
                        (void)threadPool->submit_task([&]() -> void {
                            extract.GenerateOTR(extractCount, totalExtract, "bk");
                            extracting = false;
                        });
                        continue;
                    }
                    default:
                        break;
                }
                break;
            }
            case ES_VERIFY: {
                const bool romO2RExists = AnyRomArchiveExists();

                if (!romO2RExists) {
                    if (LighthouseGui::PopupsQueued() == 0) {
                        std::string errorMsg;
                        if (!GameExtractor::sLastError.empty()) {
                            std::string wrapped = GameExtractor::sLastError;
                            const size_t wrapCol = 80;
                            size_t pos = 0;
                            while (pos + wrapCol < wrapped.size()) {
                                size_t breakAt = wrapped.rfind(' ', pos + wrapCol);
                                if (breakAt == std::string::npos || breakAt <= pos) {
                                    breakAt = pos + wrapCol;
                                }
                                wrapped.insert(breakAt, "\n");
                                pos = breakAt + 1;
                            }
                            errorMsg = "ROM extraction failed:\n\n" + wrapped +
                                       "\n\nCheck logs/Lighthouse.log for full details.";
                        } else {
                            errorMsg = "No ROM O2R file detected.\nPlease generate a ROM O2R and relaunch.";
                        }
                        LighthouseGui::RegisterPopup("Extraction Error", errorMsg.c_str(), "OK", "", [&]() {
                            threadPool = nullptr;
                            lhFast3dWindow = nullptr;
                            context = nullptr;
                            exit(0);
                        });
                    }
                    continue;
                }
                extractDone = true;
                continue;
            }
            default:
                break;
        }

    render:
        if (!WindowIsRunning()) {
            threadPool = nullptr;
            lhFast3dWindow = nullptr;
            context = nullptr;
            exit(0);
        }
        wnd->HandleEvents();
        UIWidgets::Colors themeColor =
            static_cast<UIWidgets::Colors>(CVarGetInteger(CVAR_SETTING("Menu.Theme"), UIWidgets::Colors::LightBlue));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, UIWidgets::ColorValues.at(themeColor));
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, UIWidgets::ColorValues.at(UIWidgets::Colors::DarkGray));
        if (!wnd->IsFrameReady()) {
            continue;
        }
        gui->StartDraw();
        lhFast3dWindow->StartFrame();
        lhFast3dWindow->RunGuiOnly();
        const bool showExtractPopup = extracting && !GameExtractor::sCustomCodePromptActive.load();
        if (showExtractPopup && !ImGui::IsPopupOpen("ROM Extraction")) {
            ImGui::OpenPopup("ROM Extraction");
        }
        if (showExtractPopup) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
            auto color = UIWidgets::ColorValues.at(THEME_COLOR);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(color.x, color.y, color.z, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(color.x, color.y, color.z, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
            if (ImGui::BeginPopupModal("ROM Extraction", NULL,
                                       ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize |
                                           ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                           ImGuiWindowFlags_NoSavedSettings)) {
                int phase = GameExtractor::sPhase;
                float progress;
                if (phase == 3) {
                    progress = 100.0f;
                } else {
                    progress = (totalExtract > 0 ? (float)extractCount / (float)totalExtract : 0) * 100.0f;
                    if (progress > 100.0f)
                        progress = 100.0f;
                }

                // Status text
                auto filename = std::filesystem::path(file).filename().string();
                if (phase == 3) {
                    ImGui::Text("Done!");
                } else if (phase >= 1) {
                    ImGui::Text("Processing %s... (Step %d/2)", filename.c_str(), phase);
                    if (Companion::Instance != nullptr && !Companion::Instance->GetCurrentAssetName().empty()) {
                        auto assetName = Companion::Instance->GetCurrentAssetName();
                        float maxWidth = 600.0f - ImGui::GetStyle().WindowPadding.x * 2;
                        ImVec2 textSize = ImGui::CalcTextSize(assetName.c_str());
                        if (textSize.x > maxWidth) {
                            // Truncate with ellipsis
                            std::string ellipsis = "...";
                            float ellipsisWidth = ImGui::CalcTextSize(ellipsis.c_str()).x;
                            while (assetName.size() > 3 &&
                                   ImGui::CalcTextSize(assetName.c_str()).x > maxWidth - ellipsisWidth) {
                                assetName.pop_back();
                            }
                            assetName += ellipsis;
                        }
                        ImGui::Text("%s", assetName.c_str());
                    }
                } else {
                    ImGui::Text("Starting up...");
                }

                // Progress bar
                std::string overlay;
                if (totalExtract > 0 && extractCount > 0) {
                    overlay = fmt::format("{:.0f}%", progress);
                } else if (phase >= 1) {
                    overlay = "Reading ROM, please wait...";
                } else {
                    overlay = "Starting up...";
                }
                ImGui::ProgressBar(progress / 100.0f, ImVec2(600.0f, 50.0f), overlay.c_str());
                ImGui::EndPopup();
            }
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar(2);
        }
        gui->EndDraw();
        lhFast3dWindow->EndFrame();
        ImGui::PopStyleColor(2);
    }
    threadPool = nullptr;

#ifdef __SWITCH__
    Ship::Switch::Init(Ship::PreInitPhase);
#elif defined(__WIIU__)
    Ship::WiiU::Init(appShortName);
#endif

#if not defined(__SWITCH__) && not defined(__WIIU__)
    CheckAndCreateModFolder();
#endif
    if (menuWasVisible) {
        gui->GetMenu()->Show();
    }
}
