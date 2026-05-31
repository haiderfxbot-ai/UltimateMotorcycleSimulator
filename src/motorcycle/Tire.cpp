#include "Tire.h"
#include <algorithm>
#include <cmath>

Tire::Tire() {}
Tire::~Tire() {}

void Tire::configure(const TireConfig& cfg) {
    m_config = cfg;
}

void Tire::reset() {
    m_state = TireState();
}

void Tire::update(float dt, float wheelSpeed, float vehicleSpeed,
                  float lateralVelocity, float normalForce,
                  float driveTorque, float brakeTorque,
                  float surfaceGrip, float steerAngle) {
    (void)dt;
    m_state.normalForce = std::max(1.0f, normalForce);

    calculateSlip(wheelSpeed, vehicleSpeed, lateralVelocity, steerAngle);
    calculateForces(surfaceGrip, driveTorque, brakeTorque);
}

void Tire::calculateSlip(float wheelSpeed, float vehicleSpeed, float lateralVelocity, float steerAngle) {
    float vx = vehicleSpeed;
    if (std::abs(vx) < 0.5f) vx = 0.5f;

    float longSlip = (wheelSpeed - vx) / vx;
    m_state.slipRatio = std::max(-1.0f, std::min(1.0f, longSlip));

    float vy = lateralVelocity - steerAngle * vx;
    float slipAng = std::atan2(vy, vx);
    m_state.slipAngle = std::max(-1.0f, std::min(1.0f, slipAng));
}

void Tire::calculateForces(float surfaceGrip, float driveTorque, float brakeTorque) {
    float grip = m_config.gripCoefficient * surfaceGrip;
    m_state.grip = grip;

    m_state.longitudinalForce = m_state.slipRatio * m_config.longitudinalStiffness * grip;
    m_state.lateralForce = m_state.slipAngle * m_config.lateralStiffness * grip;

    float maxFriction = grip * m_state.normalForce;
    float totalForce = std::sqrt(m_state.longitudinalForce * m_state.longitudinalForce +
                                  m_state.lateralForce * m_state.lateralForce);

    if (totalForce > maxFriction) {
        float scale = maxFriction / (totalForce + 0.01f);
        m_state.longitudinalForce *= scale;
        m_state.lateralForce *= scale;
        m_state.isSlipping = true;
    } else {
        m_state.isSlipping = false;
    }

    m_state.isLocked = (brakeTorque > m_config.maxBrakeTorque * 0.9f && std::abs(m_state.slipRatio) > 0.3f);
}
