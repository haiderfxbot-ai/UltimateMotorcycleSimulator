#include "Renderer.h"
#include "../engine/Camera.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <glm/gtc/constants.hpp>

static const char* VERTEX_SRC = R"(
#version 300 es
precision highp float;
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uPV;
uniform mat4 uModel;

out vec3 vNormal;
out vec3 vFragPos;
out float vFogDepth;

void main() {
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    vFragPos = worldPos.xyz;
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;
    gl_Position = uPV * worldPos;
    vFogDepth = gl_Position.z / gl_Position.w;
}
)";

static const char* FRAGMENT_SRC = R"(
#version 300 es
precision highp float;
in vec3 vNormal;
in vec3 vFragPos;
in float vFogDepth;

uniform vec3 uColor;
uniform vec3 uLightDir;
uniform vec3 uAmbientColor;
uniform float uAmbientIntensity;
uniform float uSunIntensity;
uniform float uFogDensity;
uniform vec3 uFogColor;
uniform float uRainIntensity;
uniform float uTime;

out vec4 fragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);

    float diff = max(dot(N, L), 0.0);
    vec3 lighting = uAmbientColor * uAmbientIntensity + uColor * diff * uSunIntensity;
    vec3 color = uColor * (uAmbientIntensity + diff * uSunIntensity);

    float fogFactor = 1.0 - exp(-uFogDensity * uFogDensity * vFogDepth * vFogDepth);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    color = mix(color, uFogColor, fogFactor);

    fragColor = vec4(color, 1.0);
}
)";

Renderer::Renderer()
    : m_window(nullptr)
    , m_glContext(nullptr)
    , m_width(1280)
    , m_height(720)
    , m_program(0)
    , m_vao(0)
    , m_vbo(0)
    , m_uPV(-1)
    , m_uModel(-1)
    , m_uColor(-1)
    , m_uLightDir(-1)
    , m_uAmbientColor(-1)
    , m_uAmbientIntensity(-1)
    , m_uSunIntensity(-1)
    , m_uFogDensity(-1)
    , m_uFogColor(-1)
    , m_uRainIntensity(-1)
    , m_uTime(-1)
    , m_planeVAO(0)
    , m_planeVBO(0)
    , m_boxVAO(0)
    , m_boxVBO(0)
    , m_boxIBO(0)
    , m_cylVAO(0)
    , m_cylVBO(0)
    , m_cylIndexCount(0)
    , m_rainVAO(0)
    , m_rainVBO(0)
    , m_rainCount(200)
    , m_rainTimer(0.0f)
{}

Renderer::~Renderer() { shutdown(); }

GLuint Renderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        SDL_Log("Shader compile error: %s", log);
        return 0;
    }
    return shader;
}

bool Renderer::loadShaders() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, VERTEX_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SRC);
    if (!vs || !fs) return false;

    m_program = glCreateProgram();
    glAttachShader(m_program, vs);
    glAttachShader(m_program, fs);
    glLinkProgram(m_program);

    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(m_program, sizeof(log), nullptr, log);
        SDL_Log("Program link error: %s", log);
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    m_uPV = glGetUniformLocation(m_program, "uPV");
    m_uModel = glGetUniformLocation(m_program, "uModel");
    m_uColor = glGetUniformLocation(m_program, "uColor");
    m_uLightDir = glGetUniformLocation(m_program, "uLightDir");
    m_uAmbientColor = glGetUniformLocation(m_program, "uAmbientColor");
    m_uAmbientIntensity = glGetUniformLocation(m_program, "uAmbientIntensity");
    m_uSunIntensity = glGetUniformLocation(m_program, "uSunIntensity");
    m_uFogDensity = glGetUniformLocation(m_program, "uFogDensity");
    m_uFogColor = glGetUniformLocation(m_program, "uFogColor");
    m_uRainIntensity = glGetUniformLocation(m_program, "uRainIntensity");
    m_uTime = glGetUniformLocation(m_program, "uTime");
    return true;
}

bool Renderer::init(int width, int height, const char* title) {
    m_width = width;
    m_height = height;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    );
    if (!m_window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        SDL_Log("GL context creation failed: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetSwapInterval(1);

    if (!loadShaders()) return false;
    if (!createGeometry()) return false;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);

    return true;
}

void Renderer::beginFrame() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_program);

    // Set lighting uniforms once per frame
    glUniform3fv(m_uLightDir, 1, &m_lighting.lightDir[0]);
    glUniform3fv(m_uAmbientColor, 1, &m_lighting.ambientColor[0]);
    glUniform1f(m_uAmbientIntensity, m_lighting.ambientIntensity);
    glUniform1f(m_uSunIntensity, m_lighting.sunIntensity);
    glUniform1f(m_uFogDensity, m_lighting.fogDensity);
    glUniform3fv(m_uFogColor, 1, &m_lighting.fogColor[0]);
    glUniform1f(m_uRainIntensity, m_lighting.rainIntensity);

    m_rainTimer += 0.016f;
    glUniform1f(m_uTime, m_rainTimer);
}

void Renderer::endFrame() {
    SDL_GL_SwapWindow(m_window);
}

void Renderer::shutdown() {
    if (m_rainVAO) glDeleteVertexArrays(1, &m_rainVAO);
    if (m_rainVBO) glDeleteBuffers(1, &m_rainVBO);
    if (m_planeVAO) glDeleteVertexArrays(1, &m_planeVAO);
    if (m_planeVBO) glDeleteBuffers(1, &m_planeVBO);
    if (m_boxVAO) glDeleteVertexArrays(1, &m_boxVAO);
    if (m_boxVBO) glDeleteBuffers(1, &m_boxVBO);
    if (m_boxIBO) glDeleteBuffers(1, &m_boxIBO);
    if (m_cylVAO) glDeleteVertexArrays(1, &m_cylVAO);
    if (m_cylVBO) glDeleteBuffers(1, &m_cylVBO);
    if (m_program) glDeleteProgram(m_program);
    if (m_glContext) SDL_GL_DeleteContext(m_glContext);
    if (m_window) SDL_DestroyWindow(m_window);
}

void Renderer::setProjectionView(const glm::mat4& pv) {
    glUniformMatrix4fv(m_uPV, 1, GL_FALSE, &pv[0][0]);
}

void Renderer::setLighting(const LightingUniforms& lighting) {
    m_lighting = lighting;
}

void Renderer::drawTriangles(const Vertex* verts, int count, const glm::mat4& model, const glm::vec3& color) {
    glUniformMatrix4fv(m_uModel, 1, GL_FALSE, &model[0][0]);
    glUniform3fv(m_uColor, 1, &color[0]);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vertex), verts, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glDrawArrays(GL_TRIANGLES, 0, count);
}

void Renderer::drawPlane(const glm::mat4& model, const glm::vec3& color) {
    glUniformMatrix4fv(m_uModel, 1, GL_FALSE, &model[0][0]);
    glUniform3fv(m_uColor, 1, &color[0]);
    glBindVertexArray(m_planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Renderer::drawBox(const glm::mat4& model, const glm::vec3& color) {
    glUniformMatrix4fv(m_uModel, 1, GL_FALSE, &model[0][0]);
    glUniform3fv(m_uColor, 1, &color[0]);
    glBindVertexArray(m_boxVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
}

void Renderer::drawCylinder(const glm::mat4& model, const glm::vec3& color, float radius, float height, int segments) {
    (void)radius; (void)height; (void)segments;
    glUniformMatrix4fv(m_uModel, 1, GL_FALSE, &model[0][0]);
    glUniform3fv(m_uColor, 1, &color[0]);
    glBindVertexArray(m_cylVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_cylIndexCount);
    glBindVertexArray(0);
}

void Renderer::drawSphere(const glm::mat4& model, const glm::vec3& color, int subdivisions) {
    glUniformMatrix4fv(m_uModel, 1, GL_FALSE, &model[0][0]);
    glUniform3fv(m_uColor, 1, &color[0]);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    int stacks = subdivisions;
    int slices = subdivisions;
    int vertCount = (stacks + 1) * (slices + 1) * 6;
    Vertex* verts = new Vertex[vertCount];
    int idx = 0;

    for (int i = 0; i < stacks; ++i) {
        float phi1 = glm::pi<float>() * (float)i / (float)stacks;
        float phi2 = glm::pi<float>() * (float)(i + 1) / (float)stacks;

        for (int j = 0; j < slices; ++j) {
            float theta1 = 2.0f * glm::pi<float>() * (float)j / (float)slices;
            float theta2 = 2.0f * glm::pi<float>() * (float)(j + 1) / (float)slices;

            glm::vec3 p00 = glm::vec3(sinf(phi1) * cosf(theta1), cosf(phi1), sinf(phi1) * sinf(theta1));
            glm::vec3 p01 = glm::vec3(sinf(phi1) * cosf(theta2), cosf(phi1), sinf(phi1) * sinf(theta2));
            glm::vec3 p10 = glm::vec3(sinf(phi2) * cosf(theta1), cosf(phi2), sinf(phi2) * sinf(theta1));
            glm::vec3 p11 = glm::vec3(sinf(phi2) * cosf(theta2), cosf(phi2), sinf(phi2) * sinf(theta2));

            verts[idx++] = { p00, glm::normalize(p00), glm::vec2(0.0f) };
            verts[idx++] = { p10, glm::normalize(p10), glm::vec2(0.0f) };
            verts[idx++] = { p01, glm::normalize(p01), glm::vec2(0.0f) };
            verts[idx++] = { p01, glm::normalize(p01), glm::vec2(0.0f) };
            verts[idx++] = { p10, glm::normalize(p10), glm::vec2(0.0f) };
            verts[idx++] = { p11, glm::normalize(p11), glm::vec2(0.0f) };
        }
    }

    glBufferData(GL_ARRAY_BUFFER, idx * sizeof(Vertex), verts, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, idx);
    delete[] verts;
}

void Renderer::drawCone(const glm::mat4& model, const glm::vec3& color, float radius, float height, int segments) {
    glUniformMatrix4fv(m_uModel, 1, GL_FALSE, &model[0][0]);
    glUniform3fv(m_uColor, 1, &color[0]);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    int vertCount = segments * 6;
    Vertex* verts = new Vertex[vertCount];
    int idx = 0;

    glm::vec3 tip(0.0f, height, 0.0f);

    for (int i = 0; i < segments; ++i) {
        float a1 = 2.0f * glm::pi<float>() * (float)i / (float)segments;
        float a2 = 2.0f * glm::pi<float>() * (float)(i + 1) / (float)segments;

        glm::vec3 b1(radius * cosf(a1), 0.0f, radius * sinf(a1));
        glm::vec3 b2(radius * cosf(a2), 0.0f, radius * sinf(a2));

        // Side triangles
        glm::vec3 n1 = glm::normalize(glm::cross(b1 - tip, b2 - b1));
        verts[idx++] = { tip, n1, glm::vec2(0.0f) };
        verts[idx++] = { b1, n1, glm::vec2(0.0f) };
        verts[idx++] = { b2, n1, glm::vec2(0.0f) };

        // Base triangles
        verts[idx++] = { b1, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f) };
        verts[idx++] = { b2, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f) };
        verts[idx++] = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f) };
    }

    glBufferData(GL_ARRAY_BUFFER, idx * sizeof(Vertex), verts, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, idx);
    delete[] verts;
}

void Renderer::drawRainOverlay() {
    if (m_lighting.rainIntensity < 0.05f) return;
    glUniformMatrix4fv(m_uModel, 1, GL_FALSE, &glm::mat4(1.0f)[0][0]);
    glUniform3fv(m_uColor, 1, &glm::vec3(0.6f, 0.7f, 0.9f)[0]);

    glBindVertexArray(m_rainVAO);
    glDrawArrays(GL_LINES, 0, m_rainCount * 2);
    glBindVertexArray(0);
}

bool Renderer::createGeometry() {
    // --- Plane (ground) ---
    {
        Vertex plane[6] = {
            { { -50.0f, 0.0f, -50.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
            { {  50.0f, 0.0f, -50.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
            { { -50.0f, 0.0f,  50.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
            { {  50.0f, 0.0f, -50.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
            { {  50.0f, 0.0f,  50.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
            { { -50.0f, 0.0f,  50.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } }
        };
        glGenVertexArrays(1, &m_planeVAO);
        glGenBuffers(1, &m_planeVBO);
        glBindVertexArray(m_planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float)*3));
        glBindVertexArray(0);
    }

    // --- Box (unit cube) ---
    {
        Vertex boxVerts[24] = {
            { { -0.5f,-0.5f,-0.5f },{ 0.0f, 0.0f,-1.0f },{ 0.0f,0.0f } },{ {  0.5f,-0.5f,-0.5f },{ 0.0f, 0.0f,-1.0f },{ 1.0f,0.0f } },{ {  0.5f, 0.5f,-0.5f },{ 0.0f, 0.0f,-1.0f },{ 1.0f,1.0f } },{ { -0.5f, 0.5f,-0.5f },{ 0.0f, 0.0f,-1.0f },{ 0.0f,1.0f } },
            { { -0.5f,-0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f },{ 0.0f,0.0f } },{ {  0.5f,-0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f },{ 1.0f,0.0f } },{ {  0.5f, 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f },{ 1.0f,1.0f } },{ { -0.5f, 0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f },{ 0.0f,1.0f } },
            { { -0.5f, 0.5f,-0.5f },{ 0.0f, 1.0f, 0.0f },{ 0.0f,0.0f } },{ {  0.5f, 0.5f,-0.5f },{ 0.0f, 1.0f, 0.0f },{ 1.0f,0.0f } },{ {  0.5f, 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f },{ 1.0f,1.0f } },{ { -0.5f, 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f },{ 0.0f,1.0f } },
            { { -0.5f,-0.5f,-0.5f },{ 0.0f,-1.0f, 0.0f },{ 0.0f,0.0f } },{ {  0.5f,-0.5f,-0.5f },{ 0.0f,-1.0f, 0.0f },{ 1.0f,0.0f } },{ {  0.5f,-0.5f, 0.5f },{ 0.0f,-1.0f, 0.0f },{ 1.0f,1.0f } },{ { -0.5f,-0.5f, 0.5f },{ 0.0f,-1.0f, 0.0f },{ 0.0f,1.0f } },
            { {  0.5f, 0.5f,-0.5f },{ 1.0f, 0.0f, 0.0f },{ 0.0f,0.0f } },{ {  0.5f,-0.5f,-0.5f },{ 1.0f, 0.0f, 0.0f },{ 1.0f,0.0f } },{ {  0.5f,-0.5f, 0.5f },{ 1.0f, 0.0f, 0.0f },{ 1.0f,1.0f } },{ {  0.5f, 0.5f, 0.5f },{ 1.0f, 0.0f, 0.0f },{ 0.0f,1.0f } },
            { { -0.5f, 0.5f,-0.5f },{-1.0f, 0.0f, 0.0f },{ 0.0f,0.0f } },{ { -0.5f,-0.5f,-0.5f },{-1.0f, 0.0f, 0.0f },{ 1.0f,0.0f } },{ { -0.5f,-0.5f, 0.5f },{-1.0f, 0.0f, 0.0f },{ 1.0f,1.0f } },{ { -0.5f, 0.5f, 0.5f },{-1.0f, 0.0f, 0.0f },{ 0.0f,1.0f } }
        };
        unsigned short boxIndices[36] = {
            0,1,2, 2,3,0, 4,5,6, 6,7,4, 8,9,10, 10,11,8,
            12,13,14, 14,15,12, 16,17,18, 18,19,16, 20,21,22, 22,23,20
        };
        glGenVertexArrays(1, &m_boxVAO);
        glGenBuffers(1, &m_boxVBO);
        glGenBuffers(1, &m_boxIBO);
        glBindVertexArray(m_boxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_boxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(boxVerts), boxVerts, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_boxIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boxIndices), boxIndices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float)*3));
        glBindVertexArray(0);
    }

    // --- Cylinder (wheels) ---
    {
        const int segs = 16;
        int vertCount = (segs + 1) * 2;
        m_cylIndexCount = vertCount;
        Vertex* cyl = new Vertex[vertCount];
        for (int i = 0; i <= segs; ++i) {
            float angle = 6.2831853f * i / segs;
            float ca = cosf(angle), sa = sinf(angle);
            cyl[i*2]   = { { ca, sa, -0.5f }, { ca, sa, 0.0f }, { (float)i/segs, 0.0f } };
            cyl[i*2+1] = { { ca, sa,  0.5f }, { ca, sa, 0.0f }, { (float)i/segs, 1.0f } };
        }
        glGenVertexArrays(1, &m_cylVAO);
        glGenBuffers(1, &m_cylVBO);
        glBindVertexArray(m_cylVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_cylVBO);
        glBufferData(GL_ARRAY_BUFFER, vertCount * sizeof(Vertex), cyl, GL_STATIC_DRAW);
        delete[] cyl;
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float)*3));
        glBindVertexArray(0);
    }

    // --- Rain lines ---
    {
        m_rainCount = 400;
        int vertCount = m_rainCount * 2;
        float* rainData = new float[vertCount * 3];
        for (int i = 0; i < m_rainCount; ++i) {
            float x = ((float)rand() / RAND_MAX - 0.5f) * 100.0f;
            float y = (float)rand() / RAND_MAX * 30.0f;
            float z = ((float)rand() / RAND_MAX - 0.5f) * 100.0f;
            float len = 0.3f + (float)rand() / RAND_MAX * 0.5f;
            rainData[i*6]   = x;
            rainData[i*6+1] = y;
            rainData[i*6+2] = z;
            rainData[i*6+3] = x - 0.1f * len;
            rainData[i*6+4] = y - len;
            rainData[i*6+5] = z + 0.05f * len;
        }
        glGenVertexArrays(1, &m_rainVAO);
        glGenBuffers(1, &m_rainVBO);
        glBindVertexArray(m_rainVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_rainVBO);
        glBufferData(GL_ARRAY_BUFFER, vertCount * 3 * sizeof(float), rainData, GL_STATIC_DRAW);
        delete[] rainData;
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindVertexArray(0);
    }

    // Shared VAO for drawTriangles
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    return true;
}
