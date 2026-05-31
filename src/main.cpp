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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL video/audio init failed: %s", SDL_GetError());
        SYSTEM_ERROR("SDL initialization failed",
            "SDL_Init returned error",
            "Check SDL installation or video driver");
        return 1;
    }

    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0) {
        SDL_Log("SDL gamecontroller init failed (non-fatal): %s", SDL_GetError());
    }

    SDL_Log("Creating GameEngine...");
    GameEngine engine;
    SDL_Log("Calling engine.init(1280, 720)...");
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

    // Flush any stale events (e.g. lifecycle SDL_QUIT from activity creation)
    {
        SDL_Event flush;
        while (SDL_PollEvent(&flush)) {}
    }

    engine.run();

    debug::ErrorLogger::instance().dumpAll();
    engine.shutdown();
    SDL_Quit();

    return 0;
}
