#ifndef DEBUG_REPORT_H
#define DEBUG_REPORT_H

#include <string>
#include <cstdio>

namespace debug {

enum class Severity {
    Info,
    Warning,
    Error,
    Critical
};

enum class Module {
    Physics,
    Audio,
    UI,
    Input,
    World,
    Renderer,
    Network,
    Engine,
    Collision,
    System
};

struct ErrorReport {
    Module      module;
    Severity    severity;
    const char* file;
    const char* function;
    int         line;
    const char* description;
    const char* rootCause;
    const char* suggestedFix;

    void print() const;
    void toJSON(char* buf, size_t size) const;
};

const char* moduleName(Module m);
const char* severityName(Severity s);

} // namespace debug

#endif
