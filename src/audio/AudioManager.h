#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <cstdint>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool init();
    void shutdown();

    void update(float dt, float rpm, float speed, float throttle,
                float slip, float steer, bool isRaining, float rainIntensity,
                bool isCrashed, float crashForce, int gear, bool engineRunning,
                bool isStalled);

    void honk();
    void crash(float force);

    bool isInitialized() const { return m_initialized; }
    float volume() const { return m_volume; }
    void setVolume(float v) { m_volume = v; }

    static const int SAMPLE_RATE = 44100;
    static const int BUFFER_MS = 50;

private:
    void generateEngineSamples(int16_t* buffer, int numSamples, float dt);
    void generateTireSamples(int16_t* buffer, int numSamples, float dt);
    void generateWindSamples(int16_t* buffer, int numSamples, float dt);
    void generateRainSamples(int16_t* buffer, int numSamples, float dt);
    void generateOneShots(int16_t* buffer, int numSamples);
    void mixBuffers(int16_t* out, const int16_t* in, int numSamples, float gain);
    float whiteNoise();

    uint32_t m_device;
    bool m_initialized;
    float m_volume;
    float m_sampleRate;

    float m_rpm;
    float m_speed;
    float m_throttle;
    float m_slip;
    float m_rainIntensity;
    bool m_engineRunning;
    bool m_isStalled;
    bool m_isCrashed;
    float m_crashTimer;
    float m_honkTimer;

    float m_enginePhase;
    float m_tirePhase;
    float m_windPhase;

    struct OneShot {
        float phase;
        float freq;
        float duration;
        float remaining;
        float amplitude;
        bool active;
        uint32_t noiseSeed;
    };
    static const int MAX_ONESHOTS = 8;
    OneShot m_oneShots[MAX_ONESHOTS];
};

#endif
