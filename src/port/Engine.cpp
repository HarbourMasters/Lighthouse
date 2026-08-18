#include "Engine.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <future>
#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <cerrno>
#include <cstring>
#endif
#include "PR/libaudio.h"
#include <libultraship/libultraship.h>

#include <fast/Fast3dWindow.h>
#include <fast/interpreter.h>
#include "fast/resource/ResourceType.h"
#include <fast/resource/factory/DisplayListFactory.h>
#include <fast/resource/factory/TextureFactory.h>
#include <fast/resource/factory/MatrixFactory.h>
#include <fast/resource/factory/VertexFactory.h>
#include <libultraship/bridge/gfxbridge.h>
#include <libultraship/controller/controldeck/ControlDeck.h>
#include <libultraship/libultra/AudioDmaRegistry.h>
#include <SDL2/SDL.h>
#include <ship/controller/controldevice/controller/mapping/ControllerDefaultMappings.h>
#include <ship/resource/factory/BlobFactory.h>
#include <ship/resource/type/Blob.h>
#include <ship/utils/StringHelper.h>
#include <ship/window/gui/Fonts.h>
#include <ship/window/gui/resource/Font.h>

#include "Audio/GameAudio.h"
#include "build.h"
#include "Extractor/ExtractFlow.h"
#include "Extractor/GameExtractor.h"
#include "ship/window/gui/FileBrowserWindow.h"
#include "Interpolation/FrameInterpolation.h"
#include "Nametag/Nametag.h"
#include "OS/OS.h"
#include "Network/Anchor/Anchor.h"
#include "port/Enhancements/Events/PortEnhancements.h"
#include "port/Patches/Patches.h"
#include "port/Save/SaveManager.h"
#include "port/UI/cvar_prefixes.h"
#include "ResourceHelpers.h"
#include "Localization/Language.h"
#include "Resource/Importers/AnimFactory.h"
#include "Resource/Importers/DemoInputFactory.h"
#include "Resource/Importers/DialogFactory.h"
#include "Resource/Importers/MapFactory.h"
#include "Resource/Importers/ModelFactory.h"
#include "Resource/Importers/SpriteFactory.h"
#include "src/port/Enhancements/Events/Hooks/Events.h"
#include "UI/LighthouseGui.hpp"
#include "UI/LighthouseModMenuWindow.h"
#include "LaunchArgs.h"

#ifdef __SWITCH__
#include <port/switch/SwitchImpl.h>
#endif

// Engine constants

#define SAMPLES_PER_FRAME (560 * 2 * 2)
#define gVIsPerFrame 2 // 30 Hz

const float imguiScaleOptionToValue[4] = { 0.75f, 1.0f, 1.5f, 2.0f };
const uint32_t defaultImGuiScale = 1;

// Engine globals

namespace {
constexpr int kDemoAudioHoldFrames = 2;
const char* sOtrSignature = "__OTR__";

// Attract-demo audio hold
std::atomic<bool> sHoldAudio{ false };
int sHoldFramesRemaining = 0;
std::vector<std::shared_ptr<Ship::IResource>> sSoundfontResources;

// Frame pacing and rendering
bool sInterpolationRecorded = false;
std::vector<std::future<void>> sMapBuildFutures;
long long sLastSubFrameNs = 0;
long long sPassBudgetNs = 0;
} // namespace

std::shared_ptr<Fast::Fast3dWindow> lhFast3dWindow;
bool portArchiveVersionMatch = false;
std::string assets_path;

int32_t previousImGuiScaleIndex = -1;
float previousImGuiScale = defaultImGuiScale;

namespace fs = std::filesystem;

extern "C" {

// Reset support
extern s32 D_80275610;

bool prevAltAssets = false;
// bool gEnableGammaBoost = true;

// Soundfont symbols
u8* soundfont1ctl_ROM_START = NULL;
u8* soundfont1ctl_ROM_END = NULL;
u8* soundfont1tbl_ROM_START = NULL;
u8* soundfont2ctl_ROM_START = NULL;
u8* soundfont2ctl_ROM_END = NULL;
u8* soundfont2tbl_ROM_START = NULL;
}

std::vector<uint8_t*> MemoryPool;
GameEngine* GameEngine::Instance;

// Construction

GameEngine::GameEngine() {
    this->context = Ship::Context::CreateUninitializedInstance("Lighthouse", "bk", "lighthouse.cfg.json");

#ifdef __SWITCH__
    Ship::Switch::Init(Ship::PreInitPhase);
    Ship::Switch::Init(Ship::PostInitPhase);
#endif

    this->context->InitConfiguration();
    this->context->InitConsoleVariables();
    assets_path = Ship::Context::LocateFileAcrossAppDirs("lighthouse.o2r");
    portArchiveVersionMatch = std::filesystem::exists(assets_path); // TODO: port archive versioning

    auto controlDeck = std::make_shared<LUS::ControlDeck>();

    this->context->InitControlDeck(controlDeck);
    this->context->InitResourceManager({ assets_path }, {}, 3, true);
    this->context->InitConsole();

    // Register console commands for menu buttons
    Ship::Context::GetRawInstance()->GetConsole()->AddCommand(
        "reset", { [](std::shared_ptr<Ship::Console>, const std::vector<std::string>&, std::string*) -> bool {
                      gPortResetPending = 1; // lets audio spin-waits exit immediately
                      setBootMap(getDefaultBootMap());
                      D_80275610 = 3 + 1; // deferred: mainLoop picks this up next frame
                      CALL_EVENT(OnReset);
                      return 0;
                  },
                   "Reset the game." });
    Ship::Context::GetRawInstance()->GetConsole()->AddCommand(
        "quit", { [](std::shared_ptr<Ship::Console>, const std::vector<std::string>&, std::string*) -> bool {
                     Ship::Context::GetRawInstance()->GetWindow()->Close();
                     return 0;
                 },
                  "Quit the game." });

    lhFast3dWindow = std::make_shared<Fast::Fast3dWindow>(std::vector<std::shared_ptr<Ship::GuiWindow>>({}));
    this->context->InitWindow(lhFast3dWindow);
    this->context->InitAudio({ .SampleRate = 22000, .SampleLength = 736, .DesiredBuffered = 2208 });

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

// Startup

static void RegisterResourceFactories(const std::shared_ptr<Ship::ResourceLoader>& loader) {
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
}

static void LoadLooseModDirectories(const std::string& patches_path) {
    if (patches_path.empty() || !std::filesystem::is_directory(patches_path)) {
        return;
    }
    for (const auto& p : std::filesystem::directory_iterator(patches_path)) {
        if (!p.is_directory()) {
            continue;
        }
        const std::string dirName = p.path().filename().generic_string();
        if (dirName == "~romhacks" || dirName == "~shared" || dirName == "~lang" || IsScopedModFolderName(dirName)) {
            continue;
        }
        SPDLOG_INFO("Found mod directory: {}", p.path().generic_string());
        Ship::Context::GetRawInstance()->GetResourceManager()->GetArchiveManager()->AddArchive(
            p.path().generic_string());
    }
}

static void LoadLanguagePacks() {
    const std::string lang_path = Ship::Context::GetPathRelativeToAppDirectory("mods/~lang");
    if (lang_path.empty() || !std::filesystem::is_directory(lang_path)) {
        return;
    }
    for (const auto& p : std::filesystem::directory_iterator(lang_path)) {
        if (p.is_regular_file() && p.path().extension() == ".o2r") {
            SPDLOG_INFO("Loading language pack: {}", p.path().generic_string());
            Ship::Context::GetRawInstance()->GetResourceManager()->GetArchiveManager()->AddArchive(
                p.path().generic_string());
        }
    }
}

void GameEngine::FinishInit() {
    for (const auto& archive : kRomArchives) {
        std::string romPath = Ship::Context::LocateFileAcrossAppDirs(archive, "bk");
        if (std::filesystem::exists(romPath)) {
            context->GetResourceManager()->GetArchiveManager()->AddArchive(romPath);
        }
    }

    const std::string patches_path = Ship::Context::GetPathRelativeToAppDirectory("mods");
    if (!patches_path.empty() && !std::filesystem::exists(patches_path)) {
        std::filesystem::create_directories(patches_path);
    }

    Lighthouse::ApplyLaunchHack();
    UpdateModFiles(true);
    LoadLooseModDirectories(patches_path);
    LoadLanguagePacks();

#if (_DEBUG)
    auto defaultLogLevel = spdlog::level::debug;
#else
    auto defaultLogLevel = spdlog::level::info;
#endif
    auto logLevel =
        static_cast<spdlog::level::level_enum>(CVarGetInteger(CVAR_DEVELOPER_TOOLS("LogLevel"), defaultLogLevel));
    context->InitLogging(logLevel, logLevel);
    Ship::Context::GetRawInstance()->GetLogger()->set_pattern("[%H:%M:%S.%e] [%s:%#] [%l] %v");
    SPDLOG_INFO("Starting Lighthouse version {} (Branch: {} | Commit: {})", (char*)gBuildVersion, (char*)gGitBranch,
                (char*)gGitCommitHash);
    Lighthouse::FlushLaunchHackLog();

    context->InitFileDropMgr();
    context->InitCrashHandler();
    context->InitEventSystem();

    lhFast3dWindow->SetTargetFps(60);
    lhFast3dWindow->SetMaximumFrameLatency(1);
    lhFast3dWindow->SetRendererUCode(ucode_f3d);

    // Opt-in to memoization
    if (auto interpreter = lhFast3dWindow->GetInterpreterWeak().lock()) {
        interpreter->SetResolvedResourceCacheEnabled(true);
    }

#ifdef USE_NETWORKING
    SDLNet_Init();
#endif

    RegisterResourceFactories(context->GetResourceManager()->GetResourceLoader());
    prevAltAssets = CVarGetInteger(CVAR_SETTING("Mods.AlternateAssets"), 1);
    context->GetResourceManager()->SetAltAssetsEnabled(prevAltAssets);

    Lighthouse::RescanLanguages();

    LighthouseGui::SetupGuiElements();
    Lighthouse::RestoreModSelectionAfterLaunchHack();
    MaybeShowModConflictPopup();
    MaybeShowRomhackBaseMismatchPopup();
    Instance->AudioInit();
    // Instance->LoadDictionary();
    // Instance->LoadPlayerAnims();
#if defined(__SWITCH__) || defined(__WIIU__)
    CVarRegisterInteger(CVAR_IMGUI_CONTROLLER_NAV, 1); // always enable controller nav on switch/wii u
#endif
}

// Fonts and ImGui scaling

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
            Ship::Context::GetRawInstance()->GetResourceManager()->LoadResource(fontPath, false, initData));
        font =
            mImGuiIo->Fonts->AddFontFromMemoryTTF(fontData->Data, static_cast<int>(fontData->DataSize), size, &config);
    }
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

// Lifecycle

void GameEngine::Create(int argc, char* argv[]) {
    Lighthouse::ParseLaunchArgs(argc, argv);
    const auto instance = Instance = new GameEngine();
    // instance->AudioInit();
    // DisplayListPatch::Run();
    // BK renders at 292x216, not the standard 320x240.
    GfxSetNativeDimensions(292, 216);
    instance->RunExtract(argc, argv);
    instance->FinishInit();
    PortEnhancements_Init();
    Anchor::Init();
    SaveManager_Init();
    ShipInit::InitAll();
    ShipInit::Init("BOOT");
    atexit([]() {
        if (Instance && Instance->context && Instance->context->GetControlDeck()) {
            for (int i = 0; i < 4; i++) {
                auto controller = Instance->context->GetControlDeck()->GetControllerByPort(i);
                if (controller) {
                    controller->GetRumble()->StopRumble();
                }
            }
        }
    });
}

extern void ResourceHelpers_ClearRefCache();
void ReleaseSoundfonts();

void GameEngine::Destroy() {
    if (Instance->context && Instance->context->GetControlDeck()) {
        for (int i = 0; i < 4; i++) {
            auto controller = Instance->context->GetControlDeck()->GetControllerByPort(i);
            if (controller) {
                controller->GetRumble()->StopRumble();
            }
        }
    }

    LighthouseGui::Destroy();
    lhFast3dWindow = nullptr;
    ResourceHelpers_ClearRefCache();
    AudioDma_Clear();
    ReleaseSoundfonts();
    if (Instance->context && Instance->context->GetResourceManager()) {
        Instance->context->GetResourceManager()->UnloadResources("*");
    }
    Instance->context = nullptr;
    // PortEnhancements_Exit();
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
            CVarSetInteger(CVAR_SETTING("Mods.AlternateAssets"),
                           !CVarGetInteger(CVAR_SETTING("Mods.AlternateAssets"), 1));
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

void GameEngine::RenderGuiFrame() const {
    if (lhFast3dWindow == nullptr) {
        return;
    }
    lhFast3dWindow->HandleEvents();
    if (!lhFast3dWindow->IsFrameReady()) {
        return;
    }
    auto gui = lhFast3dWindow->GetGui();
    gui->StartDraw();
    lhFast3dWindow->StartFrame();
    lhFast3dWindow->RunGuiOnly();
    gui->EndDraw();
    lhFast3dWindow->EndFrame();
}

bool GameEngine::sRelaunchRequested = false;

void GameEngine::RelaunchIfRequested(int argc, char* argv[]) {
    if (!sRelaunchRequested) {
        return;
    }
#ifdef _WIN32
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(nullptr, exePath, MAX_PATH) > 0) {
        STARTUPINFOW si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};
        if (CreateProcessW(exePath, nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        } else {
            SPDLOG_ERROR("Relaunch failed: CreateProcess error {}", GetLastError());
        }
    }
#elif defined(__linux__) || defined(__APPLE__)
    execv(argv[0], argv);
    SPDLOG_ERROR("Relaunch failed: execv error {}", strerror(errno));
#endif
}

// Audio

extern "C" uint32_t GameEngine_GetSamplesPerFrame() {
    return SAMPLES_PER_FRAME;
}

extern "C" void port_beginDemoAudioHold(void) {
    if (kDemoAudioHoldFrames <= 0) {
        return;
    }
    sHoldFramesRemaining = kDemoAudioHoldFrames;
    sHoldAudio.store(true);
}

extern "C" void port_tickDemoAudioHold(void) {
    if (sHoldAudio.load() && --sHoldFramesRemaining <= 0) {
        sHoldAudio.store(false);
    }
}

extern "C" int port_audioHeld(void) {
    return sHoldAudio.load() ? 1 : 0;
}

void ReleaseSoundfonts() {
    sSoundfontResources.clear();
}

static void LoadSoundfonts() {
    auto rm = Ship::Context::GetRawInstance()->GetResourceManager();
    sSoundfontResources.clear();

    auto loadBlob = [&rm](const char* path, uint8_t*& start, uint8_t*& end) {
        auto res = rm->LoadResource(path);
        if (res) {
            start = (uint8_t*)res->GetRawPointer();
            end = start + res->GetPointerSize();
            AudioDma_Register(start, res->GetPointerSize());
            sSoundfontResources.push_back(res);
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
            AudioDma_Register(start, res->GetPointerSize());
            sSoundfontResources.push_back(res);
        } else {
            SPDLOG_ERROR("[Audio] Failed to load soundfont '{}'", path);
        }
    };

    loadTbl("soundfont/soundfont1tbl", soundfont1tbl_ROM_START);
    loadTbl("soundfont/soundfont2tbl", soundfont2tbl_ROM_START);
}

void GameEngine::AudioInit() {
    LoadSoundfonts();
}

// Frame pacing and rendering

namespace {
using Clock = std::chrono::steady_clock;
inline long long NsSince(Clock::time_point t0) {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - t0).count();
}
} // namespace

void GameEngine::RunCommands(Gfx* Commands, const std::vector<std::unordered_map<Mtx*, MtxF>>& mtx_replacements,
                             size_t frameCount) {
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetRawInstance()->GetWindow());
    if (wnd == nullptr) {
        return;
    }
    auto interpreter = wnd->GetInterpreterWeak().lock().get();
    wnd->HandleEvents();
    interpreter->mInterpolationIndex = 0;
    auto wndBase = Ship::Context::GetRawInstance()->GetWindow();
    const auto passT0 = Clock::now();
    for (size_t frameIdx = 0; frameIdx < frameCount; frameIdx++) {
        if (frameIdx >= 1 && frameIdx - 1 < sMapBuildFutures.size()) {
            sMapBuildFutures[frameIdx - 1].wait();
        }
        if (frameIdx > 0 && sLastSubFrameNs > 0 && (sPassBudgetNs - NsSince(passT0)) < sLastSubFrameNs) {
            break;
        }
        const auto& m = mtx_replacements[frameIdx];
        const float subframeBlend = (frameCount > 1) ? (float)(frameIdx + 1) / (float)frameCount : 1.0f;
        if (frameCount > 1) {
            FrameInterpolation_ApplyAnimVertices(subframeBlend);
        }
        Nametag::SetSubframeBlend(subframeBlend);
        bool isFinalFrame = (frameIdx == frameCount - 1);
        if (frameCount > 1 || wndBase->IsFrameReady()) {
            auto runT0 = Clock::now();
            auto gui = wndBase->GetGui();
            wndBase->GetMouseStateManager()->StartFrame();
            gui->StartDraw();
            interpreter->StartFrame();
            interpreter->Run(Commands, m);
            if (OS_ViBlackActive()) {
                interpreter->mGfxFrameBuffer = 0;
                auto rapi = interpreter->GetCurrentRenderingAPI();
                rapi->StartDrawToFramebuffer(0, 1.0f);
                rapi->ClearFramebuffer(true, false);
            }
            gui->EndDraw();
            sLastSubFrameNs = NsSince(runT0);
            interpreter->EndFrame();
            CALL_EVENT(FrameDrawEnd);
        }
        interpreter->mInterpolationIndex++;
    }
    bool curAltAssets = CVarGetInteger(CVAR_SETTING("Mods.AlternateAssets"), 1);
    if (prevAltAssets != curAltAssets) {
        prevAltAssets = curAltAssets;
        Ship::Context::GetRawInstance()->GetResourceManager()->SetAltAssetsEnabled(curAltAssets);
        gfx_texture_cache_clear();
    }
}

void GameEngine::SetInterpolationRecorded(bool recorded) {
    sInterpolationRecorded = recorded;
}

namespace {
struct SubframePacing {
    int subframes;
    int fps;
    int viPerTick;
};

int CurrentViPerTick() {
    int viPerTick = port_getDemoViCount();
    if (viPerTick <= 0) {
        viPerTick = gVIsPerFrame + port_getCutsceneExtraVis();
    }
    if (viPerTick < gVIsPerFrame) {
        viPerTick = gVIsPerFrame;
    }
    // Clamp to 15 for demo playbacks.
    if (viPerTick > 15) {
        viPerTick = 15;
    }
    return viPerTick;
}

int EffectiveLogicFps() {
    int fps = 60 / CurrentViPerTick();
    return (fps < 1) ? 1 : fps;
}

int SubframesForTarget(int targetFps) {
    int subframes = targetFps / EffectiveLogicFps();
    return (subframes < 1) ? 1 : subframes;
}

SubframePacing ComputeSubframePacing() {
    int target_fps = (int)GameEngine::Instance->GetInterpolationFPS();
    int viPerTick = CurrentViPerTick();
    int subframesPerTick = SubframesForTarget(target_fps);

    if (!sInterpolationRecorded) {
        subframesPerTick = 1;
    }

    int fps = subframesPerTick * 60 / viPerTick;
    if (fps < 1) {
        fps = 1;
    }

    return { subframesPerTick, fps, viPerTick };
}
} // namespace

bool GameEngine::IsInterpolationEnabled() {
    return (int)GetInterpolationFPS() > EffectiveLogicFps();
}

void GameEngine::ProcessGfxCommands(Gfx* commands) {
    auto wnd = std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetRawInstance()->GetWindow());

    if (wnd == nullptr) {
        return;
    }

    // if(gEnableGammaBoost) {
    //     wnd->EnableSRGBMode();
    // }
    wnd->SetRendererUCode(UcodeHandlers::ucode_f3dex);

    static std::vector<std::unordered_map<Mtx*, MtxF>> mtx_replacements;

    const SubframePacing pacing = ComputeSubframePacing();
    const int subframesPerTick = pacing.subframes;
    const int fps = pacing.fps;

    if ((int)mtx_replacements.size() < subframesPerTick) {
        mtx_replacements.resize(subframesPerTick);
    }
    size_t activeFrames = 0;
    sMapBuildFutures.clear();
    for (int i = 1; i <= subframesPerTick; i++) {
        if (i < subframesPerTick) {
            float t = (float)i / (float)subframesPerTick;
            if (i == 1) {
                FrameInterpolation_Interpolate(t, mtx_replacements[activeFrames]);
            } else {
                auto* map = &mtx_replacements[activeFrames];
                sMapBuildFutures.push_back(
                    std::async(std::launch::async, [t, map] { FrameInterpolation_Interpolate(t, *map); }));
            }
        } else {
            mtx_replacements[activeFrames].clear();
        }
        activeFrames++;
    }

    sPassBudgetNs = 1000000000LL * pacing.viPerTick / 60;

    if (wnd != nullptr) {
        wnd->SetTargetFps(fps);
        wnd->SetMaximumFrameLatency(2);
    }

    if (GfxDebuggerIsDebugging()) {
        if (mtx_replacements.empty()) {
            mtx_replacements.emplace_back();
        }
        mtx_replacements[0].clear();
        activeFrames = 1;
    }

    RunCommands(commands, mtx_replacements, activeFrames);

    for (auto& f : sMapBuildFutures) {
        if (f.valid()) {
            f.wait();
        }
    }
    sMapBuildFutures.clear();
}

uint32_t GameEngine::GetInterpolationFPS() {
    if (CVarGetInteger(CVAR_SETTING("MatchRefreshRate"), 0)) {
        return Ship::Context::GetRawInstance()->GetWindow()->GetCurrentRefreshRate();

    } else if (CVarGetInteger(CVAR_VSYNC_ENABLED, 1) ||
               !Ship::Context::GetRawInstance()->GetWindow()->CanDisableVerticalSync()) {
        return std::min<uint32_t>(Ship::Context::GetRawInstance()->GetWindow()->GetCurrentRefreshRate(),
                                  CVarGetInteger(CVAR_SETTING("InterpolationFPS"), 60));
    }

    return CVarGetInteger(CVAR_SETTING("InterpolationFPS"), 30);
}

uint32_t GameEngine::GetInterpolationFrameCount() {
    return static_cast<uint32_t>(SubframesForTarget((int)GetInterpolationFPS()));
}

extern "C" uint32_t GameEngine_GetInterpolationFrameCount() {
    return GameEngine::GetInterpolationFrameCount();
}

// Version reporting and message boxes

void GameEngine::ShowMessage(const char* title, const char* message, SDL_MessageBoxFlags type) {
#if defined(__SWITCH__)
    SPDLOG_ERROR(message);
#else
    SDL_ShowSimpleMessageBox(type, title, message, nullptr);
    SPDLOG_ERROR(message);
#endif
}

bool GameEngine::HasVersion(BKVersion ver) {
    auto versions = Ship::Context::GetRawInstance()->GetResourceManager()->GetArchiveManager()->GetGameVersions();
    return std::find(versions.begin(), versions.end(), ver) != versions.end();
}

extern "C" bool GameEngine_HasVersion(BKVersion ver) {
    return GameEngine::HasVersion(ver);
}

std::vector<BKVersion> GameEngine::GetAvailableVersions() {
    static constexpr BKVersion kKnown[] = { BK_VER_US_10, BK_VER_US_11, BK_VER_PAL, BK_VER_JP };
    auto loaded = Ship::Context::GetRawInstance()->GetResourceManager()->GetArchiveManager()->GetGameVersions();
    std::vector<BKVersion> present;
    for (BKVersion ver : kKnown) {
        if (std::find(loaded.begin(), loaded.end(), static_cast<uint32_t>(ver)) != loaded.end()) {
            present.push_back(ver);
        }
    }
    return present;
}

extern "C" uint32_t GameEngine_GetSampleRate() {
    auto player = Ship::Context::GetRawInstance()->GetAudio()->GetAudioPlayer();
    if (player == nullptr) {
        return 0;
    }

    if (!player->IsInitialized()) {
        return 0;
    }

    return player->GetSampleRate();
}

// End

// C ABI shims

Fast::Interpreter* GameEngine_GetInterpreter() {
    return std::dynamic_pointer_cast<Fast::Fast3dWindow>(Ship::Context::GetRawInstance()->GetWindow())
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
        Ship::Context::GetRawInstance()->GetResourceManager()->LoadResourceProcess(path));
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

static float OTRDimensionFromEdge(float v, float aspectRatio, bool fromRight) {
    auto interpreter = GameEngine_GetInterpreter();
    const uint32_t nativeWidth = interpreter->mNativeDimensions.width;
    const float aspect = (aspectRatio > 0) ? aspectRatio : interpreter->mCurDimensions.aspect_ratio;
    const float halfSpan = (nativeWidth * 3.0f / 4.0f / 2.0f) * aspect;

    return fromRight ? (nativeWidth / 2 + halfSpan - (nativeWidth - v)) : (nativeWidth / 2 - halfSpan + v);
}

extern "C" float OTRGetDimensionFromLeftEdge(float v) {
    return OTRDimensionFromEdge(v, 0.0f, false);
}

extern "C" float OTRGetDimensionFromRightEdge(float v) {
    return OTRDimensionFromEdge(v, 0.0f, true);
}

extern "C" float OTRGetDimensionFromLeftEdgeForcedAspect(float v, float aspectRatio) {
    return OTRDimensionFromEdge(v, aspectRatio, false);
}

extern "C" float OTRGetDimensionFromRightEdgeForcedAspect(float v, float aspectRatio) {
    return OTRDimensionFromEdge(v, aspectRatio, true);
}

extern "C" float OTRGetDimensionFromLeftEdgeOverride(float v) {
    return OTRDimensionFromEdge(v, OTRGetHUDAspectRatio(), false);
}

extern "C" float OTRGetDimensionFromRightEdgeOverride(float v) {
    return OTRDimensionFromEdge(v, OTRGetHUDAspectRatio(), true);
}

extern "C" uint32_t OTRGetGameRenderWidth() {
    auto interpreter = GameEngine_GetInterpreter();
    return interpreter->mCurDimensions.width;
}

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
    int32_t hudWidth = static_cast<int32_t>(hudHeight * hudAspectRatio);
    float hudScreenRatio = (hudWidth / (float)SCREEN_WIDTH);
    float hudCoord = v * hudScreenRatio;
    float gameOffset = static_cast<float>((gameWidth - hudWidth) / 2);
    float gameCoord = hudCoord + gameOffset;
    float gameScreenRatio = ((float)SCREEN_WIDTH / gameWidth);
    float screenScaledCoord = gameCoord * gameScreenRatio;
    int32_t screenScaledCoordInt = static_cast<int32_t>(screenScaledCoord);
    return screenScaledCoordInt;
}

extern "C" void* GameEngine_Malloc(size_t size) {
    MemoryPool.push_back((uint8_t*)malloc(size));
    return MemoryPool.back();
}

extern "C" void GameEngine_Free(void* ptr) {
    for (auto it = MemoryPool.begin(); it != MemoryPool.end(); ++it) {
        if (*it == ptr) {
            free(ptr);
            MemoryPool.erase(it);
            break;
        }
    }
}
