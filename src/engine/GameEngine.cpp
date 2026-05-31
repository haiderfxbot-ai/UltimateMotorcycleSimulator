#include "GameEngine.h"
#include "InputManager.h"
#include "Camera.h"
#include "../renderer/Renderer.h"
#include "../motorcycle/Motorcycle.h"
#include "../world/World.h"

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

    if (m_input->keyPressed(SDL_SCANCODE_ESCAPE)) {
        m_running = false;
    }
}

void GameEngine::update(double dt) {
    float throttle = 0.0f, brake = 0.0f, steer = 0.0f;

    if (m_input->keyHeld(SDL_SCANCODE_UP) || m_input->keyHeld(SDL_SCANCODE_W))
        throttle = 1.0f;
    if (m_input->keyHeld(SDL_SCANCODE_DOWN) || m_input->keyHeld(SDL_SCANCODE_S))
        brake = 1.0f;
    if (m_input->keyHeld(SDL_SCANCODE_LEFT) || m_input->keyHeld(SDL_SCANCODE_A))
        steer = -1.0f;
    if (m_input->keyHeld(SDL_SCANCODE_RIGHT) || m_input->keyHeld(SDL_SCANCODE_D))
        steer = 1.0f;

    if (m_input->keyPressed(SDL_SCANCODE_R))
        m_bike->reset();

    m_bike->update((float)dt, throttle, brake, steer);
    m_camera->update((float)dt);
}

void GameEngine::render() {
    m_renderer->beginFrame();
    m_world->render(m_renderer, m_camera);
    m_bike->render(m_renderer, m_camera);
    m_renderer->endFrame();
}

void GameEngine::calculateFPS() {
    m_frameCount++;
    m_fpsTimer += m_deltaTime;
    if (m_fpsTimer >= 1.0) {
        m_fps = m_frameCount;
        m_frameCount = 0;
        m_fpsTimer -= 1.0;
        char buf[64];
        snprintf(buf, sizeof(buf), "Ultimate Motorcycle Simulator - %d FPS", m_fps);
        SDL_SetWindowTitle(m_renderer->window(), buf);
    }
}
