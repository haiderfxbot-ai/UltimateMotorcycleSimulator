#ifndef WORLD_COLLISION_H
#define WORLD_COLLISION_H

#include <glm/glm.hpp>
#include <vector>

class Motorcycle;
class World;
class TrafficManager;
class WorldChunkManager;

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;
};

struct CollisionResult {
    bool hit;
    glm::vec3 contactPoint;
    glm::vec3 normal;
    float penetration;
    float impactSpeed;
    void* hitObject;
};

class WorldCollision {
public:
    WorldCollision();
    ~WorldCollision();

    void init();
    void update(float dt, Motorcycle* bike, World* world, TrafficManager* traffic);

    bool isColliding() const { return m_colliding; }
    const CollisionResult& lastCollision() const { return m_lastCollision; }

private:
    bool sphereVsAABB(const glm::vec3& center, float radius, const BoundingBox& box, CollisionResult& result);
    bool checkBikeVsTraffic(const glm::vec3& bikePos, float bikeRadius, Motorcycle* bike, TrafficManager* traffic);
    bool checkBikeVsBuildings(const glm::vec3& bikePos, float bikeRadius, Motorcycle* bike, WorldChunkManager* cm);
    bool checkBikeVsEnvironment(const glm::vec3& bikePos, float bikeRadius, Motorcycle* bike, World* world);
    void triggerCrash(Motorcycle* bike, const CollisionResult& result);

    CollisionResult m_lastCollision;
    bool m_colliding;
    float m_cooldown;
};

#endif
