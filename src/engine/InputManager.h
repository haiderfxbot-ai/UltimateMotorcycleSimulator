#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <cstdint>
#include <cstring>

#ifdef ANDROID
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

struct InputState {
    float throttle = 0.0f;
    float rearBrake = 0.0f;
    float frontBrake = 0.0f;
    float steer = 0.0f;
    float clutch = 0.0f;
    bool gearUp = false;
    bool gearDown = false;
    bool startEngine = false;
    bool reset = false;
    bool quit = false;
};

class InputManager {
public:
    InputManager();
    ~InputManager();

    void poll();
    InputState getInputState() const { return m_inputState; }

    bool keyPressed(SDL_Scancode code) const;
    bool keyHeld(SDL_Scancode code) const;
    bool keyReleased(SDL_Scancode code) const;
    bool quitRequested() const { return m_inputState.quit; }

private:
    void processKeyboard();
    void processGamepad();

    InputState m_inputState;
    const Uint8* m_keyboardState;
    Uint8 m_prevKeyboardState[SDL_NUM_SCANCODES];

    SDL_GameController* m_gamepad;
};

#endif
