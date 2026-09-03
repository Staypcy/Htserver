#include<htserver/logger/log_level.h>
#include<algorithm>
#include<array>
#include<cctype>
#include<string>

namespace htserver{

std::string_view LogLevel::ToString(Level level)noexcept{
    switch(level){
#define XX(name) case Level::LOG_LV_##name: return #name;
            XX(DEBUG)
            XX(INFO)
            XX(WARN)
            XX(ERROR)
            XX(FATAL)
            XX(OFF)
#undef XX
        default:
            return "UNKNOWN";
    }
}


LogLevel::Level LogLevel::FromString(std::string_view value)noexcept{
    std::array<char,6> normalized{};
    std::transform(value.begin(),value.end(),normalized.begin(),[](unsigned char c){
        return static_cast<char>(std::toupper(c));
    });

    const std::string_view upper_value(normalized.data(),normalized.size());
#define XX(name) if(upper_value==#name) return Level::LOG_LV_##name;
    XX(DEBUG)
    XX(INFO)
    XX(WARN)
    XX(ERROR)
    XX(FATAL)
    XX(OFF)
#undef XX
    return Level::LOG_LV_OFF;
}

}