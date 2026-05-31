#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <cstdint>

class InputManager;
class Renderer;
class Motorcycle;
class Camera;
class World;
class AudioManager;
class HUD;
class TouchControls;
class WorldCollision;
class GameStateManager;
struct InputState;

class GameEngine {
public:
    GameEngine();
    ~GameEngine();

    bool init(int width, int height, const char* title);
    void run();
    void shutdown();

    double deltaTime() const { return m_deltaTime; }
    double totalTime() const { return m_totalTime; }
    int fps() const { return m_fps; }

private:
    void processInput();
    void update(double dt);
    void render();
    void calculateFPS();

    bool m_running;
    double m_deltaTime;
    double m_totalTime;
    int m_fps;
    int m_frameCount;
    double m_fpsTimer;

    InputManager* m_input;
    Renderer* m_renderer;
    Motorcycle* m_bike;
    Camera* m_camera;
    World* m_world;
    AudioManager* m_audio;
    HUD* m_hud;
    TouchControls* m_touch;
    WorldCollision* m_collision;
    GameStateManager* m_stateManager;
};

#endif
