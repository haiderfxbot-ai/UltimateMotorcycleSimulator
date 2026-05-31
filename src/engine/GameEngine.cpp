#include "GameEngine.h"
#include "InputManager.h"
#include "Camera.h"
#include "../renderer/Renderer.h"
#include "../motorcycle/Motorcycle.h"
#include "../world/World.h"
#include "../world/DayNightCycle.h"
#include "../world/WeatherController.h"
#include "../world/Surface.h"
#include "../world/RoadNetwork.h"
#include "../world/TrafficVehicle.h"
#include "../audio/AudioManager.h"
#include <cmath>
#include <algorithm>
#include "../renderer/HUD.h"
#include "../input/TouchControls.h"
#include "../collision/WorldCollision.h"
#include "../engine/GameStateManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdio>
#include <cmath>

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
    , m_audio(nullptr)
    , m_hud(nullptr)
    , m_touch(nullptr)
    , m_collision(nullptr)
    , m_stateManager(nullptr)
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
    m_audio = new AudioManager();
    m_hud = new HUD();
    m_touch = new TouchControls();
    m_collision = new WorldCollision();
    m_stateManager = new GameStateManager();

    if (!m_renderer->init(width, height, title)) {
        SDL_Log("Renderer initialization failed");
        return false;
    }

    if (!m_world->init(m_renderer)) {
        SDL_Log("World initialization failed");
        return false;
    }

    m_audio->init();
    m_hud->init(width, height);
    m_touch->init(width, height);
    m_input->setTouchControls(m_touch);
    m_collision->init();
    m_stateManager->init(width, height);

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
    delete m_hud;         m_hud = nullptr;
    delete m_touch;       m_touch = nullptr;
    delete m_collision;   m_collision = nullptr;
    delete m_stateManager; m_stateManager = nullptr;
    delete m_audio;       m_audio = nullptr;
    delete m_input;       m_input = nullptr;
}

void GameEngine::processInput() {
    m_input->poll();

    if (m_input->quitRequested()) {
        m_running = false;
        return;
    }

    if (m_input->getInputState().honk) {
        m_audio->honk();
    }

    GameState state = m_stateManager->state();
    if (state == GameState::Menu) {
        if (m_input->keyPressed(SDL_SCANCODE_UP)) {
            m_stateManager->selectUp();
        }
        if (m_input->keyPressed(SDL_SCANCODE_DOWN)) {
            m_stateManager->selectDown();
        }
        if (m_input->keyPressed(SDL_SCANCODE_RETURN) || m_input->keyPressed(SDL_SCANCODE_SPACE)) {
            if (m_stateManager->selectedItem() == 0) {
                m_stateManager->setState(GameState::Playing);
                m_bike->reset();
            } else {
                m_running = false;
            }
        }
        if (m_input->keyPressed(SDL_SCANCODE_ESCAPE)) {
            m_running = false;
        }
    } else if (state == GameState::Playing) {
        if (m_input->keyPressed(SDL_SCANCODE_ESCAPE) || m_input->keyPressed(SDL_SCANCODE_P)) {
            m_stateManager->setState(GameState::Paused);
        }
    } else if (state == GameState::Paused) {
        if (m_input->keyPressed(SDL_SCANCODE_UP)) {
            m_stateManager->selectUp();
        }
        if (m_input->keyPressed(SDL_SCANCODE_DOWN)) {
            m_stateManager->selectDown();
        }
        if (m_input->keyPressed(SDL_SCANCODE_ESCAPE) || m_input->keyPressed(SDL_SCANCODE_P)) {
            m_stateManager->setState(GameState::Playing);
        }
        if (m_input->keyPressed(SDL_SCANCODE_RETURN) || m_input->keyPressed(SDL_SCANCODE_SPACE)) {
            if (m_stateManager->selectedItem() == 0) {
                m_stateManager->setState(GameState::Playing);
            } else {
                m_stateManager->setState(GameState::Menu);
                m_stateManager->selectUp();
            }
        }
    } else if (state == GameState::GameOver) {
        if (m_input->keyPressed(SDL_SCANCODE_RETURN) || m_input->keyPressed(SDL_SCANCODE_SPACE) ||
            m_input->keyPressed(SDL_SCANCODE_ESCAPE)) {
            m_stateManager->setState(GameState::Menu);
        }
    }
}

void GameEngine::update(double dt) {
    m_stateManager->update((float)dt);

    GameState state = m_stateManager->state();
    if (state != GameState::Playing) return;

    InputState inputState = m_input->getInputState();
    m_touch->update(inputState);

    m_world->update((float)dt, m_bike->position().x, m_bike->position().z);

    if (inputState.reset) {
        m_bike->reset();
    }

    if (inputState.startEngine && !m_bike->engineRunning()) {
        m_bike->reset();
    }

    m_bike->update(
        (float)dt,
        inputState.throttle,
        inputState.rearBrake,
        inputState.frontBrake,
        inputState.steer,
        inputState.clutch,
        inputState.gearUp,
        inputState.gearDown,
        inputState.startEngine
    );

    float baseGrip = m_bike->surfaceGrip();
    float weatherFactor = m_world->state().gripFactor;
    m_bike->setSurfaceGrip(baseGrip * weatherFactor);

    m_collision->update((float)dt, m_bike, m_world, m_world->traffic());

    m_audio->update((float)dt,
        m_bike->rpm(), m_bike->speed(), inputState.throttle,
        std::abs(inputState.steer) * 0.5f + (inputState.rearBrake > 0.1f ? 0.3f : 0.0f),
        inputState.steer,
        m_world->weather()->isRaining(), m_world->weather()->rainIntensity(),
        m_bike->isCrashed(), m_bike->crashInfo().impactForce,
        m_bike->gear(), m_bike->engineRunning(), m_bike->isStalled());

    m_camera->setTarget(&m_bike->position(), &m_bike->rotation());
    m_camera->update((float)dt);

    if (m_bike->isCrashed() && m_stateManager->state() == GameState::Playing) {
        m_stateManager->setState(GameState::GameOver);
    }
}

void GameEngine::render() {
    GameState state = m_stateManager->state();

    const auto& dnLighting = m_world->dayNight()->lighting();
    LightingUniforms lighting;
    lighting.lightDir = dnLighting.sunDirection;
    lighting.ambientColor = dnLighting.ambientColor;
    lighting.sunColor = dnLighting.sunColor;
    lighting.ambientIntensity = dnLighting.ambientIntensity;
    lighting.sunIntensity = dnLighting.sunIntensity;
    lighting.fogDensity = m_world->weather()->fogDensity();
    lighting.fogColor = dnLighting.fogColor;
    lighting.rainIntensity = m_world->weather()->rainIntensity();
    m_renderer->setLighting(lighting);

    glClearColor(dnLighting.fogColor.r, dnLighting.fogColor.g, dnLighting.fogColor.b, 1.0f);
    m_renderer->beginFrame();

    if (state != GameState::Menu) {
        m_world->render(m_renderer, m_camera);
        m_bike->render(m_renderer, m_camera);
    }

    if (state == GameState::Playing) {
        m_hud->render(m_renderer, m_bike, m_world, m_fps, m_world->state().isNight);
        m_touch->render(m_renderer);
    }

    m_stateManager->render(m_renderer);

    m_renderer->endFrame();
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
        snprintf(buf, sizeof(buf),             "Ultimate MC Sim Phase5 | FPS:%d Spd:%.0fkmh Gear:%d RPM:%.0f %s%s",
            m_fps,
            bike->speed() * 3.6f,
            bike->gear(),
            bike->rpm(),
            bike->engineRunning() ? (bike->isStalled() ? "STALLED" : "RUN") : "OFF",
            bike->isCrashed() ? " CRASHED" : "");

        SDL_SetWindowTitle(m_renderer->window(), buf);
    }
}
