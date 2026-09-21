#ifndef HTSERVER_INCLUDE_HTSERVER_UTILS_SYSTEM_UTILS
#define HTSERVER_INCLUDE_HTSERVER_UTILS_SYSTEM_UTILS

#include<chrono>

#include<htserver/macros.h>
#include<htserver/export.h>

namespace htserver{
[[nodiscard]] HTSERVER_API std::chrono::steady_clock::duration GetElapsedTime()noexcept;

[[nodiscard]] HTSERVER_API uint64_t GetFiberId()noexcept;

[[nodiscard]] HTSERVER_API std::string GetBacktrace(int size=20,int skip=1,const std::string& prefix="");
}

#endif // HTSERVER_INCLUDE_HTSERVER_UTILS_SYSTEM_UTILS