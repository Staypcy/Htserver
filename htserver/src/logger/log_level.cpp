#include<htserver/logger/log_level.h>
#include<algorithm>
#include<array>
#include<cctype>
#include<string>

namespace htserver{

namespace {
[[nodiscard]] bool EqualsIgnoreCase(std::string_view val1,std::string_view val2)noexcept{
    if(val1.size()!=val2.size()){
        return false;
    }
    for(std::size_t index=0;index<val1.size();index++){
        if(std::toupper(static_cast<unsigned char>(val1[index]))!=val2[index]){
            return false;
        }
    }
    return true;
}
}

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

#define XX(name) if(EqualsIgnoreCase(value,#name)) return Level::LOG_LV_##name;
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