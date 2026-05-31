#include "RoadNetwork.h"
#include "../renderer/Renderer.h"
#include "../engine/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <cstdlib>

RoadNetwork::RoadNetwork()
    : m_worldSize(200.0f)
    , m_seed(0)
{}

RoadNetwork::~RoadNetwork() {}

glm::vec2 RoadSegment::evaluate(float t) const {
    if (type == RoadType::Straight) {
        return start + (end - start) * t;
    }
    float u = 1.0f - t;
    return u * u * u * start +
           3.0f * u * u * t * control1 +
           3.0f * u * t * t * control2 +
           t * t * t * end;
}

glm::vec2 RoadSegment::tangent(float t) const {
    if (type == RoadType::Straight) {
        return glm::normalize(end - start);
    }
    float u = 1.0f - t;
    glm::vec2 deriv = -3.0f * u * u * start +
                       3.0f * (u * u - 2.0f * u * t) * control1 +
                       3.0f * (2.0f * u * t - t * t) * control2 +
                       3.0f * t * t * end;
    float len = glm::length(deriv);
    if (len < 0.001f) return glm::vec2(0.0f, 1.0f);
    return deriv / len;
}

float RoadSegment::length() const {
    if (type == RoadType::Straight) {
        return glm::distance(start, end);
    }
    float len = 0.0f;
    glm::vec2 prev = evaluate(0.0f);
    for (int i = 1; i <= 20; ++i) {
        glm::vec2 cur = evaluate((float)i / 20.0f);
        len += glm::distance(cur, prev);
        prev = cur;
    }
    return len;
}

void RoadNetwork::init(int seed, float worldSize) {
    m_seed = seed;
    m_worldSize = worldSize;
    std::srand(seed);

    m_segments.clear();
    m_intersections.clear();

    generateMainRoad();
    generateSideRoads();
    generateHighwayLoop();
    generateIntersections();
}

void RoadNetwork::generateMainRoad() {
    // Main road runs along Z axis with gentle curves
    float zExtent = m_worldSize * 0.5f;

    // Segment 1: slight S-curve from far south to center
    {
        RoadSegment seg;
        seg.type = RoadType::Curve;
        seg.start = glm::vec2(0.0f, -zExtent);
        seg.end = glm::vec2(0.0f, 0.0f);
        seg.control1 = glm::vec2(3.0f, -zExtent * 0.6f);
        seg.control2 = glm::vec2(-2.0f, -zExtent * 0.3f);
        seg.width = 6.0f;
        seg.surface = SurfaceType::Asphalt;
        seg.isMainRoad = true;
        m_segments.push_back(seg);
    }

    // Segment 2: center to far north with curve
    {
        RoadSegment seg;
        seg.type = RoadType::Curve;
        seg.start = glm::vec2(0.0f, 0.0f);
        seg.end = glm::vec2(0.0f, zExtent);
        seg.control1 = glm::vec2(2.0f, zExtent * 0.3f);
        seg.control2 = glm::vec2(-3.0f, zExtent * 0.6f);
        seg.width = 6.0f;
        seg.surface = SurfaceType::Asphalt;
        seg.isMainRoad = true;
        m_segments.push_back(seg);
    }

    // Cross road (East-West) at center
    {
        RoadSegment seg;
        seg.type = RoadType::Straight;
        seg.start = glm::vec2(-30.0f, 0.0f);
        seg.end = glm::vec2(30.0f, 0.0f);
        seg.width = 5.0f;
        seg.surface = SurfaceType::Asphalt;
        seg.isMainRoad = true;
        m_segments.push_back(seg);
    }

    // Diagonal road: SE to NW
    {
        RoadSegment seg;
        seg.type = RoadType::Straight;
        seg.start = glm::vec2(25.0f, -40.0f);
        seg.end = glm::vec2(-25.0f, 40.0f);
        seg.width = 4.0f;
        seg.surface = SurfaceType::Asphalt;
        seg.isMainRoad = true;
        m_segments.push_back(seg);
    }

    // Diagonal road: SW to NE
    {
        RoadSegment seg;
        seg.type = RoadType::Straight;
        seg.start = glm::vec2(-25.0f, -40.0f);
        seg.end = glm::vec2(25.0f, 40.0f);
        seg.width = 4.0f;
        seg.surface = SurfaceType::Asphalt;
        seg.isMainRoad = true;
        m_segments.push_back(seg);
    }
}

void RoadNetwork::generateSideRoads() {
    // Side roads branching from main road at various Z positions
    float sideRoadZs[] = { -60.0f, -35.0f, 35.0f, 60.0f };
    float sideLengths[] = { 15.0f, 20.0f, 18.0f, 25.0f };
    float sideDirs[] = { 1.0f, -1.0f, 1.0f, -1.0f };

    for (int i = 0; i < 4; ++i) {
        float z = sideRoadZs[i];
        float len = sideLengths[i];
        float dir = sideDirs[i];

        RoadSegment seg;
        seg.type = RoadType::Straight;
        seg.start = glm::vec2(3.0f * dir, z);
        seg.end = glm::vec2((3.0f + len) * dir, z);
        seg.width = 3.5f;
        seg.surface = SurfaceType::Asphalt;
        seg.isMainRoad = false;
        m_segments.push_back(seg);
    }

    // Curved side road branching off at Z=-20
    {
        RoadSegment seg;
        seg.type = RoadType::Curve;
        seg.start = glm::vec2(3.0f, -20.0f);
        seg.end = glm::vec2(12.0f, -28.0f);
        seg.control1 = glm::vec2(7.0f, -20.0f);
        seg.control2 = glm::vec2(12.0f, -23.0f);
        seg.width = 3.5f;
        seg.surface = SurfaceType::Concrete;
        seg.isMainRoad = false;
        m_segments.push_back(seg);
    }

    // Another curved side road at Z=25
    {
        RoadSegment seg;
        seg.type = RoadType::Curve;
        seg.start = glm::vec2(-3.0f, 25.0f);
        seg.end = glm::vec2(-14.0f, 32.0f);
        seg.control1 = glm::vec2(-7.0f, 25.0f);
        seg.control2 = glm::vec2(-14.0f, 28.0f);
        seg.width = 3.5f;
        seg.surface = SurfaceType::Concrete;
        seg.isMainRoad = false;
        m_segments.push_back(seg);
    }
}

void RoadNetwork::generateHighwayLoop() {
    // A wide highway loop around the outskirts
    float r = m_worldSize * 0.35f;

    RoadSegment seg;
    seg.type = RoadType::Curve;
    seg.start = glm::vec2(r, 0.0f);
    seg.end = glm::vec2(0.0f, r);
    seg.control1 = glm::vec2(r, r * 0.5f);
    seg.control2 = glm::vec2(r * 0.5f, r);
    seg.width = 5.0f;
    seg.surface = SurfaceType::Concrete;
    seg.isMainRoad = true;
    m_segments.push_back(seg);

    RoadSegment seg2;
    seg2.type = RoadType::Curve;
    seg2.start = glm::vec2(0.0f, r);
    seg2.end = glm::vec2(-r, 0.0f);
    seg2.control1 = glm::vec2(-r * 0.5f, r);
    seg2.control2 = glm::vec2(-r, r * 0.5f);
    seg2.width = 5.0f;
    seg2.surface = SurfaceType::Concrete;
    seg2.isMainRoad = true;
    m_segments.push_back(seg2);

    RoadSegment seg3;
    seg3.type = RoadType::Curve;
    seg3.start = glm::vec2(-r, 0.0f);
    seg3.end = glm::vec2(0.0f, -r);
    seg3.control1 = glm::vec2(-r, -r * 0.5f);
    seg3.control2 = glm::vec2(-r * 0.5f, -r);
    seg3.width = 5.0f;
    seg3.surface = SurfaceType::Concrete;
    seg3.isMainRoad = true;
    m_segments.push_back(seg3);

    RoadSegment seg4;
    seg4.type = RoadType::Curve;
    seg4.start = glm::vec2(0.0f, -r);
    seg4.end = glm::vec2(r, 0.0f);
    seg4.control1 = glm::vec2(r * 0.5f, -r);
    seg4.control2 = glm::vec2(r, -r * 0.5f);
    seg4.width = 5.0f;
    seg4.surface = SurfaceType::Concrete;
    seg4.isMainRoad = true;
    m_segments.push_back(seg4);
}

void RoadNetwork::generateIntersections() {
    // Find crossings and create intersection data
    IntersectionData mainCross;
    mainCross.position = glm::vec2(0.0f, 0.0f);
    mainCross.size = 8.0f;
    mainCross.hasTrafficLight = true;
    for (int i = 0; i < (int)m_segments.size(); ++i) {
        if (glm::distance(m_segments[i].evaluate(0.5f), glm::vec2(0.0f)) < 1.0f) {
            mainCross.connectedSegments.push_back(i);
        }
    }
    m_intersections.push_back(mainCross);
}

void RoadNetwork::render(Renderer* renderer, Camera* camera) {
    (void)camera;

    for (const auto& seg : m_segments) {
        if (seg.type == RoadType::Straight) {
            // Straight road as a long quad
            float len = glm::distance(seg.start, seg.end);
            glm::vec2 dir = glm::normalize(seg.end - seg.start);
            float angle = atan2f(dir.y, dir.x);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(
                (seg.start.x + seg.end.x) * 0.5f, 0.01f,
                (seg.start.y + seg.end.y) * 0.5f));
            model = glm::rotate(model, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(seg.width, 1.0f, len));

            renderer->drawPlane(model, glm::vec3(0.25f, 0.25f, 0.25f));

            // Edge lines
            for (int side = -1; side <= 1; side += 2) {
                glm::mat4 edge = glm::mat4(1.0f);
                edge = glm::translate(edge, glm::vec3(
                    (seg.start.x + seg.end.x) * 0.5f + side * (seg.width * 0.5f - 0.1f), 0.02f,
                    (seg.start.y + seg.end.y) * 0.5f));
                edge = glm::rotate(edge, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
                edge = glm::scale(edge, glm::vec3(0.15f, 1.0f, len - 0.5f));
                renderer->drawPlane(edge, glm::vec3(0.95f, 0.95f, 0.95f));
            }

            // Center line dashes
            if (seg.isMainRoad) {
                int dashCount = (int)(len / 4.0f);
                for (int d = 0; d < dashCount; ++d) {
                    float t = (float)d / (float)dashCount;
                    glm::vec2 pos = seg.start + (seg.end - seg.start) * t;
                    glm::mat4 dash = glm::mat4(1.0f);
                    dash = glm::translate(dash, glm::vec3(pos.x, 0.02f, pos.y));
                    dash = glm::rotate(dash, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
                    dash = glm::scale(dash, glm::vec3(0.1f, 1.0f, 1.5f));
                    renderer->drawPlane(dash, glm::vec3(0.9f, 0.9f, 0.2f));
                }
            }
        } else {
            // Curved road: render as series of quads along bezier
            int subdiv = 30;
            glm::vec2 prev = seg.evaluate(0.0f);
            glm::vec2 prevTan = seg.tangent(0.0f);

            for (int i = 1; i <= subdiv; ++i) {
                float t = (float)i / (float)subdiv;
                glm::vec2 cur = seg.evaluate(t);
                glm::vec2 mid = (prev + cur) * 0.5f;

                float len = glm::distance(cur, prev);
                if (len < 0.05f) { prev = cur; continue; }

                glm::vec2 dir = glm::normalize(cur - prev);
                float angle = atan2f(dir.y, dir.x);

                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(mid.x, 0.01f, mid.y));
                model = glm::rotate(model, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, glm::vec3(seg.width, 1.0f, len));

                renderer->drawPlane(model, glm::vec3(0.25f, 0.25f, 0.25f));

                // Edge lines
                for (int side = -1; side <= 1; side += 2) {
                    glm::vec2 edgePos = mid + glm::vec2(-dir.y, dir.x) * (seg.width * 0.5f - 0.1f) * (float)side;
                    glm::mat4 edge = glm::mat4(1.0f);
                    edge = glm::translate(edge, glm::vec3(edgePos.x, 0.02f, edgePos.y));
                    edge = glm::rotate(edge, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
                    edge = glm::scale(edge, glm::vec3(0.15f, 1.0f, len - 0.1f));
                    renderer->drawPlane(edge, glm::vec3(0.95f, 0.95f, 0.95f));
                }

                prev = cur;
                prevTan = seg.tangent(t);
            }
        }
    }

    // Render intersections
    for (const auto& inter : m_intersections) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(inter.position.x, 0.015f, inter.position.y));
        model = glm::scale(model, glm::vec3(inter.size, 1.0f, inter.size));
        renderer->drawPlane(model, glm::vec3(0.22f, 0.22f, 0.22f));
    }
}

bool RoadNetwork::isOnRoad(float x, float z, float* outDist) const {
    float minDist = 1e10f;
    for (const auto& seg : m_segments) {
        float halfW = seg.width * 0.5f;
        if (seg.type == RoadType::Straight) {
            glm::vec2 p(x, z);
            glm::vec2 s = seg.start;
            glm::vec2 e = seg.end;
            glm::vec2 se = e - s;
            float len2 = glm::dot(se, se);
            if (len2 < 0.001f) continue;
            float t = glm::clamp(glm::dot(p - s, se) / len2, 0.0f, 1.0f);
            glm::vec2 closest = s + se * t;
            float d = glm::distance(p, closest);
            float dw = d - halfW;
            if (dw < minDist) minDist = dw;
        } else {
            int subdiv = 20;
            for (int i = 0; i <= subdiv; ++i) {
                float t = (float)i / (float)subdiv;
                glm::vec2 pt = seg.evaluate(t);
                float d = glm::distance(glm::vec2(x, z), pt);
                float dw = d - halfW;
                if (dw < minDist) minDist = dw;
            }
        }
    }
    if (outDist) *outDist = minDist;
    return minDist < 0.0f;
}

float RoadNetwork::getRoadHeightAt(float x, float z) const {
    (void)x; (void)z;
    return 0.0f;
}

SurfaceType RoadNetwork::getSurfaceAt(float x, float z) const {
    for (const auto& seg : m_segments) {
        float halfW = seg.width * 0.5f;
        glm::vec2 p(x, z);
        if (seg.type == RoadType::Straight) {
            glm::vec2 s = seg.start;
            glm::vec2 e = seg.end;
            glm::vec2 se = e - s;
            float len2 = glm::dot(se, se);
            if (len2 < 0.001f) continue;
            float t = glm::clamp(glm::dot(p - s, se) / len2, 0.0f, 1.0f);
            glm::vec2 closest = s + se * t;
            if (glm::distance(p, closest) < halfW) {
                return seg.surface;
            }
        } else {
            int subdiv = 20;
            for (int i = 0; i <= subdiv; ++i) {
                float t = (float)i / (float)subdiv;
                glm::vec2 pt = seg.evaluate(t);
                if (glm::distance(p, pt) < halfW) {
                    return seg.surface;
                }
            }
        }
    }
    return SurfaceType::Grass;
}

glm::vec2 RoadNetwork::getNearestRoadPoint(float x, float z, int* outSegment) const {
    float minDist = 1e10f;
    glm::vec2 bestPt(0.0f);
    int bestSeg = -1;

    for (int si = 0; si < (int)m_segments.size(); ++si) {
        const auto& seg = m_segments[si];
        glm::vec2 p(x, z);

        if (seg.type == RoadType::Straight) {
            glm::vec2 s = seg.start;
            glm::vec2 e = seg.end;
            glm::vec2 se = e - s;
            float len2 = glm::dot(se, se);
            if (len2 < 0.001f) continue;
            float t = glm::clamp(glm::dot(p - s, se) / len2, 0.0f, 1.0f);
            glm::vec2 closest = s + se * t;
            float d = glm::distance(p, closest);
            if (d < minDist) {
                minDist = d;
                bestPt = closest;
                bestSeg = si;
            }
        } else {
            int subdiv = 20;
            for (int i = 0; i <= subdiv; ++i) {
                float t = (float)i / (float)subdiv;
                glm::vec2 pt = seg.evaluate(t);
                float d = glm::distance(p, pt);
                if (d < minDist) {
                    minDist = d;
                    bestPt = pt;
                    bestSeg = si;
                }
            }
        }
    }

    if (outSegment) *outSegment = bestSeg;
    return bestPt;
}
