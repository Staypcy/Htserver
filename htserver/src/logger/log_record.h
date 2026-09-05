#ifndef HTSERVER_SRC_LOGGER_LOG_RECORD_H
#define HTSERVER_SRC_LOGGER_LOG_RECORD_H

#include <chrono>
#include <string_view>

#include <htserver/logger/log_level.h>
#include <htserver/macros.h>

namespace htserver::detail {

struct LogRecordView
{
    LogLevel::Level m_level = LogLevel::Level::LOG_LV_DEBUG;
    std::string_view m_loggerName; // 日志器的名字
    std::string_view m_message; // 日志内容
    std::chrono::system_clock::time_point m_timestamp; // 时间戳
    std::chrono::steady_clock::duration m_elapsed; // 起服已过时间
    uint16_t m_threadId = 0; // 线程ID
    uint64_t m_fiberId = 0; // 协程ID
    std::string_view m_threadName; // 线程名
    std::string_view m_fileName; // 文件名
    uint32_t m_line = 0; // 行号
};

} // namespace htserver::detail

#endif // HTSERVER_SRC_LOGGER_LOG_RECORD_H
