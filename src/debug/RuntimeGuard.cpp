#include "RuntimeGuard.h"
#include <cstdio>

namespace debug {

RuntimeGuard& RuntimeGuard::instance() {
    static RuntimeGuard guard;
    return guard;
}

void RuntimeGuard::frameBegin() {
    m_warningsThisFrame = 0;
    m_frameCount++;
}

void RuntimeGuard::frameEnd() {
    if (m_warningsThisFrame > 10) {
        fprintf(stderr, "[RUNTIME] %d warnings this frame - possible issue\n", m_warningsThisFrame);
    }
}

void RuntimeGuard::trackAllocation(const char* name) {
    m_allocationCount++;
}

void RuntimeGuard::trackDeallocation(const char* name) {
    if (m_allocationCount > 0) m_allocationCount--;
}

void RuntimeGuard::logPhysicsState(const char* desc, float speed, float rpm, float lean) {
    if (speed > 100.0f) {
        LOG_WARNING(debug::Module::Physics, "Extreme speed detected", "Physics instability", "Check friction/integration");
    }
    if (lean > 1.5f) {
        LOG_WARNING(debug::Module::Physics, "Extreme lean angle", "Bike about to tip", "Crash detection should handle");
    }
}

void RuntimeGuard::logAudioState(bool initialized, int queuedBytes) {
    if (!initialized) {
        LOG_WARNING(debug::Module::Audio, "Audio used before init", "AudioManager not initialized", "Call init() before update()");
    }
    if (queuedBytes > 48000) {
        LOG_WARNING(debug::Module::Audio, "Audio buffer growing", "QueueAudio not draining", "Check audio thread");
    }
}

void RuntimeGuard::logRenderState(int drawCalls, int fps) {
    if (drawCalls > 500) {
        LOG_WARNING(debug::Module::Renderer, "High draw call count", "Too many objects", "Consider instancing or culling");
    }
    if (fps > 0 && fps < 15) {
        LOG_WARNING(debug::Module::Renderer, "Low FPS", "Performance issue", "Reduce draw calls or optimize rendering");
    }
}

void RuntimeGuard::logInputState(bool touchActive, int activeFingers) {
    (void)touchActive;
    if (activeFingers > 5) {
        LOG_WARNING(debug::Module::Input, "Too many touch fingers", "Multi-touch limit", "Cap at 5 fingers");
    }
}

void RuntimeGuard::checkMemory(const char* file, int line) {
    if (m_allocationCount > 10000) {
        fprintf(stderr, "[MEMORY] %s:%d - %d live allocations - possible leak\n",
                file, line, m_allocationCount);
    }
}

void RuntimeGuard::checkPerformance(double frameTime, const char* file, int line) {
    if (frameTime > m_peakFrameTime) m_peakFrameTime = frameTime;
    if (frameTime > 0.05) { // > 50ms = < 20 FPS
        ErrorReport r;
        r.module = Module::Renderer;
        r.severity = Severity::Warning;
        r.file = file;
        r.function = "update";
        r.line = line;
        r.description = "Frame time exceeds 50ms";
        r.rootCause = "Heavy rendering or physics load";
        r.suggestedFix = "Optimize draw calls, reduce polygon count, or cull distant objects";
        ErrorLogger::instance().log(r);
    }
}

void RuntimeGuard::warnIfNull(const void* ptr, const char* name,
                               const char* file, const char* func, int line) {
    if (!ptr) {
        LOG_CRITICAL(Module::System, "Null pointer: %s", "Unexpected null", "Add null guard before usage");
        m_warningsThisFrame++;
    }
}

void RuntimeGuard::resetFrameCounters() {
    m_warningsThisFrame = 0;
}

} // namespace debug
