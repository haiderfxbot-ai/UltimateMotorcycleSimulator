#ifndef ROAD_NETWORK_H
#define ROAD_NETWORK_H

#include <glm/glm.hpp>
#include <vector>
#include "../world/Surface.h"

class Renderer;
class Camera;

enum class RoadType {
    Straight,
    Curve,
    Intersection,
    Roundabout,
    Highway
};

struct RoadSegment {
    RoadType type;
    glm::vec2 start;
    glm::vec2 end;
    glm::vec2 control1;
    glm::vec2 control2;
    float width;
    SurfaceType surface;
    bool isMainRoad;

    glm::vec2 evaluate(float t) const;
    glm::vec2 tangent(float t) const;
    float length() const;
};

struct IntersectionData {
    glm::vec2 position;
    float size;
    std::vector<int> connectedSegments;
    bool hasTrafficLight;
};

struct RoadLane {
    glm::vec2 center;
    float width;
    int direction; // -1 or 1
};

class RoadNetwork {
public:
    RoadNetwork();
    ~RoadNetwork();

    void init(int seed, float worldSize);
    void render(Renderer* renderer, Camera* camera);

    int segmentCount() const { return (int)m_segments.size(); }
    const RoadSegment& getSegment(int i) const { return m_segments[i]; }

    bool isOnRoad(float x, float z, float* outDist = nullptr) const;
    float getRoadHeightAt(float x, float z) const;
    SurfaceType getSurfaceAt(float x, float z) const;
    glm::vec2 getNearestRoadPoint(float x, float z, int* outSegment = nullptr) const;

    const std::vector<RoadSegment>& segments() const { return m_segments; }
    const std::vector<IntersectionData>& intersections() const { return m_intersections; }

private:
    void generateMainRoad();
    void generateSideRoads();
    void generateIntersections();
    void generateHighwayLoop();

    std::vector<RoadSegment> m_segments;
    std::vector<IntersectionData> m_intersections;
    float m_worldSize;
    int m_seed;
};

#endif
