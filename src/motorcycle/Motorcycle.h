#ifndef MOTORCYCLE_H
#define MOTORCYCLE_H

#include <glm/glm.hpp>

class Renderer;
class Camera;

class Motorcycle {
public:
    Motorcycle();
    ~Motorcycle();

    void reset();
    void update(float dt, float throttleInput, float brakeInput, float steerInput);

    void render(Renderer* renderer, Camera* camera);

    const glm::vec3& position() const { return m_position; }
    const glm::vec3& rotation() const { return m_rotation; }

    float speed() const { return m_speed; }
    float rpm() const { return m_rpm; }
    int gear() const { return m_gear; }
    bool engineRunning() const { return m_engineRunning; }

private:
    void updatePhysics(float dt, float throttle, float brake, float steer);
    void renderBike(Renderer* renderer, Camera* camera);

    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_velocity;

    float m_speed;
    float m_maxSpeed;
    float m_acceleration;
    float m_brakingForce;
    float m_steeringSpeed;
    float m_leanAngle;
    float m_targetLean;

    float m_rpm;
    float m_idleRPM;
    float m_maxRPM;
    float m_redlineRPM;

    int m_gear;
    int m_maxGear;
    float m_gearRatios[6];

    bool m_engineRunning;
    bool m_clutchEngaged;

    // Visual dimensions
    float m_length;
    float m_width;
    float m_height;
    float m_wheelRadius;
    float m_wheelWidth;
    float m_wheelbase;
};

#endif
