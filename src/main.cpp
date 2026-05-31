#include "engine/GameEngine.h"
#include "debug/ErrorLogger.h"
#include "debug/RuntimeGuard.h"

#ifdef ANDROID
#include <SDL.h>
#include <SDL_main.h>
#else
#include <SDL2/SDL.h>
#endif

extern "C" int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    debug::ErrorLogger::instance().log(debug::Module::System, debug::Severity::Info,
        __FILE__, __func__, __LINE__, "Application starting",
        nullptr, nullptr);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        SDL_Log("SDL init failed: %s", SDL_GetError());
        SYSTEM_ERROR("SDL initialization failed",
            "SDL_Init returned error",
            "Check SDL installation or video driver");
        return 1;
    }

    GameEngine engine;
    if (!engine.init(1280, 720, "Ultimate Motorcycle Simulator")) {
        SDL_Log("Engine init failed");
        ENGINE_ERROR("GameEngine initialization failed",
            "Renderer or world init returned false",
            "Check init() order and dependencies");
        SDL_Quit();
        return 1;
    }

    debug::ErrorLogger::instance().log(debug::Module::Engine, debug::Severity::Info,
        __FILE__, __func__, __LINE__, "Engine initialized, entering main loop",
        nullptr, nullptr);

    engine.run();

    debug::ErrorLogger::instance().dumpAll();
    engine.shutdown();
    SDL_Quit();

    return 0;
}
