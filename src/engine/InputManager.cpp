#include "InputManager.h"

InputManager::InputManager()
    : m_keyboardState(nullptr)
    , m_quit(false)
    , m_gamepad(nullptr)
{
    std::memset(m_prevKeyboardState, 0, sizeof(m_prevKeyboardState));
    std::memset(m_axes, 0, sizeof(m_axes));

    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            m_gamepad = SDL_GameControllerOpen(i);
            if (m_gamepad) break;
        }
    }
}

InputManager::~InputManager() {
    if (m_gamepad) {
        SDL_GameControllerClose(m_gamepad);
    }
}

void InputManager::poll() {
    if (m_keyboardState) {
        std::memcpy(m_prevKeyboardState, m_keyboardState, SDL_NUM_SCANCODES);
    }
    m_keyboardState = SDL_GetKeyboardState(nullptr);

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            m_quit = true;
        }
        if (e.type == SDL_CONTROLLERDEVICEADDED && !m_gamepad) {
            m_gamepad = SDL_GameControllerOpen(e.cdevice.which);
        }
        if (e.type == SDL_CONTROLLERDEVICEREMOVED && m_gamepad) {
            SDL_GameControllerClose(m_gamepad);
            m_gamepad = nullptr;
        }
    }

    if (m_gamepad) {
        m_axes[0] = SDL_GameControllerGetAxis(m_gamepad, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
        m_axes[1] = SDL_GameControllerGetAxis(m_gamepad, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0f;
        m_axes[2] = SDL_GameControllerGetAxis(m_gamepad, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0f;
        m_axes[3] = SDL_GameControllerGetAxis(m_gamepad, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0f;
        m_axes[4] = SDL_GameControllerGetAxis(m_gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f;
        m_axes[5] = SDL_GameControllerGetAxis(m_gamepad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;
    }
}

bool InputManager::keyPressed(SDL_Scancode code) const {
    if (!m_keyboardState) return false;
    return m_keyboardState[code] && !m_prevKeyboardState[code];
}

bool InputManager::keyHeld(SDL_Scancode code) const {
    if (!m_keyboardState) return false;
    return m_keyboardState[code];
}

bool InputManager::keyReleased(SDL_Scancode code) const {
    if (!m_keyboardState) return false;
    return !m_keyboardState[code] && m_prevKeyboardState[code];
}

float InputManager::axisValue(int axis) const {
    if (axis >= 0 && axis < 6) return m_axes[axis];
    return 0.0f;
}
