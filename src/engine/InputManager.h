#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <cstdint>
#include <cstring>

#ifdef ANDROID
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

class InputManager {
public:
    InputManager();
    ~InputManager();

    void poll();
    bool keyPressed(SDL_Scancode code) const;
    bool keyHeld(SDL_Scancode code) const;
    bool keyReleased(SDL_Scancode code) const;
    bool quitRequested() const { return m_quit; }

    float axisValue(int axis) const;

private:
    const Uint8* m_keyboardState;
    Uint8 m_prevKeyboardState[SDL_NUM_SCANCODES];
    bool m_quit;

    SDL_GameController* m_gamepad;
    float m_axes[6];
};

#endif
