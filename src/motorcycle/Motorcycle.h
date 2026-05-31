#ifndef MOTORCYCLE_H
#define MOTORCYCLE_H

#include <glm/glm.hpp>
#include "EngineSim.h"
#include "Transmission.h"
#include "Chassis.h"
#include "Tire.h"
#include "Rider.h"

class Renderer;
class Camera;

struct MotorcycleConfig {
    float mass = 200.0f;
    float wheelbase = 1.4f;
    float cogHeight = 0.6f;
    float maxSteerAngle = 0.5f;
    float steeringSpeed = 2.5f;
    float maxSpeed = 65.0f;
    int numGears = 5;
    float gearRatios[5] = { 3.2f, 2.1f, 1.5f, 1.1f, 0.85f };
    EngineConfig engineCfg;
};

struct CrashInfo {
    bool active = false;
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 position = glm::vec3(0.0f);
    float impactForce = 0.0f;
};

class Motorcycle {
public:
    Motorcycle();
    ~Motorcycle();

    void configure(const MotorcycleConfig& cfg);
    void reset();

    void update(float dt, float throttleInput, float brakeInput, float frontBrakeInput,
                float steerInput, float clutchInput, bool gearUp, bool gearDown,
                bool startEngine);

    void render(Renderer* renderer, Camera* camera);

    const glm::vec3& position() const { return m_position; }
    const glm::vec3& rotation() const { return m_rotation; }

    float speed() const { return m_speed; }
    float rpm() const { return m_engine ? m_engine->rpm() : 0.0f; }
    int gear() const { return m_transmission ? m_transmission->gear() : 0; }
    bool engineRunning() const { return m_engine ? m_engine->isRunning() : false; }
    bool isStalled() const { return m_engine ? m_engine->isStalled() : false; }
    float leanAngle() const { return m_chassis ? m_chassis->leanAngle() : 0.0f; }
    float clutchEngagement() const { return m_transmission ? (1.0f - m_transmission->clutch()) : 0.0f; }

    const EngineSim* engine() const { return m_engine; }
    const Transmission* transmission() const { return m_transmission; }
    const Chassis* chassis() const { return m_chassis; }
    const Rider* rider() const { return m_rider; }
    const CrashInfo& crashInfo() const { return m_crashInfo; }

    float surfaceGrip() const { return m_currentGrip; }
    bool isCrashed() const { return m_crashed; }

    void setSurfaceGrip(float grip) { m_currentGrip = grip; }
    void setSurfaceHeight(float h) { m_surfaceHeight = h; }

private:
    void updatePhysics(float dt, float throttle, float brake, float frontBrake,
                       float steer, float clutch, bool gearUp, bool gearDown, bool startEngine);
    void updateWheelSpeed(float dt, float driveTorque, float brakeTorque);
    void checkCrash(float dt);
    void renderBike(Renderer* renderer, Camera* camera);

    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_velocity;

    float m_speed;
    float m_lateralSpeed;
    float m_wheelSpeed;
    float m_steerAngle;
    float m_targetSteer;

    float m_currentGrip;
    float m_surfaceHeight;

    bool m_crashed;
    CrashInfo m_crashInfo;
    float m_crashTimer;
    float m_resetTimer;

    // Subsystems
    EngineSim* m_engine;
    Transmission* m_transmission;
    Chassis* m_chassis;
    Tire* m_frontTire;
    Tire* m_rearTire;
    Rider* m_rider;

    MotorcycleConfig m_config;

    // Visual
    float m_length;
    float m_width;
    float m_height;
    float m_wheelRadius;
    float m_wheelWidth;
    float m_wheelbase;
};

#endif
