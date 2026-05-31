#include "Surface.h"
#include <cmath>
#include <cstdlib>

static bool seeded = false;

SurfaceProperties Surface::getProperties(SurfaceType type) {
    switch (type) {
        case SurfaceType::Asphalt:      return { 1.00f, 0.015f, 1.00f, 0.02f, "Asphalt" };
        case SurfaceType::Concrete:     return { 0.95f, 0.018f, 0.95f, 0.03f, "Concrete" };
        case SurfaceType::WetAsphalt:   return { 0.55f, 0.020f, 0.60f, 0.25f, "Wet Road" };
        case SurfaceType::Gravel:       return { 0.45f, 0.045f, 0.50f, 0.35f, "Gravel" };
        case SurfaceType::Dirt:         return { 0.40f, 0.050f, 0.45f, 0.40f, "Dirt" };
        case SurfaceType::Mud:          return { 0.25f, 0.080f, 0.30f, 0.60f, "Mud" };
        case SurfaceType::Sand:         return { 0.30f, 0.070f, 0.35f, 0.50f, "Sand" };
        case SurfaceType::Grass:        return { 0.35f, 0.060f, 0.40f, 0.45f, "Grass" };
        case SurfaceType::Pothole:      return { 0.30f, 0.100f, 0.30f, 0.70f, "Pothole" };
        default:                        return { 1.00f, 0.015f, 1.00f, 0.02f, "Asphalt" };
    }
}

SurfaceType Surface::getSurfaceAt(float x, float z) {
    if (!seeded) { std::srand(12345); seeded = true; }

    float roadX = 4.0f;
    bool onMainRoad = (std::abs(x) < roadX) && (std::abs(z) < 100.0f);
    bool onCrossRoad = (std::abs(x) < 40.0f) && (std::abs(z) < roadX);

    if (onMainRoad || onCrossRoad) return SurfaceType::Asphalt;

    float dist = std::sqrt(x * x + z * z);
    if (dist < 15.0f) return SurfaceType::Grass;

    int cellX = (int)(x / 20.0f);
    int cellZ = (int)(z / 20.0f);
    int hash = (cellX * 73856093 + cellZ * 19349663) % 100;

    if (hash < 5)  return SurfaceType::Dirt;
    if (hash < 10) return SurfaceType::Gravel;
    if (hash < 12) return SurfaceType::Sand;
    if (hash < 15) return SurfaceType::Grass;

    return SurfaceType::Asphalt;
}

float Surface::getHeightAt(float x, float z) {
    float h = 0.0f;

    if (hasPothole(x, z)) {
        float dx = x - (float)((int)(x / 3.0f) * 3);
        float dz = z - (float)((int)(z / 3.0f) * 3);
        float dist = std::sqrt(dx * dx + dz * dz);
        if (dist < 1.0f) {
            h -= 0.1f * (1.0f - dist);
        }
    }

    float noise = std::sin(x * 0.1f) * std::cos(z * 0.13f) * 0.03f;
    return h + noise;
}

bool Surface::hasPothole(float x, float z) {
    int cellX = (int)(x / 3.0f);
    int cellZ = (int)(z / 3.0f);
    int hash = (cellX * 378551 + cellZ * 63689) % 100;
    return hash < 3;
}

float Surface::getPotholeDepth(float x, float z) {
    if (!hasPothole(x, z)) return 0.0f;
    return 0.08f + ((float)(std::rand() % 50) / 500.0f);
}
