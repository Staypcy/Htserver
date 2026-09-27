#include <cassert>
#include <cstddef>
#include <iostream>
#include <source_location>
#include <string>
#include <string_view>
#include <vector>

namespace htserver::detail {

template <typename T>
[[nodiscard]] consteval std::string_view TypeNameProbe() {
#if defined(_MSC_VER)
    return __FUNCSIG__;
#else
    return std::source_location::current().function_name();
#endif
}

[[nodiscard]] constexpr std::string_view ExtractMsvcTypeName(std::string_view signature) noexcept {
    constexpr std::string_view marker = "TypeNameProbe<";
    constexpr std::string_view suffix = ">(void)";

    const std::size_t marker_pos = signature.rfind(marker);
    if (marker_pos == std::string_view::npos) {
        return signature;
    }

    const std::size_t begin = marker_pos + marker.size();
    const std::size_t end = signature.rfind(suffix);
    if (end == std::string_view::npos || end <= begin) {
        return signature;
    }
    return signature.substr(begin, end - begin);
}

template <typename T>
[[nodiscard]] consteval std::string_view ExtractTypeName() noexcept {
    constexpr std::string_view signature = TypeNameProbe<T>();

#if defined(_MSC_VER)
    constexpr std::string_view type_name = ExtractMsvcTypeName(signature);
    static_assert(type_name != signature, "无法识别当前的MSVC的函数签名格式");
    return type_name;
#else
    constexpr std::string_view marker = "T = ";
    constexpr std::size_t marker_pos = signature.find(marker);

    if constexpr (marker_pos == std::string_view::npos) {
        return signature;
    } else {
        constexpr std::size_t begin = marker_pos + marker.size();
        constexpr std::size_t semicolon = signature.find(';', begin);
        constexpr std::size_t bracket = signature.find(']');
        constexpr std::size_t end = semicolon != std::string_view::npos ? semicolon : bracket;
        if constexpr (end == std::string_view::npos || end <= begin) {
            return signature.substr(begin);
        } else {
            return signature.substr(begin, end - begin);
        }
    }
#endif
}

template <typename T>
inline constexpr std::string_view TYPE_NAME = ExtractTypeName<T>();

} // namespace htserver::detail

namespace htserver {

template <typename T>
[[nodiscard]] consteval std::string_view TypeName() noexcept {
    return detail::TYPE_NAME<T>;
}

} // namespace htserver

namespace {

struct AnyStruct {};

enum class AnyEnum : unsigned int {
    Debug,
    Release,
};

template <typename T, int Size>
struct FixedArray {
    T values[Size];
};

template <typename T>
void PrintType(std::string_view label) {
    constexpr std::string_view type_name = htserver::TypeName<T>();
    static_assert(!type_name.empty());
    std::cout << label << " -> " << type_name << '\n';
}

} // namespace

int main() {
    static_assert(htserver::TypeName<int>() == "int");
    static_assert(htserver::TypeName<double>() == "double");
    static_assert(htserver::TypeName<std::vector<int>>() == "std::vector<int>");

    using Counter = int;
    static_assert(htserver::TypeName<Counter>() == htserver::TypeName<int>());

    constexpr std::string_view first = htserver::TypeName<AnyStruct>();
    constexpr std::string_view second = htserver::TypeName<AnyStruct>();
    static_assert(first == second);

    assert(first.data() == second.data());

    static_assert(htserver::TypeName<const int&>() != htserver::TypeName<int>());
    static_assert(htserver::TypeName<int*>() != htserver::TypeName<int>());
    static_assert(htserver::TypeName<int[3]>() != htserver::TypeName<int>());

    PrintType<int>("int");
    PrintType<const int>("const int");
    PrintType<const int&>("const int&");
    PrintType<int*>("int pointer");
    PrintType<int[3]>("int array");
    PrintType<std::string>("std::string");
    PrintType<std::vector<int>>("std::vector<int>");
    PrintType<AnyStruct>("user-defined type");
    PrintType<AnyEnum>("enum");
    PrintType<FixedArray<long, 4>>("class template");
    PrintType<int(*)(double)>("function pointer");

    const auto lambda = [](int value) { return value + 1; };
    PrintType<decltype(lambda)>("lambda");

    std::cout << "alias Counter -> " << htserver::TypeName<Counter>()
              << " (会被还原成本来类型)" << std::endl;
    return 0;
}