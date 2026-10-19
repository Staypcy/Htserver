#ifndef HTSERVER_INCLUDE_HTSERVER_UTILS_FS_UTILS_H
#define HTSERVER_INCLUDE_HTSERVER_UTILS_FS_UTILS_H

#include<string>
#include<string_view>
#include<vector>

#include<htserver/export.h>

namespace htserver{

class HTSERVER_API FSUtils{    
public:
    FSUtils() = delete;
    static void ListAllFile(std::vector<std::string>& files,std::string_view path, std::string_view suffix= {});
    [[nodiscard]] static bool Mkdir(std::string_view dirname)noexcept;
    [[nodiscard]] static bool IsRuningPidfile(std::string_view pidfile)noexcept;
    [[nodiscard]] static bool Unlinke(std::string_view filename,bool exist = false)noexcept;
};
}

#endif // HTSERVER_INCLUDE_HTSERVER_UTILS_FS_UTILS_H
