#ifndef TRANSMISSION_H
#define TRANSMISSION_H

struct GearRatio {
    float ratio;
    float maxSpeed;
};

class Transmission {
public:
    Transmission();
    ~Transmission();

    void configure(int numGears, const float* ratios);
    void reset();

    void shiftUp();
    void shiftDown();
    void shiftToNeutral();

    void setClutch(float clutchValue);
    float clutch() const { return m_clutch; }

    int gear() const { return m_gear; }
    int maxGear() const { return m_numGears; }
    float ratio() const;
    float engagedRatio() const;

    bool isNeutral() const { return m_gear == 0; }
    bool isShifting() const { return m_shiftTimer > 0.0f; }

    float wheelRPMToEngineRPM(float wheelRPM) const;
    float engineRPMToWheelRPM(float engineRPM) const;

    float calculateClutchTorque(float engineRPM, float wheelRPM, float dt) const;
    void update(float dt);

private:
    int m_gear;
    int m_numGears;
    GearRatio* m_ratios;

    float m_clutch;
    float m_clutchSpeed;
    float m_shiftTimer;
    float m_shiftDelay;

    static constexpr int MAX_GEARS = 8;
};

#endif
