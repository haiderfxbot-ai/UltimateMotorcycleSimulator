#include "HUD.h"
#include "Renderer.h"
#include "../motorcycle/Motorcycle.h"
#include "../world/World.h"
#include "../world/RoadNetwork.h"
#include "../world/TrafficVehicle.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <algorithm>

HUD::HUD()
    : m_width(1280)
    , m_height(720)
    , m_scale(1.0f)
{}

HUD::~HUD() {}

void HUD::init(int screenWidth, int screenHeight) {
    m_width = screenWidth;
    m_height = screenHeight;
    m_scale = (float)screenHeight / 720.0f;
}

void HUD::render(Renderer* renderer, const Motorcycle* bike, const World* world,
                 int fps, bool headlightOn) {
    if (!renderer || !bike) return;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glm::mat4 ortho = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height, -1.0f, 1.0f);
    renderer->setProjectionView(ortho);

    drawSpeedometer(renderer, bike);
    drawTachometer(renderer, bike);
    drawGearIndicator(renderer, bike);
    drawFuelGauge(renderer, bike);
    drawStatusIndicators(renderer, bike, world, headlightOn);
    drawMinimap(renderer, bike, world);
    drawFPS(renderer, fps);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

float HUD::drawGaugeBackground(Renderer* renderer, float cx, float cy, float radius) {
    // Outer ring
    glm::mat4 ring = glm::mat4(1.0f);
    ring = glm::translate(ring, glm::vec3(cx, cy, 0.0f));
    ring = glm::scale(ring, glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f));
    renderer->drawPlane(ring, glm::vec3(0.08f, 0.08f, 0.1f));

    // Inner ring (slightly smaller, darker)
    glm::mat4 inner = glm::mat4(1.0f);
    inner = glm::translate(inner, glm::vec3(cx, cy, 0.001f));
    inner = glm::scale(inner, glm::vec3(radius * 1.7f, radius * 1.7f, 1.0f));
    renderer->drawPlane(inner, glm::vec3(0.04f, 0.04f, 0.05f));

    return radius;
}

void HUD::drawGaugeTicks(Renderer* renderer, float cx, float cy, float radius,
                         int numTicks, float startAngle, float endAngle,
                         float redlineStart) {
    float range = endAngle - startAngle;

    for (int i = 0; i <= numTicks; ++i) {
        float t = (float)i / (float)numTicks;
        float angle = startAngle + t * range;

        float cosA = cosf(angle);
        float sinA = sinf(angle);

        bool isMajor = (i % 5 == 0);
        bool isRedline = (redlineStart > 0.0f && t >= redlineStart);

        float innerR = isMajor ? radius * 0.6f : radius * 0.7f;
        float outerR = radius * 0.85f;
        float tickWidth = isMajor ? 2.0f * m_scale : 1.0f * m_scale;
        float tickLen = outerR - innerR;

        glm::vec3 tickColor = isRedline ? glm::vec3(0.8f, 0.1f, 0.1f) : glm::vec3(0.6f, 0.6f, 0.6f);

        float midR = (innerR + outerR) * 0.5f;
        float mx = cx + midR * cosA;
        float my = cy + midR * sinA;

        glm::mat4 tick = glm::mat4(1.0f);
        tick = glm::translate(tick, glm::vec3(mx, my, 0.005f));
        tick = glm::rotate(tick, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        tick = glm::scale(tick, glm::vec3(tickWidth, tickLen, 1.0f));
        renderer->drawPlane(tick, tickColor);
    }
}

void HUD::drawGaugeArc(Renderer* renderer, float cx, float cy, float radius,
                       float startAngle, float endAngle, float value,
                       float redlineStart, const glm::vec3& color) {
    (void)redlineStart;
    int segments = 40;
    float range = endAngle - startAngle;
    float currentAngle = startAngle + range * value;

    float segAngle = 6.2832f / 360.0f;
    int numSegs = std::max(2, (int)((currentAngle - startAngle) / segAngle));

    for (int i = 0; i < numSegs; ++i) {
        float a1 = startAngle + range * (float)i / 40.0f;
        float a2 = startAngle + range * (float)(i + 1) / 40.0f;

        if (a2 > currentAngle) a2 = currentAngle;
        if (a1 > currentAngle) break;

        float innerR = radius * 0.55f;
        float outerR = radius * 0.6f;

        glm::mat4 seg = glm::mat4(1.0f);
        float midAngle = (a1 + a2) * 0.5f;
        float midR = (innerR + outerR) * 0.5f;
        seg = glm::translate(seg, glm::vec3(
            cx + midR * cosf(midAngle),
            cy + midR * sinf(midAngle),
            0.01f));
        seg = glm::rotate(seg, midAngle, glm::vec3(0.0f, 0.0f, 1.0f));

        float w = outerR - innerR;
        float h = midAngle * 2.0f * midR;
        seg = glm::scale(seg, glm::vec3(w, h, 1.0f));
        renderer->drawPlane(seg, color);
    }
}

void HUD::drawNeedle(Renderer* renderer, float cx, float cy, float radius,
                     float angle, const glm::vec3& color) {
    float outerLen = radius * 0.75f;
    float innerLen = radius * 0.2f;
    float width = 3.0f * m_scale;

    float cosA = cosf(angle);
    float sinA = sinf(angle);

    float tipX = cx + outerLen * cosA;
    float tipY = cy + outerLen * sinA;
    float baseX = cx - innerLen * cosA;
    float baseY = cy - innerLen * sinA;

    glm::vec3 perp(-sinA, cosA, 0.0f);
    float hw = width * 0.5f;

    Vertex needleVerts[4] = {
        { glm::vec3(tipX, tipY, 0.02f), glm::vec3(0.0f), glm::vec2(0.0f) },
        { glm::vec3(baseX + perp.x * hw, baseY + perp.y * hw, 0.02f), glm::vec3(0.0f), glm::vec2(0.0f) },
        { glm::vec3(baseX - perp.x * hw, baseY - perp.y * hw, 0.02f), glm::vec3(0.0f), glm::vec2(0.0f) }
    };

    glm::mat4 identity(1.0f);
    renderer->drawTriangles(needleVerts, 3, identity, color);

    // Center hub
    glm::mat4 hub = glm::mat4(1.0f);
    hub = glm::translate(hub, glm::vec3(cx, cy, 0.025f));
    hub = glm::scale(hub, glm::vec3(6.0f * m_scale, 6.0f * m_scale, 1.0f));
    renderer->drawPlane(hub, glm::vec3(0.7f, 0.7f, 0.7f));
}

void HUD::drawSpeedometer(Renderer* renderer, const Motorcycle* bike) {
    float cx = m_width - 120.0f * m_scale;
    float cy = 140.0f * m_scale;
    float radius = 80.0f * m_scale;

    drawGaugeBackground(renderer, cx, cy, radius);

    float startAngle = glm::pi<float>() * 0.75f;
    float endAngle = glm::pi<float>() * 2.25f;
    float maxSpeed = 200.0f;
    float speed = bike->speed() * 3.6f;
    float value = std::min(1.0f, speed / maxSpeed);

    drawGaugeTicks(renderer, cx, cy, radius, 20, startAngle, endAngle, 1.0f);
    drawGaugeArc(renderer, cx, cy, radius, startAngle, endAngle, value, 1.0f, glm::vec3(0.2f, 0.6f, 0.9f));

    float needleAngle = startAngle + (endAngle - startAngle) * value;
    drawNeedle(renderer, cx, cy, radius, needleAngle, glm::vec3(1.0f, 0.3f, 0.1f));

    // Speed label
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", (int)speed);
    float labelW = 30.0f * m_scale;
    float labelH = 20.0f * m_scale;
    glm::mat4 label = glm::mat4(1.0f);
    label = glm::translate(label, glm::vec3(cx - labelW * 0.5f, cy - radius * 0.3f, 0.03f));
    label = glm::scale(label, glm::vec3(labelW, labelH, 1.0f));
    renderer->drawPlane(label, glm::vec3(0.9f, 0.9f, 0.9f));

    // "km/h" indicator
    glm::mat4 unit = glm::mat4(1.0f);
    unit = glm::translate(unit, glm::vec3(cx - 8.0f * m_scale, cy - radius * 0.6f, 0.03f));
    unit = glm::scale(unit, glm::vec3(16.0f * m_scale, 4.0f * m_scale, 1.0f));
    renderer->drawPlane(unit, glm::vec3(0.5f, 0.5f, 0.5f));
}

void HUD::drawTachometer(Renderer* renderer, const Motorcycle* bike) {
    float cx = 120.0f * m_scale;
    float cy = 140.0f * m_scale;
    float radius = 80.0f * m_scale;

    drawGaugeBackground(renderer, cx, cy, radius);

    float startAngle = glm::pi<float>() * 0.75f;
    float endAngle = glm::pi<float>() * 2.25f;
    float maxRPM = 12000.0f;
    float rpm = bike->rpm();
    float value = std::min(1.0f, rpm / maxRPM);
    float redline = 7500.0f / maxRPM;

    drawGaugeTicks(renderer, cx, cy, radius, 20, startAngle, endAngle, redline);
    drawGaugeArc(renderer, cx, cy, radius, startAngle, endAngle, value, redline,
                 glm::vec3(0.1f, 0.8f, 0.2f));

    float needleAngle = startAngle + (endAngle - startAngle) * value;
    drawNeedle(renderer, cx, cy, radius, needleAngle, glm::vec3(1.0f, 0.3f, 0.1f));

    float labelW = 20.0f * m_scale;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", (int)rpm);
    glm::mat4 label = glm::mat4(1.0f);
    label = glm::translate(label, glm::vec3(cx - labelW * 0.5f, cy - radius * 0.3f, 0.03f));
    label = glm::scale(label, glm::vec3(labelW, 12.0f * m_scale, 1.0f));
    renderer->drawPlane(label, glm::vec3(0.9f, 0.9f, 0.9f));

    // "x100" indicator
    glm::mat4 unit = glm::mat4(1.0f);
    unit = glm::translate(unit, glm::vec3(cx - 10.0f * m_scale, cy - radius * 0.6f, 0.03f));
    unit = glm::scale(unit, glm::vec3(20.0f * m_scale, 4.0f * m_scale, 1.0f));
    renderer->drawPlane(unit, glm::vec3(0.5f, 0.5f, 0.5f));
}

void HUD::drawGearIndicator(Renderer* renderer, const Motorcycle* bike) {
    float cx = m_width * 0.5f;
    float cy = 80.0f * m_scale;

    int gear = bike->gear();
    char buf[4];
    if (gear == 0) snprintf(buf, sizeof(buf), "N");
    else snprintf(buf, sizeof(buf), "%d", gear);

    float size = 36.0f * m_scale;

    // Gear background
    glm::mat4 bg = glm::mat4(1.0f);
    bg = glm::translate(bg, glm::vec3(cx - size * 0.5f, cy - size * 0.5f, 0.0f));
    bg = glm::scale(bg, glm::vec3(size, size, 1.0f));
    renderer->drawPlane(bg, glm::vec3(0.08f, 0.08f, 0.1f));

    // Clutch engagement indicator
    float clutch = 1.0f - bike->clutchEngagement();
    if (clutch > 0.05f) {
        glm::mat4 clutchBar = glm::mat4(1.0f);
        clutchBar = glm::translate(clutchBar, glm::vec3(cx - size * 0.5f, cy - size - 6.0f * m_scale, 0.01f));
        clutchBar = glm::scale(clutchBar, glm::vec3(size * clutch, 4.0f * m_scale, 1.0f));
        renderer->drawPlane(clutchBar, glm::vec3(0.8f, 0.5f, 0.1f));
    }

    // Shift indicator arrows
    if (bike->speed() > 0.5f && bike->rpm() > 7000.0f) {
        glm::mat4 upArrow = glm::mat4(1.0f);
        upArrow = glm::translate(upArrow, glm::vec3(cx + size * 0.8f, cy, 0.01f));
        upArrow = glm::scale(upArrow, glm::vec3(8.0f * m_scale, 8.0f * m_scale, 1.0f));
        renderer->drawPlane(upArrow, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void HUD::drawFuelGauge(Renderer* renderer, const Motorcycle* bike) {
    (void)bike;
    float cx = m_width * 0.5f;
    float cy = 20.0f * m_scale;
    float width = 120.0f * m_scale;
    float height = 8.0f * m_scale;

    // Background bar
    glm::mat4 bg = glm::mat4(1.0f);
    bg = glm::translate(bg, glm::vec3(cx - width * 0.5f, cy, 0.0f));
    bg = glm::scale(bg, glm::vec3(width, height, 1.0f));
    renderer->drawPlane(bg, glm::vec3(0.1f, 0.1f, 0.12f));

    // Fuel level (simulated 80% for now)
    float fuel = 0.8f;
    glm::mat4 fill = glm::mat4(1.0f);
    fill = glm::translate(fill, glm::vec3(cx - width * 0.5f, cy, 0.001f));
    fill = glm::scale(fill, glm::vec3(width * fuel, height, 1.0f));
    renderer->drawPlane(fill, glm::vec3(0.0f, 0.6f, 0.0f));
}

void HUD::drawStatusIndicators(Renderer* renderer, const Motorcycle* bike,
                               const World* world, bool headlightOn) {
    (void)world;
    float x = m_width * 0.5f - 60.0f * m_scale;
    float y = m_height - 30.0f * m_scale;
    float spacing = 50.0f * m_scale;

    // Engine status
    {
        glm::vec3 color;
        if (bike->isStalled()) color = glm::vec3(1.0f, 0.0f, 0.0f);
        else if (bike->engineRunning()) color = glm::vec3(0.0f, 1.0f, 0.0f);
        else color = glm::vec3(0.5f, 0.5f, 0.5f);

        glm::mat4 led = glm::mat4(1.0f);
        led = glm::translate(led, glm::vec3(x, y, 0.01f));
        led = glm::scale(led, glm::vec3(6.0f * m_scale, 6.0f * m_scale, 1.0f));
        renderer->drawPlane(led, color);
    }

    // Headlight
    {
        glm::vec3 color = headlightOn ? glm::vec3(1.0f, 0.95f, 0.7f) : glm::vec3(0.3f, 0.3f, 0.3f);
        glm::mat4 led = glm::mat4(1.0f);
        led = glm::translate(led, glm::vec3(x + spacing, y, 0.01f));
        led = glm::scale(led, glm::vec3(6.0f * m_scale, 6.0f * m_scale, 1.0f));
        renderer->drawPlane(led, color);
    }

    // Night indicator
    if (world && world->state().isNight) {
        glm::mat4 led = glm::mat4(1.0f);
        led = glm::translate(led, glm::vec3(x + spacing * 2.0f, y, 0.01f));
        led = glm::scale(led, glm::vec3(6.0f * m_scale, 6.0f * m_scale, 1.0f));
        renderer->drawPlane(led, glm::vec3(0.1f, 0.1f, 0.8f));
    }

    // Crash indicator
    if (bike->isCrashed()) {
        float flash = sinf(bike->crashInfo().impactForce * 10.0f) * 0.5f + 0.5f;
        glm::mat4 crashLed = glm::mat4(1.0f);
        crashLed = glm::translate(crashLed, glm::vec3(m_width * 0.5f, m_height * 0.5f, 0.01f));
        crashLed = glm::scale(crashLed, glm::vec3(30.0f * m_scale, 10.0f * m_scale, 1.0f));
        renderer->drawPlane(crashLed, glm::vec3(flash, 0.0f, 0.0f));
    }
}

void HUD::drawMinimap(Renderer* renderer, const Motorcycle* bike, const World* world) {
    if (!world || !world->roadNetwork()) return;

    float mapSize = 120.0f * m_scale;
    float mapWorldSize = 100.0f;
    float mapX = m_width - mapSize - 15.0f * m_scale;
    float mapY = m_height - mapSize - 150.0f * m_scale;
    float ppu = mapSize / mapWorldSize;

    glm::mat4 bg = glm::mat4(1.0f);
    bg = glm::translate(bg, glm::vec3(mapX + mapSize * 0.5f, mapY + mapSize * 0.5f, 0.0f));
    bg = glm::scale(bg, glm::vec3(mapSize, mapSize, 1.0f));
    renderer->drawPlane(bg, glm::vec3(0.08f, 0.08f, 0.1f));

    glm::mat4 border = glm::mat4(1.0f);
    border = glm::translate(border, glm::vec3(mapX + mapSize * 0.5f, mapY + mapSize * 0.5f, 0.001f));
    border = glm::scale(border, glm::vec3(mapSize + 2.0f, mapSize + 2.0f, 1.0f));
    renderer->drawPlane(border, glm::vec3(0.25f, 0.25f, 0.28f));

    float px = bike->position().x;
    float pz = bike->position().z;

    const auto* roadNet = world->roadNetwork();
    for (int si = 0; si < roadNet->segmentCount(); ++si) {
        const auto& seg = roadNet->getSegment(si);
        int steps = (seg.type == RoadType::Straight) ? 1 : 15;
        for (int i = 0; i < steps; ++i) {
            float t0 = (float)i / (float)steps;
            float t1 = (float)(i + 1) / (float)steps;
            glm::vec2 p0 = seg.evaluate(t0);
            glm::vec2 p1 = seg.evaluate(t1);

            float sx0 = mapX + mapSize * 0.5f + (p0.x - px) * ppu;
            float sy0 = mapY + mapSize * 0.5f + (p0.y - pz) * ppu;
            float sx1 = mapX + mapSize * 0.5f + (p1.x - px) * ppu;
            float sy1 = mapY + mapSize * 0.5f + (p1.y - pz) * ppu;

            float cx = (sx0 + sx1) * 0.5f;
            float cy = (sy0 + sy1) * 0.5f;
            float dx = sx1 - sx0;
            float dy = sy1 - sy0;
            float len = sqrtf(dx * dx + dy * dy);
            if (len < 1.0f) continue;

            float roadW = seg.width * ppu;
            glm::mat4 road = glm::mat4(1.0f);
            road = glm::translate(road, glm::vec3(cx, cy, 0.005f));
            road = glm::rotate(road, atan2f(dy, dx), glm::vec3(0.0f, 0.0f, 1.0f));
            road = glm::scale(road, glm::vec3(roadW, len, 1.0f));
            renderer->drawPlane(road, seg.isMainRoad ?
                glm::vec3(0.25f, 0.25f, 0.25f) : glm::vec3(0.2f, 0.2f, 0.2f));
        }
    }

    // Traffic
    for (const auto* v : world->traffic()->vehicles()) {
        if (!v->isActive()) continue;
        float vx = mapX + mapSize * 0.5f + (v->position().x - px) * ppu;
        float vy = mapY + mapSize * 0.5f + (v->position().z - pz) * ppu;
        if (vx < mapX || vx > mapX + mapSize || vy < mapY || vy > mapY + mapSize) continue;

        glm::mat4 car = glm::mat4(1.0f);
        car = glm::translate(car, glm::vec3(vx, vy, 0.01f));
        car = glm::scale(car, glm::vec3(2.5f * ppu, 1.8f * ppu, 1.0f));
        renderer->drawPlane(car, glm::vec3(0.7f, 0.2f, 0.15f));
    }

    // Player arrow
    float ppX = mapX + mapSize * 0.5f;
    float ppY = mapY + mapSize * 0.5f;
    float bikeAngle = bike->rotation().y;
    float arrowSize = 5.0f * m_scale;

    Vertex arrow[3] = {
        { glm::vec3(ppX + arrowSize * sinf(bikeAngle), ppY + arrowSize * cosf(bikeAngle), 0.02f), glm::vec3(0.0f), glm::vec2(0.0f) },
        { glm::vec3(ppX + arrowSize * 0.4f * sinf(bikeAngle + 2.5f), ppY + arrowSize * 0.4f * cosf(bikeAngle + 2.5f), 0.02f), glm::vec3(0.0f), glm::vec2(0.0f) },
        { glm::vec3(ppX + arrowSize * 0.4f * sinf(bikeAngle - 2.5f), ppY + arrowSize * 0.4f * cosf(bikeAngle - 2.5f), 0.02f), glm::vec3(0.0f), glm::vec2(0.0f) }
    };
    glm::mat4 identity(1.0f);
    renderer->drawTriangles(arrow, 3, identity, glm::vec3(0.0f, 1.0f, 0.0f));
}

void HUD::drawFPS(Renderer* renderer, int fps) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d FPS", fps);
    float fw = 28.0f * m_scale;
    float fh = 10.0f * m_scale;

    glm::mat4 fpsLabel = glm::mat4(1.0f);
    fpsLabel = glm::translate(fpsLabel, glm::vec3(10.0f, m_height - fh - 10.0f, 0.01f));
    fpsLabel = glm::scale(fpsLabel, glm::vec3(fw, fh, 1.0f));
    renderer->drawPlane(fpsLabel, fps > 30 ? glm::vec3(0.0f, 0.8f, 0.0f) : glm::vec3(1.0f, 0.3f, 0.1f));
}
