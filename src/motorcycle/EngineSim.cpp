#include "EngineSim.h"
#include <algorithm>
#include <cmath>

EngineSim::EngineSim() : m_crankRPM(300.0f) {
    reset();
}

EngineSim::~EngineSim() {}

void EngineSim::configure(const EngineConfig& cfg) {
    m_config = cfg;
}

void EngineSim::reset() {
    m_state.running = false;
    m_state.stalled = false;
    m_state.rpm = 0.0f;
    m_state.torque = 0.0f;
    m_state.power = 0.0f;
    m_state.fuel = m_config.fuelCapacity;
    m_state.temperature = 80.0f;
    m_state.throttlePos = 0.0f;
    m_state.load = 0.0f;
    m_state.starterTimer = 0.0f;
}

void EngineSim::start() {
    if (m_state.fuel <= 0.0f) return;
    m_state.running = true;
    m_state.stalled = false;
    m_state.rpm = m_config.idleRPM;
    m_state.starterTimer = 0.0f;
}

void EngineSim::stop() {
    m_state.running = false;
    m_state.rpm = 0.0f;
    m_state.torque = 0.0f;
}

void EngineSim::setStalled(bool s) {
    m_state.stalled = s;
    if (s) {
        m_state.running = false;
        m_state.rpm = 0.0f;
    }
}

void EngineSim::update(float dt, float throttleInput, float engineLoad, bool clutchEngaged, float wheelSpeed) {
    if (!m_state.running || m_state.fuel <= 0.0f) {
        if (m_state.rpm > 0.0f) m_state.rpm -= 200.0f * dt;
        if (m_state.rpm < 0.0f) m_state.rpm = 0.0f;
        m_state.torque = 0.0f;
        return;
    }

    m_state.throttlePos = throttleInput;

    float loadFactor = clutchEngaged ? engineLoad : 0.0f;

    calculateCombustion(dt, throttleInput, loadFactor);
    calculateCooling(dt);
    checkStall(engineLoad, clutchEngaged, wheelSpeed);

    if (m_state.running) {
        m_state.fuel -= m_config.fuelConsumptionRate * (m_state.rpm / m_config.maxRPM) * dt;
        if (m_state.fuel < 0.0f) m_state.fuel = 0.0f;
    }
}

void EngineSim::calculateCombustion(float dt, float throttle, float load) {
    float targetRPM = m_config.idleRPM;

    if (throttle > 0.01f) {
        float torqueCurve = 1.0f - std::abs((m_state.rpm - 5500.0f) / 4500.0f);
        torqueCurve = std::max(0.0f, torqueCurve);
        m_state.torque = m_config.maxTorque * torqueCurve * throttle;
        m_state.power = m_state.torque * (m_state.rpm / 5250.0f);

        float rpmGain = (m_state.torque - load * 0.3f) / m_config.inertia * dt * 500.0f;
        targetRPM = m_state.rpm + rpmGain;
    } else {
        m_state.torque = 0.0f;
        m_state.power = 0.0f;
        targetRPM = m_state.rpm - m_config.engineBraking * load * dt;
    }

    m_state.rpm = std::max(m_config.idleRPM, std::min(targetRPM, m_config.maxRPM));

    if (m_state.rpm > m_config.redlineRPM && throttle > 0.01f) {
        m_state.rpm -= (m_state.rpm - m_config.redlineRPM) * 3.0f * dt;
    }

    m_state.load = load;
}

void EngineSim::calculateCooling(float dt) {
    float targetTemp = 80.0f;
    float heatGen = (m_state.rpm / m_config.maxRPM) * 20.0f;
    m_state.temperature += (heatGen - (m_state.temperature - targetTemp) * 0.1f) * dt;
    m_state.temperature = std::max(30.0f, std::min(m_state.temperature, 150.0f));
}

void EngineSim::checkStall(float engineLoad, bool clutchEngaged, float wheelSpeed) {
    if (!clutchEngaged) return;
    if (m_state.rpm < m_config.stallRPM) {
        setStalled(true);
    }
    float speedRatio = wheelSpeed * 10.0f / (m_state.rpm + 0.1f);
    if (speedRatio > 2.0f && clutchEngaged) {
        setStalled(true);
    }
}
