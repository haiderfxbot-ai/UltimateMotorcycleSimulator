#ifndef ERROR_LOGGER_H
#define ERROR_LOGGER_H

#include "DebugReport.h"

namespace debug {

class ErrorLogger {
public:
    static ErrorLogger& instance();

    void log(const ErrorReport& report);
    void log(Module module, Severity severity,
             const char* file, const char* function, int line,
             const char* description, const char* rootCause = nullptr,
             const char* suggestedFix = nullptr);

    void logError(Module module, const char* file, const char* func, int line,
                  const char* desc, const char* cause, const char* fix) {
        log(module, Severity::Error, file, func, line, desc, cause, fix);
    }

    void logWarning(Module module, const char* file, const char* func, int line,
                    const char* desc, const char* cause = nullptr, const char* fix = nullptr) {
        log(module, Severity::Warning, file, func, line, desc, cause, fix);
    }

    void logCritical(Module module, const char* file, const char* func, int line,
                     const char* desc, const char* cause, const char* fix) {
        log(module, Severity::Critical, file, func, line, desc, cause, fix);
    }

    int errorCount() const { return m_errorCount; }
    int warningCount() const { return m_warningCount; }
    void resetCounters() { m_errorCount = 0; m_warningCount = 0; }

    void dumpAll();
    bool hasErrors() const { return m_errorCount > 0 || m_criticalCount > 0; }

private:
    ErrorLogger() : m_errorCount(0), m_warningCount(0), m_criticalCount(0) {}
    ~ErrorLogger() {}
    ErrorLogger(const ErrorLogger&) = delete;
    ErrorLogger& operator=(const ErrorLogger&) = delete;

    int m_errorCount;
    int m_warningCount;
    int m_criticalCount;
    static const int MAX_REPORTS = 128;
    ErrorReport m_reports[MAX_REPORTS];
    int m_reportCount;
};

// Convenience macros
#define LOG_ERROR(mod, desc, cause, fix) \
    debug::ErrorLogger::instance().logError( \
        mod, __FILE__, __func__, __LINE__, desc, cause, fix)

#define LOG_WARNING(mod, desc, cause, fix) \
    debug::ErrorLogger::instance().logWarning( \
        mod, __FILE__, __func__, __LINE__, desc, cause, fix)

#define LOG_CRITICAL(mod, desc, cause, fix) \
    debug::ErrorLogger::instance().logCritical( \
        mod, __FILE__, __func__, __LINE__, desc, cause, fix)

// Module-specific logging helpers
#define PHYSICS_ERROR(desc, cause, fix)    LOG_ERROR(debug::Module::Physics, desc, cause, fix)
#define AUDIO_ERROR(desc, cause, fix)      LOG_ERROR(debug::Module::Audio, desc, cause, fix)
#define UI_ERROR(desc, cause, fix)         LOG_ERROR(debug::Module::UI, desc, cause, fix)
#define INPUT_ERROR(desc, cause, fix)      LOG_ERROR(debug::Module::Input, desc, cause, fix)
#define WORLD_ERROR(desc, cause, fix)      LOG_ERROR(debug::Module::World, desc, cause, fix)
#define RENDER_ERROR(desc, cause, fix)     LOG_ERROR(debug::Module::Renderer, desc, cause, fix)
#define ENGINE_ERROR(desc, cause, fix)     LOG_ERROR(debug::Module::Engine, desc, cause, fix)
#define COLLISION_ERROR(desc, cause, fix)  LOG_ERROR(debug::Module::Collision, desc, cause, fix)
#define SYSTEM_ERROR(desc, cause, fix)     LOG_ERROR(debug::Module::System, desc, cause, fix)

#define PHYSICS_WARN(desc, cause, fix)     LOG_WARNING(debug::Module::Physics, desc, cause, fix)
#define AUDIO_WARN(desc, cause, fix)       LOG_WARNING(debug::Module::Audio, desc, cause, fix)
#define RENDER_WARN(desc, cause, fix)      LOG_WARNING(debug::Module::Renderer, desc, cause, fix)
#define INPUT_WARN(desc, cause, fix)       LOG_WARNING(debug::Module::Input, desc, cause, fix)
#define COLLISION_WARN(desc, cause, fix)   LOG_WARNING(debug::Module::Collision, desc, cause, fix)

} // namespace debug

#endif
