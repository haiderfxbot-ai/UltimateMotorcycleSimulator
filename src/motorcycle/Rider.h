#ifndef RIDER_H
#define RIDER_H

#include <glm/glm.hpp>

class Renderer;
class Camera;

enum class CrashType {
    None,
    LowSide,
    HighSide,
    StoppieFlip,
    RearFlip,
    Slide,
    Ejection
};

struct RagdollJoint {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 rotation;
    glm::vec3 angularVelocity;
    float mass;
};

class Rider {
public:
    Rider();
    ~Rider();

    void reset();
    void update(float dt, float bikeSpeed, float leanAngle, float pitchAngle,
                bool crashed, const glm::vec3& crashVelocity, const glm::vec3& crashPosition);

    void render(Renderer* renderer, Camera* camera, const glm::vec3& bikePosition, float bikeRotation);

    bool isCrashed() const { return m_crashed; }
    CrashType crashType() const { return m_crashType; }
    glm::vec3 position() const { return m_position; }
    float ragdollTimer() const { return m_ragdollTimer; }

    void triggerCrash(CrashType type, const glm::vec3& velocity, const glm::vec3& position);

private:
    void renderNormal(Renderer* renderer, Camera* camera, const glm::vec3& bikePosition, float bikeRotation);
    void renderRagdoll(Renderer* renderer, Camera* camera);
    void updateRagdoll(float dt);

    glm::vec3 m_position;
    glm::vec3 m_velocity;
    float m_leanOffset;

    bool m_crashed;
    CrashType m_crashType;
    float m_ragdollTimer;

    RagdollJoint m_joints[6];
    int m_numJoints;
};

#endif
