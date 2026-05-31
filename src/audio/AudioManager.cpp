#include "AudioManager.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

#ifdef ANDROID
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

AudioManager::AudioManager()
    : m_device(0)
    , m_initialized(false)
    , m_volume(0.5f)
    , m_sampleRate(44100.0f)
    , m_rpm(0.0f)
    , m_speed(0.0f)
    , m_throttle(0.0f)
    , m_slip(0.0f)
    , m_rainIntensity(0.0f)
    , m_engineRunning(false)
    , m_isStalled(false)
    , m_isCrashed(false)
    , m_crashTimer(0.0f)
    , m_honkTimer(0.0f)
    , m_enginePhase(0.0f)
    , m_tirePhase(0.0f)
    , m_windPhase(0.0f)
{
    for (int i = 0; i < MAX_ONESHOTS; ++i) {
        m_oneShots[i].active = false;
        m_oneShots[i].noiseSeed = (uint32_t)(rand() * 1000 + i);
    }
}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::init() {
    SDL_AudioSpec desired, obtained;
    SDL_zero(desired);
    desired.freq = SAMPLE_RATE;
    desired.format = AUDIO_S16SYS;
    desired.channels = 1;
    desired.samples = SAMPLE_RATE * BUFFER_MS / 1000;
    desired.callback = nullptr;

    m_device = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (!m_device) {
        SDL_Log("Audio init failed: %s", SDL_GetError());
        return false;
    }

    m_sampleRate = (float)obtained.freq;
    m_initialized = true;
    SDL_PauseAudioDevice(m_device, 0);
    SDL_Log("Audio initialized: %d Hz, %d channels", obtained.freq, obtained.channels);
    return true;
}

void AudioManager::shutdown() {
    if (m_device) {
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
    }
    m_initialized = false;
}

float AudioManager::whiteNoise() {
    return (float)rand() * (1.0f / 32767.0f) * 2.0f - 1.0f;
}

void AudioManager::update(float dt, float rpm, float speed, float throttle,
                          float slip, float steer, bool isRaining, float rainIntensity,
                          bool isCrashed, float crashForce, int gear, bool engineRunning,
                          bool isStalled) {
    (void)steer;
    (void)gear;
    if (!m_initialized) return;

    m_rpm = rpm;
    m_speed = speed;
    m_throttle = throttle;
    m_slip = slip;
    m_rainIntensity = rainIntensity;
    m_engineRunning = engineRunning && !isStalled;
    m_isStalled = isStalled;

    if (isCrashed && !m_isCrashed) {
        crash(crashForce);
    }
    m_isCrashed = isCrashed;

    m_crashTimer = std::max(0.0f, m_crashTimer - dt);
    m_honkTimer = std::max(0.0f, m_honkTimer - dt);

    for (int i = 0; i < MAX_ONESHOTS; ++i) {
        if (m_oneShots[i].active) {
            m_oneShots[i].remaining -= dt;
            if (m_oneShots[i].remaining <= 0.0f) {
                m_oneShots[i].active = false;
            }
        }
    }

    int totalSamples = (int)(m_sampleRate * dt);
    if (totalSamples <= 0) totalSamples = (int)(m_sampleRate * 0.016f);

    int16_t* mixBuffer = new int16_t[totalSamples];
    std::fill(mixBuffer, mixBuffer + totalSamples, 0);

    // Generate and mix all layers
    int16_t* layerBuffer = new int16_t[totalSamples];

    if (m_engineRunning) {
        generateEngineSamples(layerBuffer, totalSamples, dt);
        mixBuffers(mixBuffer, layerBuffer, totalSamples, 1.0f);
    }

    if (m_slip > 0.3f && m_speed > 5.0f) {
        generateTireSamples(layerBuffer, totalSamples, dt);
        mixBuffers(mixBuffer, layerBuffer, totalSamples, 0.7f * m_slip);
    }

    if (m_speed > 2.0f) {
        generateWindSamples(layerBuffer, totalSamples, dt);
        mixBuffers(mixBuffer, layerBuffer, totalSamples, 0.3f * std::min(1.0f, m_speed / 50.0f));
    }

    if (m_rainIntensity > 0.05f) {
        generateRainSamples(layerBuffer, totalSamples, dt);
        mixBuffers(mixBuffer, layerBuffer, totalSamples, 0.4f * m_rainIntensity);
    }

    generateOneShots(layerBuffer, totalSamples);
    mixBuffers(mixBuffer, layerBuffer, totalSamples, 1.0f);

    if (m_isStalled && m_rpm > 100.0f) {
        generateEngineSamples(layerBuffer, totalSamples, dt);
        for (int i = 0; i < totalSamples; ++i) {
            layerBuffer[i] = (int16_t)(layerBuffer[i] * 0.3f * 
                std::max(0.0f, 1.0f - (float)i / totalSamples));
        }
        mixBuffers(mixBuffer, layerBuffer, totalSamples, 1.0f);
    }

    // Volume apply
    for (int i = 0; i < totalSamples; ++i) {
        mixBuffer[i] = (int16_t)(mixBuffer[i] * m_volume * 0.5f);
    }

    SDL_QueueAudio(m_device, mixBuffer, totalSamples * sizeof(int16_t));

    delete[] layerBuffer;
    delete[] mixBuffer;
}

void AudioManager::generateEngineSamples(int16_t* buffer, int numSamples, float dt) {
    float freq = m_rpm / 60.0f * 2.5f;
    if (freq < 30.0f) freq = 30.0f;
    if (freq > 800.0f) freq = 800.0f;

    float phaseInc = freq / m_sampleRate;
    float load = std::min(1.0f, m_throttle * 0.8f + 0.2f);

    for (int i = 0; i < numSamples; ++i) {
        m_enginePhase += phaseInc;
        if (m_enginePhase > 1.0f) m_enginePhase -= 1.0f;

        float t = m_enginePhase;
        float sample = 0.0f;

        // Fundamental (saw-like)
        sample += (t * 2.0f - 1.0f) * 0.4f;

        // 2nd harmonic
        float t2 = fmodf(t * 2.0f, 1.0f);
        sample += sinf(t2 * 6.2832f) * 0.25f;

        // 3rd harmonic
        float t3 = fmodf(t * 3.0f, 1.0f);
        sample += sinf(t3 * 6.2832f) * 0.15f;

        // Sub harmonic for rumble
        sample += sinf(t * 3.1416f) * 0.2f;

        // Load modulation
        sample *= load;

        // Rev limiter effect at high RPM (crackle)
        if (m_rpm > 6500.0f && m_rpm < 7500.0f) {
            float limiter = sinf(t * 100.0f) * 0.5f + 0.5f;
            sample *= 0.7f + 0.3f * limiter;
        }

        buffer[i] = (int16_t)(sample * 12000.0f);
    }
}

void AudioManager::generateTireSamples(int16_t* buffer, int numSamples, float dt) {
    (void)dt;
    float freq = 800.0f + m_slip * 2000.0f;
    float phaseInc = freq / m_sampleRate;

    for (int i = 0; i < numSamples; ++i) {
        m_tirePhase += phaseInc;
        if (m_tirePhase > 1.0f) m_tirePhase -= 1.0f;

        float sample = whiteNoise() * 0.3f;

        // Add screech tone
        sample += sinf(m_tirePhase * 6.2832f) * 0.5f;
        sample += sinf(m_tirePhase * 12.5664f) * 0.25f;

        // Amplitude modulated by slip
        float amp = std::min(1.0f, (m_slip - 0.3f) / 0.7f);
        sample *= amp * 0.5f;

        buffer[i] = (int16_t)(sample * 8000.0f);
    }
}

void AudioManager::generateWindSamples(int16_t* buffer, int numSamples, float dt) {
    (void)dt;
    for (int i = 0; i < numSamples; ++i) {
        m_windPhase += 0.001f;
        if (m_windPhase > 1.0f) m_windPhase -= 1.0f;

        float sample = whiteNoise() * 0.5f;

        // Low-pass filter: average with previous (simple one-pole)
        static float prevWind = 0.0f;
        sample = prevWind * 0.8f + sample * 0.2f;
        prevWind = sample;

        float amp = std::min(1.0f, m_speed / 50.0f);
        sample *= amp;

        buffer[i] = (int16_t)(sample * 6000.0f);
    }
}

void AudioManager::generateRainSamples(int16_t* buffer, int numSamples, float dt) {
    (void)dt;
    for (int i = 0; i < numSamples; ++i) {
        float sample = whiteNoise() * 0.3f;

        // Pitter-patter pulses
        if ((rand() % 100) < 2) {
            sample += whiteNoise() * 0.7f;
        }

        sample *= m_rainIntensity;
        buffer[i] = (int16_t)(sample * 5000.0f);
    }
}

void AudioManager::generateOneShots(int16_t* buffer, int numSamples) {
    std::fill(buffer, buffer + numSamples, 0);

    for (int oi = 0; oi < MAX_ONESHOTS; ++oi) {
        auto& shot = m_oneShots[oi];
        if (!shot.active) continue;

        float phaseInc = shot.freq / m_sampleRate;
        int remainingSamples = (int)(shot.remaining * m_sampleRate);

        for (int i = 0; i < numSamples && i < remainingSamples; ++i) {
            shot.phase += phaseInc;
            if (shot.phase > 1.0f) shot.phase -= 1.0f;

            float env = std::max(0.0f, 1.0f - (float)i / (remainingSamples + 1));
            float sample = sinf(shot.phase * 6.2832f);

            // Noise component for crash
            if (shot.duration > 0.5f) {
                shot.noiseSeed = shot.noiseSeed * 1103515245 + 12345;
                float noise = (float)(shot.noiseSeed & 0x7fff) / 32768.0f * 2.0f - 1.0f;
                sample = sample * 0.3f + noise * 0.7f;
            }

            sample *= env * shot.amplitude;
            buffer[i] += (int16_t)(sample * 12000.0f);
        }
    }
}

void AudioManager::mixBuffers(int16_t* out, const int16_t* in, int numSamples, float gain) {
    for (int i = 0; i < numSamples; ++i) {
        int sum = (int)(out[i]) + (int)(in[i] * gain);
        if (sum > 32767) sum = 32767;
        if (sum < -32768) sum = -32768;
        out[i] = (int16_t)sum;
    }
}

void AudioManager::honk() {
    if (!m_initialized || m_honkTimer > 0.0f) return;
    m_honkTimer = 0.5f;

    for (int i = 0; i < MAX_ONESHOTS; ++i) {
        if (!m_oneShots[i].active) {
            m_oneShots[i].phase = 0.0f;
            m_oneShots[i].freq = 350.0f;
            m_oneShots[i].duration = 0.3f;
            m_oneShots[i].remaining = 0.3f;
            m_oneShots[i].amplitude = 0.6f;
            m_oneShots[i].active = true;
            break;
        }
    }
}

void AudioManager::crash(float force) {
    if (!m_initialized || m_crashTimer > 0.0f) return;
    m_crashTimer = 1.0f;

    for (int i = 0; i < MAX_ONESHOTS; ++i) {
        if (!m_oneShots[i].active) {
            m_oneShots[i].phase = 0.0f;
            m_oneShots[i].freq = 100.0f + force * 50.0f;
            m_oneShots[i].duration = 0.5f + force * 0.3f;
            m_oneShots[i].remaining = m_oneShots[i].duration;
            m_oneShots[i].amplitude = std::min(1.0f, 0.3f + force * 0.1f);
            m_oneShots[i].active = true;
            break;
        }
    }
}
