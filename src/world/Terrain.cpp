#include "Terrain.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

Terrain::Terrain()
    : m_seed(1234.5678f)
    , m_flatHeight(0.0f)
{
    std::srand(42);
}

Terrain::~Terrain() {}

float Terrain::interpolate(float a, float b, float t) const {
    float t2 = t * t * (3.0f - 2.0f * t);
    return a + (b - a) * t2;
}

float Terrain::valueNoise(int ix, int iz) const {
    int h = ix * 374761393 + iz * 668265263;
    h = (h ^ (h >> 13)) * 1274126177;
    h = h ^ (h >> 16);
    return (float)(h & 0x7fffffff) / 0x7fffffff * 2.0f - 1.0f;
}

float Terrain::smoothNoise(float x, float z) const {
    int ix = (int)floorf(x);
    int iz = (int)floorf(z);
    float fx = x - (float)ix;
    float fz = z - (float)iz;

    float v00 = valueNoise(ix, iz);
    float v10 = valueNoise(ix + 1, iz);
    float v01 = valueNoise(ix, iz + 1);
    float v11 = valueNoise(ix + 1, iz + 1);

    float vx0 = interpolate(v00, v10, fx);
    float vx1 = interpolate(v01, v11, fx);
    return interpolate(vx0, vx1, fz);
}

float Terrain::sampleNoise(float x, float z, int octaves, float persistence) const {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 0.04f;
    float maxVal = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += smoothNoise(x * frequency + m_seed, z * frequency + m_seed * 0.7f) * amplitude;
        maxVal += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxVal;
}

float Terrain::getHeightAt(float x, float z) const {
    float h = 0.0f;

    // Gentle rolling hills
    h += sampleNoise(x, z, 3, 0.5f) * 2.0f;

    // Occasional steeper slopes
    float steepNoise = sampleNoise(x * 0.5f, z * 0.5f, 2, 0.4f);
    if (steepNoise > 0.3f) {
        h += (steepNoise - 0.3f) * 4.0f;
    }

    // Flat road corridor along Z axis (keep ~8 units wide flat)
    float distFromCenterX = std::abs(x);
    if (distFromCenterX < 4.0f) {
        float blend = std::max(0.0f, (distFromCenterX - 2.0f) / 2.0f);
        h *= blend;
    }

    // Cross road corridor
    float distFromCenterZ = std::abs(z);
    if (distFromCenterZ < 4.0f && std::abs(x) < 20.0f) {
        float blend = std::max(0.0f, (distFromCenterZ - 2.0f) / 2.0f);
        h *= blend;
    }

    // Clamp for gameplay
    h = std::max(-2.5f, std::min(6.0f, h));

    return m_flatHeight + h;
}

glm::vec3 Terrain::getNormalAt(float x, float z) const {
    float eps = 0.1f;
    float hx = getHeightAt(x + eps, z);
    float hz = getHeightAt(x, z + eps);
    float hc = getHeightAt(x, z);
    glm::vec3 n = glm::normalize(glm::vec3(hc - hx, eps, hc - hz));
    return n;
}

float Terrain::getSlopeAt(float x, float z) const {
    float eps = 0.1f;
    float hx = getHeightAt(x + eps, z);
    float hz = getHeightAt(x, z + eps);
    float hc = getHeightAt(x, z);
    float dx = (hc - hx) / eps;
    float dz = (hc - hz) / eps;
    return std::sqrt(dx * dx + dz * dz);
}
