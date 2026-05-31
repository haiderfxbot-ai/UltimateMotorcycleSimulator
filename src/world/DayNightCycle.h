#ifndef DAY_NIGHT_CYCLE_H
#define DAY_NIGHT_CYCLE_H

#include <glm/glm.hpp>

struct LightingState {
    glm::vec3 sunDirection;
    glm::vec3 ambientColor;
    glm::vec3 sunColor;
    float ambientIntensity;
    float sunIntensity;
    float fogDensity;
    glm::vec3 fogColor;
};

class DayNightCycle {
public:
    DayNightCycle();
    ~DayNightCycle();

    void update(float dt, float timeScale = 1.0f);
    void setTimeOfDay(float hours) { m_timeOfDay = hours; }
    float timeOfDay() const { return m_timeOfDay; }

    const LightingState& lighting() const { return m_lighting; }
    float sunAngle() const;
    bool isNight() const { return m_timeOfDay < 6.0f || m_timeOfDay > 19.0f; }
    bool isDawnOrDusk() const { return (m_timeOfDay >= 5.5f && m_timeOfDay <= 7.5f) ||
                                        (m_timeOfDay >= 17.5f && m_timeOfDay <= 19.5f); }

private:
    void recalculateLighting();

    float m_timeOfDay;
    float m_dayLength;

    LightingState m_lighting;
};

#endif
