#ifndef WORLD_H
#define WORLD_H

#include <glm/glm.hpp>

class Renderer;
class Camera;
class TrafficManager;
class DayNightCycle;
class WeatherController;
class Terrain;
class RoadNetwork;
class WorldChunkManager;

struct WorldState {
    float timeOfDay = 12.0f;
    float rainIntensity = 0.0f;
    float gripFactor = 1.0f;
    bool isNight = false;
    glm::vec3 playerPos = glm::vec3(0.0f);
};

class World {
public:
    World();
    ~World();

    bool init(Renderer* renderer);
    void update(float dt, float playerX, float playerZ);
    void render(Renderer* renderer, Camera* camera);

    TrafficManager* traffic() const { return m_traffic; }
    DayNightCycle* dayNight() const { return m_dayNight; }
    WeatherController* weather() const { return m_weather; }
    Terrain* terrain() const { return m_terrain; }
    RoadNetwork* roadNetwork() const { return m_roadNetwork; }
    WorldChunkManager* chunkManager() const { return m_chunkManager; }
    const WorldState& state() const { return m_state; }

    float getGripAt(float x, float z) const;

private:
    void renderTerrain(Renderer* renderer, Camera* camera);
    void renderGround(Renderer* renderer, Camera* camera);
    void renderSurfaceVariations(Renderer* renderer, Camera* camera);
    void renderRoad(Renderer* renderer, Camera* camera);
    void renderTrees(Renderer* renderer, Camera* camera);
    void renderBuildings(Renderer* renderer, Camera* camera);
    void renderTrafficLights(Renderer* renderer, Camera* camera);
    void renderRain(Renderer* renderer, Camera* camera);
    void renderEnvironmentObjects(Renderer* renderer, Camera* camera);

    TrafficManager* m_traffic;
    DayNightCycle* m_dayNight;
    WeatherController* m_weather;
    Terrain* m_terrain;
    RoadNetwork* m_roadNetwork;
    WorldChunkManager* m_chunkManager;

    WorldState m_state;
    int m_initialSpawnCount;
    float m_worldSize;
};

#endif
