#include <algorithm>
#include <filesystem>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <vector>

#include<htserver/config/config.h>
#include<htserver/utils/fs_utils.h>
#include<htserver/macros.h>

namespace htserver{

namespace{

using ConfigVarMap = Config::ConfigVarMap;
using FileWriteTimestampMap = std::unordered_map<std::string,std::filesystem::file_time_type>;

ConfigVarMap& GetDatas(){
    static ConfigVarMap s_datas;
    return s_datas;
}

std::shared_mutex& GetMutex(){
    static std::shared_mutex s_mutex;
    return s_mutex;
}

FileWriteTimestampMap& GetLoadedFileWriteTimestamps()
{
    static FileWriteTimestampMap s_write_timestamps;
    return s_write_timestamps;
}

std::mutex& GetLoadFileMutex(){
    static std::mutex s_mutex;
    return s_mutex;
}

bool IsFileUnchanged(const std::string& path, const std::filesystem::file_time_type& write_time){
    std::lock_guard<std::mutex>lock(GetLoadFileMutex());
    const auto it = GetLoadedFileWriteTimestamps().find(path);
    return it != GetLoadedFileWriteTimestamps().end() && it->second == write_time;
}

void MarkFileLoaded(std::string path, const std::filesystem::file_time_type& write_time){
    std::lock_guard<std::mutex>lock(GetLoadFileMutex());
    GetLoadedFileWriteTimestamps().insert_or_assign(std::move(path), write_time);
}

ConfigVarBase::ptr LookupFromMap(const std::string& name)
{
    std::shared_lock<std::shared_mutex> lock(GetMutex());
    const auto it = GetDatas().find(name);
    return it ==GetDatas().end()? nullptr: it->second;
}

void CollectMembers(const std::string& prefix, const YAML::Node& node, std::vector<std::pair<std::string,YAML::Node>>& output){
    if(!prefix.empty()){
        output.emplace_back(prefix,node);
    }

    if(!node.IsMap()){
        return ;
    }

    for(const auto& entry: node){
        if(!entry.first.IsScalar()){
            detail::ReportConfigError("Config::LoadFromYaml",prefix,{},"映射的键必须是标量,已跳过");
            ASSERT_CONTINUE(false);
        }

        std::string key =detail::NormalizeConfigName(entry.first.Scalar());
        if(!detail::IsValidConfigName(key)){
            detail::ReportConfigError("Config::LoadFromYaml",key,{},"配置项名包含非法字符，已跳过");
            ASSERT_CONTINUE(false);
        }
        if(!prefix.empty()){
            key.insert(0,1,'.');
            key.insert(0,prefix);
        }
        CollectMembers(key,entry.second,output);
    }
} 

}// namespace

ConfigVarBase::ptr Config::LookupBase(std::string_view name){
    return LookupFromMap(NormalizeName(name));
}

bool Config::IsValidName(std::string_view name) noexcept{
    return detail::IsValidConfigName(name);
}

std::string Config::NormalizeName(std::string_view name){
    return detail::NormalizeConfigName(std::string(name));
}

std::size_t Config::Size(){
    std::shared_lock<std::shared_mutex>lock(GetMutex());
    return GetDatas().size();
}

ConfigVarBase::ptr Config::CheckType(ConfigVarBase::ptr var,std::string_view type_name){
    if(var->getTypeName() !=type_name){
        detail::ReportConfigError("Congif::Loopup",var->getName(),type_name,std::string("已注册为其他类型：").append(var->getTypeName()));
    }
}

}
