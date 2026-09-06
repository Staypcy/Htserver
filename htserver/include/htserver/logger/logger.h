#ifndef HTSERVER_INCLUDE_HTSERVER_LOGGER_LOGGER_H
#define HTSERVER_INCLUDE_HTSERVER_LOGGER_LOGGER_H

#include<memory>
#include<string>
#include<string_view>

#include<htserver/export.h>
#include<htserver/logger/appender.h>
#include<htserver/logger/log_level.h>

namespace htserver{

namespace detail{
class LoggerAccess; //桥接类，代理logger的访问权限
}

class HTSERVER_API Logger final{
public:
    class Impl;
    explicit Logger(std::string name ="root");
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    //是否构造日志
    [[nodiscard]] bool shouldLog(LogLevel::Level level)const noexcept;
    
    //操作接口
    void setLevel(LogLevel::Level level)noexcept;
    [[nodiscard]] LogLevel::Level getLevel()const noexcept;
    [[nodiscard]] std::string_view getName()const noexcept;

    void setFormatter(std::string_view pattern);
    [[nodiscard]] std::string getFormatterPattern()const;
    void addAppender(AppenderPtr appender);
    void removeAppender(const AppenderPtr& appender);
    void clearAppenders();

    void flush();
    void sync();
private:
    std::unique_ptr<Impl>m_impl;
    friend class detail::LoggerAccess;
};

using LoggerPtr =std::shared_ptr<Logger>;

//获取主日志器
[[nodiscard]] HTSERVER_API LoggerPtr GetRootLogger();
//获取日志器
[[nodiscard]] HTSERVER_API LoggerPtr GetLogger(std::string_view name);
}

#endif //HTSERVER_INCLUDE_HTSERVER_LOGGER_LOGGER_H