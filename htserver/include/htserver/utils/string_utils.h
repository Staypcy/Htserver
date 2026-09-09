#ifndef HTSERVER_INCLUDE_HTSERVER_UTILS_STRING_UTILS_H
#define HTSERVER_INCLUDE_HTSERVER_UTILS_STRING_UTILS_H

#include<cstring>
#include<cstdarg>
#include<string_view>

#include<htserver/export.h>

namespace htserver{
/*
*@brief StringUtils类，来承载通用字符操作
*/
class HTSERVER_API StringUtils final{
public:
    //静态类
    StringUtils()=delete;

    /*
    * @brief 宽字符转化位UTF-8，跨平台兼容
    * 
    * @param test
    * @return std::string 转化失败会断言，返回空字符串
    */
    [[nodiscard]] static std::string WStringToString(std::wstring_view text) noexcept;

    /* 
    * @brief UTF-8字符转化为宽字符
    * 
    * @prama text
    * @return std::wstring 转化失败会断言，返回空字符串
    */
    [[nodiscard]] static std::wstring StringToWString(std::string_view text) noexcept;
};
}


#endif // HTSERVER_INCLUDE_HTSERVER_UTILS_STRING_UTILS_H