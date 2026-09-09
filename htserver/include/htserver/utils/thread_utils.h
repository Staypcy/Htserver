#ifndef HTSERVER_INCLUDE_HTSERVER_UTILS_THREAD_UTILS_H
#define HTSERVER_INCLUDE_HTSERVER_UTILS_THREAD_UTILS_H

#include<string>
#include<string_view>

#include<htserver/export.h>
#include<htserver/macros.h>

namespace htserver{
/* 
* @brief 获取线程id
* 在Linux下返回内核线程id，在windows下返回系统线程id
*/
[[nodiscard]] HTSERVER_API uint64_t GetThreadId()noexcept;

/**
 * @brief 设置当前线程的名称。
 */
HTSERVER_API void SetThreadName(std::string name);

/**
 * @brief 获取当前线程的名称
 */
[[nodiscard]] HTSERVER_API std::string_view GetThreadName() noexcept;
}

#endif