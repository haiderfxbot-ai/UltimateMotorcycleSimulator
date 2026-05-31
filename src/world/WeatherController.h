#ifndef WEATHER_CONTROLLER_H
#define WEATHER_CONTROLLER_H

#include <glm/glm.hpp>

struct WeatherState {
    float rainIntensity = 0.0f;
    float targetRainIntensity = 0.0f;
    float wetness = 0.0f;
    float fogDensity = 0.0f;
    float windStrength = 0.0f;
    glm::vec3 windDirection = glm::vec3(1.0f, 0.0f, 0.5f);
    bool isRaining = false;
    float transitionTimer = 0.0f;
};

class WeatherController {
public:
    WeatherController();
    ~WeatherController();

    void update(float dt);
    void setRain(float intensity);
    void clearWeather();

    float rainIntensity() const { return m_state.rainIntensity; }
    float wetness() const { return m_state.wetness; }
    float fogDensity() const { return m_state.fogDensity; }
    float gripFactor() const { return 1.0f - m_state.wetness * 0.45f; }
    float visibilityFactor() const { return 1.0f - m_state.fogDensity * 0.8f; }
    bool isRaining() const { return m_state.rainIntensity > 0.1f; }
    const WeatherState& state() const { return m_state; }

private:
    WeatherState m_state;
    float m_transitionSpeed;
};

#endif
