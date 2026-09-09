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
    {}

    [[nodiscard]] LogRecordView getRecord()const noexcept{

    };

    Logger& m_logger;
    LogLevel::Level m_level;
    uint32_t m_line;
    std::string_view m_fileName;
    std::chrono::system_clock::time_point m_timmestamp;
    std::chrono::system_clock::duration m_elapsed;
    uint64_t m_threadId=0;
    uint64_t m_fiberId=0;
    std::string_view m_threadName;
    InlineBuffer<LOG_MESSAGE_INLINE_CAPACITY>m_inlineBuffer;
    SmallStreamBuffer<LOG_MESSAGE_INLINE_CAPACITY>m_streamBuffer;
    std::ostream m_stream;
};

}