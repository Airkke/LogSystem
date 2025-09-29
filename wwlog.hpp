#pragma once

#include "manage.hpp"

namespace wwlog {

AsyncLogger::Ptr GetLogger(const std::string& name) { return LoggerManager::GetInstance().GetLogger(name); }
AsyncLogger::Ptr DefaultLogger() { return LoggerManager::GetInstance().DefaultLogger(); }

#define Trace(fmt, ...) Trace(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Debug(fmt, ...) Debug(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Info(fmt, ...) Info(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Warn(fmt, ...) Warn(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Error(fmt, ...) Error(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define Fatal(fmt, ...) Fatal(__FILE__, __LINE__, fmt, ##__VA_ARGS__)


#define LOGTRACE(fmt, ...) wwlog::DefaultLogger()->Trace(fmt, ##__VA_ARGS__)
#define LOGDEBUGDEFAULT(fmt, ...) wwlog::DefaultLogger()->Debug(fmt, ##__VA_ARGS__)
#define LOGINFODEFAULT(fmt, ...) wwlog::DefaultLogger()->Info(fmt, ##__VA_ARGS__)
#define LOGWARNDEFAULT(fmt, ...) wwlog::DefaultLogger()->Warn(fmt, ##__VA_ARGS__)
#define LOGERRORDEFAULT(fmt, ...) wwlog::DefaultLogger()->Error(fmt, ##__VA_ARGS__)
#define LOGFATALDEFAULT(fmt, ...) wwlog::DefaultLogger()->Fatal(fmt, ##__VA_ARGS__)


}  // namespace wwlog
