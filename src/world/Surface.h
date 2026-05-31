#ifndef SURFACE_H
#define SURFACE_H

enum class SurfaceType {
    Asphalt = 0,
    Concrete,
    WetAsphalt,
    Gravel,
    Dirt,
    Mud,
    Sand,
    Grass,
    Pothole,
    Count
};

struct SurfaceProperties {
    float grip;
    float rollingResistance;
    float brakingEfficiency;
    float slipProbability;
    const char* name;
};

class Surface {
public:
    static SurfaceProperties getProperties(SurfaceType type);
    static SurfaceType getSurfaceAt(float x, float z);
    static float getHeightAt(float x, float z);
    static bool hasPothole(float x, float z);
    static float getPotholeDepth(float x, float z);

    static const int GRID_SIZE = 200;
    static const int GRID_RESOLUTION = 4;
};

#endif
