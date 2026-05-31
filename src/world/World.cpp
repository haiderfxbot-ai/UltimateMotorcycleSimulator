#include "World.h"
#include "Surface.h"
#include "TrafficVehicle.h"
#include "DayNightCycle.h"
#include "WeatherController.h"
#include "Terrain.h"
#include "RoadNetwork.h"
#include "WorldChunk.h"
#include "../renderer/Renderer.h"
#include "../engine/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <cstdlib>
#include <cmath>

World::World()
    : m_traffic(nullptr)
    , m_dayNight(nullptr)
    , m_weather(nullptr)
    , m_terrain(nullptr)
    , m_roadNetwork(nullptr)
    , m_chunkManager(nullptr)
    , m_initialSpawnCount(10)
    , m_worldSize(200.0f)
{}

World::~World() {
    delete m_chunkManager;
    delete m_roadNetwork;
    delete m_terrain;
    delete m_traffic;
    delete m_dayNight;
    delete m_weather;
}

bool World::init(Renderer* renderer) {
    (void)renderer;
    std::srand(42);

    m_terrain = new Terrain();
    m_roadNetwork = new RoadNetwork();
    m_roadNetwork->init(42, m_worldSize);

    m_chunkManager = new WorldChunkManager();
    m_chunkManager->init(42);

    m_traffic = new TrafficManager();
    m_traffic->init(m_initialSpawnCount);

    m_dayNight = new DayNightCycle();
    m_weather = new WeatherController();

    m_state.timeOfDay = 12.0f;
    m_state.rainIntensity = 0.0f;
    m_state.gripFactor = 1.0f;

    return true;
}

void World::update(float dt, float playerX, float playerZ) {
    m_state.playerPos = glm::vec3(playerX, 0.0f, playerZ);

    m_dayNight->update(dt);
    m_weather->update(dt);

    float weatherGrip = m_weather->gripFactor();
    m_traffic->update(dt, playerX, playerZ, weatherGrip);

    m_chunkManager->update(playerX, playerZ, 120.0f);

    m_state.timeOfDay = m_dayNight->timeOfDay();
    m_state.rainIntensity = m_weather->rainIntensity();
    m_state.gripFactor = weatherGrip;
    m_state.isNight = m_dayNight->isNight();
}

void World::render(Renderer* renderer, Camera* camera) {
    if (!renderer || !camera) return;

    glm::mat4 pv = camera->projectionMatrix() * camera->viewMatrix();
    renderer->setProjectionView(pv);

    renderTerrain(renderer, camera);
    renderRoad(renderer, camera);
    renderSurfaceVariations(renderer, camera);

    float darken = 0.5f + m_dayNight->lighting().ambientIntensity * 0.5f;
    m_chunkManager->renderObjects(renderer, camera, darken);
    m_traffic->render(renderer, camera);
    renderTrafficLights(renderer, camera);

    renderEnvironmentObjects(renderer, camera);
    renderRain(renderer, camera);
}

void World::renderTerrain(Renderer* renderer, Camera* camera) {
    (void)camera;
    const auto& lighting = m_dayNight->lighting();
    float darken = 0.5f + lighting.ambientIntensity * 0.5f;

    // Render ground plane
    glm::mat4 groundModel = glm::mat4(1.0f);
    renderer->drawPlane(groundModel, glm::vec3(0.3f * darken, 0.55f * darken, 0.2f * darken));

    // Render terrain height patches near player
    float px = m_state.playerPos.x;
    float pz = m_state.playerPos.z;
    int range = 4;

    for (int ix = -range; ix <= range; ++ix) {
        for (int iz = -range; iz <= range; ++iz) {
            float wx = px + (float)ix * 10.0f;
            float wz = pz + (float)iz * 10.0f;

            // Skip road areas (rendered separately)
            if (m_roadNetwork->isOnRoad(wx, wz)) continue;

            float h = m_terrain->getHeightAt(wx, wz);
            if (std::abs(h) < 0.1f && std::abs(wx) < 50.0f && std::abs(wz) < 50.0f) continue;

            glm::mat4 patch = glm::mat4(1.0f);
            patch = glm::translate(patch, glm::vec3(wx, h, wz));
            patch = glm::scale(patch, glm::vec3(9.0f, 1.0f, 9.0f));

            glm::vec3 color;
            if (h < 0.0f) color = glm::vec3(0.4f * darken, 0.5f * darken, 0.2f * darken);
            else if (h > 3.0f) color = glm::vec3(0.3f * darken, 0.4f * darken, 0.15f * darken);
            else color = glm::vec3(0.25f * darken, 0.55f * darken, 0.15f * darken);

            renderer->drawPlane(patch, color);
        }
    }
}

void World::renderGround(Renderer* renderer, Camera* camera) {
    (void)camera;
    (void)renderer;
}

void World::renderSurfaceVariations(Renderer* renderer, Camera* camera) {
    (void)camera;
    float darken = 0.5f + m_dayNight->lighting().ambientIntensity * 0.5f;

    float px = m_state.playerPos.x;
    float pz = m_state.playerPos.z;

    for (int x = (int)(px - 50.0f) / 10 * 10 - 10; x <= (int)(px + 50.0f) / 10 * 10 + 10; x += 10) {
        for (int z = (int)(pz - 50.0f) / 10 * 10 - 10; z <= (int)(pz + 50.0f) / 10 * 10 + 10; z += 10) {
            if (m_roadNetwork->isOnRoad((float)x, (float)z)) continue;

            SurfaceType type = Surface::getSurfaceAt((float)x, (float)z);
            if (type == SurfaceType::Asphalt || type == SurfaceType::Concrete) continue;

            glm::mat4 patchModel = glm::mat4(1.0f);
            float h = m_terrain->getHeightAt((float)x, (float)z);
            patchModel = glm::translate(patchModel, glm::vec3((float)x, h + 0.005f, (float)z));
            patchModel = glm::scale(patchModel, glm::vec3(8.0f, 1.0f, 8.0f));

            glm::vec3 color;
            switch (type) {
                case SurfaceType::Dirt:     color = glm::vec3(0.4f, 0.3f, 0.15f); break;
                case SurfaceType::Gravel:   color = glm::vec3(0.5f, 0.5f, 0.45f); break;
                case SurfaceType::Sand:     color = glm::vec3(0.7f, 0.65f, 0.4f); break;
                case SurfaceType::Grass:    color = glm::vec3(0.25f, 0.5f, 0.12f); break;
                case SurfaceType::Mud:      color = glm::vec3(0.35f, 0.25f, 0.1f); break;
                default:                    color = glm::vec3(0.3f, 0.3f, 0.3f); break;
            }
            renderer->drawPlane(patchModel, color * darken);
        }
    }
}

void World::renderRoad(Renderer* renderer, Camera* camera) {
    m_roadNetwork->render(renderer, camera);

    float roadWet = m_weather->wetness() * 0.15f;
    // Cross road at center
    glm::mat4 crossModel = glm::mat4(1.0f);
    crossModel = glm::translate(crossModel, glm::vec3(0.0f, 0.01f, 0.0f));
    crossModel = glm::rotate(crossModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    crossModel = glm::scale(crossModel, glm::vec3(6.0f, 1.0f, 8.0f));
    glm::vec3 roadColor(0.25f + roadWet, 0.25f + roadWet, 0.25f + roadWet);
    renderer->drawPlane(crossModel, roadColor);
}

void World::renderTrafficLights(Renderer* renderer, Camera* camera) {
    (void)camera;
    for (const auto& light : m_traffic->lights()) {
        glm::mat4 pole = glm::mat4(1.0f);
        pole = glm::translate(pole, glm::vec3(light.position.x, light.position.y * 0.5f, light.position.z));
        pole = glm::scale(pole, glm::vec3(0.08f, 1.0f, 0.08f));
        renderer->drawBox(pole, glm::vec3(0.3f, 0.3f, 0.3f));

        glm::vec3 lightColor;
        switch (light.state) {
            case TrafficLightState::Green:  lightColor = glm::vec3(0.0f, 1.0f, 0.0f); break;
            case TrafficLightState::Yellow: lightColor = glm::vec3(1.0f, 1.0f, 0.0f); break;
            case TrafficLightState::Red:    lightColor = glm::vec3(1.0f, 0.0f, 0.0f); break;
        }
        glm::mat4 bulb = glm::mat4(1.0f);
        bulb = glm::translate(bulb, glm::vec3(light.position.x, light.position.y, light.position.z));
        bulb = glm::scale(bulb, glm::vec3(0.15f, 0.15f, 0.05f));
        renderer->drawBox(bulb, lightColor);
    }
}

void World::renderTrees(Renderer* renderer, Camera* camera) {
    (void)renderer;
    (void)camera;
}

void World::renderBuildings(Renderer* renderer, Camera* camera) {
    (void)renderer;
    (void)camera;
}

void World::renderEnvironmentObjects(Renderer* renderer, Camera* camera) {
    (void)camera;
    float darken = 0.5f + m_dayNight->lighting().ambientIntensity * 0.5f;

    // Guardrails along main road edges
    float px = m_state.playerPos.x;
    float pz = m_state.playerPos.z;
    int railSegments = 20;

    for (int si = 0; si < railSegments; ++si) {
        float z = pz - 40.0f + (float)si * 4.0f;
        if (std::abs(z) > 50.0f) continue;

        for (int side = -1; side <= 1; side += 2) {
            float x = (float)side * 3.5f;
            float h = m_terrain->getHeightAt(x, z);

            glm::mat4 rail = glm::mat4(1.0f);
            rail = glm::translate(rail, glm::vec3(x, h + 0.3f, z));
            rail = glm::scale(rail, glm::vec3(0.05f, 0.3f, 0.05f));
            renderer->drawBox(rail, glm::vec3(0.6f, 0.6f, 0.6f) * darken);
        }
    }

    // Road signs along main road
    for (int si = -2; si <= 2; ++si) {
        if (si == 0) continue;
        float z = (float)si * 15.0f;
        float x = 4.5f;
        float h = m_terrain->getHeightAt(x, z);

        glm::mat4 pole = glm::mat4(1.0f);
        pole = glm::translate(pole, glm::vec3(x, h + 0.8f, z));
        pole = glm::scale(pole, glm::vec3(0.05f, 1.6f, 0.05f));
        renderer->drawBox(pole, glm::vec3(0.4f, 0.4f, 0.4f) * darken);

        glm::mat4 sign = glm::mat4(1.0f);
        sign = glm::translate(sign, glm::vec3(x + 0.05f, h + 1.6f, z));
        sign = glm::scale(sign, glm::vec3(0.45f, 0.35f, 0.02f));
        glm::vec3 signColor = (si > 0) ? glm::vec3(0.0f, 0.4f, 0.6f) : glm::vec3(0.6f, 0.6f, 0.0f);
        renderer->drawBox(sign, signColor * darken);
    }

    // Simple bridge over the cross road at center
    {
        glm::mat4 bridge = glm::mat4(1.0f);
        bridge = glm::translate(bridge, glm::vec3(0.0f, 0.8f, -8.0f));
        bridge = glm::scale(bridge, glm::vec3(6.5f, 0.2f, 3.0f));
        renderer->drawBox(bridge, glm::vec3(0.4f, 0.35f, 0.3f) * darken);

        glm::mat4 bridge2 = glm::mat4(1.0f);
        bridge2 = glm::translate(bridge2, glm::vec3(0.0f, 0.8f, 8.0f));
        bridge2 = glm::scale(bridge2, glm::vec3(6.5f, 0.2f, 3.0f));
        renderer->drawBox(bridge2, glm::vec3(0.4f, 0.35f, 0.3f) * darken);
    }

    // Lamp posts along roads
    for (int li = -3; li <= 3; ++li) {
        float z = (float)li * 12.0f;
        if (std::abs(z) > 50.0f) continue;

        for (int side = -1; side <= 1; side += 2) {
            float x = (float)side * 4.0f;
            float h = m_terrain->getHeightAt(x, z);
            float poleH = 2.5f;

            glm::mat4 pole = glm::mat4(1.0f);
            pole = glm::translate(pole, glm::vec3(x, h + poleH * 0.5f, z));
            pole = glm::scale(pole, glm::vec3(0.06f, poleH, 0.06f));
            renderer->drawBox(pole, glm::vec3(0.2f, 0.2f, 0.2f) * darken);

            glm::mat4 arm = glm::mat4(1.0f);
            arm = glm::translate(arm, glm::vec3(x - (float)side * 0.25f, h + poleH, z));
            arm = glm::scale(arm, glm::vec3(0.3f, 0.04f, 0.04f));
            renderer->drawBox(arm, glm::vec3(0.2f, 0.2f, 0.2f) * darken);

            if (m_dayNight->isNight()) {
                glm::mat4 glow = glm::mat4(1.0f);
                glow = glm::translate(glow, glm::vec3(x - (float)side * 0.25f, h + poleH - 0.1f, z));
                glow = glm::scale(glow, glm::vec3(0.08f, 0.08f, 0.08f));
                renderer->drawSphere(glow, glm::vec3(1.0f, 0.9f, 0.6f));
            }
        }
    }
}

void World::renderRain(Renderer* renderer, Camera* camera) {
    (void)camera;
    renderer->drawRainOverlay();
}

float World::getGripAt(float x, float z) const {
    SurfaceType surface = m_roadNetwork->getSurfaceAt(x, z);
    if (surface == SurfaceType::Asphalt || surface == SurfaceType::Concrete) {
        SurfaceProperties props = Surface::getProperties(surface);
        return props.grip * m_weather->gripFactor();
    }
    SurfaceType surf = Surface::getSurfaceAt(x, z);
    SurfaceProperties props = Surface::getProperties(surf);
    float grip = props.grip;
    grip *= m_weather->gripFactor();
    return grip;
}
