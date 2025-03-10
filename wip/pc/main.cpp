#ifdef LIGHTHOUSE_P

#include <iostream>
#include "libultraship/libultraship.h"
#include "compat.h"
#include "main.h"
#include "functions.h"
#include "GameEngine.h"

class MyGame {
  public:
    bool Initialize() {
        GameEngine::Create();
        std::cout << "Engine created\n";
        return true;
    }

    void Update() {
        // game logic
    }

    void Render() {
        // graphics logic
    }

    void Shutdown() {
        // cleanup
    }
};

// platform options `main` opening definition
#ifdef _WIN32
int SDL_main(int argc, char** argv) {
#else
#if defined(__cplusplus) && defined(PLATFORM_IOS)
extern "C"
#endif
int main(int argc, char* argv[]) {
#endif
    MyGame game;
    if (!game.Initialize()) {
        fprintf(stderr, "Game initialization failed\n");
        return -1;
    }
    std::cout << "Game initialized\n";
    bool running = false;
    while (running) {

        game.Update();
        game.Render();
    }
    game.Shutdown();
    return 0;
}

#endif