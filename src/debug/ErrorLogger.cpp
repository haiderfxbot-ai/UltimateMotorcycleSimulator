#include "ErrorLogger.h"
#include <cstdio>
#include <cstring>

namespace debug {

ErrorLogger& ErrorLogger::instance() {
    static ErrorLogger logger;
    return logger;
}

void ErrorLogger::log(const ErrorReport& report) {
    if (m_reportCount < MAX_REPORTS) {
        m_reports[m_reportCount++] = report;
    }
    switch (report.severity) {
        case Severity::Error:    m_errorCount++;    break;
        case Severity::Warning:  m_warningCount++;  break;
        case Severity::Critical: m_criticalCount++; break;
        default: break;
    }
    report.print();
}

void ErrorLogger::log(Module module, Severity severity,
                      const char* file, const char* function, int line,
                      const char* desc, const char* rootCause,
                      const char* suggestedFix) {
    ErrorReport r;
    r.module = module;
    r.severity = severity;
    r.file = file;
    r.function = function;
    r.line = line;
    r.description = desc;
    r.rootCause = rootCause ? rootCause : "";
    r.suggestedFix = suggestedFix ? suggestedFix : "";
    log(r);
}

void ErrorLogger::dumpAll() {
    fprintf(stderr, "\n===== ERROR LOGGER DUMP =====\n");
    fprintf(stderr, "Errors: %d, Warnings: %d, Critical: %d\n",
            m_errorCount, m_warningCount, m_criticalCount);
    for (int i = 0; i < m_reportCount; ++i) {
        char buf[512];
        m_reports[i].toJSON(buf, sizeof(buf));
        fprintf(stderr, "  %s\n", buf);
    }
    fprintf(stderr, "==============================\n");
}

} // namespace debug
