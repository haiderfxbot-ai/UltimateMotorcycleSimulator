#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>
#include <cmath>

Camera::Camera()
    : m_targetPos(nullptr)
    , m_targetRot(nullptr)
    , m_position(0.0f, 3.0f, -8.0f)
    , m_forward(0.0f, 0.0f, -1.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_distance(8.0f)
    , m_height(3.0f)
    , m_lookHeight(1.5f)
    , m_smoothSpeed(5.0f)
    , m_fov(65.0f)
    , m_aspect(16.0f / 9.0f)
    , m_near(0.1f)
    , m_far(500.0f)
    , m_projDirty(true)
{}

void Camera::setTarget(glm::vec3* pos, glm::vec3* rot) {
    m_targetPos = pos;
    m_targetRot = rot;
}

void Camera::update(float dt) {
    if (!m_targetPos || !m_targetRot) return;

    float yaw = -(*m_targetRot).y;
    float pitch = 0.15f;

    glm::vec3 desiredOffset;
    desiredOffset.x = m_distance * std::sin(yaw) * std::cos(pitch);
    desiredOffset.y = m_height;
    desiredOffset.z = m_distance * std::cos(yaw) * std::cos(pitch);

    glm::vec3 desiredPos = *m_targetPos + desiredOffset;

    float t = std::min(1.0f, m_smoothSpeed * dt);
    m_position = m_position * (1.0f - t) + desiredPos * t;

    glm::vec3 targetLook = *m_targetPos + glm::vec3(0.0f, m_lookHeight, 0.0f);
    m_forward = glm::normalize(targetLook - m_position);

    if (m_projDirty) rebuildProjection();
}

glm::mat4 Camera::viewMatrix() const {
    return glm::lookAt(m_position, m_position + m_forward, m_up);
}

glm::mat4 Camera::projectionMatrix() const {
    return m_projection;
}

void Camera::rebuildProjection() {
    m_projection = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
    m_projDirty = false;
}
