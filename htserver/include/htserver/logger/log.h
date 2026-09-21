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

HTSERVER_API void LogPrintf(Logger& logger, LogLevel::Level level, uint32_t line, std::string_view file_name, const char* format, ...);

} // namespace htserver::detail

#define HTSERVER_LOG_LEVEL(logger,level)\
    if(auto htserver_log_logger = (logger);!htserver_log_logger){}\
    else if(const auto htserver_log_level=(level);!htserver_log_logger->shouldLog(htserver_log_level)){}\
    else htserver::detail::LogLine(*htserver_log_logger,htserver_log_level,__LINE__,__FILE__).stream()

#define HTSERVER_LOG_DEBUG(logger) HTSERVER_LOG_LEVEL((logger),htserver::LogLevel::Level::LOG_LV_DEBUG)
#define HTSERVER_LOG_INFO(logger) HTSERVER_LOG_LEVEL((logger),htserver::LogLevel::Level::LOG_LV_INFO)
#define HTSERVER_LOG_WARN(logger) HTSERVER_LOG_LEVEL((logger),htserver::LogLevel::Level::LOG_LV_WARN)
#define HTSERVER_LOG_ERROR(logger) HTSERVER_LOG_LEVEL((logger),htserver::LogLevel::Level::LOG_LV_ERROR)
#define HTSERVER_LOG_FATAL(logger) HTSERVER_LOG_LEVEL((logger),htserver::LogLevel::Level::LOG_LV_FATAL)

#define HTSERVER_LOG_FMT_LEVEL(logger,level,format,...)\
    if(auto htserver_log_logger=(logger);!htserver_log_logger){}\
    else if (const auto htserver_log_level = (level); !htserver_log_logger->shouldLog(htserver_log_level)) {} \
    else htserver::detail::LogPrintf(*htserver_log_logger,htserver_log_level,__LINE__,__FILE__,(format) __VA_OPT__(,)__VA_ARGS__)

#define HTSERVER_LOG_FMT_DEBUG(logger,format,...)\
    HTSERVER_LOG_FMT_LEVEL((logger),htserver::LogLevel::Level::LOG_LV_DEBUG,(format) __VA_OPT__(,)__VA_ARGS__)
#define HTSERVER_LOG_FMT_INFO(logger,format,...)\
    HTSERVER_LOG_FMT_LEVEL((logger),htserver::LogLevel::Level::LOG_LV_INFO,(format) __VA_OPT__(,)__VA_ARGS__)
#define HTSERVER_LOG_FMT_WARN(logger,format,...)\
    HTSERVER_LOG_FMT_LEVEL((logger),htserver::LogLevel::Level::LOG_LV_WARN,(format) __VA_OPT__(,)__VA_ARGS__)
#define HTSERVER_LOG_FMT_ERROR(logger,format,...)\
    HTSERVER_LOG_FMT_LEVEL((logger),htserver::LogLevel::Level::LOG_LV_ERROR,(format) __VA_OPT__(,)__VA_ARGS__)
#define HTSERVER_LOG_FMT_FATAL(logger,format,...)\
    HTSERVER_LOG_FMT_LEVEL((logger),htserver::LogLevel::Level::LOG_LV_FATAL,(format) __VA_OPT__(,)__VA_ARGS__) 


#define HTSERVER_LOG_ROOT() htserver::GetRootLogger()

#define HTSERVER_LOG_NAME(name) htserver::GetLogger((name))

#define LOG_DEBUG HTSERVER_LOG_DEBUG(g_logger)
#define LOG_INFO HTSERVER_LOG_INFO(g_logger)
#define LOG_WARN HTSERVER_LOG_WARN(g_logger)
#define LOG_ERROR HTSERVER_LOG_ERROR(g_logger)
#define LOG_FATAL HTSERVER_LOG_FATAL(g_logger)

#endif // HTSERVER_INCLUDE_HTSERVER_LOGGER_LOG_H