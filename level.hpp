#pragma once

#include <string>

namespace wwlog {
class LogLevel {
public:
    enum class Value { kTrace, kDebug, kInfo, kWarn, kError, kFatal };

    static const char *ToString(Value level)
    {
        switch (level) {
            case Value::kTrace: return "TRACE";
            case Value::kDebug: return "DEBUG";
            case Value::kInfo: return "INFO";
            case Value::kWarn: return "WARN";
            case Value::kError: return "ERROR";
            case Value::kFatal: return "FATAL";
        }
        return "UNKNOWN";
    }
};

}  // namespace wwlog