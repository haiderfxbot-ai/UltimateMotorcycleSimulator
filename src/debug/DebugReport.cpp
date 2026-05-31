#include "DebugReport.h"
#include <cstdio>

namespace debug {

const char* moduleName(Module m) {
    switch (m) {
        case Module::Physics:   return "Physics";
        case Module::Audio:     return "Audio";
        case Module::UI:        return "UI";
        case Module::Input:     return "Input";
        case Module::World:     return "World";
        case Module::Renderer:  return "Renderer";
        case Module::Network:   return "Network";
        case Module::Engine:    return "Engine";
        case Module::Collision: return "Collision";
        case Module::System:    return "System";
        default:                return "Unknown";
    }
}

const char* severityName(Severity s) {
    switch (s) {
        case Severity::Info:     return "INFO";
        case Severity::Warning:  return "WARNING";
        case Severity::Error:    return "ERROR";
        case Severity::Critical: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

void ErrorReport::print() const {
    fprintf(stderr,
        "\n[%s] %s\n"
        "  Module:     %s\n"
        "  File:       %s\n"
        "  Function:   %s\n"
        "  Line:       %d\n"
        "  Desc:       %s\n"
        "  RootCause:  %s\n"
        "  Suggested:  %s\n",
        severityName(severity),
        moduleName(module),
        moduleName(module),
        file ? file : "?",
        function ? function : "?",
        line,
        description ? description : "?",
        rootCause ? rootCause : "?",
        suggestedFix ? suggestedFix : "?"
    );
}

void ErrorReport::toJSON(char* buf, size_t size) const {
    snprintf(buf, size,
        "{\"severity\":\"%s\",\"module\":\"%s\",\"file\":\"%s\","
        "\"function\":\"%s\",\"line\":%d,\"desc\":\"%s\","
        "\"rootCause\":\"%s\",\"suggestedFix\":\"%s\"}",
        severityName(severity), moduleName(module),
        file ? file : "?", function ? function : "?", line,
        description ? description : "?",
        rootCause ? rootCause : "?",
        suggestedFix ? suggestedFix : "?"
    );
}

} // namespace debug
