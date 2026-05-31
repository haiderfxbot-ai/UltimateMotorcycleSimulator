#ifndef TIRE_H
#define TIRE_H

#include <glm/glm.hpp>

struct TireConfig {
    float gripCoefficient = 1.0f;
    float slipThreshold = 0.15f;
    float lateralStiffness = 30000.0f;
    float longitudinalStiffness = 40000.0f;
    float rollingResistance = 0.015f;
    float maxBrakeTorque = 2500.0f;
    float radius = 0.3f;
    float width = 0.12f;
};

struct TireState {
    float slipRatio = 0.0f;
    float slipAngle = 0.0f;
    float grip = 1.0f;
    float lateralForce = 0.0f;
    float longitudinalForce = 0.0f;
    float normalForce = 0.0f;
    bool isSlipping = false;
    bool isLocked = false;
    float spinSpeed = 0.0f;
};

class Tire {
public:
    Tire();
    ~Tire();

    void configure(const TireConfig& cfg);
    void reset();
    void update(float dt, float wheelSpeed, float vehicleSpeed,
                float lateralVelocity, float normalForce,
                float driveTorque, float brakeTorque,
                float surfaceGrip, float steerAngle);

    const TireState& state() const { return m_state; }
    float grip() const { return m_state.grip; }
    bool isSlipping() const { return m_state.isSlipping; }

private:
    void calculateSlip(float wheelSpeed, float vehicleSpeed, float lateralVelocity, float steerAngle);
    void calculateForces(float surfaceGrip, float driveTorque, float brakeTorque);

    TireConfig m_config;
    TireState m_state;
};

#endif
