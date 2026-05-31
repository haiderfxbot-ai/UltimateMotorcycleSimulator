#include "GameStateManager.h"
#include "../renderer/Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLES3/gl3.h>

GameStateManager::GameStateManager()
    : m_state(GameState::Menu)
    , m_selectedItem(0)
    , m_width(1280)
    , m_height(720)
    , m_animTimer(0.0f)
    , m_gameOverTimer(0.0f)
{}

GameStateManager::~GameStateManager() {}

void GameStateManager::init(int screenWidth, int screenHeight) {
    m_width = screenWidth;
    m_height = screenHeight;
}

void GameStateManager::update(float dt) {
    m_animTimer += dt;
    if (m_state == GameState::GameOver) {
        m_gameOverTimer += dt;
    } else {
        m_gameOverTimer = 0.0f;
    }
}

void GameStateManager::selectUp() {
    if (m_state == GameState::Menu) {
        m_selectedItem = (m_selectedItem - 1 + 2) % 2;
    } else if (m_state == GameState::Paused) {
        m_selectedItem = (m_selectedItem - 1 + 2) % 2;
    }
}

void GameStateManager::selectDown() {
    if (m_state == GameState::Menu) {
        m_selectedItem = (m_selectedItem + 1) % 2;
    } else if (m_state == GameState::Paused) {
        m_selectedItem = (m_selectedItem + 1) % 2;
    }
}

void GameStateManager::render(Renderer* renderer) {
    if (m_state == GameState::Menu) {
        renderMenu(renderer);
    } else if (m_state == GameState::Paused) {
        renderPause(renderer);
    } else if (m_state == GameState::GameOver) {
        renderGameOver(renderer);
    }
}

void GameStateManager::drawButton(Renderer* renderer, const std::string& text,
                                   float x, float y, float w, float h, bool selected) {
    glm::mat4 ortho = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height, -1.0f, 1.0f);
    renderer->setProjectionView(ortho);

    float pulse = selected ? (0.8f + 0.2f * std::sin(m_animTimer * 4.0f)) : 1.0f;
    glm::vec3 btnColor = selected ?
        glm::vec3(0.2f * pulse, 0.5f * pulse, 0.7f * pulse) :
        glm::vec3(0.3f, 0.3f, 0.35f);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x + w * 0.5f, y + h * 0.5f, 0.0f));
    model = glm::scale(model, glm::vec3(w * 0.9f, h * 0.9f, 1.0f));
    renderer->drawPlane(model, btnColor);
}

void GameStateManager::renderMenu(Renderer* renderer) {
    glDisable(GL_DEPTH_TEST);

    float cx = m_width * 0.5f;
    float cy = m_height * 0.5f;
    float bw = 200.0f;
    float bh = 50.0f;
    float spacing = 15.0f;

    // Title
    float titleY = cy + 80.0f;
    {
        glm::mat4 ortho = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height, -1.0f, 1.0f);
        renderer->setProjectionView(ortho);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(cx, titleY, 0.0f));
        model = glm::scale(model, glm::vec3(280.0f, 30.0f, 1.0f));
        float pulse = 0.7f + 0.3f * std::sin(m_animTimer * 2.0f);
        renderer->drawPlane(model, glm::vec3(0.8f * pulse, 0.4f * pulse, 0.1f * pulse));
    }

    // Start Game
    float startY = cy - spacing * 0.5f - bh;
    drawButton(renderer, "START GAME", cx - bw * 0.5f, startY, bw, bh, m_selectedItem == 0);

    // Quit
    float quitY = cy - spacing * 0.5f;
    drawButton(renderer, "QUIT", cx - bw * 0.5f, quitY, bw, bh, m_selectedItem == 1);

    // Controls hint
    {
        glm::mat4 ortho = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height, -1.0f, 1.0f);
        renderer->setProjectionView(ortho);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(cx, 40.0f, 0.0f));
        model = glm::scale(model, glm::vec3(300.0f, 20.0f, 1.0f));
        renderer->drawPlane(model, glm::vec3(0.2f, 0.2f, 0.2f));
    }

    glEnable(GL_DEPTH_TEST);
}

void GameStateManager::renderPause(Renderer* renderer) {
    glDisable(GL_DEPTH_TEST);

    float cx = m_width * 0.5f;
    float cy = m_height * 0.5f;
    float bw = 200.0f;
    float bh = 50.0f;
    float spacing = 15.0f;

    // Semi-transparent overlay
    glm::mat4 ortho = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height, -1.0f, 1.0f);
    renderer->setProjectionView(ortho);
    glm::mat4 overlay = glm::mat4(1.0f);
    overlay = glm::translate(overlay, glm::vec3(m_width * 0.5f, m_height * 0.5f, 0.0f));
    overlay = glm::scale(overlay, glm::vec3(m_width, m_height, 1.0f));
    renderer->drawPlane(overlay, glm::vec3(0.0f, 0.0f, 0.0f) * 0.4f);

    // Pause title
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(cx, cy + 80.0f, 0.0f));
        model = glm::scale(model, glm::vec3(160.0f, 30.0f, 1.0f));
        renderer->drawPlane(model, glm::vec3(0.7f, 0.7f, 0.2f));
    }

    // Resume
    float resumeY = cy - spacing * 0.5f - bh;
    drawButton(renderer, "RESUME", cx - bw * 0.5f, resumeY, bw, bh, m_selectedItem == 0);

    // Quit to Menu
    float quitY = cy - spacing * 0.5f;
    drawButton(renderer, "QUIT TO MENU", cx - bw * 0.5f, quitY, bw, bh, m_selectedItem == 1);

    glEnable(GL_DEPTH_TEST);
}

void GameStateManager::renderGameOver(Renderer* renderer) {
    glDisable(GL_DEPTH_TEST);

    float cx = m_width * 0.5f;
    float cy = m_height * 0.5f;

    glm::mat4 ortho = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height, -1.0f, 1.0f);
    renderer->setProjectionView(ortho);

    // Overlay
    {
        glm::mat4 overlay = glm::mat4(1.0f);
        overlay = glm::translate(overlay, glm::vec3(m_width * 0.5f, m_height * 0.5f, 0.0f));
        overlay = glm::scale(overlay, glm::vec3(m_width, m_height, 1.0f));
        renderer->drawPlane(overlay, glm::vec3(0.3f, 0.0f, 0.0f) * 0.5f);
    }

    // GAME OVER text
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(cx, cy + 40.0f, 0.0f));
        model = glm::scale(model, glm::vec3(250.0f, 35.0f, 1.0f));
        float pulse = 0.7f + 0.3f * std::sin(m_animTimer * 3.0f);
        renderer->drawPlane(model, glm::vec3(1.0f * pulse, 0.1f * pulse, 0.1f * pulse));
    }

    // Timer bar
    float barW = 200.0f;
    float barH = 8.0f;
    float barX = cx - barW * 0.5f;
    float barY = cy - 10.0f;
    int maxTimer = 4;
    float fill = std::min(1.0f, m_gameOverTimer / (float)maxTimer);

    // Background bar
    {
        glm::mat4 bg = glm::mat4(1.0f);
        bg = glm::translate(bg, glm::vec3(cx, barY, 0.0f));
        bg = glm::scale(bg, glm::vec3(barW, barH, 1.0f));
        renderer->drawPlane(bg, glm::vec3(0.2f, 0.2f, 0.2f));
    }
    // Fill bar
    {
        glm::mat4 fg = glm::mat4(1.0f);
        fg = glm::translate(fg, glm::vec3(cx - barW * 0.5f * (1.0f - fill), barY, 0.0f));
        fg = glm::scale(fg, glm::vec3(barW * fill, barH, 1.0f));
        renderer->drawPlane(fg, glm::vec3(1.0f, 0.3f, 0.3f));
    }

    // Hint text
    {
        glm::mat4 hint = glm::mat4(1.0f);
        hint = glm::translate(hint, glm::vec3(cx, 30.0f, 0.0f));
        hint = glm::scale(hint, glm::vec3(250.0f, 16.0f, 1.0f));
        renderer->drawPlane(hint, glm::vec3(0.3f, 0.3f, 0.3f));
    }

    glEnable(GL_DEPTH_TEST);
}
