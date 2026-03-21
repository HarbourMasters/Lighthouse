#include "Engine.h"
#include "ship/utils/StringHelper.h"
#include "ship/window/gui/Fonts.h"
#include "ship/window/gui/resource/Font.h"
#include "extractor/GameExtractor.h"
#include <libultraship/controller/controldeck/ControlDeck.h>
#include "ship/controller/controldevice/controller/mapping/ControllerDefaultMappings.h"

#include <fast/Fast3dWindow.h>
#include "fast/resource/ResourceType.h"
#include <fast/resource/factory/DisplayListFactory.h>
#include <fast/resource/factory/TextureFactory.h>
#include <fast/resource/factory/MatrixFactory.h>
#include <fast/resource/factory/VertexFactory.h>
#include <ship/resource/factory/BlobFactory.h>
#include <ship/resource/type/Blob.h>
#include "resource/importers/AnimFactory.h"
#include "resource/importers/DemoInputFactory.h"
#include "resource/importers/DialogFactory.h"
#include "resource/importers/MapFactory.h"
#include "resource/importers/ModelFactory.h"
#include "resource/importers/SpriteFactory.h"
#include "audio/GameAudio.h"
#include "build.h"
#include "port/ui/cvar_prefixes.h"
#include "ui/LighthouseGui.hpp"
#include "2.0L/PR/libaudio.h"
// #include "port/patches/DisplayListPatch.h"
#include "port/enhancements/events/PortEnhancements.h"

#include <fast/interpreter.h>
#include <libultraship/bridge/gfxbridge.h>
#include <filesystem>
#include <fstream>
#include "FrameInterpolation.h"
#include <libultraship/libultraship.h>

#ifdef __SWITCH__
#include <port/switch/SwitchImpl.h>
#endif

const float imguiScaleOptionToValue[4] = { 0.75f, 1.0f, 1.5f, 2.0f };
std::shared_ptr<Fast::Fast3dWindow> lhFast3dWindow;
const uint32_t defaultImGuiScale = 1;
int32_t previousImGuiScaleIndex = -1;
float previousImGuiScale = defaultImGuiScale;
bool portArchiveVersionMatch = false;
std::string assets_path;

namespace fs = std::filesystem;

extern "C" {
bool prevAltAssets = false;
// bool gEnableGammaBoost = true;

// [port] Audio synthesis entry point (decomp n_synthesizer.c)
Acmd* n_alAudioFrame(Acmd* cmdList, s32* cmdLen, s16* outBuf, s32 outLen);
// [port] DMA cache cleanup (decomp audio_manager.c)
void func_802403F0(void);
void func_80250650(void);

// [port] Soundfont ROM symbols — loaded from OTR in LoadSoundfonts()
u8* soundfont1ctl_ROM_START = NULL;
u8* soundfont1ctl_ROM_END = NULL;
u8* soundfont1tbl_ROM_START = NULL;
u8* soundfont2ctl_ROM_START = NULL;
u8* soundfont2ctl_ROM_END = NULL;
u8* soundfont2tbl_ROM_START = NULL;
}

std::vector<uint8_t*> MemoryPool;
GameEngine* GameEngine::Instance;

typedef struct {
    uint16_t major;
    uint16_t minor;
    uint16_t patch;
} OTRVersion;

// Read the port version from an OTR file
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

// Checks the program version stored in the otr and compares the major value to soh
// For Windows/Mac/Linux if the version doesn't match, offer to
OTRVersion DetectOTRVersion(std::string fileName) {
    bool isOtrOld = false;
    std::string otrPath = Ship::Context::LocateFileAcrossAppDirs(fileName);

    // Doesn't exist so nothing to do here
    if (!std::filesystem::exists(otrPath)) {
        SPDLOG_WARN("O2R file not found at path: {}", otrPath);
        return { INT16_MAX, INT16_MAX, INT16_MAX };
    }

    return ReadPortVersionFromOTR(otrPath);
}

bool VerifyArchiveVersion(OTRVersion version) {
    return version.major == gBuildVersionMajor && version.minor == gBuildVersionMinor;
}

GameEngine::GameEngine() {
    this->context = Ship::Context::CreateUninitializedInstance("Lighthouse", "bk", "lighthouse.cfg.json");

#ifdef __SWITCH__
    Ship::Switch::Init(Ship::PreInitPhase);
    Ship::Switch::Init(Ship::PostInitPhase);
#endif

    this->context->InitConfiguration();    // without this line InitConsoleVariables fails at Config::Reload()
    this->context->InitConsoleVariables(); // without this line the controldeck constructor failes in
    // ShipDeviceIndexMappingManager::UpdateControllerNamesFromConfig()

    assets_path = Ship::Context::LocateFileAcrossAppDirs("lighthouse.o2r");
    portArchiveVersionMatch = std::filesystem::exists(assets_path); // TODO: port archive versioning

    auto controlDeck = std::make_shared<LUS::ControlDeck>();

    this->context->InitControlDeck(controlDeck);
    this->context->InitResourceManager({ assets_path }, {}, 3, true);
    this->context->InitConsole();

    lhFast3dWindow = std::make_shared<Fast::Fast3dWindow>(std::vector<std::shared_ptr<Ship::GuiWindow>>({}));
    this->context->InitWindow(lhFast3dWindow);

    LighthouseGui::SetupMenu();

    if (portArchiveVersionMatch) {
        fontMono = CreateFontWithSize(16.0f, "fonts/Inconsolata-Regular.ttf");
        fontMonoLarger = CreateFontWithSize(20.0f, "fonts/Inconsolata-Regular.ttf");
        fontMonoLargest = CreateFontWithSize(24.0f, "fonts/Inconsolata-Regular.ttf");
        fontStandard = CreateFontWithSize(16.0f, "fonts/Montserrat-Regular.ttf");
        fontStandardLarger = CreateFontWithSize(20.0f, "fonts/Montserrat-Regular.ttf");
        fontStandardLargest = CreateFontWithSize(24.0f, "fonts/Montserrat-Regular.ttf");
        ImGui::GetIO().FontDefault = fontStandardLarger;
    }

    previousImGuiScaleIndex = -1;
    previousImGuiScale = defaultImGuiScale;
    ScaleImGui();
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
    PS_DUPE,
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

bool PathTestCleanup(FILE* tfile) {
    try {
        if (std::filesystem::exists("./text.txt"))
            std::filesystem::remove("./text.txt");
        if (std::filesystem::exists("./test/"))
            std::filesystem::remove("./test/");
    } catch (std::filesystem::filesystem_error const& ex) { return false; }
    return true;
}

void CheckAndCreateModFolder() {
    try {
        std::string modsPath = Ship::Context::LocateFileAcrossAppDirs("mods", "bk");
        if (!std::filesystem::exists(modsPath)) {
            // Create mods folder relative to app dir
            modsPath = Ship::Context::GetPathRelativeToAppDirectory("mods", "bk");
            std::string filePath = modsPath + "/custom_mod_files_go_here.txt";
            if (std::filesystem::create_directories(modsPath)) {
                std::ofstream(filePath).close();
            }
        }
    } catch (std::filesystem::filesystem_error const& ex) {
        // Couldn't make the folder, continue silently
        return;
    }
}

void GameEngine::FinishInit() {
    std::string romPath = Ship::Context::LocateFileAcrossAppDirs("bk.o2r", "bk");
    if (std::filesystem::exists(romPath)) {
        context->GetResourceManager()->GetArchiveManager()->AddArchive(romPath);
    }

    const std::string patches_path = Ship::Context::GetPathRelativeToAppDirectory("mods");

    if (!patches_path.empty()) {
        if (!std::filesystem::exists(patches_path)) {
            std::filesystem::create_directories(patches_path);
        }

        if (std::filesystem::is_directory(patches_path)) {
            for (const auto& p : std::filesystem::recursive_directory_iterator(patches_path)) {
                const auto ext = p.path().extension().string();
                if (StringHelper::IEquals(ext, ".otr") || StringHelper::IEquals(ext, ".o2r")) {
                    Ship::Context::GetInstance()->GetResourceManager()->GetArchiveManager()->AddArchive(
                        p.path().generic_string());
                }

                if (StringHelper::IEquals(ext, ".zip")) {
                    SPDLOG_WARN("Zip files should be only used for development purposes, not for distribution");
                    Ship::Context::GetInstance()->GetResourceManager()->GetArchiveManager()->AddArchive(
                        p.path().generic_string());
                }
            }

            for (const auto& p : std::filesystem::directory_iterator(patches_path)) {
                if (p.is_directory()) {
                    SPDLOG_INFO("Found mod directory: {}", p.path().generic_string());
                    Ship::Context::GetInstance()->GetResourceManager()->GetArchiveManager()->AddArchive(
                        p.path().generic_string());
                }
            }
        }
    }

#if (_DEBUG)
    auto defaultLogLevel = spdlog::level::debug;
#else
    auto defaultLogLevel = spdlog::level::info;
#endif
    auto logLevel =
        static_cast<spdlog::level::level_enum>(CVarGetInteger(CVAR_DEVELOPER_TOOLS("LogLevel"), defaultLogLevel));
    context->InitLogging(logLevel, logLevel);
    Ship::Context::GetInstance()->GetLogger()->set_pattern("[%H:%M:%S.%e] [%s:%#] [%l] %v");
    SPDLOG_INFO("Starting Lighthouse version {} (Branch: {} | Commit: {})", (char*)gBuildVersion, (char*)gGitBranch,
                (char*)gGitCommitHash);

    context->InitGfxDebugger();
    context->InitFileDropMgr();
    context->InitCrashHandler();

    this->context->InitAudio({ .SampleRate = 22000, .SampleLength = 736, .DesiredBuffered = 2208 });

    lhFast3dWindow->SetTargetFps(60);
    lhFast3dWindow->SetMaximumFrameLatency(1);
    lhFast3dWindow->SetRendererUCode(ucode_f3d);

    auto loader = context->GetResourceManager()->GetResourceLoader();
    auto blobFactory = std::make_shared<Ship::ResourceFactoryBinaryBlobV0>();
    loader->RegisterResourceFactory(std::make_shared<Factories::ResourceFactoryBinarySpriteV0>(),
                                    RESOURCE_FORMAT_BINARY, "Sprite",
                                    static_cast<uint32_t>(Torch::ResourceType::BKSprite), 0);
    loader->RegisterResourceFactory(std::make_shared<Factories::ResourceFactoryBinaryModelV0>(), RESOURCE_FORMAT_BINARY,
                                    "Model", static_cast<uint32_t>(Torch::ResourceType::BKModel), 0);
    loader->RegisterResourceFactory(std::make_shared<Factories::ResourceFactoryBinaryBKAnimationV0>(),
                                    RESOURCE_FORMAT_BINARY, "BKAnimation",
                                    static_cast<uint32_t>(Torch::ResourceType::BKAnimation), 0);
    loader->RegisterResourceFactory(std::make_shared<Factories::ResourceFactoryBinaryBKDialogV0>(),
                                    RESOURCE_FORMAT_BINARY, "BKDialog",
                                    static_cast<uint32_t>(Torch::ResourceType::BKDialog), 0);
    loader->RegisterResourceFactory(std::make_shared<Factories::ResourceFactoryBinaryBKQuizQuestionV0>(),
                                    RESOURCE_FORMAT_BINARY, "BKQuizQuestion",
                                    static_cast<uint32_t>(Torch::ResourceType::BKQuizQuestion), 0);
    loader->RegisterResourceFactory(std::make_shared<Factories::ResourceFactoryBinaryBKGruntyQuestionV0>(),
                                    RESOURCE_FORMAT_BINARY, "BKGruntyQuestion",
                                    static_cast<uint32_t>(Torch::ResourceType::BKGruntyQuestion), 0);
    loader->RegisterResourceFactory(std::make_shared<Factories::ResourceFactoryBinaryBKDemoInputV0>(),
                                    RESOURCE_FORMAT_BINARY, "BKDemoInput",
                                    static_cast<uint32_t>(Torch::ResourceType::BKDemoInput), 0);
    loader->RegisterResourceFactory(std::make_shared<Factories::ResourceFactoryBinaryBKMapV0>(), RESOURCE_FORMAT_BINARY,
                                    "BKMap", static_cast<uint32_t>(Torch::ResourceType::BKMap), 0);
    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryBinaryTextureV0>(), RESOURCE_FORMAT_BINARY,
                                    "Texture", static_cast<uint32_t>(Fast::ResourceType::Texture), 0);
    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryBinaryTextureV1>(), RESOURCE_FORMAT_BINARY,
                                    "Texture", static_cast<uint32_t>(Fast::ResourceType::Texture), 1);

    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryBinaryVertexV0>(), RESOURCE_FORMAT_BINARY,
                                    "Vertex", static_cast<uint32_t>(Fast::ResourceType::Vertex), 0);
    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryXMLVertexV0>(), RESOURCE_FORMAT_XML, "Vertex",
                                    static_cast<uint32_t>(Fast::ResourceType::Vertex), 0);

    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryBinaryDisplayListV0>(),
                                    RESOURCE_FORMAT_BINARY, "DisplayList",
                                    static_cast<uint32_t>(Fast::ResourceType::DisplayList), 0);
    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryXMLDisplayListV0>(), RESOURCE_FORMAT_XML,
                                    "DisplayList", static_cast<uint32_t>(Fast::ResourceType::DisplayList), 0);

    loader->RegisterResourceFactory(std::make_shared<Fast::ResourceFactoryBinaryMatrixV0>(), RESOURCE_FORMAT_BINARY,
                                    "Matrix", static_cast<uint32_t>(Fast::ResourceType::Matrix), 0);

    loader->RegisterResourceFactory(std::make_shared<Ship::ResourceFactoryBinaryBlobV0>(), RESOURCE_FORMAT_BINARY,
                                    "Blob", static_cast<uint32_t>(Ship::ResourceType::Blob), 0);
    loader->RegisterResourceFactory(blobFactory, RESOURCE_FORMAT_BINARY, "Blob",
                                    static_cast<uint32_t>(Ship::ResourceType::Blob), 0);
    prevAltAssets = CVarGetInteger("gEnhancements.Mods.AlternateAssets", 1);
    context->GetResourceManager()->SetAltAssetsEnabled(prevAltAssets);

    LighthouseGui::SetupGuiElements();
    Instance->AudioInit();
    // Instance->LoadDictionary();
    // Instance->LoadPlayerAnims();
#if defined(__SWITCH__) || defined(__WIIU__)
    CVarRegisterInteger("gControlNav", 1); // always enable controller nav on switch/wii u
#endif
    // DevConsole_Init();
    // PortEnhancements_Init();
    ShipInit::InitAll();
}

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

    OTRVersion romArchiveVersion = DetectOTRVersion("bk.o2r");

    bool shouldRegen = !VerifyArchiveVersion(romArchiveVersion) && romArchiveVersion.major != INT16_MAX;

    std::filesystem::path ownPath;
    std::vector<std::string> args;
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            args.push_back(argv[argc]);
        }
    }
    GameExtractor extract;
    PromptSteps promptStep = PS_FILE_CHECK;
    std::atomic<bool> extracting = false;
    std::atomic<size_t> extractCount{ 0 }, totalExtract{ 0 };

    std::string installPath = Ship::Context::GetAppBundlePath();
    std::string file;

#if defined(__SWITCH__)
    LighthouseGui::RegisterPopup("Outdated ROM Archives",
                                 "\x1b[2;2HYou've launched the Ship with an old ROM O2R file."
                                 "\x1b[4;2HPlease regenerate a new ROM O2R and relaunch."
                                 "\x1b[6;2HPress the Home button to exit...",
                                 "OK", "", [&]() { exit(1); });
#elif defined(__WIIU__)
    LighthouseGui::RegisterPopup("Outdated ROM Archives",
                                 "You've launched the Ship with an old a ROM O2R file.\n\n"
                                 "Please generate a ROM O2R and relaunch.\n\n"
                                 "Press and hold the Power button to shutdown...",
                                 "OK", "", [&]() { exit(1); });
    OSFatal();
#endif

    if (!std::filesystem::exists(Ship::Context::LocateFileAcrossAppDirs("/assets"))) {
        LighthouseGui::RegisterPopup(
            "Extractor assets not found",
            "No O2R files found. Missing 'assets/' folder needed to generate OTR file.\nPlease "
            "re-extract them from the download or.\n\nExiting...",
            "OK", "", [&]() {
                lhFast3dWindow = nullptr;
                context = nullptr;
                exit(1);
            });
    } else if (shouldRegen) {
        LighthouseGui::RegisterPopup("Outdated ROM Archives",
                                     "Your bk.o2r was created with incompatible versions of Lighthouse.\n"
                                     "You will now be redirected to re-extract them.");
        std::filesystem::remove("bk.o2r");
    }

    std::shared_ptr<BS::thread_pool> threadPool = std::make_shared<BS::thread_pool>(1);
    while (!extractDone) {
        if (LighthouseGui::PopupsQueued() > 0 || extracting) {
            goto render;
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
                    msg = "Please extract the lighthouse.o2r from the Ship of Harkinian download\nto your "
                          "folder.\n\nPress "
                          "and hold the power\n"
                          "button to shutdown...";
#else
                    msg = "Please extract the lighthouse.o2r from the Ship of Harkinian download to your "
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
                        } catch (std::filesystem::filesystem_error const& ex) {
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
                        } catch (std::filesystem::filesystem_error const& ex) { error = true; }
                        if (tfile == NULL || error) {
                            LighthouseGui::RegisterPopup(
                                "Lighthouse Permissions Error",
                                "Lighthouse does not have proper file permissions.\nPlease move it to a "
                                "folder that does and run again.",
                                "OK", "", [&]() {
                                    fclose(tfile);
                                    PathTestCleanup(tfile);
                                    threadPool = nullptr;
                                    lhFast3dWindow = nullptr;
                                    context = nullptr;
                                    exit(0);
                                });
                        } else {
                            fclose(tfile);
                            if (!PathTestCleanup(tfile)) {
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
                            if (!std::filesystem::exists(Ship::Context::GetAppDirectoryPath("bk") + "/bk.o2r")) {
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
                            threadPool->submit_task([&]() -> void {
                                extract.GenerateOTR(extractCount, totalExtract, "bk");
                                extracting = false;
                                extractCount = totalExtract = 0;
                            });
                        });
                    } else {
                        extracting = true;
                        threadPool->submit_task([&]() -> void {
                            extract.GenerateOTR(extractCount, totalExtract, "bk");
                            extracting = false;
                            extractCount = totalExtract = 0;
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
                        const bool romO2RExists =
                            std::filesystem::exists(Ship::Context::LocateFileAcrossAppDirs("bk.o2r", "bk"));

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
                        extract.SetSearchPath(installPath);
                        extract.GetRoms(args);
                        extract.SetSearchPath(Ship::Context::GetAppDirectoryPath("bk"));
                        extract.GetRoms(args);
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
                        if (args.empty() && !extract.SelectGameFromUI()) {
                            promptStep = PS_FILE_CHECK;
                            continue;
                        }
                        extracting = true;
                        file = extract.GetRomPath();
                        threadPool->submit_task([&]() -> void {
                            extract.GenerateOTR(extractCount, totalExtract, "bk");
                            extracting = false;
                            extractStep = ES_VERIFY;
                            extractCount = 0;
                            totalExtract = 0;
                        });
                        continue;
                    }
                    default:
                        break;
                }
                break;
            }
            case ES_VERIFY: {
                const bool romO2RExists =
                    std::filesystem::exists(Ship::Context::LocateFileAcrossAppDirs("bk.o2r", "bk"));

                if (!romO2RExists) {
                    LighthouseGui::RegisterPopup("No ROM Archive",
                                                 "No ROM O2R file detected. Please generate a ROM O2R and relaunch.",
                                                 "OK", "", [&]() {
                                                     threadPool = nullptr;
                                                     lhFast3dWindow = nullptr;
                                                     context = nullptr;
                                                     exit(0);
                                                 });
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
        // Process window events for resize, mouse, keyboard events
        wnd->HandleEvents();
        UIWidgets::Colors themeColor =
            static_cast<UIWidgets::Colors>(CVarGetInteger(CVAR_SETTING("Menu.Theme"), UIWidgets::Colors::LightBlue));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, UIWidgets::ColorValues.at(themeColor));
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, UIWidgets::ColorValues.at(UIWidgets::Colors::DarkGray));

        // Skip dropped frames
        if (!wnd->IsFrameReady()) {
            continue;
        }
        gui->StartDraw();
        lhFast3dWindow->StartFrame();
        lhFast3dWindow->RunGuiOnly();
        if (extracting && !ImGui::IsPopupOpen("ROM Extraction")) {
            ImGui::OpenPopup("ROM Extraction");
        }
        if (extracting) {
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

ImFont* GameEngine::CreateFontWithSize(float size, std::string fontPath) {
    auto mImGuiIo = &ImGui::GetIO();
    ImFont* font;
    if (fontPath == "") {
        ImFontConfig fontCfg = ImFontConfig();
        fontCfg.OversampleH = fontCfg.OversampleV = 1;
        fontCfg.PixelSnapH = true;
        fontCfg.SizePixels = size;
        font = mImGuiIo->Fonts->AddFontDefault(&fontCfg);
    } else {
        auto initData = std::make_shared<Ship::ResourceInitData>();
        ImFontConfig config;
        config.FontDataOwnedByAtlas = false;

        initData->Format = RESOURCE_FORMAT_BINARY;
        initData->Type = static_cast<uint32_t>(RESOURCE_TYPE_FONT);
        initData->ResourceVersion = 0;
        initData->Path = fontPath;
        std::shared_ptr<Ship::Font> fontData = std::static_pointer_cast<Ship::Font>(
            Ship::Context::GetInstance()->GetResourceManager()->LoadResource(fontPath, false, initData));
        font = mImGuiIo->Fonts->AddFontFromMemoryTTF(fontData->Data, fontData->DataSize, size, &config);
    }
    // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly
    float iconFontSize = size * 2.0f / 3.0f;
    static const ImWchar sIconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphMinAdvanceX = iconFontSize;
    mImGuiIo->Fonts->AddFontFromMemoryCompressedBase85TTF(fontawesome_compressed_data_base85, iconFontSize,
                                                          &iconsConfig, sIconsRanges);

    return font;
}

void GameEngine::ScaleImGui() {
    int32_t imGuiScaleIndex = CVarGetInteger("gSettings.ImGuiScale", defaultImGuiScale);
    if (imGuiScaleIndex == previousImGuiScaleIndex) {
        return;
    }

    float scale = imguiScaleOptionToValue[imGuiScaleIndex];
    float newScale = scale / previousImGuiScale;
    ImGui::GetStyle().ScaleAllSizes(newScale);
    ImGui::GetIO().FontGlobalScale = scale;
    previousImGuiScale = scale;
    previousImGuiScaleIndex = imGuiScaleIndex;
}

void GameEngine::Create(int argc, char* argv[]) {
    const auto instance = Instance = new GameEngine();
    // instance->AudioInit();
    // DisplayListPatch::Run();
    // [port] BK renders at 292x216, not the standard 320x240.
    GfxSetNativeDimensions(292, 216);
    instance->RunExtract(argc, argv);
    instance->FinishInit();
    LighthouseGui::SetupGuiElements();
    //#if defined(__SWITCH__) || defined(__WIIU__)
    //    CVarRegisterInteger("gControlNav", 1); // always enable controller nav on switch/wii u
    //    osSetTime(0);
    //#endif
    PortEnhancements_Init();
    ShipInit::InitAll();
}

extern void ResourceHelpers_ClearRefCache();

void GameEngine::Destroy() {
    LighthouseGui::Destroy();
    lhFast3dWindow = nullptr;

    // Flush all resource refs so destructors run while spdlog is still active.
    // sResourceRefCache holds shared_ptrs that outlive the LUS cache otherwise.
    ResourceHelpers_ClearRefCache();
    if (Instance->context && Instance->context->GetResourceManager()) {
        Instance->context->GetResourceManager()->UnloadResources("*");
    }
    Instance->context = nullptr;
    // PortEnhancements_Exit();
    AudioExit();
    for (auto ptr : MemoryPool) {
        free(ptr);
    }
    MemoryPool.clear();
#ifdef __SWITCH__
    Ship::Switch::Exit();
#endif
}

void GameEngine::StartFrame() const {
    using Ship::KbScancode;
    const int32_t dwScancode = this->context->GetWindow()->GetLastScancode();
    this->context->GetWindow()->SetLastScancode(-1);

    switch (dwScancode) {
        case KbScancode::LUS_KB_TAB: {
            // Toggle HD Assets
            CVarSetInteger("gEnhancements.Mods.AlternateAssets",
                           !CVarGetInteger("gEnhancements.Mods.AlternateAssets", 0));
            break;
        }
        case KbScancode::LUS_KB_F4: {
            // gNextGameState = GSTATE_BOOT;
            break;
        }
        default:
            break;
    }
}

#if 0
// Values for 44100 hz
#define SAMPLES_HIGH 752
#define SAMPLES_LOW 720
#else
// Values for 32000 hz
#define SAMPLES_HIGH 560
#define SAMPLES_LOW 528

#endif

// [port] 2 VIs per game frame (30fps)
#define gVIsPerFrame 2

// [port] 736 samples per audio update (44000/60, aligned to 184-sample boundary)
#define AlFrameSize 736

void GameEngine::HandleAudioThread() {
    int16_t audioBuffer[AlFrameSize * 2];
    Acmd cmdList[0x800];

    while (audio.running) {
        {
            std::unique_lock<std::mutex> lock(audio.mutex);
            while (!audio.processing && audio.running) {
                audio.cv_to_thread.wait(lock);
            }
            if (!audio.running) {
                break;
            }
        }

        // [port] generate audio chunks until backend buffer is full
        while (AudioPlayerBuffered() < AudioPlayerGetDesiredBuffered()) {
            int32_t cmdLen = 0;
            int samplesToGen = AlFrameSize * 2 * sizeof(int16_t);

            memset(audioBuffer, 0, samplesToGen);

            func_802403F0(); // [port] recycle stale DMA cache entries
            n_alAudioFrame(cmdList, &cmdLen, audioBuffer, AlFrameSize);
            func_80250650(); // [port] process channel volume/tempo fades (originally in audioManager_handleFrameMsg)
            AudioPlayerPlayFrame((uint8_t*)audioBuffer, samplesToGen);
        }

        {
            std::unique_lock<std::mutex> lock(audio.mutex);
            audio.processing = false;
        }
        audio.cv_from_thread.notify_one();
    }
}

void GameEngine::StartAudioFrame() {
    {
        std::unique_lock<std::mutex> Lock(audio.mutex);
        audio.processing = true;
    }
    audio.cv_to_thread.notify_one();
}

void GameEngine::EndAudioFrame() {
    {
        std::unique_lock<std::mutex> Lock(audio.mutex);
        while (audio.processing) {
            audio.cv_from_thread.wait(Lock);
        }
    }
}

// [port] Load soundfont BLOBs from OTR and set ROM symbol pointers
static void LoadSoundfonts() {
    auto rm = Ship::Context::GetInstance()->GetResourceManager();

    auto loadBlob = [&rm](const char* path, uint8_t*& start, uint8_t*& end) {
        auto res = rm->LoadResource(path);
        if (res) {
            start = (uint8_t*)res->GetRawPointer();
            end = start + res->GetPointerSize();
        } else {
            SPDLOG_ERROR("[Audio] Failed to load soundfont '{}'", path);
        }
    };

    loadBlob("soundfont/soundfont1ctl", soundfont1ctl_ROM_START, soundfont1ctl_ROM_END);
    loadBlob("soundfont/soundfont2ctl", soundfont2ctl_ROM_START, soundfont2ctl_ROM_END);

    // tbl assets don't need END — only START is referenced
    auto loadTbl = [&rm](const char* path, uint8_t*& start) {
        auto res = rm->LoadResource(path);
        if (res) {
            start = (uint8_t*)res->GetRawPointer();
        } else {
            SPDLOG_ERROR("[Audio] Failed to load soundfont '{}'", path);
        }
    };

    loadTbl("soundfont/soundfont1tbl", soundfont1tbl_ROM_START);
    loadTbl("soundfont/soundfont2tbl", soundfont2tbl_ROM_START);
}

void GameEngine::AudioInit() {
    LoadSoundfonts();
    if (!audio.running) {
        audio.running = true;
        audio.thread = std::thread(HandleAudioThread);
    }
}

void GameEngine::AudioExit() {
    {
        std::unique_lock lock(audio.mutex);
        audio.running = false;
    }
    audio.cv_to_thread.notify_all();
    // Wait until the audio thread quit
    audio.thread.join();
}

// [port] GPU→CPU framebuffer readback — defined in Game.cpp
void Framebuffer_ReadbackGPU_FromBackbuffer(Fast::Interpreter* interpreter);
extern "C" int port_isViBlack(void);

void GameEngine::RunCommands(Gfx* Commands, const std::vector<std::unordered_map<Mtx*, MtxF>>& mtx_replacements) {
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow());

    if (wnd == nullptr) {
        return;
    }

    auto interpreter = wnd->GetInterpreterWeak().lock().get();

    // Process window events for resize, mouse, keyboard events
    wnd->HandleEvents();

    interpreter->mInterpolationIndex = 0;

    // [port] Expand DrawAndRunGraphicsCommands so we can read the backbuffer between
    // Run() (frame rendered) and EndFrame() (buffer swap). On N64, CPU/RDP shared
    // physical memory so gFramebuffers always had valid pixel data after rendering.
    auto wndBase = Ship::Context::GetInstance()->GetWindow();
    size_t frameIdx = 0;
    size_t frameCount = mtx_replacements.size();
    for (const auto& m : mtx_replacements) {
        bool isFinalFrame = (frameIdx == frameCount - 1);
        // [port] Bypass IsFrameReady() when interpolation is active — render all
        // frames per tick and let vsync pace them.
        if (frameCount > 1 || wndBase->IsFrameReady()) {
            auto gui = wndBase->GetGui();
            wndBase->GetMouseStateManager()->StartFrame();
            gui->StartDraw();
            interpreter->StartFrame();
            interpreter->Run(Commands, m);
            // [port] Only readback on the final (real) frame, not interpolated ones.
            if (isFinalFrame) {
                Framebuffer_ReadbackGPU_FromBackbuffer(interpreter);
            }
            // [port] Emulate N64 osViBlack: after readback captured the world,
            // clear the game framebuffer to black so the player sees nothing.
            // On N64, osViBlack blanked TV output but the RDP still rendered.
            if (port_isViBlack()) {
                int gameFb = interpreter->mRendersToFb ? interpreter->mGameFb : 0;
                interpreter->mRapi->StartDrawToFramebuffer(gameFb, 1);
                interpreter->mRapi->ClearFramebuffer(true, false);
            }
            gui->EndDraw();
            interpreter->EndFrame();
        }
        interpreter->mInterpolationIndex++;
        frameIdx++;
    }

    bool curAltAssets = CVarGetInteger("gEnhancements.Mods.AlternateAssets", 0);
    if (prevAltAssets != curAltAssets) {
        prevAltAssets = curAltAssets;
        Ship::Context::GetInstance()->GetResourceManager()->SetAltAssetsEnabled(curAltAssets);
        gfx_texture_cache_clear();
    }
}

void GameEngine::ProcessGfxCommands(Gfx* commands) {
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow());

    if (wnd == nullptr) {
        return;
    }

    // if(gEnableGammaBoost) {
    //     wnd->EnableSRGBMode();
    // }
    wnd->SetRendererUCode(UcodeHandlers::ucode_f3dex);

    std::vector<std::unordered_map<Mtx*, MtxF>> mtx_replacements;
    int target_fps = GameEngine::Instance->GetInterpolationFPS();
    static int last_fps;
    static int last_update_rate;
    static int time;
    int fps = target_fps;
    int original_fps = 60 / gVIsPerFrame;

    if (target_fps == 20 || original_fps > target_fps) {
        fps = original_fps;
    }

    if (last_fps != fps || last_update_rate != gVIsPerFrame) {
        time = 0;
    }

    // time_base = fps * original_fps (one second)
    int next_original_frame = fps;

    // [port] Scan the display list for matrices and record for interpolation
    FrameInterpolation_RecordFrame(commands);

    while (time + original_fps <= next_original_frame) {
        time += original_fps;
        if (time != next_original_frame) {
            mtx_replacements.push_back(FrameInterpolation_Interpolate((float)time / next_original_frame));
        } else {
            mtx_replacements.emplace_back();
        }
    }

    time -= fps;

    if (wnd != nullptr) {
        wnd->SetTargetFps(fps);
        wnd->SetMaximumFrameLatency(
            2); // [port] Hardcoded: CVarGetInteger crashes due to heap corruption in debug builds
    }

    // When the gfx debugger is active, only run with the final mtx
    if (GfxDebuggerIsDebugging()) {
        mtx_replacements.clear();
        mtx_replacements.emplace_back();
    }

    RunCommands(commands, mtx_replacements);

    last_fps = fps;
    last_update_rate = gVIsPerFrame;
}

uint32_t GameEngine::GetInterpolationFPS() {
    if (CVarGetInteger("gMatchRefreshRate", 0)) {
        return Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate();

    } else if (CVarGetInteger("gVsyncEnabled", 1) ||
               !Ship::Context::GetInstance()->GetWindow()->CanDisableVerticalSync()) {
        return std::min<uint32_t>(Ship::Context::GetInstance()->GetWindow()->GetCurrentRefreshRate(),
                                  CVarGetInteger("gInterpolationFPS", 60));
    }

    return CVarGetInteger("gInterpolationFPS", 60);
}

uint32_t GameEngine::GetInterpolationFrameCount() {
    return ceil((float)GetInterpolationFPS() / (60.0f / gVIsPerFrame));
}

extern "C" uint32_t GameEngine_GetInterpolationFrameCount() {
    return GameEngine::GetInterpolationFrameCount();
}

void GameEngine::ShowMessage(const char* title, const char* message, SDL_MessageBoxFlags type) {
#if defined(__SWITCH__)
    SPDLOG_ERROR(message);
#else
    SDL_ShowSimpleMessageBox(type, title, message, nullptr);
    SPDLOG_ERROR(message);
#endif
}

bool GameEngine::HasVersion(BKVersion ver) {
    auto versions = Ship::Context::GetInstance()->GetResourceManager()->GetArchiveManager()->GetGameVersions();
    return std::find(versions.begin(), versions.end(), ver) != versions.end();
}

extern "C" bool GameEngine_HasVersion(BKVersion ver) {
    return GameEngine::HasVersion(ver);
}

extern "C" uint32_t GameEngine_GetSampleRate() {
    auto player = Ship::Context::GetInstance()->GetAudio()->GetAudioPlayer();
    if (player == nullptr) {
        return 0;
    }

    if (!player->IsInitialized()) {
        return 0;
    }

    return player->GetSampleRate();
}

#define SAMPLES_PER_FRAME ADPCMFSIZE

extern "C" uint32_t GameEngine_GetSamplesPerFrame() {
    return SAMPLES_PER_FRAME;
}

// End

Fast::Interpreter* GameEngine_GetInterpreter() {
    return std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetInstance()->GetWindow())
        ->GetInterpreterWeak()
        .lock()
        .get();
}

extern "C" float GameEngine_GetAspectRatio() {
    auto interpreter = GameEngine_GetInterpreter();
    return interpreter->mCurDimensions.aspect_ratio;
}

extern "C" uint32_t GameEngine_GetGameVersion() {
    return 0x00000001;
}

static const char* sOtrSignature = "__OTR__";

extern "C" uint8_t GameEngine_OTRSigCheck(const char* data) {
    if (data == nullptr) {
        return 0;
    }
    return strncmp(data, sOtrSignature, strlen(sOtrSignature)) == 0;
}

extern "C" void GameEngine_GetTextureInfo(const char* path, int32_t* width, int32_t* height, float* scale,
                                          bool* custom) {
    if (GameEngine_OTRSigCheck(path) != 1) {
        *custom = false;
        return;
    }
    std::shared_ptr<Fast::Texture> tex = std::static_pointer_cast<Fast::Texture>(
        Ship::Context::GetInstance()->GetResourceManager()->LoadResourceProcess(path));
    *width = tex->Width;
    *height = tex->Height;
    *scale = tex->VPixelScale;
    *custom = tex->Flags & (1 << 0);
}

// Gets the width of the main ImGui window
extern "C" uint32_t OTRGetCurrentWidth() {
    return GameEngine::Instance->context->GetWindow()->GetWidth();
}

// Gets the height of the main ImGui window
extern "C" uint32_t OTRGetCurrentHeight() {
    return GameEngine::Instance->context->GetWindow()->GetHeight();
}

extern "C" float OTRGetHUDAspectRatio() {
    if (CVarGetInteger("gHUDAspectRatio.Enabled", 0) == 0 || CVarGetInteger("gHUDAspectRatio.X", 0) == 0 ||
        CVarGetInteger("gHUDAspectRatio.Y", 0) == 0) {
        return GameEngine_GetAspectRatio();
    }
    return ((float)CVarGetInteger("gHUDAspectRatio.X", 1) / (float)CVarGetInteger("gHUDAspectRatio.Y", 1));
}

extern "C" float OTRGetDimensionFromLeftEdge(float v) {
    auto interpreter = GameEngine_GetInterpreter();
    return (interpreter->mNativeDimensions.width / 2 -
            interpreter->mNativeDimensions.height / 2 * interpreter->mCurDimensions.aspect_ratio + (v));
}

extern "C" float OTRGetDimensionFromRightEdge(float v) {
    auto interpreter = GameEngine_GetInterpreter();
    return (interpreter->mNativeDimensions.width / 2 +
            interpreter->mNativeDimensions.height / 2 * interpreter->mCurDimensions.aspect_ratio -
            (interpreter->mNativeDimensions.width - v));
}

extern "C" float OTRGetDimensionFromLeftEdgeForcedAspect(float v, float aspectRatio) {
    auto interpreter = GameEngine_GetInterpreter();
    return (interpreter->mNativeDimensions.width / 2 -
            interpreter->mNativeDimensions.height / 2 *
                (aspectRatio > 0 ? aspectRatio : interpreter->mCurDimensions.aspect_ratio) +
            (v));
}

extern "C" float OTRGetDimensionFromRightEdgeForcedAspect(float v, float aspectRatio) {
    auto interpreter = GameEngine_GetInterpreter();
    return (interpreter->mNativeDimensions.width / 2 +
            interpreter->mNativeDimensions.height / 2 *
                (aspectRatio > 0 ? aspectRatio : interpreter->mCurDimensions.aspect_ratio) -
            (interpreter->mNativeDimensions.width - v));
}

extern "C" float OTRGetDimensionFromLeftEdgeOverride(float v) {
    return OTRGetDimensionFromLeftEdgeForcedAspect(v, OTRGetHUDAspectRatio());
}

extern "C" float OTRGetDimensionFromRightEdgeOverride(float v) {
    return OTRGetDimensionFromRightEdgeForcedAspect(v, OTRGetHUDAspectRatio());
}

// Gets the width of the current render target area
extern "C" uint32_t OTRGetGameRenderWidth() {
    auto interpreter = GameEngine_GetInterpreter();
    return interpreter->mCurDimensions.width;
}

// Gets the height of the current render target area
extern "C" uint32_t OTRGetGameRenderHeight() {
    auto interpreter = GameEngine_GetInterpreter();
    return interpreter->mCurDimensions.height;
}

extern "C" int16_t OTRGetRectDimensionFromLeftEdge(float v) {
    return ((int)floorf(OTRGetDimensionFromLeftEdge(v)));
}

extern "C" int16_t OTRGetRectDimensionFromRightEdge(float v) {
    return ((int)ceilf(OTRGetDimensionFromRightEdge(v)));
}

extern "C" int16_t OTRGetRectDimensionFromLeftEdgeForcedAspect(float v, float aspectRatio) {
    return ((int)floorf(OTRGetDimensionFromLeftEdgeForcedAspect(v, aspectRatio)));
}

extern "C" int16_t OTRGetRectDimensionFromRightEdgeForcedAspect(float v, float aspectRatio) {
    return ((int)ceilf(OTRGetDimensionFromRightEdgeForcedAspect(v, aspectRatio)));
}

extern "C" int16_t OTRGetRectDimensionFromLeftEdgeOverride(float v) {
    return OTRGetRectDimensionFromLeftEdgeForcedAspect(v, OTRGetHUDAspectRatio());
}

extern "C" int16_t OTRGetRectDimensionFromRightEdgeOverride(float v) {
    return OTRGetRectDimensionFromRightEdgeForcedAspect(v, OTRGetHUDAspectRatio());
}

extern "C" int32_t OTRConvertHUDXToScreenX(int32_t v) {
    auto interpreter = GameEngine_GetInterpreter();
    float gameAspectRatio = interpreter->mCurDimensions.aspect_ratio;
    int32_t gameHeight = interpreter->mCurDimensions.height;
    int32_t gameWidth = interpreter->mCurDimensions.width;
    float hudAspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
    int32_t hudHeight = gameHeight;
    int32_t hudWidth = hudHeight * hudAspectRatio;
    float hudScreenRatio = (hudWidth / (float)SCREEN_WIDTH);
    float hudCoord = v * hudScreenRatio;
    float gameOffset = (gameWidth - hudWidth) / 2;
    float gameCoord = hudCoord + gameOffset;
    float gameScreenRatio = ((float)SCREEN_WIDTH / gameWidth);
    float screenScaledCoord = gameCoord * gameScreenRatio;
    int32_t screenScaledCoordInt = screenScaledCoord;
    return screenScaledCoordInt;
}

extern "C" void* GameEngine_Malloc(size_t size) {
    MemoryPool.push_back(new uint8_t[size]);
    return (void*)MemoryPool.back();
}

extern "C" void GameEngine_Free(void* ptr) {
    for (auto it = MemoryPool.begin(); it != MemoryPool.end(); ++it) {
        if (*it == ptr) {
            delete[] static_cast<uint8_t*>(ptr); // [port] match new uint8_t[] with delete[]
            MemoryPool.erase(it);
            break;
        }
    }
}
