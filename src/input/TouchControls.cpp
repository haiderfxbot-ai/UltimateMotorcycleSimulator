#include "TouchControls.h"
#include "../renderer/Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

#ifdef ANDROID
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

TouchControls::TouchControls()
    : m_width(1280)
    , m_height(720)
    , m_scale(1.0f)
    , m_active(false)
{}

TouchControls::~TouchControls() {}

void TouchControls::init(int screenWidth, int screenHeight) {
    m_width = screenWidth;
    m_height = screenHeight;
    m_scale = (float)screenHeight / 720.0f;
    setupButtons();
}

void TouchControls::setupButtons() {
    m_buttons.clear();
    float s = m_scale;
    float bw = 60.0f * s;
    float bh = 60.0f * s;
    float pad = 10.0f * s;
    float edgePad = 15.0f * s;

    // Right side: throttle (top), brake (bottom)
    {
        TouchButton btn;
        btn.type = TouchButton::Throttle;
        btn.x = m_width - edgePad - bw;
        btn.y = m_height - edgePad - bh * 2.0f - pad;
        btn.w = bw;
        btn.h = bh;
        btn.value = 0.0f;
        btn.fingerId = -1;
        btn.pressed = false;
        btn.wasPressed = false;
        m_buttons.push_back(btn);
    }
    {
        TouchButton btn;
        btn.type = TouchButton::Brake;
        btn.x = m_width - edgePad - bw;
        btn.y = m_height - edgePad - bh;
        btn.w = bw;
        btn.h = bh;
        btn.value = 0.0f;
        btn.fingerId = -1;
        btn.pressed = false;
        btn.wasPressed = false;
        m_buttons.push_back(btn);
    }

    // Left side: steer left (left half), steer right (right half)
    bw = 70.0f * s;
    bh = 80.0f * s;
    {
        TouchButton btn;
        btn.type = TouchButton::SteerLeft;
        btn.x = edgePad;
        btn.y = m_height - edgePad - bh;
        btn.w = bw;
        btn.h = bh;
        btn.value = 0.0f;
        btn.fingerId = -1;
        btn.pressed = false;
        btn.wasPressed = false;
        m_buttons.push_back(btn);
    }
    {
        TouchButton btn;
        btn.type = TouchButton::SteerRight;
        btn.x = edgePad + bw + pad;
        btn.y = m_height - edgePad - bh;
        btn.w = bw;
        btn.h = bh;
        btn.value = 0.0f;
        btn.fingerId = -1;
        btn.pressed = false;
        btn.wasPressed = false;
        m_buttons.push_back(btn);
    }

    // Clutch (center-left area)
    bw = 50.0f * s;
    bh = 70.0f * s;
    {
        TouchButton btn;
        btn.type = TouchButton::Clutch;
        btn.x = edgePad + bw * 2.0f + pad * 2.0f;
        btn.y = m_height - edgePad - bh;
        btn.w = bw;
        btn.h = bh;
        btn.value = 0.0f;
        btn.fingerId = -1;
        btn.pressed = false;
        btn.wasPressed = false;
        m_buttons.push_back(btn);
    }

    // Front brake (left side, above steer)
    bw = 50.0f * s;
    bh = 40.0f * s;
    {
        TouchButton btn;
        btn.type = TouchButton::FrontBrake;
        btn.x = edgePad + 20.0f * s;
        btn.y = m_height - edgePad - bh - 90.0f * s;
        btn.w = bw;
        btn.h = bh;
        btn.value = 0.0f;
        btn.fingerId = -1;
        btn.pressed = false;
        btn.wasPressed = false;
        m_buttons.push_back(btn);
    }

    // Gear Up / Down (center)
    bw = 35.0f * s;
    bh = 35.0f * s;
    {
        TouchButton btn;
        btn.type = TouchButton::GearUp;
        btn.x = m_width * 0.5f - bw - pad * 0.5f;
        btn.y = m_height - edgePad - bh;
        btn.w = bw;
        btn.h = bh;
        btn.value = 0.0f;
        btn.fingerId = -1;
        btn.pressed = false;
        btn.wasPressed = false;
        m_buttons.push_back(btn);
    }
    {
        TouchButton btn;
        btn.type = TouchButton::GearDown;
        btn.x = m_width * 0.5f + pad * 0.5f;
        btn.y = m_height - edgePad - bh;
        btn.w = bw;
        btn.h = bh;
        btn.value = 0.0f;
        btn.fingerId = -1;
        btn.pressed = false;
        btn.wasPressed = false;
        m_buttons.push_back(btn);
    }

    // Start Engine / Reset (bottom center area)
    bw = 50.0f * s;
    bh = 30.0f * s;
    {
        TouchButton btn;
        btn.type = TouchButton::StartEngine;
        btn.x = m_width * 0.5f - bw - pad;
        btn.y = edgePad + 10.0f * s;
        btn.w = bw;
        btn.h = bh;
        btn.value = 0.0f;
        btn.fingerId = -1;
        btn.pressed = false;
        btn.wasPressed = false;
        m_buttons.push_back(btn);
    }
    {
        TouchButton btn;
        btn.type = TouchButton::Reset;
        btn.x = m_width * 0.5f + pad;
        btn.y = edgePad + 10.0f * s;
        btn.w = bw;
        btn.h = bh;
        btn.value = 0.0f;
        btn.fingerId = -1;
        btn.pressed = false;
        btn.wasPressed = false;
        m_buttons.push_back(btn);
    }
}

int TouchControls::getButtonAt(float x, float y) const {
    for (int i = 0; i < (int)m_buttons.size(); ++i) {
        const auto& btn = m_buttons[i];
        if (x >= btn.x && x <= btn.x + btn.w &&
            y >= btn.y && y <= btn.y + btn.h) {
            return i;
        }
    }
    return -1;
}

void TouchControls::processEvent(const void* event) {
    const SDL_Event* e = (const SDL_Event*)event;

    if (e->type == SDL_FINGERDOWN) {
        m_active = true;
        float tx = e->tfinger.x * m_width;
        float ty = e->tfinger.y * m_height;
        int idx = getButtonAt(tx, ty);
        if (idx >= 0) {
            auto& btn = m_buttons[idx];
            btn.pressed = true;
            btn.wasPressed = true;
            btn.fingerId = e->tfinger.fingerId;
            if (btn.type == TouchButton::Throttle ||
                btn.type == TouchButton::Brake ||
                btn.type == TouchButton::Clutch ||
                btn.type == TouchButton::FrontBrake) {
                btn.value = 1.0f;
            }
            m_activeFingers.push_back(e->tfinger.fingerId);
        }
    }

    if (e->type == SDL_FINGERUP) {
        float tx = e->tfinger.x * m_width;
        float ty = e->tfinger.y * m_height;
        for (auto& btn : m_buttons) {
            if (btn.fingerId == e->tfinger.fingerId) {
                btn.pressed = false;
                btn.value = 0.0f;
                btn.fingerId = -1;
            }
        }
        m_activeFingers.erase(
            std::remove(m_activeFingers.begin(), m_activeFingers.end(),
                        e->tfinger.fingerId),
            m_activeFingers.end());
        if (m_activeFingers.empty()) m_active = false;
    }

    if (e->type == SDL_FINGERMOTION) {
        float tx = e->tfinger.x * m_width;
        float ty = e->tfinger.y * m_height;

        for (auto& btn : m_buttons) {
            if (btn.fingerId == e->tfinger.fingerId) {
                bool inside = (tx >= btn.x && tx <= btn.x + btn.w &&
                               ty >= btn.y && ty <= btn.y + btn.h);
                btn.pressed = inside;
                if (btn.type == TouchButton::Throttle ||
                    btn.type == TouchButton::Brake ||
                    btn.type == TouchButton::Clutch ||
                    btn.type == TouchButton::FrontBrake) {
                    btn.value = inside ? 1.0f : 0.0f;
                }
            }
        }
    }
}

void TouchControls::update(InputState& state) {
    if (!m_active) return;

    for (const auto& btn : m_buttons) {
        switch (btn.type) {
            case TouchButton::Throttle:
                state.throttle = std::max(state.throttle, btn.value);
                break;
            case TouchButton::Brake:
                state.rearBrake = std::max(state.rearBrake, btn.value);
                break;
            case TouchButton::FrontBrake:
                state.frontBrake = std::max(state.frontBrake, btn.value);
                break;
            case TouchButton::SteerLeft:
                if (btn.pressed) state.steer = std::min(state.steer, -1.0f);
                break;
            case TouchButton::SteerRight:
                if (btn.pressed) state.steer = std::max(state.steer, 1.0f);
                break;
            case TouchButton::Clutch:
                state.clutch = std::max(state.clutch, btn.value);
                break;
            case TouchButton::GearUp:
                if (btn.wasPressed) state.gearUp = true;
                break;
            case TouchButton::GearDown:
                if (btn.wasPressed) state.gearDown = true;
                break;
            case TouchButton::StartEngine:
                if (btn.wasPressed) state.startEngine = true;
                break;
            case TouchButton::Reset:
                if (btn.wasPressed) state.reset = true;
                break;
        }
    }

    // Clear momentary flags
    for (auto& btn : m_buttons) {
        if (btn.wasPressed) btn.wasPressed = false;
    }
}

void TouchControls::render(Renderer* renderer) {
    if (!m_active && m_activeFingers.empty()) return;

    glDisable(GL_DEPTH_TEST);

    glm::mat4 ortho = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height, -1.0f, 1.0f);
    renderer->setProjectionView(ortho);

    for (const auto& btn : m_buttons) {
        renderButton(renderer, btn);
    }

    glEnable(GL_DEPTH_TEST);
}

void TouchControls::renderButton(Renderer* renderer, const TouchButton& btn) {
    glm::vec3 color;
    float alpha = btn.pressed ? 0.6f : 0.25f;

    switch (btn.type) {
        case TouchButton::Throttle:    color = glm::vec3(0.0f, 0.7f, 0.0f); break;
        case TouchButton::Brake:       color = glm::vec3(0.7f, 0.0f, 0.0f); break;
        case TouchButton::FrontBrake:  color = glm::vec3(0.9f, 0.3f, 0.0f); break;
        case TouchButton::SteerLeft:   color = glm::vec3(0.3f, 0.3f, 0.7f); break;
        case TouchButton::SteerRight:  color = glm::vec3(0.3f, 0.3f, 0.7f); break;
        case TouchButton::Clutch:      color = glm::vec3(0.5f, 0.5f, 0.5f); break;
        case TouchButton::GearUp:      color = glm::vec3(0.6f, 0.6f, 0.2f); break;
        case TouchButton::GearDown:    color = glm::vec3(0.6f, 0.4f, 0.2f); break;
        case TouchButton::StartEngine: color = glm::vec3(0.2f, 0.6f, 0.6f); break;
        case TouchButton::Reset:       color = glm::vec3(0.7f, 0.2f, 0.2f); break;
    }

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(btn.x + btn.w * 0.5f, btn.y + btn.h * 0.5f, 0.0f));
    model = glm::scale(model, glm::vec3(btn.w * 0.9f, btn.h * 0.9f, 1.0f));
    renderer->drawPlane(model, color * alpha);
}
