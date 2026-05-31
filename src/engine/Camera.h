#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
    Camera();

    void setTarget(glm::vec3* pos, glm::vec3* rot);
    void update(float dt);

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;
    glm::vec3 position() const { return m_position; }
    glm::vec3 forward() const { return m_forward; }

    void setAspect(float aspect) { m_aspect = aspect; m_projDirty = true; }
    float fov() const { return m_fov; }
    void setFov(float fov) { m_fov = fov; m_projDirty = true; }

private:
    void rebuildProjection();

    glm::vec3* m_targetPos;
    glm::vec3* m_targetRot;

    glm::vec3 m_position;
    glm::vec3 m_forward;
    glm::vec3 m_up;

    float m_distance;
    float m_height;
    float m_lookHeight;
    float m_smoothSpeed;

    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;
    bool m_projDirty;
    glm::mat4 m_projection;
};

#endif
