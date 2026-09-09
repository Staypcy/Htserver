#ifndef HTSERVER_INCLUDE_HTSERVER_UTILS_SYSTEM_UTILS
#define HTSERVER_INCLUDE_HTSERVER_UTILS_SYSTEM_UTILS

#include<chrono>

#include<htserver/macros.h>
#include<htserver/export.h>

namespace htserver{
[[nodiscard]] HTSERVER_API std::chrono::steady_clock::duration GetElapsedTIme()noexcept;

[[nodiscard]] HTSERVER_API uint64_t GetFiberId()noexcept;
}

#endif // HTSERVER_INCLUDE_HTSERVER_UTILS_SYSTEM_UTILS