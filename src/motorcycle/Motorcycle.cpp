#include "Motorcycle.h"
#include "../renderer/Renderer.h"
#include "../engine/Camera.h"
#include "../world/Surface.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>
#include <cmath>

Motorcycle::Motorcycle()
    : m_position(0.0f, 0.3f, 0.0f)
    , m_rotation(0.0f)
    , m_velocity(0.0f)
    , m_speed(0.0f)
    , m_lateralSpeed(0.0f)
    , m_wheelSpeed(0.0f)
    , m_steerAngle(0.0f)
    , m_targetSteer(0.0f)
    , m_currentGrip(1.0f)
    , m_surfaceHeight(0.0f)
    , m_crashed(false)
    , m_crashTimer(0.0f)
    , m_resetTimer(0.0f)
    , m_engine(nullptr)
    , m_transmission(nullptr)
    , m_chassis(nullptr)
    , m_frontTire(nullptr)
    , m_rearTire(nullptr)
    , m_rider(nullptr)
    , m_length(2.0f)
    , m_width(0.6f)
    , m_height(1.0f)
    , m_wheelRadius(0.3f)
    , m_wheelWidth(0.12f)
    , m_wheelbase(1.4f)
{
    m_engine = new EngineSim();
    m_transmission = new Transmission();
    m_chassis = new Chassis();
    m_frontTire = new Tire();
    m_rearTire = new Tire();
    m_rider = new Rider();

    MotorcycleConfig defaultCfg;
    configure(defaultCfg);
}

Motorcycle::~Motorcycle() {
    delete m_rider;
    delete m_rearTire;
    delete m_frontTire;
    delete m_chassis;
    delete m_transmission;
    delete m_engine;
}

void Motorcycle::configure(const MotorcycleConfig& cfg) {
    m_config = cfg;
    m_engine->configure(cfg.engineCfg);

    int numGears = cfg.numGears;
    float ratios[8];
    for (int i = 0; i < numGears; ++i) ratios[i] = cfg.gearRatios[i];
    m_transmission->configure(numGears, ratios);

    m_chassis->configure(cfg.mass, cfg.wheelbase, cfg.cogHeight);

    TireConfig tireCfg;
    tireCfg.radius = m_wheelRadius;
    tireCfg.width = m_wheelWidth;
    m_frontTire->configure(tireCfg);
    m_rearTire->configure(tireCfg);
}

void Motorcycle::reset() {
    m_position = glm::vec3(0.0f, 0.3f, 0.0f);
    m_rotation = glm::vec3(0.0f);
    m_velocity = glm::vec3(0.0f);
    m_speed = 0.0f;
    m_lateralSpeed = 0.0f;
    m_wheelSpeed = 0.0f;
    m_steerAngle = 0.0f;
    m_targetSteer = 0.0f;
    m_crashed = false;
    m_crashInfo = CrashInfo();
    m_crashTimer = 0.0f;
    m_resetTimer = 0.0f;

    if (m_engine) m_engine->reset();
    if (m_transmission) m_transmission->reset();
    if (m_chassis) m_chassis->reset();
    if (m_frontTire) m_frontTire->reset();
    if (m_rearTire) m_rearTire->reset();
    if (m_rider) m_rider->reset();
}

void Motorcycle::triggerExternalCrash(float impactForce, const glm::vec3& impactVelocity) {
    if (m_crashed) return;
    m_crashed = true;
    m_crashInfo.active = true;
    m_crashInfo.velocity = impactVelocity;
    m_crashInfo.position = m_position;
    m_crashInfo.impactForce = impactForce;
    m_velocity = impactVelocity;
    m_speed = glm::length(impactVelocity);
}

void Motorcycle::update(float dt, float throttleInput, float brakeInput, float frontBrakeInput,
                        float steerInput, float clutchInput, bool gearUp, bool gearDown,
                        bool startEngine) {
    if (m_crashed) {
        m_crashTimer += dt;
        if (m_rider) m_rider->update(dt, 0.0f, 0.0f, 0.0f, true, glm::vec3(0.0f), glm::vec3(0.0f));
        m_speed *= 0.97f;
        m_velocity = glm::vec3(0.0f, 0.0f, -m_speed);
        m_velocity = glm::rotateY(m_velocity, m_rotation.y);
        m_position += m_velocity * dt;

        if (m_crashTimer > 3.0f) {
            m_resetTimer += dt;
            if (m_resetTimer > 1.0f) reset();
        }
        return;
    }

    SurfaceType surface = Surface::getSurfaceAt(m_position.x, m_position.z);
    SurfaceProperties props = Surface::getProperties(surface);
    m_currentGrip = props.grip;

    float height = Surface::getHeightAt(m_position.x, m_position.z);
    m_surfaceHeight = height;
    m_position.y = 0.3f + height;

    if (Surface::hasPothole(m_position.x, m_position.z)) {
        m_chassis->applyBump(-0.08f, -0.05f);
    }

    updatePhysics(dt, throttleInput, brakeInput, frontBrakeInput,
                  steerInput, clutchInput, gearUp, gearDown, startEngine);

    if (m_rider) {
        glm::vec3 crashVel = m_crashed ? glm::vec3(0.0f, -2.0f, m_speed) : glm::vec3(0.0f);
        m_rider->update(dt, m_speed, m_chassis->leanAngle(), m_chassis->pitchAngle(),
                        m_crashed, crashVel, m_position);
    }
}

void Motorcycle::updatePhysics(float dt, float throttle, float brake, float frontBrake,
                               float steer, float clutch, bool gearUp, bool gearDown, bool startEngine) {
    m_transmission->update(dt);

    if (gearUp && !m_crashed) m_transmission->shiftUp();
    if (gearDown && !m_crashed) m_transmission->shiftDown();
    m_transmission->setClutch(clutch);

    bool clutchEngaged = clutch < 0.1f;

    if (startEngine && !m_engine->isRunning() && !m_crashed) {
        m_engine->start();
    }

    float engineLoad = m_speed * m_transmission->ratio() * 0.1f;
    float wheelRPM = m_wheelSpeed * 30.0f / (3.14159f * m_wheelRadius);
    m_engine->update(dt, throttle, engineLoad, clutchEngaged, wheelRPM);

    if (m_engine->isStalled() && !m_crashed) {
        m_crashed = true;
        m_crashInfo.active = true;
        m_crashInfo.velocity = m_velocity;
        m_crashInfo.position = m_position;
    }

    float engineTorque = 0.0f;
    if (m_engine->isRunning() && !m_engine->isStalled()) {
        engineTorque = m_engine->torque();
    }

    float clutchTorque = m_transmission->calculateClutchTorque(
        m_engine->rpm(), wheelRPM, dt);

    float driveTorque = (engineTorque + clutchTorque) * throttle;

    float brakeTorque = brake * 2000.0f + frontBrake * 3000.0f;

    float wheelDriveForce = 0.0f;
    float wheelBrakeForce = 0.0f;

    if (m_transmission->gear() > 0 && !m_engine->isStalled()) {
        wheelDriveForce = driveTorque / m_wheelRadius * m_transmission->ratio();
    }

    if (brake > 0.01f) wheelBrakeForce += brake * m_config.mass * 8.0f;
    if (frontBrake > 0.01f) {
        wheelBrakeForce += frontBrake * m_config.mass * 12.0f;
        if (frontBrake > 0.8f && m_speed > 10.0f) {
            m_rotation.x -= frontBrake * dt * 0.4f;
        }
    }

    float drag = m_speed * m_speed * 0.025f * (m_speed > 0 ? 1.0f : -1.0f);
    if (std::abs(m_speed) < 0.5f) drag = 0.0f;

    float rollingResistance = -m_speed * m_currentGrip * 0.3f;

    float netForce = wheelDriveForce - wheelBrakeForce + drag + rollingResistance;
    m_speed += netForce / m_config.mass * dt;

    if (m_crashed) m_speed *= 0.95f;
    if (std::abs(m_speed) < 0.01f) m_speed = 0.0f;
    m_speed = std::max(-10.0f, std::min(m_speed, m_config.maxSpeed));

    m_wheelSpeed = m_speed;
    updateWheelSpeed(dt, driveTorque, brakeTorque * m_wheelRadius);

    float inputSteer = steer;
    float speedRatio = std::min(1.0f, std::abs(m_speed) / 15.0f);
    m_targetSteer = inputSteer * m_config.maxSteerAngle * (0.3f + speedRatio * 0.7f);
    m_steerAngle += (m_targetSteer - m_steerAngle) * std::min(1.0f, 6.0f * dt);

    m_chassis->update(dt, m_speed, m_steerAngle, 0.0f, brake * 5000.0f,
                      wheelDriveForce * 0.5f, m_currentGrip);

    if (m_chassis->isTipped() && !m_crashed) {
        m_crashed = true;
        m_crashInfo.active = true;
        m_crashInfo.velocity = m_velocity;
        m_crashInfo.position = m_position;
    }

    float effectiveSteer = m_steerAngle * speedRatio;
    if (std::abs(m_speed) > 0.5f) {
        m_rotation.y += effectiveSteer * dt * 2.0f;
    } else if (std::abs(m_speed) > 0.01f) {
        m_rotation.y += steer * 1.5f * dt;
    }

    m_lateralSpeed = -m_speed * std::sin(m_steerAngle) * 0.3f * m_currentGrip;
    m_lateralSpeed *= std::max(0.0f, 1.0f - 3.0f * dt);

    m_velocity = glm::vec3(m_lateralSpeed, 0.0f, -m_speed);
    m_velocity = glm::rotateY(m_velocity, m_rotation.y);
    m_position += m_velocity * dt;

    if (m_engine->isRunning() && !m_engine->isStalled()) {
        m_rotation.x = m_chassis->leanAngle() + m_chassis->pitchAngle() * 0.3f;
    }

    checkCrash(dt);
}

void Motorcycle::updateWheelSpeed(float dt, float driveTorque, float brakeTorque) {
    float inertia = 0.5f;
    float angularAccel = (driveTorque - brakeTorque) / inertia;
    m_wheelSpeed += angularAccel * dt;

    float speedTraction = m_speed * m_currentGrip;
    m_wheelSpeed += (speedTraction - m_wheelSpeed) * 0.1f;

    if (m_transmission->gear() == 0) {
        m_wheelSpeed *= 0.95f;
    }
}

void Motorcycle::checkCrash(float dt) {
    if (m_crashed) return;

    if (m_engine->isStalled() && m_speed > 5.0f) {
        m_crashed = true;
        m_crashInfo.active = true;
        m_crashInfo.impactForce = m_speed * 0.5f;
        m_crashInfo.velocity = m_velocity;
        m_crashInfo.position = m_position;
        return;
    }

    if (std::abs(m_rotation.x) > 1.5f) {
        m_crashed = true;
        m_crashInfo.impactForce = std::abs(m_speed) * 0.3f;
        m_crashInfo.velocity = m_velocity;
        m_crashInfo.position = m_position;
        return;
    }

    if (std::abs(m_lateralSpeed) > 5.0f && std::abs(m_speed) > 15.0f) {
        m_crashed = true;
        m_crashInfo.impactForce = std::abs(m_speed) * 0.4f;
        m_crashInfo.velocity = glm::vec3(m_lateralSpeed, 0.0f, m_speed);
        m_crashInfo.position = m_position;
        return;
    }
}

void Motorcycle::render(Renderer* renderer, Camera* camera) {
    if (!renderer || !camera) return;

    glm::mat4 pv = camera->projectionMatrix() * camera->viewMatrix();
    renderer->setProjectionView(pv);

    if (m_rider) {
        m_rider->render(renderer, camera, m_position, m_rotation.y);
    }

    if (!m_crashed || m_crashTimer < 0.5f) {
        renderBike(renderer, camera);
    }
}

void Motorcycle::renderBike(Renderer* renderer, Camera* camera) {
    (void)camera;

    glm::mat4 bikeModel = glm::mat4(1.0f);
    bikeModel = glm::translate(bikeModel, m_position);
    bikeModel = glm::rotate(bikeModel, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    bikeModel = glm::rotate(bikeModel, m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));

    float compFront = m_chassis->frontSuspension().travel;
    float compRear = m_chassis->rearSuspension().travel;

    glm::mat4 bodyModel = glm::translate(bikeModel, glm::vec3(0.0f, 0.4f + compFront * 0.5f + compRear * 0.5f, 0.0f));
    bodyModel = glm::scale(bodyModel, glm::vec3(m_width, m_height * 0.5f, m_length * 0.5f));
    renderer->drawBox(bodyModel, glm::vec3(0.9f, 0.3f, 0.15f));

    glm::mat4 seatModel = glm::translate(bikeModel, glm::vec3(0.0f, 0.6f + compRear * 1.5f, -0.1f));
    seatModel = glm::scale(seatModel, glm::vec3(m_width * 0.8f, 0.1f, 0.3f));
    renderer->drawBox(seatModel, glm::vec3(0.15f, 0.15f, 0.15f));

    glm::mat4 headModel = glm::translate(bikeModel, glm::vec3(0.0f, 0.5f + compFront * 1.5f, 0.8f));
    headModel = glm::scale(headModel, glm::vec3(m_width * 0.7f, m_height * 0.3f, 0.15f));
    renderer->drawBox(headModel, glm::vec3(0.95f, 0.4f, 0.2f));

    glm::mat4 tailModel = glm::translate(bikeModel, glm::vec3(0.0f, 0.35f + compRear * 1.5f, -0.8f));
    tailModel = glm::scale(tailModel, glm::vec3(m_width * 0.6f, m_height * 0.25f, 0.3f));
    renderer->drawBox(tailModel, glm::vec3(0.85f, 0.25f, 0.1f));

    glm::mat4 fwModel = glm::translate(bikeModel, glm::vec3(0.0f, m_wheelRadius + compFront, m_wheelbase * 0.5f));
    fwModel = glm::rotate(fwModel, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    fwModel = glm::scale(fwModel, glm::vec3(m_wheelRadius, m_wheelRadius, m_wheelWidth));
    renderer->drawCylinder(fwModel, glm::vec3(0.1f, 0.1f, 0.1f), 1.0f, 1.0f, 12);

    glm::mat4 rwModel = glm::translate(bikeModel, glm::vec3(0.0f, m_wheelRadius + compRear, -m_wheelbase * 0.5f));
    rwModel = glm::rotate(rwModel, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    rwModel = glm::scale(rwModel, glm::vec3(m_wheelRadius, m_wheelRadius, m_wheelWidth));
    renderer->drawCylinder(rwModel, glm::vec3(0.1f, 0.1f, 0.1f), 1.0f, 1.0f, 12);

    glm::mat4 barModel = glm::translate(bikeModel, glm::vec3(0.0f, 0.7f + compFront * 1.5f, 0.75f));
    barModel = glm::scale(barModel, glm::vec3(0.5f, 0.05f, 0.05f));
    renderer->drawBox(barModel, glm::vec3(0.3f, 0.3f, 0.3f));
}
