#include "WeatherController.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

WeatherController::WeatherController()
    : m_transitionSpeed(0.1f)
{
    m_state.rainIntensity = 0.0f;
    m_state.targetRainIntensity = 0.0f;
    m_state.wetness = 0.0f;
    m_state.fogDensity = 0.0f;
    m_state.windStrength = 0.0f;
    m_state.isRaining = false;
    m_state.transitionTimer = 0.0f;
}

WeatherController::~WeatherController() {}

void WeatherController::update(float dt) {
    m_state.rainIntensity += (m_state.targetRainIntensity - m_state.rainIntensity) * m_transitionSpeed * dt;
    m_state.rainIntensity = std::max(0.0f, std::min(1.0f, m_state.rainIntensity));

    float targetWetness = m_state.rainIntensity * 0.85f;
    m_state.wetness += (targetWetness - m_state.wetness) * 0.5f * dt;
    m_state.wetness = std::max(0.0f, std::min(1.0f, m_state.wetness));

    float targetFog = m_state.rainIntensity * 0.12f + (m_state.rainIntensity > 0.5f ? 0.08f : 0.0f);
    m_state.fogDensity += (targetFog - m_state.fogDensity) * 0.3f * dt;
    m_state.fogDensity = std::max(0.0f, std::min(0.3f, m_state.fogDensity));

    float targetWind = m_state.rainIntensity * 8.0f;
    m_state.windStrength += (targetWind - m_state.windStrength) * 0.2f * dt;
    m_state.windStrength = std::max(0.0f, m_state.windStrength);

    m_state.isRaining = m_state.rainIntensity > 0.1f;

    // Random weather changes
    m_state.transitionTimer += dt;
    if (m_state.transitionTimer > 15.0f) {
        m_state.transitionTimer = 0.0f;
        if (std::rand() % 100 < 5) {
            if (m_state.targetRainIntensity < 0.1f) {
                setRain(0.3f + (float)(std::rand() % 70) / 100.0f);
            } else {
                clearWeather();
            }
        }
    }
}

void WeatherController::setRain(float intensity) {
    m_state.targetRainIntensity = std::max(0.0f, std::min(1.0f, intensity));
}

void WeatherController::clearWeather() {
    m_state.targetRainIntensity = 0.0f;
}
