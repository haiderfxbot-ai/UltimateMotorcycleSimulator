#include "TrafficVehicle.h"
#include "../renderer/Renderer.h"
#include "../engine/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>

// ============================
// TrafficVehicle
// ============================

TrafficVehicle::TrafficVehicle()
    : m_position(0.0f, 0.0f, 0.0f)
    , m_velocity(0.0f)
    , m_speed(0.0f)
    , m_targetSpeed(0.0f)
    , m_direction(0.0f)
    , m_steerAngle(0.0f)
    , m_length(4.0f)
    , m_width(1.8f)
    , m_height(1.5f)
    , m_active(false)
    , m_lifeTimer(0.0f)
    , maxLifeTime(60.0f)
{}

TrafficVehicle::~TrafficVehicle() {}

void TrafficVehicle::spawn(const glm::vec3& pos, float direction, float speed) {
    m_position = pos;
    m_direction = direction;
    m_targetSpeed = speed;
    m_speed = speed;
    m_active = true;
    m_lifeTimer = 0.0f;
    m_steerAngle = 0.0f;
    m_velocity = glm::vec3(
        -std::sin(direction) * m_speed,
        0.0f,
        -std::cos(direction) * m_speed
    );
}

void TrafficVehicle::update(float dt, const std::vector<TrafficVehicle*>& allVehicles,
                            const std::vector<TrafficLight>& lights, float playerX, float playerZ,
                            float weatherGripFactor) {
    if (!m_active) return;
    m_lifeTimer += dt;

    float distToPlayer = std::sqrt(
        (m_position.x - playerX) * (m_position.x - playerX) +
        (m_position.z - playerZ) * (m_position.z - playerZ));

    if (distToPlayer > DESPAWN_DIST && m_lifeTimer > 10.0f) {
        m_active = false;
        return;
    }

    float obstacleDist = checkObstacleAhead(allVehicles);
    float lightDist = 0.0f;
    TrafficLightState lightState = getNearestLightState(lights, lightDist);

    float desiredSpeed = m_targetSpeed;

    if (obstacleDist < 8.0f) {
        float factor = obstacleDist / 8.0f;
        desiredSpeed *= factor * factor;
        if (obstacleDist < 2.0f) desiredSpeed = 0.0f;
    }

    if (lightDist < 15.0f && lightState != TrafficLightState::Green) {
        float factor = lightDist / 15.0f;
        desiredSpeed *= factor * 0.5f;
        if (lightDist < 5.0f) desiredSpeed = 0.0f;
    }

    desiredSpeed *= weatherGripFactor;

    m_speed += (desiredSpeed - m_speed) * 2.0f * dt;
    m_speed = std::max(0.0f, m_speed);

    m_velocity = glm::vec3(
        -std::sin(m_direction) * m_speed,
        0.0f,
        -std::cos(m_direction) * m_speed
    );

    m_position += m_velocity * dt;

    if (std::abs(m_position.x) < 4.0f && std::abs(m_position.z) > 50.0f) {
        m_active = false;
    }

    float halfLen = m_length * 0.5f;
    if (m_position.z > 50.0f - halfLen) m_position.z = 50.0f - halfLen;
    if (m_position.z < -50.0f + halfLen) m_position.z = -50.0f + halfLen;
}

float TrafficVehicle::checkObstacleAhead(const std::vector<TrafficVehicle*>& allVehicles) {
    float minDist = 100.0f;
    for (auto* other : allVehicles) {
        if (other == this || !other->isActive()) continue;

        glm::vec3 diff = other->position() - m_position;
        float dDir = std::abs(std::atan2(diff.x, diff.z) - m_direction);
        if (dDir > 0.5f) continue;

        float dist = glm::length(diff);
        if (dist < minDist && dist > 0.1f && dist < 30.0f) {
            minDist = dist;
        }
    }
    return minDist;
}

TrafficLightState TrafficVehicle::getNearestLightState(const std::vector<TrafficLight>& lights, float& distToLight) {
    float minDist = 100.0f;
    TrafficLightState state = TrafficLightState::Green;

    for (const auto& light : lights) {
        float dist = glm::distance(m_position, light.position);
        if (dist < minDist) {
            minDist = dist;
            state = light.state;
        }
    }
    distToLight = minDist;
    return state;
}

void TrafficVehicle::render(Renderer* renderer, Camera* camera) {
    if (!m_active || !renderer) return;

    glm::mat4 pv = camera->projectionMatrix() * camera->viewMatrix();
    renderer->setProjectionView(pv);

    renderCar(renderer, camera);
}

void TrafficVehicle::renderCar(Renderer* renderer, Camera* camera) {
    (void)camera;

    glm::mat4 carModel = glm::mat4(1.0f);
    carModel = glm::translate(carModel, m_position);
    carModel = glm::rotate(carModel, m_direction, glm::vec3(0.0f, 1.0f, 0.0f));

    // Main body
    glm::mat4 body = glm::translate(carModel, glm::vec3(0.0f, m_height * 0.4f, 0.0f));
    body = glm::scale(body, glm::vec3(m_width, m_height * 0.5f, m_length));
    renderer->drawBox(body, glm::vec3(0.3f, 0.6f, 0.9f));

    // Cabin
    glm::mat4 cabin = glm::translate(carModel, glm::vec3(0.0f, m_height * 0.75f, -0.2f));
    cabin = glm::scale(cabin, glm::vec3(m_width * 0.85f, m_height * 0.35f, m_length * 0.5f));
    renderer->drawBox(cabin, glm::vec3(0.4f, 0.7f, 0.95f));

    // Wheels
    float wheelR = 0.25f;
    float wheelW = 0.1f;
    for (int side = -1; side <= 1; side += 2) {
        for (int axle = -1; axle <= 1; axle += 2) {
            glm::mat4 wheel = glm::translate(carModel, glm::vec3(
                side * m_width * 0.55f, wheelR, axle * m_length * 0.35f));
            wheel = glm::rotate(wheel, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            wheel = glm::scale(wheel, glm::vec3(wheelR, wheelR, wheelW));
            renderer->drawCylinder(wheel, glm::vec3(0.1f, 0.1f, 0.1f), 1.0f, 1.0f, 8);
        }
    }
}

// ============================
// TrafficManager
// ============================

TrafficManager::TrafficManager()
    : m_maxVehicles(12)
    , m_spawnTimer(0.0f)
{}

TrafficManager::~TrafficManager() {
    for (auto* v : m_vehicles) delete v;
    m_vehicles.clear();
}

void TrafficManager::init(int numVehicles) {
    m_maxVehicles = numVehicles;
    for (int i = 0; i < numVehicles; ++i) {
        m_vehicles.push_back(new TrafficVehicle());
    }

    // Traffic lights at intersection
    m_lights.resize(4);
    glm::vec3 positions[] = {
        glm::vec3(-3.0f, 2.0f, 0.0f),
        glm::vec3(3.0f, 2.0f, 0.0f),
        glm::vec3(0.0f, 2.0f, -3.0f),
        glm::vec3(0.0f, 2.0f, 3.0f)
    };
    for (int i = 0; i < 4; ++i) {
        m_lights[i].position = positions[i];
        m_lights[i].state = (i < 2) ? TrafficLightState::Green : TrafficLightState::Red;
        m_lights[i].timer = 0.0f;
        m_lights[i].greenDuration = 8.0f + (float)(std::rand() % 4);
        m_lights[i].yellowDuration = 2.0f;
        m_lights[i].redDuration = 10.0f + (float)(std::rand() % 4);
    }

    spawnVehicle();
}

void TrafficManager::update(float dt, float playerX, float playerZ, float weatherGripFactor) {
    updateLights(dt);

    // Update all active vehicles
    for (auto* v : m_vehicles) {
        if (v->isActive()) {
            v->update(dt, m_vehicles, m_lights, playerX, playerZ, weatherGripFactor);
        }
    }

    // Spawn new vehicles periodically
    m_spawnTimer += dt;
    if (m_spawnTimer > 2.0f) {
        m_spawnTimer = 0.0f;
        int activeCount = 0;
        for (auto* v : m_vehicles) if (v->isActive()) activeCount++;

        if (activeCount < m_maxVehicles) {
            spawnVehicle();
        }
    }
}

void TrafficManager::spawnVehicle() {
    for (auto* v : m_vehicles) {
        if (!v->isActive()) {
            int lane = std::rand() % 3 - 1;
            float z = 48.0f;
            float x = lane * 1.8f;
            float dir = 3.14159f;
            float speed = 8.0f + (float)(std::rand() % 40) / 10.0f;

            if (std::rand() % 2 == 0) {
                z = -48.0f;
                dir = 0.0f;
            }

            v->spawn(glm::vec3(x, 0.15f, z), dir, speed);
            break;
        }
    }
}

void TrafficManager::updateLights(float dt) {
    for (auto& light : m_lights) {
        light.timer += dt;
        switch (light.state) {
            case TrafficLightState::Green:
                if (light.timer > light.greenDuration) {
                    light.state = TrafficLightState::Yellow;
                    light.timer = 0.0f;
                }
                break;
            case TrafficLightState::Yellow:
                if (light.timer > light.yellowDuration) {
                    light.state = TrafficLightState::Red;
                    light.timer = 0.0f;
                }
                break;
            case TrafficLightState::Red:
                if (light.timer > light.redDuration) {
                    light.state = TrafficLightState::Green;
                    light.timer = 0.0f;
                }
                break;
        }
    }
}

void TrafficManager::render(Renderer* renderer, Camera* camera) {
    for (auto* v : m_vehicles) {
        v->render(renderer, camera);
    }
}
