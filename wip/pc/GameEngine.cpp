#ifdef LIGHTHOUSE_P

#include <iostream>
#include "GameEngine.h"
#include "GameUI.h"
// #include "starship/patches/DisplayListPatch.h"
#include "graphic/Fast3D/Fast3dWindow.h"
#include "GameExtractor.h"

namespace fs = std::filesystem;

GameEngine* GameEngine::Instance;

GameEngine::GameEngine() {
    std::vector<std::string> archiveFiles;

    const std::string main_path = Ship::Context::GetPathRelativeToAppDirectory("bk64.o2r");
    const std::string assets_path = Ship::Context::GetPathRelativeToAppDirectory("lighthouse.o2r");
    std::cout << "Engine variables created\n";

    if (std::filesystem::exists(main_path)) {
        archiveFiles.push_back(main_path);
        std::cout << "File already exists, moving on.\n";
    } else {
        std::cout << "File doesn't exist, preparing selection.\n";
        if (ShowYesNoBox("No O2R Files", "No O2R files found. Generate one now?") == IDYES) {
            if (!GenAssetFile()) {
                ShowMessage("Error", "An error occured, no O2R file was generated.\n\nExiting...");
                exit(1);
            } else {
                archiveFiles.push_back(main_path);
                std::cout << "File selected, moving on.\n";
            }

            if (ShowYesNoBox("Extraction Complete", "ROM Extracted. Extract another?") == IDYES) {
                if (!GenAssetFile()) {
                    ShowMessage("Error", "An error occured, no O2R file was generated.");
                }
            }
        } else {
            exit(1);
        }
    }

    if (std::filesystem::exists(assets_path)) {
        std::cout << "Mods/Patches Found, moving on.\n";
        archiveFiles.push_back(assets_path);
    }

    std::cout << "Creating uninitialized context instance.\n";
    this->context = Ship::Context::CreateUninitializedInstance("Lighthouse", "ship", "lighthouse.cfg.json");
    std::cout << "Context Configuration initializing...\n";
    this->context->InitConfiguration();                      // needed for libultraship
    std::cout << "Context Configuration initizalized, initializing variables...\n";
    this->context->InitConsoleVariables();                   // needed for libultraship
    std::cout << "Variables initialized, initializing resource manager...\n";
    this->context->InitResourceManager(archiveFiles, {}, 3); // needed for libultraship
    std::cout << "Resource manager initialized, initializing console...\n";
    this->context->InitConsole();                            // needed for libultraship
    std::cout << "Console initialized, initializing window...\n";
    auto window = std::make_shared<Fast::Fast3dWindow>(std::vector<std::shared_ptr<Ship::GuiWindow>>({}));
    std::cout << "Window initialized, initializing default mappings...\n";
    auto defaultMappings = std::make_shared<Ship::ControllerDefaultMappings>(
        // KeyboardKeyToButtonMappings - use built-in LUS defaults
        std::unordered_map<CONTROLLERBUTTONS_T, std::unordered_set<Ship::KbScancode>>(),
        // KeyboardKeyToAxisDirectionMappings - use built-in LUS defaults
        std::unordered_map<Ship::StickIndex, std::vector<std::pair<Ship::Direction, Ship::KbScancode>>>(),
        // SDLButtonToButtonMappings
        std::unordered_map<CONTROLLERBUTTONS_T, std::unordered_set<SDL_GameControllerButton>>{
            { BTN_A, { SDL_CONTROLLER_BUTTON_A } },
            { BTN_B, { SDL_CONTROLLER_BUTTON_X } },
            { BTN_START, { SDL_CONTROLLER_BUTTON_START } },
            { BTN_CLEFT, { SDL_CONTROLLER_BUTTON_Y } },
            { BTN_CDOWN, { SDL_CONTROLLER_BUTTON_B } },
            { BTN_DUP, { SDL_CONTROLLER_BUTTON_DPAD_UP } },
            { BTN_DDOWN, { SDL_CONTROLLER_BUTTON_DPAD_DOWN } },
            { BTN_DLEFT, { SDL_CONTROLLER_BUTTON_DPAD_LEFT } },
            { BTN_DRIGHT, { SDL_CONTROLLER_BUTTON_DPAD_RIGHT } },
            { BTN_R, { SDL_CONTROLLER_BUTTON_RIGHTSHOULDER } },
            { BTN_Z, { SDL_CONTROLLER_BUTTON_LEFTSHOULDER } } },
        // SDLButtonToAxisDirectionMappings - use built-in LUS defaults
        std::unordered_map<Ship::StickIndex, std::vector<std::pair<Ship::Direction, SDL_GameControllerButton>>>(),
        // SDLAxisDirectionToButtonMappings
        std::unordered_map<CONTROLLERBUTTONS_T, std::vector<std::pair<SDL_GameControllerAxis, int32_t>>>{
            { BTN_R, { { SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 1 } } },
            { BTN_Z, { { SDL_CONTROLLER_AXIS_TRIGGERLEFT, 1 } } },
            { BTN_CUP, { { SDL_CONTROLLER_AXIS_RIGHTY, -1 } } },
            { BTN_CRIGHT, { { SDL_CONTROLLER_AXIS_RIGHTX, 1 } } } },
        // SDLAxisDirectionToAxisDirectionMappings - use built-in LUS defaults
        std::unordered_map<Ship::StickIndex,
                           std::vector<std::pair<Ship::Direction, std::pair<SDL_GameControllerAxis, int32_t>>>>());
    std::cout << "Default mappings set, creating control deck...\n";
    auto controlDeck = std::make_shared<LUS::ControlDeck>(std::vector<CONTROLLERBUTTONS_T>(), defaultMappings);
    std::cout << "Control deck created, initializing context...\n";
    this->context->Init(archiveFiles, {}, 3, { 32000, 1024, 1680 }, window, controlDeck);
    std::cout << "Context initialized, setting log level...\n";
    Ship::Context::GetInstance()->GetLogger()->set_level(
        (spdlog::level::level_enum) CVarGetInteger("gDeveloperTools.LogLevel", 1));
    std::cout << "Log level set, setting log pattern...\n";
    Ship::Context::GetInstance()->GetLogger()->set_pattern("[%H:%M:%S.%e] [%s:%#] [%l] %v");
    std::cout << "Log pattern set, initializing resource manager...\n";
    auto loader = context->GetResourceManager()->GetResourceLoader();

    std::cout << "Gameengine::Gameengine finished...\n";
}

void GameEngine::StartFrame() const {
    // Implementation
}

bool GameEngine::GenAssetFile() {
    auto extractor = new GameExtractor();

    if (!extractor->SelectGameFromUI()) {
        ShowMessage("Error", "No ROM selected.\n\nExiting...");
        exit(1);
    }
    std::cout << "File selected, 02r generation begins.\n";
    auto game = extractor->ValidateChecksum();
    if (!game.has_value()) {
        ShowMessage("Unsupported ROM",
                    "The provided ROM is not supported.\n\nCheck the readme for a list of supported versions.");
        exit(1);
    }

    ShowMessage(("Found " + game.value()).c_str(),
                "The extraction process will now begin.\n\nThis may take a few minutes.", SDL_MESSAGEBOX_INFORMATION);

    return extractor->GenerateOTR();
}

void GameEngine::Create() {
    const auto instance = Instance = new GameEngine();
    // DisplayListPatch::Run();
    GameUI::SetupGuiElements();
    // Implementation
}

void GameEngine::HandleAudioThread() {
    // Implementation
}

void GameEngine::StartAudioFrame() {
    // Implementation
}

void GameEngine::EndAudioFrame() {
    // Implementation
}

void GameEngine::AudioInit() {
    // Implementation
}

void GameEngine::AudioExit() {
    // Implementation
}

void GameEngine::RunCommands(Gfx* Commands, const std::vector<std::unordered_map<Mtx*, MtxF>>& mtx_replacements) {
    // Implementation
}

void GameEngine::Destroy() {
    // Implementation
}

uint32_t GameEngine::GetInterpolationFPS() {
    // Implementation
    return 60;
}

void GameEngine::ProcessGfxCommands(Gfx* commands) {
    // Implementation
}

int GameEngine::ShowYesNoBox(const char* title, const char* box) {
    int ret;
#ifdef _WIN32
    ret = MessageBoxA(nullptr, box, title, MB_YESNO | MB_ICONQUESTION);
#else
    SDL_MessageBoxData boxData = { 0 };
    SDL_MessageBoxButtonData buttons[2] = { { 0 } };

    buttons[0].buttonid = IDYES;
    buttons[0].text = "Yes";
    buttons[0].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    buttons[1].buttonid = IDNO;
    buttons[1].text = "No";
    buttons[1].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    boxData.numbuttons = 2;
    boxData.flags = SDL_MESSAGEBOX_INFORMATION;
    boxData.message = box;
    boxData.title = title;
    boxData.buttons = buttons;
    SDL_ShowMessageBox(&boxData, &ret);
#endif
    return ret;
}

void GameEngine::ShowMessage(const char* title, const char* message, SDL_MessageBoxFlags type) {
    // Implementation
}

bool GameEngine::HasVersion(BK64Version ver) {
    // Implementation
    return true;
}

#endif