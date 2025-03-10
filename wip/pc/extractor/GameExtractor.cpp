#include "GameExtractor.h"

#include <string>
#include "portable-file-dialogs.h"
#include "Companion.h"

std::unordered_map<std::string, std::string> mGameList = {
    { "1fe1632098865f639e22c11b9a81ee8f29c75d7a", "Banjo Kazooie (U) (V1.0)" },
    { "1fb13cad402518d3ae9a8dc4b52c5c54b2a4adc7", "Banjo Kazooie (U) (V1.0) (Uncompressed)" },
    { "ded6ee166e740ad1bc810fd678a84b48e245ab80", "Banjo Kazooie (U) (V1.1)" },
    { "90726d7e7cd5bf6cdfd38f45c9acbf4d45bd9fd8", "Banjo Kazooie (J) (V1.0)" },
    { "bb359a75941df74bf7290212c89fbc6e2c5601fe", "Banjo Kazooie (P) (V1.0)" }
};

bool GameExtractor::SelectGameFromUI() {
#if !defined(__IOS__) || !defined(__ANDROID__) || !defined(__SWITCH__)
    auto selection = pfd::open_file("Select a file", ".", { "N64 Roms", "*.z64" }).result();

    if (selection.empty()) {
        std::cout << "File not selected.\n";
        return false;
    }
    std::cout << "Changing path variable to selected rom.\n";
    this->mGamePath = selection[0];
#else
    this->mGamePath = Ship::Context::GetPathRelativeToAppDirectory("baserom.z64");
#endif

    std::ifstream file(this->mGamePath, std::ios::binary);
    this->mGameData = std::vector<uint8_t>( std::istreambuf_iterator( file ), {} );
    std::cout << "File.close is about to happen.\n";
    file.close();
    std::cout << "File closed, returning true.\n";
    return true;
}

std::optional<std::string> GameExtractor::ValidateChecksum() const {
    std::cout << "ValidateChecksum beginning now.\n";
    
    const auto rom = new N64::Cartridge(this->mGameData);
    std::cout << "ValidateChecksum variables set, about to initialize rom.\n";

    rom->Initialize();
    std::cout << "rom initialized, bout to get hash.\n";

    auto hash = rom->GetHash();
    std::cout << "Hash: " << hash << "\n";
    std::cout << "Hash collected, time to check it.\n";
    
    if (mGameList.find(hash) == mGameList.end()) {
        std::cout << "Hash not in list, returning NULL.\n";
        return std::nullopt;
    }

    std::cout << "Hash in list, returning game name: " << mGameList[hash] << "\n";
    return mGameList[hash];
}

bool GameExtractor::GenerateOTR() const {
    std::cout << "Generate O2R beginning now.\n";
    Companion::Instance = new Companion(this->mGameData, ArchiveType::O2R, false);

    try {
        std::cout << "Initializing O2R conversion.\n";
        Companion::Instance->Init(ExportType::Binary);
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
        std::cout << "O2R generation exception occured, returning false.\n";
        return false;
    }
    std::cout << "File converted, returning true.\n";
    return true;
}