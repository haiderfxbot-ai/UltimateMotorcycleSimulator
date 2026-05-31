#ifndef ENGINE_SIM_H
#define ENGINE_SIM_H

struct EngineConfig {
    float idleRPM = 1100.0f;
    float maxRPM = 11500.0f;
    float redlineRPM = 9800.0f;
    float stallRPM = 800.0f;
    float maxPower = 80.0f;
    float maxTorque = 70.0f;
    float inertia = 0.15f;
    float fuelCapacity = 15.0f;
    float fuelConsumptionRate = 0.002f;
    float engineBraking = 3.0f;
};

struct EngineState {
    bool running = false;
    bool stalled = false;
    float rpm = 0.0f;
    float torque = 0.0f;
    float power = 0.0f;
    float fuel = 15.0f;
    float temperature = 80.0f;
    float throttlePos = 0.0f;
    float load = 0.0f;
    float starterTimer = 0.0f;
};

class EngineSim {
public:
    EngineSim();
    ~EngineSim();

    void configure(const EngineConfig& cfg);
    void start();
    void stop();
    void reset();

    void update(float dt, float throttleInput, float engineLoad, bool clutchEngaged, float wheelSpeed);

    bool isRunning() const { return m_state.running; }
    bool isStalled() const { return m_state.stalled; }
    float rpm() const { return m_state.rpm; }
    float torque() const { return m_state.torque; }
    float power() const { return m_state.power; }
    float fuel() const { return m_state.fuel; }
    float temperature() const { return m_state.temperature; }
    float throttlePos() const { return m_state.throttlePos; }

    void setStalled(bool s);
    const EngineConfig& config() const { return m_config; }

private:
    void calculateCombustion(float dt, float throttle, float load);
    void calculateCooling(float dt);
    void checkStall(float engineLoad, bool clutchEngaged, float wheelSpeed);

    EngineConfig m_config;
    EngineState m_state;
    float m_crankRPM;
};

#endif
