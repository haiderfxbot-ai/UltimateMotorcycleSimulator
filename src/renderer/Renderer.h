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
    GLuint vao() const { return m_vao; }
    GLuint vbo() const { return m_vbo; }

    void setProjectionView(const glm::mat4& pv);

    void drawTriangles(const Vertex* verts, int count, const glm::mat4& model, const glm::vec3& color);
    void drawPlane(const glm::mat4& model, const glm::vec3& color);
    void drawBox(const glm::mat4& model, const glm::vec3& color);
    void drawCylinder(const glm::mat4& model, const glm::vec3& color, float radius, float height, int segments);

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
    GLuint m_ibo;

    GLint m_uPV;
    GLint m_uModel;
    GLint m_uColor;

    GLuint m_planeVAO, m_planeVBO;
    GLuint m_boxVAO, m_boxVBO, m_boxIBO;
    GLuint m_cylVAO, m_cylVBO;
    int m_cylIndexCount;
};

#endif
