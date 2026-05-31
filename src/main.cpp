#include "engine/GameEngine.h"
#include "engine/InputManager.h"
#include "renderer/Renderer.h"
#include "motorcycle/Motorcycle.h"
#include "world/World.h"

#ifdef ANDROID
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        return 1;
    }

    GameEngine engine;
    if (!engine.init(1280, 720, "Ultimate Motorcycle Simulator - Phase 2")) {
        SDL_Log("Engine init failed");
        SDL_Quit();
        return 1;
    }

    engine.run();
    engine.shutdown();
    SDL_Quit();

    return 0;
}
