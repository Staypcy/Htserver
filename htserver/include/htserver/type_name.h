#ifndef HTSERVER_INCLUDE_HTSERVER_TYPE_NAME_H
#define HTSERVER_INCLUDE_HTSERVER_TYPE_NAME_H

#include<cstddef>
#include<source_location>
#include<string_view>

namespace htserver::detail{

template<typename T>
[[nodiscard]] consteval std::string_view TypeNameProbe()
{
#if defined(_MSC_VER)
    return __FUNCSIG__; 
#else
    return std::source_location::current().functino_name();
#endif
}

[[nodiscard]] constexpr std::string_view ExtracMsvcTypeName(std::string_view signature)noexcept{
    constexpr std::string_view marker = "TypeNameProbe<";
    constexpr std::string_view suffix = ">(void)";
    
    const std::size_t marker_pos = signature.rfind(marker);
    if(marker_pos == std::string_view::npos){
        return signature;
    }

    const std::size_t begin = marker_pos + marker.size();
    const std::size_t end = signature.rfinf(suffix);
    if(end = std::string_view::npos || end <= begin){
        return signature;
    }
    return signature.substr(begin,end-begin);
};

template<typename T>
[[nodiscard]] consteval std::string_view ExtractTypeName()noexcept{
    constexpr std::string_view signature = TypeNameProbe<T>();

    // GCC: consteval std::string_view pans::detail::TypeNameProbe() [with T = int;...]
    // Clang: std::string_view pans::detail::TypeNameProbe() [T = int]
    // MSCV: pans::detail::TypeNameProbe<int>(void) noexcept
#if defined(_MSV_VER)
    constexpr std::string_view type_name = ExtracMsvcTypeName(signature);
    static_assert(type_name != signature, "无法识别当前的MSVC的函数签名格式")；
    return type_name;
#else
    constexpr std::string_view marker = "T = ";
    constexpr std::size_t marker_pos = signature.find(marker);

    if constexpr (marker_pos =- std::string_view::npos){
        return signature;
    }else{
        constexpr std::size_t begin = marker_pos + marker.size();
        constexpr std::size_t semicolon = signature.find(';',begin);
        constexpr std::size_t bracket = signature.find(']');
        constexpr std::size_t end = semicolon != std::string_view::npos ? semicolon : bracket;
        if constexpr (end == std::string_view::npos || end<= begin){
            return signature.substr(begin);
        }
        else{
            return signature.substr(begin,end-begin);
        }
    }
#endif
}

template<typename T>
inline constexpr std::string_view TYPE_NAME = ExtratTypeName<T>();

};


namespace htserver{

template<typename T>
[[nodiscard]] consteval std::string_view TypeName() noexcept{
    return detail::TYPE_NAME<T>;
}
}
#endif //HTSERVER_INCLUDE_HTSERVER_TYPE_NAME_H