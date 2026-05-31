#ifndef CHASSIS_H
#define CHASSIS_H

#include <glm/glm.hpp>

struct SuspensionState {
    float travel = 0.0f;
    float velocity = 0.0f;
    float force = 0.0f;
};

class Chassis {
public:
    Chassis();
    ~Chassis();

    void configure(float mass, float wheelbase, float cogHeight);
    void reset();

    void update(float dt, float speed, float steerInput, float leanInput,
                float brakingForce, float throttleForce, float surfaceGrip);

    float leanAngle() const { return m_leanAngle; }
    float pitchAngle() const { return m_pitchAngle; }
    float leanVelocity() const { return m_leanVelocity; }
    bool isTipped() const { return m_tipped; }

    const SuspensionState& frontSuspension() const { return m_frontSusp; }
    const SuspensionState& rearSuspension() const { return m_rearSusp; }

    void applyBump(float frontHeight, float rearHeight);
    void setTipped(bool t) { m_tipped = t; }

private:
    void updateBalance(float dt, float speed, float steerInput, float leanInput, float surfaceGrip);
    void updateSuspension(float dt, float brakingForce, float throttleForce);
    void updatePitch(float dt, float brakingForce, float throttleForce);

    float m_mass;
    float m_wheelbase;
    float m_cogHeight;

    float m_leanAngle;
    float m_leanVelocity;
    float m_targetLean;
    float m_pitchAngle;

    float m_balanceThreshold;
    float m_lowSpeedStability;
    bool m_tipped;

    SuspensionState m_frontSusp;
    SuspensionState m_rearSusp;

    float m_frontSpring;
    float m_frontDamp;
    float m_rearSpring;
    float m_rearDamp;
    float m_maxTravel;
};

#endif
