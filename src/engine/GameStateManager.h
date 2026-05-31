#ifndef GAME_STATE_MANAGER_H
#define GAME_STATE_MANAGER_H

#include <glm/glm.hpp>
#include <string>

class Renderer;

enum class GameState {
    Menu,
    Playing,
    Paused,
    GameOver
};

class GameStateManager {
public:
    GameStateManager();
    ~GameStateManager();

    void init(int screenWidth, int screenHeight);
    void update(float dt);
    void render(Renderer* renderer);

    GameState state() const { return m_state; }
    void setState(GameState s) { m_state = s; }

    int selectedItem() const { return m_selectedItem; }
    void selectUp();
    void selectDown();
    int menuItemCount() const { return 2; }

    float gameOverTimer() const { return m_gameOverTimer; }

private:
    void renderMenu(Renderer* renderer);
    void renderPause(Renderer* renderer);
    void renderGameOver(Renderer* renderer);
    void drawButton(Renderer* renderer, const std::string& text, float x, float y, float w, float h, bool selected);

    GameState m_state;
    int m_selectedItem;
    int m_width;
    int m_height;
    float m_animTimer;
    float m_gameOverTimer;
};

#endif
