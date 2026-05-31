#ifndef RENDERER_H
#define RENDERER_H

#ifdef ANDROID
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include <GLES3/gl3.h>
#include <glm/glm.hpp>

class Camera;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

struct LightingUniforms {
    glm::vec3 lightDir = glm::vec3(0.5f, 0.8f, 0.3f);
    glm::vec3 ambientColor = glm::vec3(0.6f, 0.7f, 1.0f);
    glm::vec3 sunColor = glm::vec3(1.0f, 0.95f, 0.85f);
    float ambientIntensity = 0.5f;
    float sunIntensity = 1.0f;
    float fogDensity = 0.0f;
    glm::vec3 fogColor = glm::vec3(0.53f, 0.81f, 0.92f);
    float rainIntensity = 0.0f;
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(int width, int height, const char* title);
    void beginFrame();
    void endFrame();
    void shutdown();

    SDL_Window* window() const { return m_window; }
    int width() const { return m_width; }
    int height() const { return m_height; }

    GLuint program() const { return m_program; }

    void setProjectionView(const glm::mat4& pv);
    void setLighting(const LightingUniforms& lighting);

    void drawTriangles(const Vertex* verts, int count, const glm::mat4& model, const glm::vec3& color);
    void drawPlane(const glm::mat4& model, const glm::vec3& color);
    void drawBox(const glm::mat4& model, const glm::vec3& color);
    void drawCylinder(const glm::mat4& model, const glm::vec3& color, float radius, float height, int segments);
    void drawSphere(const glm::mat4& model, const glm::vec3& color, int subdivisions = 12);
    void drawCone(const glm::mat4& model, const glm::vec3& color, float radius, float height, int segments = 12);
    void drawRainOverlay();

private:
    bool loadShaders();
    bool createGeometry();
    GLuint compileShader(GLenum type, const char* source);

    SDL_Window* m_window;
    SDL_GLContext m_glContext;
    int m_width;
    int m_height;

    GLuint m_program;
    GLuint m_vao;
    GLuint m_vbo;

    GLint m_uPV;
    GLint m_uModel;
    GLint m_uColor;
    GLint m_uLightDir;
    GLint m_uAmbientColor;
    GLint m_uAmbientIntensity;
    GLint m_uSunIntensity;
    GLint m_uFogDensity;
    GLint m_uFogColor;
    GLint m_uRainIntensity;
    GLint m_uTime;

    GLuint m_planeVAO, m_planeVBO;
    GLuint m_boxVAO, m_boxVBO, m_boxIBO;
    GLuint m_cylVAO, m_cylVBO;
    int m_cylIndexCount;

    GLuint m_rainVAO, m_rainVBO;
    int m_rainCount;
    float m_rainTimer;

    LightingUniforms m_lighting;
};

#endif
