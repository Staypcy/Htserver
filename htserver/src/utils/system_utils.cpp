#include<htserver/utils/system_utils.h>

namespace htserver{
std::chrono::steady_clock::duration GetElapsedTIme()noexcept{
    static auto START_TIME =std::chrono::steady_clock::now();
    return std::chrono::steady_clock::now()-START_TIME;
}

uint64_t GetFiberId()noexcept{
    return 0;
}

}