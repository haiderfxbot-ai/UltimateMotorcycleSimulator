#ifndef TERRAIN_H
#define TERRAIN_H

#include <glm/glm.hpp>

class Terrain {
public:
    Terrain();
    ~Terrain();

    float getHeightAt(float x, float z) const;
    glm::vec3 getNormalAt(float x, float z) const;
    float getSlopeAt(float x, float z) const;

    float sampleNoise(float x, float z, int octaves, float persistence) const;
    float getFlatHeight() const { return m_flatHeight; }

private:
    float valueNoise(int ix, int iz) const;
    float smoothNoise(float x, float z) const;
    float interpolate(float a, float b, float t) const;

    float m_seed;
    float m_flatHeight;
};

#endif
