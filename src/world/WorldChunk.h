#ifndef WORLD_CHUNK_H
#define WORLD_CHUNK_H

#include <glm/glm.hpp>
#include <vector>

class Renderer;
class Camera;

struct ChunkObject {
    enum Type { Tree, Building, Streetlight, Sign, Barrier, Bridge, LampPost };
    Type type;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 color;
    float extraParam;
};

struct Chunk {
    int cx, cz;
    bool active;
    std::vector<ChunkObject> objects;
    float heightMap[8][8];
    Chunk() : cx(0), cz(0), active(false) {}
};

class WorldChunkManager {
public:
    WorldChunkManager();
    ~WorldChunkManager();

    void init(int seed);
    void update(float playerX, float playerZ, float viewDist);
    void render(Renderer* renderer, Camera* camera);
    void renderObjects(Renderer* renderer, Camera* camera, float darken = 1.0f);

    static const int CHUNK_SIZE = 50;
    int activeChunkCount() const { return (int)m_activeChunks.size(); }
    const std::vector<Chunk*>& activeChunks() const { return m_activeChunks; }

private:
    Chunk* getOrCreateChunk(int cx, int cz);
    void generateChunkContent(Chunk& chunk);
    void releaseChunk(Chunk* chunk);
    int chunkKey(int cx, int cz) const { return cx * 100003 + cz; }

    std::vector<Chunk*> m_activeChunks;
    int m_seed;
    int m_viewRadius;
};

#endif
