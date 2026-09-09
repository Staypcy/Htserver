//日志工具,利用RAII
#ifndef HTSERVER_INCLUDE_HTSERVER_LOGGER_LOG_H
#define HTSERVER_INCLUDE_HTSERVER_LOGGER_LOG_H

#include<cstddef>
#include<iosfwd> //比iostream轻量
#include<string_view>

#include<htserver/logger/appender.h>
#include<htserver/logger/log_level.h>
#include<htserver/logger/logger.h>
#include<htserver/macros.h>

namespace htserver::detail{

class HTSERVER_API LogLine final{
public:
    LogLine(Logger& logger,LogLevel::Level level,uint32_t line,std::string_view file_name);
    ~LogLine()noexcept;

    LogLine(const LogLine&)=delete;
    LogLine& operator=(const LogLine&)=delete;
    LogLine(LogLine&&)=delete;
    LogLine& operator=(LogLine&&)=delete;

    [[nodiscard]] std::ostream& stream()noexcept;
private:
    struct Impl;

    static constexpr std::size_t LOG_LINE_IMPL_SIZE=1024;
    alignas(std::max_align_t)std::byte m_implStorage[LOG_LINE_IMPL_SIZE];

    [[nodiscard]] Impl& getImpl()noexcept;
};

HTSERVER_API void LogPrintf(Logger& logger, LogLevel::Level level, u32 line, std::string_view file_name, const char* format, ...);

} // namespace htserver::detail

#endif // HTSERVER_INCLUDE_HTSERVER_LOGGER_LOG_H