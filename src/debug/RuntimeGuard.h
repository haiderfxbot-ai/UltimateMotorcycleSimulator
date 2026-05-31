#ifndef RUNTIME_GUARD_H
#define RUNTIME_GUARD_H

#include "ErrorLogger.h"
#include <cstdint>

namespace debug {

class RuntimeGuard {
public:
    static RuntimeGuard& instance();

    void frameBegin();
    void frameEnd();

    void trackAllocation(const char* name);
    void trackDeallocation(const char* name);
    int liveAllocations() const { return m_allocationCount; }

    void logPhysicsState(const char* desc, float speed, float rpm, float lean);
    void logAudioState(bool initialized, int queuedBytes);
    void logRenderState(int drawCalls, int fps);
    void logInputState(bool touchActive, int activeFingers);

    void checkMemory(const char* file, int line);
    void checkPerformance(double frameTime, const char* file, int line);

    void warnIfNull(const void* ptr, const char* name,
                    const char* file, const char* func, int line);

    int warningCount() const { return m_warningsThisFrame; }
    void resetFrameCounters();

private:
    RuntimeGuard()
        : m_allocationCount(0)
        , m_warningsThisFrame(0)
        , m_frameCount(0)
        , m_peakFrameTime(0.0)
    {}
    ~RuntimeGuard() {}
    RuntimeGuard(const RuntimeGuard&) = delete;
    RuntimeGuard& operator=(const RuntimeGuard&) = delete;

    int m_allocationCount;
    int m_warningsThisFrame;
    int m_frameCount;
    double m_peakFrameTime;
};

// Guard macro: checks a pointer before use
#define GUARD_NON_NULL(ptr, module, name)                              \
    debug::RuntimeGuard::instance().warnIfNull(                        \
        (ptr), (name), __FILE__, __func__, __LINE__)

// Guard macro: checks performance
#define GUARD_PERF(frameTime)                                          \
    debug::RuntimeGuard::instance().checkPerformance(                  \
        (frameTime), __FILE__, __LINE__)

} // namespace debug

#endif
