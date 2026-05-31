#include "WorldCollision.h"
#include "../motorcycle/Motorcycle.h"
#include "../world/World.h"
#include "../world/TrafficVehicle.h"
#include "../world/WorldChunk.h"
#include "../world/Terrain.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>

WorldCollision::WorldCollision()
    : m_colliding(false)
    , m_cooldown(0.0f)
{}

WorldCollision::~WorldCollision() {}

void WorldCollision::init() {}

void WorldCollision::update(float dt, Motorcycle* bike, World* world, TrafficManager* traffic) {
    if (!bike || bike->isCrashed()) return;

    m_cooldown = std::max(0.0f, m_cooldown - dt);

    glm::vec3 bikePos = bike->position();
    glm::vec3 bikeCenter(bikePos.x, bikePos.y + 0.5f, bikePos.z);
    float bikeRadius = 0.8f;

    m_colliding = false;

    CollisionResult result;
    if (checkBikeVsTraffic(bikeCenter, bikeRadius, bike, traffic)) return;
    if (checkBikeVsBuildings(bikeCenter, bikeRadius, bike, world->chunkManager())) return;
    if (checkBikeVsEnvironment(bikeCenter, bikeRadius, bike, world)) return;
}

bool WorldCollision::sphereVsAABB(const glm::vec3& center, float radius,
                                   const BoundingBox& box, CollisionResult& result) {
    glm::vec3 closest(
        std::max(box.min.x, std::min(center.x, box.max.x)),
        std::max(box.min.y, std::min(center.y, box.max.y)),
        std::max(box.min.z, std::min(center.z, box.max.z))
    );

    glm::vec3 diff = center - closest;
    float distSq = glm::dot(diff, diff);

    if (distSq >= radius * radius) return false;

    float dist = std::sqrt(distSq);
    result.hit = true;
    result.contactPoint = closest;
    result.normal = (dist > 0.001f) ? diff / dist : glm::vec3(0.0f, 1.0f, 0.0f);
    result.penetration = radius - dist;
    return true;
}

bool WorldCollision::checkBikeVsTraffic(const glm::vec3& bikeCenter, float bikeRadius,
                                         Motorcycle* bike, TrafficManager* traffic) {
    const auto& vehicles = traffic->vehicles();
    for (auto* v : vehicles) {
        if (!v->isActive()) continue;

        glm::vec3 vPos = v->position();
        float hw = v->getLength() * 0.5f;
        float hh = v->getHeight() * 0.5f;
        float hd = v->getWidth() * 0.5f;

        BoundingBox box;
        box.min = glm::vec3(vPos.x - hw, vPos.y, vPos.z - hd);
        box.max = glm::vec3(vPos.x + hw, vPos.y + v->getHeight(), vPos.z + hd);

        CollisionResult result;
        if (sphereVsAABB(bikeCenter, bikeRadius, box, result)) {
            result.impactSpeed = bike->speed() * std::abs(glm::dot(result.normal, glm::vec3(0.0f, 0.0f, -1.0f)));
            result.hitObject = (void*)v;
            triggerCrash(bike, result);
            return true;
        }
    }
    return false;
}

bool WorldCollision::checkBikeVsBuildings(const glm::vec3& bikeCenter, float bikeRadius,
                                           Motorcycle* bike, WorldChunkManager* cm) {
    const auto& activeChunks = cm->activeChunks();
    for (auto* chunk : activeChunks) {
        for (const auto& obj : chunk->objects) {
            if (obj.type != ChunkObject::Building) continue;

            float hw = obj.scale.x * 0.5f;
            float hd = obj.scale.z * 0.5f;

            BoundingBox box;
            box.min = glm::vec3(obj.position.x - hw, obj.position.y, obj.position.z - hd);
            box.max = glm::vec3(obj.position.x + hw, obj.position.y + obj.scale.y, obj.position.z + hd);

            CollisionResult result;
            if (sphereVsAABB(bikeCenter, bikeRadius, box, result)) {
                result.impactSpeed = 15.0f;
                triggerCrash(bike, result);
                return true;
            }
        }
    }
    return false;
}

bool WorldCollision::checkBikeVsEnvironment(const glm::vec3& bikeCenter, float bikeRadius,
                                             Motorcycle* bike, World* world) {
    Terrain* terrain = world->terrain();
    (void)terrain;

    // Guardrail posts at x=±3.5, z in [-50, 50], spaced 4 apart
    for (int side = -1; side <= 1; side += 2) {
        for (int si = 0; si < 25; ++si) {
            float z = -50.0f + (float)si * 4.0f;
            float x = (float)side * 3.5f;
            float h = terrain->getHeightAt(x, z);

            BoundingBox box;
            box.min = glm::vec3(x - 0.2f, h, z - 0.2f);
            box.max = glm::vec3(x + 0.2f, h + 0.6f, z + 0.2f);

            CollisionResult result;
            if (sphereVsAABB(bikeCenter, bikeRadius, box, result)) {
                result.impactSpeed = 10.0f;
                triggerCrash(bike, result);
                return true;
            }
        }
    }

    // Road signs at x=4.5, z=±15,±30
    for (int si = -2; si <= 2; ++si) {
        if (si == 0) continue;
        float z = (float)si * 15.0f;
        float x = 4.5f;
        float h = terrain->getHeightAt(x, z);

        {
            BoundingBox box;
            box.min = glm::vec3(x - 0.15f, h, z - 0.15f);
            box.max = glm::vec3(x + 0.15f, h + 1.6f, z + 0.15f);
            CollisionResult result;
            if (sphereVsAABB(bikeCenter, bikeRadius, box, result)) {
                result.impactSpeed = 10.0f;
                triggerCrash(bike, result);
                return true;
            }
        }
        {
            BoundingBox box;
            box.min = glm::vec3(x - 0.5f, h + 1.4f, z - 0.1f);
            box.max = glm::vec3(x + 0.5f, h + 1.8f, z + 0.1f);
            CollisionResult result;
            if (sphereVsAABB(bikeCenter, bikeRadius, box, result)) {
                result.impactSpeed = 8.0f;
                triggerCrash(bike, result);
                return true;
            }
        }
    }

    // Bridge structures at z=±8
    for (int side = -1; side <= 1; side += 2) {
        float z = (float)side * 8.0f;

        BoundingBox box;
        box.min = glm::vec3(-3.25f, 0.7f, z - 1.5f);
        box.max = glm::vec3(3.25f, 1.0f, z + 1.5f);

        CollisionResult result;
        if (sphereVsAABB(bikeCenter, bikeRadius, box, result)) {
            result.impactSpeed = 5.0f;
            triggerCrash(bike, result);
            return true;
        }
    }

    // Lamp posts at x=±4, z in [-36, 36] step 12
    for (int side = -1; side <= 1; side += 2) {
        for (int li = -3; li <= 3; ++li) {
            float z = (float)li * 12.0f;
            if (std::abs(z) > 50.0f) continue;
            float x = (float)side * 4.0f;
            float h = terrain->getHeightAt(x, z);

            BoundingBox box;
            box.min = glm::vec3(x - 0.15f, h, z - 0.15f);
            box.max = glm::vec3(x + 0.15f, h + 2.5f, z + 0.15f);

            CollisionResult result;
            if (sphereVsAABB(bikeCenter, bikeRadius, box, result)) {
                result.impactSpeed = 10.0f;
                triggerCrash(bike, result);
                return true;
            }
        }
    }

    return false;
}

void WorldCollision::triggerCrash(Motorcycle* bike, const CollisionResult& result) {
    if (m_cooldown > 0.0f) return;
    m_colliding = true;
    m_lastCollision = result;
    m_cooldown = 0.5f;

    if (bike && !bike->isCrashed()) {
        glm::vec3 impactVel = glm::vec3(0.0f, 0.0f, -result.impactSpeed);
        bike->triggerExternalCrash(result.impactSpeed, impactVel);
    }
}
