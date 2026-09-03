#ifndef HTSERVER_INCLUDE_HTSERVER_LOGGER_LOG_LEVEL_H
#define HTSERVER_INCLUDE_HTSERVER_LOGGER_LOG_LEVEL_H

#include<cstdint>
#include<string_view>

#include<htserver/export.h>

namespace htserver{

class HTSERVER_API LogLevel final
{ 
public:
    enum class Level : std::uint8_t{
        LOG_LV_DEBUG=1,//记录调试细节，如变量值，程序执行流程
        LOG_LV_INFO=2,//记录程序运行状态，如启动，关闭，配置文件加载成功等
        LOG_LV_WARN=3,//记录程序运行中出现的异常情况，但不影响程序继续运行，如配置文件中某个参数不合法，使用默认值
        LOG_LV_ERROR=4,//记录程序运行中出现的错误情况，可能会影响程序继续运行
        LOG_LV_FATAL=5,//记录程序运行中出现的严重错误情况，可能会导致程序崩溃或无法继续运行
        LOG_LV_OFF=6,//关闭日志记录
    };

    [[nodiscard]] static std::string_view ToString(Level level)noexcept;
    [[nodiscard]] static LogLevel::Level FromString(std::string_view value)noexcept;

    
};

}
#endif // HTSERVER_INCLUDE_HTSERVER_LOGGER_LOG_LEVEL_H