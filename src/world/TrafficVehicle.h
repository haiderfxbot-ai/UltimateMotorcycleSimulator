#ifndef TRAFFIC_VEHICLE_H
#define TRAFFIC_VEHICLE_H

#include <glm/glm.hpp>
#include <vector>

class Renderer;
class Camera;

enum class TrafficLightState {
    Green, Yellow, Red
};

struct TrafficLight {
    glm::vec3 position;
    TrafficLightState state;
    float timer;
    float greenDuration;
    float yellowDuration;
    float redDuration;
};

class TrafficVehicle {
public:
    TrafficVehicle();
    ~TrafficVehicle();

    void spawn(const glm::vec3& pos, float direction, float speed);
    void update(float dt, const std::vector<TrafficVehicle*>& allVehicles,
                const std::vector<TrafficLight>& lights, float playerX, float playerZ,
                float weatherGripFactor);
    void render(Renderer* renderer, Camera* camera);

    glm::vec3 position() const { return m_position; }
    glm::vec3 velocity() const { return m_velocity; }
    float speed() const { return m_speed; }
    bool isActive() const { return m_active; }
    float direction() const { return m_direction; }

    float getLength() const { return m_length; }

private:
    void renderCar(Renderer* renderer, Camera* camera);
    float checkObstacleAhead(const std::vector<TrafficVehicle*>& allVehicles);
    TrafficLightState getNearestLightState(const std::vector<TrafficLight>& lights, float& distToLight);

    glm::vec3 m_position;
    glm::vec3 m_velocity;
    float m_speed;
    float m_targetSpeed;
    float m_direction;
    float m_steerAngle;

    float m_length;
    float m_width;
    float m_height;

    bool m_active;
    float m_lifeTimer;
    float maxLifeTime;

    static constexpr float DESPAWN_DIST = 120.0f;
};

class TrafficManager {
public:
    TrafficManager();
    ~TrafficManager();

    void init(int numVehicles);
    void update(float dt, float playerX, float playerZ, float weatherGripFactor);
    void render(Renderer* renderer, Camera* camera);

    std::vector<TrafficLight>& lights() { return m_lights; }
    const std::vector<TrafficVehicle*>& vehicles() const { return m_vehicles; }

private:
    void spawnVehicle();
    void updateLights(float dt);

    std::vector<TrafficVehicle*> m_vehicles;
    std::vector<TrafficLight> m_lights;
    int m_maxVehicles;
    float m_spawnTimer;
};

#endif
