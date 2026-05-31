#ifndef HUD_H
#define HUD_H

#include <glm/glm.hpp>

class Renderer;
class Motorcycle;
class World;

class HUD {
public:
    HUD();
    ~HUD();

    void init(int screenWidth, int screenHeight);
    void render(Renderer* renderer, const Motorcycle* bike, const World* world,
                int fps, bool headlightOn);

private:
    void drawGaugeArc(Renderer* renderer, float cx, float cy, float radius,
                      float startAngle, float endAngle, float value,
                      float redlineStart, const glm::vec3& color);
    void drawNeedle(Renderer* renderer, float cx, float cy, float radius,
                    float angle, const glm::vec3& color);
    void drawSpeedometer(Renderer* renderer, const Motorcycle* bike);
    void drawTachometer(Renderer* renderer, const Motorcycle* bike);
    void drawGearIndicator(Renderer* renderer, const Motorcycle* bike);
    void drawFuelGauge(Renderer* renderer, const Motorcycle* bike);
    void drawStatusIndicators(Renderer* renderer, const Motorcycle* bike,
                              const World* world, bool headlightOn);
    void drawFPS(Renderer* renderer, int fps);
    void drawMinimap(Renderer* renderer, const Motorcycle* bike, const World* world);

    float drawGaugeBackground(Renderer* renderer, float cx, float cy, float radius);
    void drawGaugeTicks(Renderer* renderer, float cx, float cy, float radius,
                        int numTicks, float startAngle, float endAngle,
                        float redlineStart);

    int m_width;
    int m_height;
    float m_scale;
};

#endif
