#ifndef WORLD_H
#define WORLD_H

#include <glm/glm.hpp>

class Renderer;
class Camera;

class World {
public:
    World();
    ~World();

    bool init(Renderer* renderer);
    void render(Renderer* renderer, Camera* camera);

private:
    void renderGround(Renderer* renderer, Camera* camera);
    void renderRoad(Renderer* renderer, Camera* camera);
    void renderTrees(Renderer* renderer, Camera* camera);
    void renderBuildings(Renderer* renderer, Camera* camera);
};

#endif
