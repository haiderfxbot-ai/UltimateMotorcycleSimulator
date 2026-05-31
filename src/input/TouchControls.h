#ifndef TOUCH_CONTROLS_H
#define TOUCH_CONTROLS_H

#include <cstdint>
#include <vector>

struct InputState;

struct TouchButton {
    float x, y, w, h;
    float value;
    int fingerId;
    bool pressed;
    bool wasPressed;
    enum Type { Throttle, Brake, FrontBrake, SteerLeft, SteerRight, Clutch, GearUp, GearDown, StartEngine, Reset };
    Type type;
};

class Renderer;

class TouchControls {
public:
    TouchControls();
    ~TouchControls();

    void init(int screenWidth, int screenHeight);
    void processEvent(const void* event);
    void update(InputState& state);
    void render(Renderer* renderer);

    bool isActive() const { return m_active; }
    void setActive(bool a) { m_active = a; }
    int touchCount() const { return (int)m_activeFingers.size(); }

private:
    void setupButtons();
    void renderButton(Renderer* renderer, const TouchButton& btn);
    int getButtonAt(float x, float y) const;

    std::vector<TouchButton> m_buttons;
    std::vector<int> m_activeFingers;
    int m_width;
    int m_height;
    float m_scale;
    bool m_active;
};

#endif
