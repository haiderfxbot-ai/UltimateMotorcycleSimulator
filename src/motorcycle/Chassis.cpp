#include "Chassis.h"
#include <algorithm>
#include <cmath>

Chassis::Chassis()
    : m_mass(200.0f)
    , m_wheelbase(1.4f)
    , m_cogHeight(0.6f)
    , m_leanAngle(0.0f)
    , m_leanVelocity(0.0f)
    , m_targetLean(0.0f)
    , m_pitchAngle(0.0f)
    , m_balanceThreshold(0.5f)
    , m_lowSpeedStability(2.0f)
    , m_tipped(false)
    , m_frontSpring(18000.0f)
    , m_frontDamp(1200.0f)
    , m_rearSpring(16000.0f)
    , m_rearDamp(1000.0f)
    , m_maxTravel(0.12f)
{}

Chassis::~Chassis() {}

void Chassis::configure(float mass, float wheelbase, float cogHeight) {
    m_mass = mass;
    m_wheelbase = wheelbase;
    m_cogHeight = cogHeight;
}

void Chassis::reset() {
    m_leanAngle = 0.0f;
    m_leanVelocity = 0.0f;
    m_targetLean = 0.0f;
    m_pitchAngle = 0.0f;
    m_tipped = false;
    m_frontSusp = SuspensionState();
    m_rearSusp = SuspensionState();
}

void Chassis::update(float dt, float speed, float steerInput, float leanInput,
                     float brakingForce, float throttleForce, float surfaceGrip) {
    if (m_tipped) return;

    updateBalance(dt, speed, steerInput, leanInput, surfaceGrip);
    updateSuspension(dt, brakingForce, throttleForce);
    updatePitch(dt, brakingForce, throttleForce);
}

void Chassis::updateBalance(float dt, float speed, float steerInput, float leanInput, float surfaceGrip) {
    float absSpeed = std::abs(speed);
    float speedFactor = std::min(1.0f, absSpeed / 20.0f);

    float gravity = 9.81f;
    float centripetalForce = steerInput * absSpeed * absSpeed / (5.0f + absSpeed * 0.3f);

    m_targetLean = -centripetalForce / (m_mass * gravity);
    m_targetLean = std::max(-1.2f, std::min(1.2f, m_targetLean));

    if (std::abs(steerInput) < 0.1f) {
        m_targetLean *= 0.9f;
    }

    float stability = 2.0f;
    if (absSpeed < 3.0f) {
        stability = m_lowSpeedStability * (1.0f - absSpeed / 3.0f);
        float riderCompensation = leanInput * 0.5f;
        m_targetLean += riderCompensation;
    } else {
        stability = 5.0f + absSpeed * 0.2f;
    }

    stability *= surfaceGrip;

    float leanAccel = (m_targetLean - m_leanAngle) * stability;
    m_leanVelocity += leanAccel * dt;
    m_leanVelocity *= std::max(0.0f, 1.0f - 2.0f * dt);
    m_leanAngle += m_leanVelocity * dt;

    float maxLean = 1.2f;
    m_leanAngle = std::max(-maxLean, std::min(maxLean, m_leanAngle));

    float tipThreshold = m_balanceThreshold + speedFactor * 0.3f;
    if (std::abs(m_leanAngle) > tipThreshold) {
        m_tipped = true;
    }
}

void Chassis::updateSuspension(float dt, float brakingForce, float throttleForce) {
    float frontForce = m_mass * 9.81f * 0.5f - brakingForce * 0.7f;
    float rearForce = m_mass * 9.81f * 0.5f + brakingForce * 0.3f - throttleForce * 0.3f;

    auto& f = m_frontSusp;
    f.force = frontForce - m_frontSpring * f.travel - m_frontDamp * f.velocity;
    f.acceleration = f.force / (m_mass * 0.5f);
    f.velocity += f.acceleration * dt;
    f.velocity *= std::max(0.0f, 1.0f - 4.0f * dt);
    f.travel += f.velocity * dt;
    f.travel = std::max(-m_maxTravel, std::min(m_maxTravel, f.travel));

    auto& r = m_rearSusp;
    r.force = rearForce - m_rearSpring * r.travel - m_rearDamp * r.velocity;
    r.acceleration = r.force / (m_mass * 0.5f);
    r.velocity += r.acceleration * dt;
    r.velocity *= std::max(0.0f, 1.0f - 4.0f * dt);
    r.travel += r.velocity * dt;
    r.travel = std::max(-m_maxTravel, std::min(m_maxTravel, r.travel));
}

void Chassis::updatePitch(float dt, float brakingForce, float throttleForce) {
    float targetPitch = (brakingForce * 0.02f) - (throttleForce * 0.01f);
    targetPitch = std::max(-0.4f, std::min(0.4f, targetPitch));
    m_pitchAngle += (targetPitch - m_pitchAngle) * 3.0f * dt;
}

void Chassis::applyBump(float frontHeight, float rearHeight) {
    m_frontSusp.velocity += frontHeight * 0.5f;
    m_rearSusp.velocity += rearHeight * 0.5f;
}
