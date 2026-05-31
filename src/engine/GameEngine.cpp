#include "GameEngine.h"
#include "InputManager.h"
#include "Camera.h"
#include "../renderer/Renderer.h"
#include "../motorcycle/Motorcycle.h"
#include "../world/World.h"
#include <cstdio>

#ifdef ANDROID
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include <GLES3/gl3.h>

GameEngine::GameEngine()
    : m_running(false)
    , m_deltaTime(0.0)
    , m_totalTime(0.0)
    , m_fps(0)
    , m_frameCount(0)
    , m_fpsTimer(0.0)
    , m_input(nullptr)
    , m_renderer(nullptr)
    , m_bike(nullptr)
    , m_camera(nullptr)
    , m_world(nullptr)
{}

GameEngine::~GameEngine() {
    shutdown();
}

bool GameEngine::init(int width, int height, const char* title) {
    m_input = new InputManager();
    m_renderer = new Renderer();
    m_bike = new Motorcycle();
    m_camera = new Camera();
    m_world = new World();

    if (!m_renderer->init(width, height, title)) {
        SDL_Log("Renderer initialization failed");
        return false;
    }

    if (!m_world->init(m_renderer)) {
        SDL_Log("World initialization failed");
        return false;
    }

    m_bike->reset();
    m_camera->setTarget(&m_bike->position(), &m_bike->rotation());
    m_camera->setAspect((float)width / (float)height);
    m_running = true;
    return true;
}

void GameEngine::run() {
    Uint64 lastTime = SDL_GetPerformanceCounter();
    const double dtFixed = 1.0 / 60.0;

    while (m_running) {
        Uint64 now = SDL_GetPerformanceCounter();
        double elapsed = (double)(now - lastTime) / SDL_GetPerformanceFrequency();
        lastTime = now;

        if (elapsed > 0.05) elapsed = 0.05;
        m_deltaTime = elapsed;
        m_totalTime += elapsed;

        processInput();
        update(elapsed);
        render();

        calculateFPS();
    }
}

void GameEngine::shutdown() {
    m_running = false;
    delete m_world;       m_world = nullptr;
    delete m_camera;      m_camera = nullptr;
    delete m_bike;        m_bike = nullptr;
    delete m_renderer;    m_renderer = nullptr;
    delete m_input;       m_input = nullptr;
}

void GameEngine::processInput() {
    m_input->poll();

    if (m_input->quitRequested()) {
        m_running = false;
        return;
    }
}

void GameEngine::update(double dt) {
    InputState state = m_input->getInputState();

    if (state.reset) {
        m_bike->reset();
    }

    if (state.startEngine && !m_bike->engineRunning()) {
        m_bike->reset();
        // Engine will start on next update cycle
    }

    m_bike->update(
        (float)dt,
        state.throttle,
        state.rearBrake,
        state.frontBrake,
        state.steer,
        state.clutch,
        state.gearUp,
        state.gearDown,
        state.startEngine
    );

    m_camera->setTarget(&m_bike->position(), &m_bike->rotation());
    m_camera->update((float)dt);
}

void GameEngine::render() {
    m_renderer->beginFrame();
    m_world->render(m_renderer, m_camera);
    m_bike->render(m_renderer, m_camera);
    renderHUD();
    m_renderer->endFrame();
}

void GameEngine::renderHUD() {
    // Simple HUD using OpenGL - bike stats overlay
    (void)0;
    // In a full implementation, we would render text here
    // For now, stats are shown in the window title via FPS counter
}

void GameEngine::calculateFPS() {
    m_frameCount++;
    m_fpsTimer += m_deltaTime;
    if (m_fpsTimer >= 1.0) {
        m_fps = m_frameCount;
        m_frameCount = 0;
        m_fpsTimer -= 1.0;

        const Motorcycle* bike = m_bike;
        char buf[128];
        snprintf(buf, sizeof(buf), "Ultimate MC Sim Phase2 | FPS:%d Spd:%.0fkmh Gear:%d RPM:%.0f %s%s",
            m_fps,
            bike->speed() * 3.6f,
            bike->gear(),
            bike->rpm(),
            bike->engineRunning() ? (bike->isStalled() ? "STALLED" : "RUN") : "OFF",
            bike->isCrashed() ? " CRASHED" : "");

        SDL_SetWindowTitle(m_renderer->window(), buf);
    }
}
