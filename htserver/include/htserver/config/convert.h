#ifndef  HTSERVER_INCLUDE_HTSERVER_CONFIG_CONVERT_H
#define HTSERVER_INCLUDE_HTSERVER_CONFIG_CONVERT_H

#include<charconv>
#include<concepts>
#include<cstddef>
#include<list>
#include<map>
#include<set>
#include<system_error>
#include<type_traits>
#include<unordered_map>
#include<unordered_set>
#include<utility>
#include<vector>

#include<yaml-cpp/yaml.h>

#include<htserver/type_name.h>
#include<htserver/utils/string_utils.h>

namespace htserver{

namespace detail{

//处理转化异常/失败
HTSERVER_API void ThrowConvertError(std::string_view type_name, std::string_view reason, std::string_view value = {});

[[nodiscard]] HTSERVER_API bool ParseBool(std::string_view text, bool& out)noexcept;

[[nodiscard]] HTSERVER_API const std::string& ScalarText(const YAML::Node& node,std::string_view type_name);

template <typename T>
requires std::is_arithmetic_v<T> && (!std::is_same_v<std::remove_cv_t<T>, bool>)
[[nodiscard]] T ParseArithmetic(std::string_view text, std::string_view type_name)
{
    std::string_view body = StringUtils::Trim(text);
    if(body.empty())
    {
        ThrowConvertError(type_name, "空标量无法解析为数值");
    }

    if(body.front() == '+')
    {
        body.remove_prefix(1);
    }

    T value{};
    std::from_chars_result result{};
    if constexpr (std::is_integral_v<T>)
    {
        int base = 10;
        if(body.size() > 2 && body[0] == '0' && (body[1] == 'x' || body[1] == 'X'))
        {
            base = 16;
            body.remove_prefix(2);
        }
        result = std::from_chars(body.data(), body.data() + body.size(), value, base);
    }
    else
    {
        result = std::from_chars(body.data(), body.data() + body.size(), value);
    }
    if(result.ec != std::errc{} || result.ptr != body.data() + body.size())
    {
        ThrowConvertError(type_name, "无法解析为数值", text);
    }
    return value;
}

template <typename T>
requires std::is_arithmetic_v<T> && (!std::is_same_v<std::remove_cv_t<T>, bool>)
[[nodiscard]] YAML::Node ArithmeticToNode(const T& value)
{
    char buffer[64]{};
    const std::to_chars_result result = std::to_chars(buffer, buffer + sizeof(buffer), value);
    if(result.ec != std::errc{})
    {
        ThrowConvertError(TypeName<T>(), "数值类型无法格式化为字符串");
    }
    return YAML::Node(std::string(buffer, static_cast<std::size_t>(result.ptr - buffer)));
}

}
}
#endif //HTSERVER_INCLUDE_HTSERVER_CONFIG_CONVERT_H
