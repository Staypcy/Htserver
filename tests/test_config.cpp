#include<cassert>
#include<filesystem>
#include<list>
#include<map>
#include<set>
#include<string>
#include<unordered_map>
#include<unordered_set>
#include<vector>

#include<htserver/config/config.h>

namespace config_test{

struct Person{
    std::string m_name;
    int m_age{0};
    bool m_sex{false};

    bool operator==(const Person&)const = default;
};
} // namespace config_test

namespace htserver{

template<>
class ConfigConvert<config_test::Person>
{
public:
    [[nodiscard]] static config_test::Person FromNode(const YAML::Node& node){
        return {
            node["name"].as<std::string>(),
            node["age"].as<int>(),
            node["sex"].as<bool>()
        };
    }

    [[nodiscard]] static YAML::Node ToNode(const config_test::Person& person){
        YAML::Node node(YAML::NodeType::Map);
        node["name"] = person.m_name;
        node["age"] = person.m_age;
        node["sex"] = person.m_sex;
        return node;
    }
};
} // namespace htserver

int main(int argc, char** argv)
{
    const std::filesystem::path config_directory = argc > 1 ? std::filesystem::path(argv[1]) : std::filesystem::path("../../etc");
    const std::string config_absolute_directory = std::filesystem::absolute(config_directory).lexically_normal().string();

    auto port = htserver::Config::Lookup("test.port", 8080, "test port");
    auto int_vector = htserver::Config::Lookup("test.int_vec", std::vector<int>{1,2,3}, "test int vector");
    auto int_list = htserver::Config::Lookup("test.int_list", std::list<int>{1,2,3}, "test int list");
    auto int_set = htserver::Config::Lookup("test.int_set", std::set<int>{1,2,3}, "test int set");
    auto int_unordered_set = htserver::Config::Lookup("test.int_unordered_set", std::unordered_set<int>{}, "test int unordered set");
    auto string_int_map = htserver::Config::Lookup("test.str_int_map", std::map<std::string, int>{}, "test string-int map");
    auto string_int_unordered_map = htserver::Config::Lookup("test.str_int_umap", std::unordered_map<std::string, int>{}, "test string-int umap");
    auto person = htserver::Config::Lookup("class.person", config_test::Person{}, "test person");
    auto person_map = htserver::Config::Lookup("class.map", std::map<std::string, config_test::Person>{}, "test person map");
    auto logs = htserver::Config::Lookup("logs", YAML::Node(YAML::NodeType::Sequence), "logger definiations");

    assert(port &&
        int_vector &&
        int_list &&
        int_set &&
        int_unordered_set &&
        string_int_map &&
        string_int_unordered_map &&
        person &&
        person_map &&
        logs);

    int port_change_count = 0;
    const htserver::ListenerId port_listener_id = port->addListener(
        [&port_change_count](const int& old_value, const int& new_value)
        {
            assert(old_value == 8080);
            assert(new_value == 5005);
            ++port_change_count;
        });
    assert(port_listener_id != 0);
    
    int logs_change_count = 0;
    const htserver::ListenerId logs_listener_id = logs->addListener(
        [&logs_change_count](const YAML::Node&, const YAML::Node&)
        {
            ++logs_change_count;
        });
    assert(logs_listener_id != 0);

    htserver::Config::LoadFromConfDir(config_absolute_directory);

    assert(port->getValue() == 5005);
    assert(port_change_count == 1);
    assert(logs_change_count == 1);
    assert(int_vector->getValue() == std::vector<int>({10, 20, 30}));
    assert(int_list->getValue() == std::list<int>({40, 50, 60, 40}));
    assert(int_set->getValue() == std::set<int>({20,30, 40}));
    assert(int_unordered_set->getValue() == std::unordered_set<int>({20, 30, 40}));
    assert((string_int_map->getValue() == std::map<std::string, int>({{"k", 30}, {"k1", 20}, {"k2", 40}})));
    assert((string_int_unordered_map->getValue() == std::unordered_map<std::string, int>({{"k", 30}, {"k1", 20}, {"k2", 40}})));

    const config_test::Person expected_person{"huajianke", 28, true};
    assert(person->getValue() == expected_person);

    const auto people = person_map->getValue();
    assert(people.size() == 2);
    assert(people.at("pans01") == config_test::Person({"花间客", 28, true}));
    assert(people.at("pans02") == config_test::Person({"花间浪子", 30, true}));

    const YAML::Node log_defs = logs->getValue();
    assert(log_defs.IsSequence());
    assert(log_defs.size() == 2);
    assert(log_defs[0]["name"].as<std::string>() == "root");
    assert(log_defs[0]["level"].as<std::string>() == "debug");
    assert(log_defs[0]["appenders"].IsSequence());
    assert(log_defs[0]["appenders"].size() == 2);
    assert(log_defs[0]["appenders"][0]["type"].as<std::string>() == "FileLogAppender");
    assert(log_defs[0]["appenders"][0]["file"].as<std::string>() == "log.txt");
    assert(log_defs[0]["appenders"][1]["type"].as<std::string>() == "StdoutLogAppender");
    assert(log_defs[1]["name"].as<std::string>() == "system");

    htserver::Config::LoadFromConfDir(config_absolute_directory);
    assert(port_change_count == 1);
    assert(logs_change_count == 1);

    htserver::Config::LoadFromConfDir(config_absolute_directory, true);
    assert(port_change_count == 1);
    assert(logs_change_count == 2);

    port->removeListener(port_listener_id);
    logs->removeListener(logs_listener_id);
    assert(port->listenersCount() == 0);
    assert(logs->listenersCount() == 0);

    port->setValue(6000);
    assert(port_change_count == 1);

    return 0;
}

