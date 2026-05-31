#include "Motorcycle.h"
#include "../renderer/Renderer.h"
#include "../engine/Camera.h"
#include <GLES3/gl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>
#include <cmath>

Motorcycle::Motorcycle()
    : m_position(0.0f, 0.3f, 0.0f)
    , m_rotation(0.0f)
    , m_velocity(0.0f)
    , m_speed(0.0f)
    , m_maxSpeed(55.0f)
    , m_acceleration(15.0f)
    , m_brakingForce(25.0f)
    , m_steeringSpeed(2.5f)
    , m_leanAngle(0.0f)
    , m_targetLean(0.0f)
    , m_rpm(1000.0f)
    , m_idleRPM(1000.0f)
    , m_maxRPM(11000.0f)
    , m_redlineRPM(9500.0f)
    , m_gear(0)
    , m_maxGear(5)
    , m_engineRunning(true)
    , m_clutchEngaged(false)
    , m_length(2.0f)
    , m_width(0.6f)
    , m_height(1.0f)
    , m_wheelRadius(0.3f)
    , m_wheelWidth(0.12f)
    , m_wheelbase(1.4f)
{
    m_gearRatios[0] = 0.0f;
    m_gearRatios[1] = 3.2f;
    m_gearRatios[2] = 2.1f;
    m_gearRatios[3] = 1.5f;
    m_gearRatios[4] = 1.1f;
    m_gearRatios[5] = 0.85f;
}

Motorcycle::~Motorcycle() {}

void Motorcycle::reset() {
    m_position = glm::vec3(0.0f, 0.3f, 0.0f);
    m_rotation = glm::vec3(0.0f);
    m_velocity = glm::vec3(0.0f);
    m_speed = 0.0f;
    m_rpm = m_idleRPM;
    m_gear = 0;
    m_engineRunning = true;
    m_leanAngle = 0.0f;
    m_targetLean = 0.0f;
}

void Motorcycle::update(float dt, float throttleInput, float brakeInput, float steerInput) {
    if (!m_engineRunning) {
        m_speed *= 0.98f;
        if (std::abs(m_speed) < 0.01f) m_speed = 0.0f;
        m_velocity = glm::vec3(0.0f, 0.0f, -m_speed);
        m_velocity = glm::rotateY(m_velocity, m_rotation.y);
        m_position += m_velocity * dt;
        return;
    }

    updatePhysics(dt, throttleInput, brakeInput, steerInput);
}

void Motorcycle::updatePhysics(float dt, float throttle, float brake, float steer) {
    float speedFactor = std::max(0.2f, 1.0f - m_speed / m_maxSpeed);

    if (m_gear == 0) {
        m_rpm = m_idleRPM + throttle * 2000.0f;
        if (m_rpm > m_redlineRPM) m_rpm = m_redlineRPM;
    } else {
        float gearRatio = m_gearRatios[m_gear];
        float engineRPM = m_speed * gearRatio * 60.0f;
        m_rpm = m_idleRPM + engineRPM;

        if (throttle > 0.0f) {
            m_rpm += throttle * 3000.0f * dt;
        } else {
            m_rpm *= 0.98f;
        }
        m_rpm = std::max(m_idleRPM, std::min(m_rpm, m_maxRPM));
    }

    // Auto gear shifting (Phase 1: automatic)
    if (m_gear > 0 && m_rpm > m_redlineRPM && m_gear < m_maxGear) {
        m_gear++;
    }
    if (m_gear > 1 && m_rpm < 3000.0f) {
        m_gear--;
    }

    // Throttle
    float driveForce = 0.0f;
    if (m_gear > 0 && throttle > 0.0f) {
        driveForce = throttle * m_acceleration * speedFactor;
    }

    // Braking
    float brakeForce = 0.0f;
    if (brake > 0.0f && m_speed > 0.1f) {
        brakeForce = brake * m_brakingForce;
        if (brake > 0.7f && m_speed > 10.0f) {
            m_rotation.x += brake * dt * 0.3f;
        }
    } else if (brake > 0.0f && m_speed < -0.1f) {
        brakeForce = brake * m_brakingForce * 0.5f;
    }

    // Rolling resistance + drag
    float drag = m_speed * m_speed * 0.02f;
    if (m_speed > 0.0f) drag = -drag;
    else if (m_speed < 0.0f) drag = std::abs(drag);

    float resistance = -m_speed * 0.5f;

    // Net acceleration
    float netForce = driveForce - brakeForce + drag + resistance;
    m_speed += netForce * dt;

    if (std::abs(m_speed) < 0.01f) m_speed = 0.0f;
    m_speed = std::max(-5.0f, std::min(m_speed, m_maxSpeed));

    // Steering
    float speedRatio = std::min(1.0f, std::abs(m_speed) / 15.0f);
    m_targetLean = -steer * speedRatio * 0.5f;
    m_leanAngle += (m_targetLean - m_leanAngle) * std::min(1.0f, 5.0f * dt);

    float steerFactor = steer * m_steeringSpeed * speedRatio;
    if (std::abs(m_speed) > 0.5f) {
        m_rotation.y += steerFactor * dt;
    }

    // Apply rotation based on lean for visual roll
    m_rotation.x = m_leanAngle;

    // Position update
    m_velocity = glm::vec3(0.0f, 0.0f, -m_speed);
    m_velocity = glm::rotateY(m_velocity, m_rotation.y);
    m_position += m_velocity * dt;
}

void Motorcycle::render(Renderer* renderer, Camera* camera) {
    if (!renderer) return;

    glm::mat4 pv = camera->projectionMatrix() * camera->viewMatrix();
    renderer->setProjectionView(pv);

    renderBike(renderer, camera);
}

void Motorcycle::renderBike(Renderer* renderer, Camera* camera) {
    (void)camera;

    glm::mat4 bikeModel = glm::mat4(1.0f);
    bikeModel = glm::translate(bikeModel, m_position);
    bikeModel = glm::rotate(bikeModel, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    bikeModel = glm::rotate(bikeModel, m_leanAngle, glm::vec3(1.0f, 0.0f, 0.0f));

    // Main body (engine + tank area)
    glm::mat4 bodyModel = glm::translate(bikeModel, glm::vec3(0.0f, 0.4f, 0.0f));
    bodyModel = glm::scale(bodyModel, glm::vec3(m_width, m_height * 0.5f, m_length * 0.5f));
    renderer->drawBox(bodyModel, glm::vec3(0.9f, 0.3f, 0.15f));

    // Seat
    glm::mat4 seatModel = glm::translate(bikeModel, glm::vec3(0.0f, 0.6f, -0.1f));
    seatModel = glm::scale(seatModel, glm::vec3(m_width * 0.8f, 0.1f, 0.3f));
    renderer->drawBox(seatModel, glm::vec3(0.15f, 0.15f, 0.15f));

    // Head / front fairing
    glm::mat4 headModel = glm::translate(bikeModel, glm::vec3(0.0f, 0.5f, 0.8f));
    headModel = glm::scale(headModel, glm::vec3(m_width * 0.7f, m_height * 0.3f, 0.15f));
    renderer->drawBox(headModel, glm::vec3(0.95f, 0.4f, 0.2f));

    // Tail
    glm::mat4 tailModel = glm::translate(bikeModel, glm::vec3(0.0f, 0.35f, -0.8f));
    tailModel = glm::scale(tailModel, glm::vec3(m_width * 0.6f, m_height * 0.25f, 0.3f));
    renderer->drawBox(tailModel, glm::vec3(0.85f, 0.25f, 0.1f));

    // Front wheel
    glm::mat4 fwModel = glm::translate(bikeModel, glm::vec3(0.0f, m_wheelRadius, m_wheelbase * 0.5f));
    fwModel = glm::rotate(fwModel, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    fwModel = glm::scale(fwModel, glm::vec3(m_wheelRadius, m_wheelRadius, m_wheelWidth));
    renderer->drawCylinder(fwModel, glm::vec3(0.1f, 0.1f, 0.1f), 1.0f, 1.0f, 12);

    // Rear wheel
    glm::mat4 rwModel = glm::translate(bikeModel, glm::vec3(0.0f, m_wheelRadius, -m_wheelbase * 0.5f));
    rwModel = glm::rotate(rwModel, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    rwModel = glm::scale(rwModel, glm::vec3(m_wheelRadius, m_wheelRadius, m_wheelWidth));
    renderer->drawCylinder(rwModel, glm::vec3(0.1f, 0.1f, 0.1f), 1.0f, 1.0f, 12);

    // Handlebar
    glm::mat4 barModel = glm::translate(bikeModel, glm::vec3(0.0f, 0.7f, 0.75f));
    barModel = glm::scale(barModel, glm::vec3(0.5f, 0.05f, 0.05f));
    renderer->drawBox(barModel, glm::vec3(0.3f, 0.3f, 0.3f));
}
