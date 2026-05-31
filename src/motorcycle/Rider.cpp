#include "Rider.h"
#include "../renderer/Renderer.h"
#include "../engine/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <algorithm>
#include <cmath>

Rider::Rider()
    : m_position(0.0f, 1.0f, 0.0f)
    , m_velocity(0.0f)
    , m_leanOffset(0.0f)
    , m_crashed(false)
    , m_crashType(CrashType::None)
    , m_ragdollTimer(0.0f)
    , m_numJoints(6)
{
    reset();
}

Rider::~Rider() {}

void Rider::reset() {
    m_position = glm::vec3(0.0f, 1.0f, 0.0f);
    m_velocity = glm::vec3(0.0f);
    m_leanOffset = 0.0f;
    m_crashed = false;
    m_crashType = CrashType::None;
    m_ragdollTimer = 0.0f;

    for (int i = 0; i < m_numJoints; ++i) {
        m_joints[i].position = glm::vec3(0.0f, 1.0f - i * 0.3f, 0.0f);
        m_joints[i].velocity = glm::vec3(0.0f);
        m_joints[i].rotation = glm::vec3(0.0f);
        m_joints[i].angularVelocity = glm::vec3(0.0f);
        m_joints[i].mass = 10.0f - i;
    }
}

void Rider::update(float dt, float bikeSpeed, float leanAngle, float pitchAngle,
                   bool crashed, const glm::vec3& crashVelocity, const glm::vec3& crashPosition) {
    if (crashed && !m_crashed) {
        CrashType type = CrashType::LowSide;
        float speed = std::abs(bikeSpeed);
        if (speed > 30.0f) type = CrashType::HighSide;
        else if (speed > 15.0f) type = CrashType::Slide;
        triggerCrash(type, crashVelocity, crashPosition);
    }

    if (m_crashed) {
        updateRagdoll(dt);
    } else {
        float targetLean = leanAngle * 0.3f;
        m_leanOffset += (targetLean - m_leanOffset) * 5.0f * dt;
    }
}

void Rider::triggerCrash(CrashType type, const glm::vec3& velocity, const glm::vec3& position) {
    m_crashed = true;
    m_crashType = type;
    m_ragdollTimer = 0.0f;
    m_position = position;

    float speed = glm::length(velocity);
    for (int i = 0; i < m_numJoints; ++i) {
        m_joints[i].position = position + glm::vec3(
            ((float)std::rand() / RAND_MAX - 0.5f) * 0.5f,
            1.2f - i * 0.25f,
            ((float)std::rand() / RAND_MAX - 0.5f) * 0.5f
        );
        m_joints[i].velocity = velocity * (0.5f + (float)std::rand() / RAND_MAX * 0.5f)
            + glm::vec3(0.0f, 2.0f, 0.0f);
        m_joints[i].angularVelocity = glm::vec3(
            (float)std::rand() / RAND_MAX * 6.0f - 3.0f,
            (float)std::rand() / RAND_MAX * 6.0f - 3.0f,
            (float)std::rand() / RAND_MAX * 6.0f - 3.0f
        );
    }
}

void Rider::updateRagdoll(float dt) {
    m_ragdollTimer += dt;
    for (int i = 0; i < m_numJoints; ++i) {
        auto& j = m_joints[i];

        j.velocity.y -= 9.81f * dt;

        glm::vec3 groundPos = j.position;
        groundPos.y = 0.05f;
        if (j.position.y < 0.05f) {
            j.position.y = 0.05f;
            j.velocity.y *= -0.3f;
            j.velocity.x *= 0.98f;
            j.velocity.z *= 0.98f;
        }

        j.position += j.velocity * dt;
        j.rotation += j.angularVelocity * dt;

        j.angularVelocity *= std::max(0.0f, 1.0f - 2.0f * dt);
        j.velocity *= std::max(0.0f, 1.0f - 0.5f * dt);
    }
}

void Rider::render(Renderer* renderer, Camera* camera, const glm::vec3& bikePosition, float bikeRotation) {
    if (!renderer || !camera) return;

    glm::mat4 pv = camera->projectionMatrix() * camera->viewMatrix();
    renderer->setProjectionView(pv);

    if (m_crashed) {
        renderRagdoll(renderer, camera);
    } else {
        renderNormal(renderer, camera, bikePosition, bikeRotation);
    }
}

void Rider::renderNormal(Renderer* renderer, Camera* camera, const glm::vec3& bikePosition, float bikeRotation) {
    (void)camera;

    glm::mat4 riderModel = glm::mat4(1.0f);
    riderModel = glm::translate(riderModel, bikePosition);
    riderModel = glm::rotate(riderModel, bikeRotation, glm::vec3(0.0f, 1.0f, 0.0f));
    riderModel = glm::rotate(riderModel, m_leanOffset, glm::vec3(1.0f, 0.0f, 0.0f));

    // Torso
    glm::mat4 torso = glm::translate(riderModel, glm::vec3(0.0f, 0.85f, 0.0f));
    torso = glm::scale(torso, glm::vec3(0.35f, 0.45f, 0.25f));
    renderer->drawBox(torso, glm::vec3(0.2f, 0.4f, 0.7f));

    // Head
    glm::mat4 head = glm::translate(riderModel, glm::vec3(0.0f, 1.25f, 0.0f));
    head = glm::scale(head, glm::vec3(0.2f, 0.2f, 0.2f));
    renderer->drawBox(head, glm::vec3(0.85f, 0.7f, 0.6f));

    // Left arm
    glm::mat4 lArm = glm::translate(riderModel, glm::vec3(-0.3f, 0.95f, 0.1f));
    lArm = glm::rotate(lArm, 0.3f, glm::vec3(0.0f, 0.0f, 1.0f));
    lArm = glm::scale(lArm, glm::vec3(0.07f, 0.35f, 0.07f));
    renderer->drawBox(lArm, glm::vec3(0.2f, 0.35f, 0.6f));

    // Right arm
    glm::mat4 rArm = glm::translate(riderModel, glm::vec3(0.3f, 0.95f, 0.1f));
    rArm = glm::rotate(rArm, -0.3f, glm::vec3(0.0f, 0.0f, 1.0f));
    rArm = glm::scale(rArm, glm::vec3(0.07f, 0.35f, 0.07f));
    renderer->drawBox(rArm, glm::vec3(0.2f, 0.35f, 0.6f));

    // Left leg
    glm::mat4 lLeg = glm::translate(riderModel, glm::vec3(-0.12f, 0.45f, -0.05f));
    lLeg = glm::scale(lLeg, glm::vec3(0.1f, 0.4f, 0.12f));
    renderer->drawBox(lLeg, glm::vec3(0.15f, 0.15f, 0.2f));

    // Right leg
    glm::mat4 rLeg = glm::translate(riderModel, glm::vec3(0.12f, 0.45f, -0.05f));
    rLeg = glm::scale(rLeg, glm::vec3(0.1f, 0.4f, 0.12f));
    renderer->drawBox(rLeg, glm::vec3(0.15f, 0.15f, 0.2f));
}

void Rider::renderRagdoll(Renderer* renderer, Camera* camera) {
    (void)camera;
    for (int i = 0; i < m_numJoints - 1; ++i) {
        const auto& j = m_joints[i];
        glm::mat4 jointModel = glm::mat4(1.0f);
        jointModel = glm::translate(jointModel, j.position);
        jointModel = glm::rotate(jointModel, j.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        jointModel = glm::rotate(jointModel, j.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        jointModel = glm::scale(jointModel, glm::vec3(0.15f, 0.15f, 0.3f));

        float colorFade = std::max(0.0f, 1.0f - m_ragdollTimer * 0.5f);
        renderer->drawBox(jointModel, glm::vec3(0.2f * colorFade, 0.4f * colorFade, 0.7f * colorFade));
    }
}
