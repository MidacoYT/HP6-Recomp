#include "platform/Diagnostics.h"

#include <cstdarg>
#include <cstdio>

namespace hp6::platform {

// ReAgent candidate: reports/pipeline/candidates/0063ccf0.cpp.
void format_diagnostic_message(const char* format, ...) {
    char buffer[2048];
    va_list args;
    va_start(args, format);
    std::vsprintf(buffer, format, args);
    va_end(args);
}

} // namespace hp6::platform
