#ifndef HTSERVER_INCLUDE_HTSREVER_LOGGER_APPENDER_H
#define HTSERVER_INCLUDE_HTSREVER_LOGGER_APPENDER_H

#include<memory>
#include<string>

#include<htserver/export.h>
#include<htserver/logger/log_level.h>

namespace htserver{

namespace detail{
class AppenderAccess;//代理内部权限,严格控制访问权限
}
class HTSERVER_API Appender final{
public:
    class Impl;
    ~Appender()=default;
    Appender(const Appender&)=delete;
    Appender& operator=(const Appender&)=delete;
    Appender(Appender&&)=delete;
    Appender& operator=(Appender&&)=delete;

    void setLevel(LogLevel::Level level)noexcept;
    [[nodiscard]] LogLevel::Level getLevel()const noexcept;

    //刷新接口
    void flush();//流缓冲到操作系统脏页
    void sync();//脏页到到磁盘缓冲区
private:
    explicit Appender(std::unique_ptr<Impl>impl)noexcept;
    std::unique_ptr<Impl>m_impl;
    friend class detail::AppenderAccess;
};

using AppenderPtr =std::shared_ptr<Appender>;

[[nodiscard]] HTSERVER_API AppenderPtr MakeStdoutAppender();
[[nodiscard]] HTSERVER_API AppenderPtr MakeFileAppender(std::string file_name);

}

#endif //HTSERVER_INCLUDE_HTSREVER_LOGGER_APPENDER_H