#include<array>
#include<cstdarg>
#include<memory>
#include<new>
#include<ostream>
#include<vector>

#include<htserver/logger/log.h>
#include<htserver/utils/system_utils.h>
#include<htserver/utils/thread_utils.h>

#include"logger/buffer.h"
#include"logger/buffer_config.h"
#include"logger/logger_impl.h"
#include"logger/log_record.h"

namespace htserver::detail{

struct LogLine::Impl final{
    Impl(Logger& logger,LogLevel::Level level,uint32_t line,std::string_view file_name)
        :m_logger(logger)
        ,m_level(level)
        ,m_line(line)
        ,m_fileName(file_name)
        ,m_timestamp(std::chrono::system_clock::now())
        ,m_elapsed(GetElapsedTIme())
        ,m_threadId(GetThreadId())
        ,m_fiberId(GetFiberId())
        ,m_threadName(GetThreadName())
        ,m_streamBuffer(m_inlineBuffer)
        ,m_stream(&m_streamBuffer)
    {}

    [[nodiscard]] LogRecordView getRecord()const noexcept{
        return{
            m_level,
            m_logger.getName(),
            m_inlineBuffer.view(),
            m_timestamp,
            m_elapsed,
            m_threadId,
            m_fiberId,
            m_threadName,
            m_fileName,
            m_line
        };
    };

    Logger& m_logger;
    LogLevel::Level m_level;
    uint32_t m_line;
    std::string_view m_fileName;
    std::chrono::system_clock::time_point m_timestamp;
    std::chrono::system_clock::duration m_elapsed;
    uint64_t m_threadId=0;
    uint64_t m_fiberId=0;
    std::string_view m_threadName;
    InlineBuffer<LOG_MESSAGE_INLINE_CAPACITY>m_inlineBuffer;
    SmallStreamBuffer<LOG_MESSAGE_INLINE_CAPACITY>m_streamBuffer;
    std::ostream m_stream;
};

LogLine::LogLine(Logger& logger,LogLevel::Level level,uint32_t line,std::string_view file_name)
{

}

LogLine::Impl &LogLine::getImpl() noexcept
{
    
}

LogLine::~LogLine()noexcept{

}

std::ostream& LogLine::stream()noexcept{

}

}