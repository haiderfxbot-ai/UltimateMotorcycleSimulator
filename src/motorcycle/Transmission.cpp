#include "Transmission.h"
#include <algorithm>
#include <cmath>

Transmission::Transmission()
    : m_gear(0)
    , m_numGears(0)
    , m_ratios(nullptr)
    , m_clutch(1.0f)
    , m_clutchSpeed(3.0f)
    , m_shiftTimer(0.0f)
    , m_shiftDelay(0.15f)
{}

Transmission::~Transmission() {
    delete[] m_ratios;
}

void Transmission::configure(int numGears, const float* ratios) {
    m_numGears = std::min(numGears, MAX_GEARS);
    delete[] m_ratios;
    m_ratios = new GearRatio[MAX_GEARS]();
    for (int i = 1; i <= m_numGears; ++i) {
        m_ratios[i].ratio = ratios[i - 1];
        m_ratios[i].maxSpeed = 200.0f / ratios[i - 1];
    }
    m_ratios[0].ratio = 0.0f;
    m_ratios[0].maxSpeed = 10.0f;
}

void Transmission::reset() {
    m_gear = 0;
    m_clutch = 1.0f;
    m_shiftTimer = 0.0f;
}

void Transmission::shiftUp() {
    if (m_shiftTimer > 0.0f) return;
    if (m_gear < m_numGears) {
        m_gear++;
        m_shiftTimer = m_shiftDelay;
    }
}

void Transmission::shiftDown() {
    if (m_shiftTimer > 0.0f) return;
    if (m_gear > 0) {
        m_gear--;
        m_shiftTimer = m_shiftDelay;
    }
}

void Transmission::shiftToNeutral() {
    if (m_shiftTimer > 0.0f) return;
    m_gear = 0;
    m_shiftTimer = m_shiftDelay;
}

void Transmission::setClutch(float clutchValue) {
    m_clutch = std::max(0.0f, std::min(1.0f, clutchValue));
}

float Transmission::ratio() const {
    if (m_gear < 0 || m_gear > m_numGears) return 0.0f;
    return m_ratios[m_gear].ratio;
}

float Transmission::engagedRatio() const {
    float r = ratio();
    float effectiveClutch = 1.0f - m_clutch;
    return r * effectiveClutch;
}

float Transmission::wheelRPMToEngineRPM(float wheelRPM) const {
    float r = ratio();
    if (r < 0.01f) return 0.0f;
    float engaged = engagedRatio();
    if (engaged < 0.01f) return 0.0f;
    return wheelRPM * engaged;
}

float Transmission::engineRPMToWheelRPM(float engineRPM) const {
    float r = ratio();
    if (r < 0.01f) return 0.0f;
    float engaged = engagedRatio();
    if (engaged < 0.01f) return 0.0f;
    return engineRPM / engaged;
}

float Transmission::calculateClutchTorque(float engineRPM, float wheelRPM, float dt) const {
    if (m_gear == 0) return 0.0f;

    float clutchEngagement = 1.0f - m_clutch;
    float rpmDiff = engineRPM - (wheelRPM * ratio());

    float maxClutchTorque = 200.0f;
    float slipTorque = rpmDiff * 0.5f;

    float torque = slipTorque * clutchEngagement;
    torque = std::max(-maxClutchTorque, std::min(torque, maxClutchTorque));

    if (clutchEngagement > 0.95f && std::abs(rpmDiff) < 50.0f) {
        torque = rpmDiff * 2.0f;
    }

    return torque * dt;
}

void Transmission::update(float dt) {
    if (m_shiftTimer > 0.0f) {
        m_shiftTimer -= dt;
    }
}
