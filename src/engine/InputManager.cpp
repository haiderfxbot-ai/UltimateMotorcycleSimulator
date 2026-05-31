#include "InputManager.h"

InputManager::InputManager()
    : m_keyboardState(nullptr)
    , m_gamepad(nullptr)
{
    std::memset(m_prevKeyboardState, 0, sizeof(m_prevKeyboardState));

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
            m_inputState.quit = true;
        }
        if (e.type == SDL_CONTROLLERDEVICEADDED && !m_gamepad) {
            m_gamepad = SDL_GameControllerOpen(e.cdevice.which);
        }
        if (e.type == SDL_CONTROLLERDEVICEREMOVED && m_gamepad) {
            SDL_GameControllerClose(m_gamepad);
            m_gamepad = nullptr;
        }
    }

    m_inputState.gearUp = false;
    m_inputState.gearDown = false;
    m_inputState.startEngine = false;
    m_inputState.reset = false;

    processKeyboard();
    processGamepad();
}

void InputManager::processKeyboard() {
    if (!m_keyboardState) return;

    // Throttle
    m_inputState.throttle = keyHeld(SDL_SCANCODE_W) || keyHeld(SDL_SCANCODE_UP) ? 1.0f : 0.0f;

    // Rear brake
    m_inputState.rearBrake = keyHeld(SDL_SCANCODE_S) || keyHeld(SDL_SCANCODE_DOWN) ? 1.0f : 0.0f;

    // Front brake (space bar)
    m_inputState.frontBrake = keyHeld(SDL_SCANCODE_SPACE) ? 1.0f : 0.0f;

    // Clutch (left shift)
    m_inputState.clutch = keyHeld(SDL_SCANCODE_LSHIFT) || keyHeld(SDL_SCANCODE_RSHIFT) ? 1.0f : 0.0f;

    // Steer
    m_inputState.steer = 0.0f;
    if (keyHeld(SDL_SCANCODE_A) || keyHeld(SDL_SCANCODE_LEFT)) m_inputState.steer -= 1.0f;
    if (keyHeld(SDL_SCANCODE_D) || keyHeld(SDL_SCANCODE_RIGHT)) m_inputState.steer += 1.0f;

    // Gear up (Q / E)
    if (keyPressed(SDL_SCANCODE_Q)) m_inputState.gearUp = true;
    if (keyPressed(SDL_SCANCODE_E)) m_inputState.gearDown = true;

    // Start engine
    if (keyPressed(SDL_SCANCODE_RETURN) || keyPressed(SDL_SCANCODE_RCTRL)) m_inputState.startEngine = true;

    // Reset
    if (keyPressed(SDL_SCANCODE_R)) m_inputState.reset = true;

    // Quit
    if (keyPressed(SDL_SCANCODE_ESCAPE)) m_inputState.quit = true;
}

void InputManager::processGamepad() {
    if (!m_gamepad) return;

    float deadzone = 0.15f;

    m_inputState.throttle = std::max(m_inputState.throttle,
        std::max(0.0f, (SDL_GameControllerGetAxis(m_gamepad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f)));

    float lt = SDL_GameControllerGetAxis(m_gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0f;
    m_inputState.clutch = std::max(m_inputState.clutch, lt);

    float lx = SDL_GameControllerGetAxis(m_gamepad, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0f;
    if (std::abs(lx) > deadzone) {
        m_inputState.steer = lx;
    }

    if (SDL_GameControllerGetButton(m_gamepad, SDL_CONTROLLER_BUTTON_A)) {
        m_inputState.startEngine = true;
    }
    if (SDL_GameControllerGetButton(m_gamepad, SDL_CONTROLLER_BUTTON_B)) {
        m_inputState.reset = true;
    }
    if (SDL_GameControllerGetButton(m_gamepad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) {
        m_inputState.frontBrake = 1.0f;
    }
    if (SDL_GameControllerGetButton(m_gamepad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) {
        m_inputState.rearBrake = 1.0f;
    }
    if (SDL_GameControllerGetButton(m_gamepad, SDL_CONTROLLER_BUTTON_DPAD_UP) ||
        SDL_GameControllerGetButton(m_gamepad, SDL_CONTROLLER_BUTTON_Y)) {
        m_inputState.gearUp = true;
    }
    if (SDL_GameControllerGetButton(m_gamepad, SDL_CONTROLLER_BUTTON_DPAD_DOWN) ||
        SDL_GameControllerGetButton(m_gamepad, SDL_CONTROLLER_BUTTON_X)) {
        m_inputState.gearDown = true;
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
