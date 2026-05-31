#include "DayNightCycle.h"
#include <algorithm>
#include <cmath>

DayNightCycle::DayNightCycle()
    : m_timeOfDay(12.0f)
    , m_dayLength(600.0f)
{
    m_lighting.sunDirection = glm::vec3(0.5f, 0.8f, 0.3f);
    m_lighting.ambientColor = glm::vec3(0.6f, 0.7f, 1.0f);
    m_lighting.sunColor = glm::vec3(1.0f, 0.95f, 0.85f);
    m_lighting.ambientIntensity = 0.5f;
    m_lighting.sunIntensity = 1.0f;
    m_lighting.fogDensity = 0.0f;
    m_lighting.fogColor = glm::vec3(0.53f, 0.81f, 0.92f);
    recalculateLighting();
}

DayNightCycle::~DayNightCycle() {}

void DayNightCycle::update(float dt, float timeScale) {
    float daySeconds = m_dayLength * timeScale;
    m_timeOfDay += dt / daySeconds * 24.0f;
    if (m_timeOfDay >= 24.0f) m_timeOfDay -= 24.0f;

    recalculateLighting();
}

float DayNightCycle::sunAngle() const {
    float angle = (m_timeOfDay - 6.0f) / 12.0f * 3.14159f;
    return std::max(-1.5f, std::min(1.5f, angle));
}

void DayNightCycle::recalculateLighting() {
    float angle = sunAngle();
    float sunHeight = std::sin(angle);

    // Sun direction rotates around Y axis based on time
    float horizAngle = (m_timeOfDay / 24.0f) * 2.0f * 3.14159f;
    m_lighting.sunDirection = glm::vec3(
        std::sin(horizAngle) * std::cos(angle),
        sunHeight,
        std::cos(horizAngle) * std::cos(angle)
    );
    if (glm::length(m_lighting.sunDirection) > 0.0f) {
        m_lighting.sunDirection = glm::normalize(m_lighting.sunDirection);
    }

    // Ambient intensity based on sun height
    float dayFactor = std::max(0.0f, std::min(1.0f, sunHeight * 1.5f + 0.2f));
    m_lighting.ambientIntensity = 0.08f + dayFactor * 0.52f;
    m_lighting.sunIntensity = std::max(0.0f, dayFactor * 1.2f);

    // Colors
    if (isNight()) {
        m_lighting.ambientColor = glm::vec3(0.05f, 0.05f, 0.15f);
        m_lighting.sunColor = glm::vec3(0.3f, 0.3f, 0.5f);
        m_lighting.fogColor = glm::vec3(0.02f, 0.02f, 0.05f);
        m_lighting.fogDensity = 0.01f;
    } else if (isDawnOrDusk()) {
        float t = std::abs(m_timeOfDay - 6.5f);
        if (t > 1.0f) t = std::abs(m_timeOfDay - 18.5f);
        t = std::max(0.0f, 1.0f - t);

        m_lighting.ambientColor = glm::vec3(0.3f + t * 0.3f, 0.15f + t * 0.55f, 0.1f + t * 0.9f);
        m_lighting.sunColor = glm::vec3(1.0f, 0.6f - t * 0.35f, 0.2f + t * 0.65f);
        m_lighting.fogColor = glm::vec3(0.3f + t * 0.23f, 0.2f + t * 0.61f, 0.15f + t * 0.77f);
        m_lighting.fogDensity = 0.03f * (1.0f - t * 0.5f);
    } else {
        m_lighting.ambientColor = glm::vec3(0.5f, 0.65f, 0.95f);
        m_lighting.sunColor = glm::vec3(1.0f, 0.95f, 0.85f);
        m_lighting.fogColor = glm::vec3(0.53f, 0.81f, 0.92f);
        m_lighting.fogDensity = 0.002f;
    }
}
