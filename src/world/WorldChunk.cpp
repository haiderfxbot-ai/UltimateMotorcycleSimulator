#include "WorldChunk.h"
#include "Terrain.h"
#include "../renderer/Renderer.h"
#include "../engine/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <cstdlib>
#include <cmath>
#include <unordered_map>

static std::unordered_map<int, Chunk*> s_chunkPool;
static Terrain s_terrain;

WorldChunkManager::WorldChunkManager()
    : m_seed(0)
    , m_viewRadius(3)
{}

WorldChunkManager::~WorldChunkManager() {
    for (auto& pair : s_chunkPool) {
        delete pair.second;
    }
    s_chunkPool.clear();
}

void WorldChunkManager::init(int seed) {
    m_seed = seed;
    std::srand(seed);
}

Chunk* WorldChunkManager::getOrCreateChunk(int cx, int cz) {
    int key = chunkKey(cx, cz);
    auto it = s_chunkPool.find(key);
    if (it != s_chunkPool.end()) {
        if (!it->second->active) {
            it->second->active = true;
            it->second->objects.clear();
            generateChunkContent(*it->second);
        }
        return it->second;
    }
    Chunk* c = new Chunk();
    c->cx = cx;
    c->cz = cz;
    c->active = true;
    generateChunkContent(*c);
    s_chunkPool[key] = c;
    return c;
}

void WorldChunkManager::releaseChunk(Chunk* chunk) {
    chunk->active = false;
    chunk->objects.clear();
}

void WorldChunkManager::update(float playerX, float playerZ, float viewDist) {
    int pcx = (int)floorf(playerX / CHUNK_SIZE);
    int pcz = (int)floorf(playerZ / CHUNK_SIZE);
    m_viewRadius = (int)ceilf(viewDist / CHUNK_SIZE);
    if (m_viewRadius < 2) m_viewRadius = 2;

    // Mark all as inactive first
    for (auto* c : m_activeChunks) {
        c->active = false;
    }

    // Activate chunks in range
    m_activeChunks.clear();
    for (int dx = -m_viewRadius; dx <= m_viewRadius; ++dx) {
        for (int dz = -m_viewRadius; dz <= m_viewRadius; ++dz) {
            int cx = pcx + dx;
            int cz = pcz + dz;
            Chunk* c = getOrCreateChunk(cx, cz);
            m_activeChunks.push_back(c);
        }
    }

    // Release chunks outside range
    for (auto it = s_chunkPool.begin(); it != s_chunkPool.end(); ) {
        if (!it->second->active) {
            delete it->second;
            it = s_chunkPool.erase(it);
        } else {
            ++it;
        }
    }
}

void WorldChunkManager::generateChunkContent(Chunk& chunk) {
    int baseSeed = m_seed + chunk.cx * 7919 + chunk.cz * 104729;
    std::srand(baseSeed);

    float wx = (float)chunk.cx * CHUNK_SIZE;
    float wz = (float)chunk.cz * CHUNK_SIZE;
    float cx = wx + CHUNK_SIZE * 0.5f;
    float cz = wz + CHUNK_SIZE * 0.5f;

    int numTrees = 3 + (std::rand() % 5);
    int numBuildings = std::rand() % 2;

    for (int i = 0; i < numTrees; ++i) {
        float ox = (float)std::rand() / RAND_MAX * CHUNK_SIZE;
        float oz = (float)std::rand() / RAND_MAX * CHUNK_SIZE;
        float tx = wx + ox;
        float tz = wz + oz;

        // Don't place trees on roads
        float distFromOriginX = std::abs(tx);
        float distFromOriginZ = std::abs(tz);
        if (distFromOriginX < 4.0f || (distFromOriginZ < 4.0f && distFromOriginX < 25.0f)) continue;

        float ty = s_terrain.getHeightAt(tx, tz);
        if (ty < -1.5f || ty > 4.0f) continue;

        ChunkObject tree;
        tree.type = ChunkObject::Tree;
        tree.position = glm::vec3(tx, ty, tz);
        tree.scale = glm::vec3(0.15f, 0.8f + (float)std::rand() / RAND_MAX * 1.2f, 0.15f);
        tree.color = glm::vec3(0.4f, 0.25f, 0.1f);
        chunk.objects.push_back(tree);

        for (int j = 0; j < 3; ++j) {
            ChunkObject foliage;
            foliage.type = ChunkObject::Tree;
            foliage.position = glm::vec3(
                tx + ((float)std::rand() / RAND_MAX - 0.5f) * 2.0f,
                ty + 0.8f + (float)std::rand() / RAND_MAX * 1.5f,
                tz + ((float)std::rand() / RAND_MAX - 0.5f) * 2.0f);
            float fs = 0.6f + (float)std::rand() / RAND_MAX * 0.8f;
            foliage.scale = glm::vec3(fs, fs, fs);
            foliage.color = glm::vec3(0.1f, 0.55f, 0.1f);
            chunk.objects.push_back(foliage);
        }
    }

    for (int i = 0; i < numBuildings; ++i) {
        float ox = 8.0f + (float)std::rand() / RAND_MAX * (CHUNK_SIZE - 16.0f);
        float oz = (float)std::rand() / RAND_MAX * CHUNK_SIZE;
        float bx = wx + ox;
        float bz = wz + oz;

        if (std::abs(bx) < 5.0f) bx += (bx > 0 ? 10.0f : -10.0f);

        float by = s_terrain.getHeightAt(bx, bz);

        ChunkObject building;
        building.type = ChunkObject::Building;
        building.position = glm::vec3(bx, by, bz);
        float bw = 2.0f + (float)std::rand() / RAND_MAX * 3.0f;
        float bd = 2.0f + (float)std::rand() / RAND_MAX * 3.0f;
        float bh = 3.0f + (float)std::rand() / RAND_MAX * 6.0f;
        building.scale = glm::vec3(bw, bh, bd);
        building.color = glm::vec3(
            0.5f + (float)std::rand() / RAND_MAX * 0.15f,
            0.5f + (float)std::rand() / RAND_MAX * 0.1f,
            0.55f + (float)std::rand() / RAND_MAX * 0.1f);
        building.extraParam = bh;
        chunk.objects.push_back(building);
    }

    // Streetlights along roads within chunk
    if (std::abs(cx) > 5.0f || std::abs(cz) < CHUNK_SIZE * 0.5f) {
        for (int li = 0; li < 2; ++li) {
            ChunkObject light;
            light.type = ChunkObject::Streetlight;
            float lx = ((float)std::rand() / RAND_MAX - 0.5f) * CHUNK_SIZE;
            float lz = ((float)std::rand() / RAND_MAX - 0.5f) * CHUNK_SIZE;
            // Place near roads
            if (std::abs(lx) > 4.0f && std::abs(lx) < 7.0f) {
                light.position = glm::vec3(
                    (lx > 0 ? 4.5f : -4.5f) + wx,
                    s_terrain.getHeightAt(lx + wx, lz + wz),
                    lz + wz);
                light.scale = glm::vec3(0.08f, 2.5f, 0.08f);
                light.color = glm::vec3(0.3f, 0.3f, 0.3f);
                chunk.objects.push_back(light);
            }
        }
    }
}

void WorldChunkManager::render(Renderer* renderer, Camera* camera) {
    (void)camera;
    (void)renderer;
    // Ground rendering is handled by World with terrain blending
}

void WorldChunkManager::renderObjects(Renderer* renderer, Camera* camera, float darken) {
    (void)camera;

    for (auto* chunk : m_activeChunks) {
        if (!chunk->active) continue;

        for (const auto& obj : chunk->objects) {
            switch (obj.type) {
                case ChunkObject::Tree: {
                    glm::mat4 trunk = glm::mat4(1.0f);
                    trunk = glm::translate(trunk, obj.position);
                    trunk = glm::scale(trunk, obj.scale);
                    renderer->drawBox(trunk, obj.color * darken * 0.8f);
                    break;
                }
                case ChunkObject::Building: {
                    glm::mat4 model = glm::mat4(1.0f);
                    model = glm::translate(model, obj.position + glm::vec3(0.0f, obj.scale.y * 0.5f, 0.0f));
                    model = glm::scale(model, obj.scale);
                    renderer->drawBox(model, obj.color * darken);
                    break;
                }
                case ChunkObject::Streetlight: {
                    glm::mat4 pole = glm::mat4(1.0f);
                    pole = glm::translate(pole, obj.position + glm::vec3(0.0f, obj.scale.y * 0.5f, 0.0f));
                    pole = glm::scale(pole, obj.scale);
                    renderer->drawBox(pole, obj.color * darken);

                    glm::mat4 arm = glm::mat4(1.0f);
                    arm = glm::translate(arm, obj.position + glm::vec3(0.3f, obj.scale.y - 0.1f, 0.0f));
                    arm = glm::scale(arm, glm::vec3(0.3f, 0.05f, 0.05f));
                    renderer->drawBox(arm, obj.color * darken);
                    break;
                }
                default:
                    break;
            }
        }
    }
}
