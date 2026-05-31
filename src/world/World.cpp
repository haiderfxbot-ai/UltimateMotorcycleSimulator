#include "World.h"
#include "../renderer/Renderer.h"
#include "../engine/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <cstdlib>
#include <cmath>

World::World() {}
World::~World() {}

bool World::init(Renderer* renderer) {
    (void)renderer;
    std::srand(42);
    return true;
}

void World::render(Renderer* renderer, Camera* camera) {
    if (!renderer || !camera) return;

    glm::mat4 pv = camera->projectionMatrix() * camera->viewMatrix();
    renderer->setProjectionView(pv);

    renderGround(renderer, camera);
    renderRoad(renderer, camera);
    renderTrees(renderer, camera);
    renderBuildings(renderer, camera);
}

void World::renderGround(Renderer* renderer, Camera* camera) {
    (void)camera;
    glm::mat4 groundModel = glm::mat4(1.0f);
    renderer->drawPlane(groundModel, glm::vec3(0.25f, 0.55f, 0.15f));
}

void World::renderRoad(Renderer* renderer, Camera* camera) {
    (void)camera;

    // Main road along Z axis
    glm::mat4 roadModel = glm::mat4(1.0f);
    roadModel = glm::translate(roadModel, glm::vec3(0.0f, 0.01f, 0.0f));
    roadModel = glm::scale(roadModel, glm::vec3(6.0f, 1.0f, 100.0f));
    renderer->drawPlane(roadModel, glm::vec3(0.25f, 0.25f, 0.25f));

    // Road center line dashes
    for (int z = -45; z < 45; z += 6) {
        glm::mat4 dashModel = glm::mat4(1.0f);
        dashModel = glm::translate(dashModel, glm::vec3(0.0f, 0.02f, (float)z));
        dashModel = glm::scale(dashModel, glm::vec3(0.1f, 1.0f, 2.0f));
        renderer->drawPlane(dashModel, glm::vec3(0.9f, 0.9f, 0.9f));
    }

    // Road edges (white lines)
    for (int side = -1; side <= 1; side += 2) {
        glm::mat4 edgeModel = glm::mat4(1.0f);
        edgeModel = glm::translate(edgeModel, glm::vec3(side * 2.8f, 0.02f, 0.0f));
        edgeModel = glm::scale(edgeModel, glm::vec3(0.2f, 1.0f, 100.0f));
        renderer->drawPlane(edgeModel, glm::vec3(0.95f, 0.95f, 0.95f));
    }

    // Cross road along X axis
    glm::mat4 crossModel = glm::mat4(1.0f);
    crossModel = glm::translate(crossModel, glm::vec3(0.0f, 0.01f, 0.0f));
    crossModel = glm::rotate(crossModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    crossModel = glm::scale(crossModel, glm::vec3(6.0f, 1.0f, 30.0f));
    renderer->drawPlane(crossModel, glm::vec3(0.25f, 0.25f, 0.25f));
}

void World::renderTrees(Renderer* renderer, Camera* camera) {
    (void)camera;

    for (int i = 0; i < 40; ++i) {
        float x = ((float)(i % 10) - 5.0f) * 6.0f + ((float)std::rand() / RAND_MAX) * 3.0f;
        float z = ((float)(i / 10) - 2.0f) * 20.0f + ((float)std::rand() / RAND_MAX) * 5.0f;

        if (std::abs(x) < 4.0f && std::abs(z) < 50.0f) continue;

        // Trunk
        glm::mat4 trunkModel = glm::mat4(1.0f);
        trunkModel = glm::translate(trunkModel, glm::vec3(x, 0.5f, z));
        trunkModel = glm::scale(trunkModel, glm::vec3(0.15f, 1.0f, 0.15f));
        renderer->drawBox(trunkModel, glm::vec3(0.4f, 0.25f, 0.1f));

        // Foliage (3 spheres approximated as boxes)
        for (int j = 0; j < 3; ++j) {
            glm::mat4 foliageModel = glm::mat4(1.0f);
            float fx = x + ((float)std::rand() / RAND_MAX - 0.5f) * 2.0f;
            float fz = z + ((float)std::rand() / RAND_MAX - 0.5f) * 2.0f;
            float fy = 1.0f + (float)std::rand() / RAND_MAX * 1.5f;
            foliageModel = glm::translate(foliageModel, glm::vec3(fx, fy, fz));
            float size = 0.8f + (float)std::rand() / RAND_MAX * 0.8f;
            foliageModel = glm::scale(foliageModel, glm::vec3(size, size, size));
            renderer->drawBox(foliageModel, glm::vec3(0.1f, 0.6f, 0.1f));
        }
    }
}

void World::renderBuildings(Renderer* renderer, Camera* camera) {
    (void)camera;

    struct Building {
        float x, z;
        float width, depth, height;
        glm::vec3 color;
    };

    Building buildings[] = {
        { -15.0f, -30.0f, 4.0f, 4.0f, 8.0f, { 0.6f, 0.6f, 0.65f } },
        { -22.0f, -30.0f, 5.0f, 3.0f, 12.0f, { 0.55f, 0.55f, 0.6f } },
        {  15.0f, -30.0f, 3.0f, 4.0f, 6.0f, { 0.65f, 0.6f, 0.55f } },
        {  22.0f, -30.0f, 4.0f, 5.0f, 15.0f, { 0.5f, 0.5f, 0.55f } },
        { -15.0f,  30.0f, 5.0f, 3.0f, 10.0f, { 0.6f, 0.55f, 0.5f } },
        { -22.0f,  30.0f, 3.0f, 4.0f, 7.0f, { 0.55f, 0.6f, 0.6f } },
        {  15.0f,  30.0f, 4.0f, 5.0f, 20.0f, { 0.5f, 0.5f, 0.55f } },
        {  22.0f,  30.0f, 5.0f, 4.0f, 9.0f, { 0.6f, 0.6f, 0.6f } },
        { -18.0f,   0.0f, 3.0f, 3.0f, 5.0f, { 0.65f, 0.6f, 0.55f } },
        {  18.0f,   0.0f, 4.0f, 3.0f, 6.0f, { 0.55f, 0.55f, 0.6f } },
    };

    for (const auto& b : buildings) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(b.x, b.height * 0.5f, b.z));
        model = glm::scale(model, glm::vec3(b.width, b.height, b.depth));
        renderer->drawBox(model, b.color);

        // Windows (simplified as lighter colored boxes on front)
        for (int wy = 0; wy < (int)(b.height / 3.0f); ++wy) {
            for (int wx = -1; wx <= 1; ++wx) {
                glm::mat4 winModel = glm::mat4(1.0f);
                winModel = glm::translate(winModel, glm::vec3(
                    b.x + wx * (b.width * 0.25f),
                    1.5f + wy * 3.0f,
                    b.z + (b.depth * 0.5f + 0.05f) * (wx >= 0 ? 1 : -1)
                ));
                winModel = glm::scale(winModel, glm::vec3(0.4f, 0.8f, 0.02f));
                renderer->drawBox(winModel, glm::vec3(0.7f, 0.75f, 0.85f));
            }
        }
    }
}
