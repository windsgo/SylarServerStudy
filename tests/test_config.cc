#include <iostream>
#include "sylar/config.h"
#include "sylar/log.h"

#include "yaml-cpp/yaml.h"

auto logger_test = std::make_shared<sylar::Logger>("test");
auto fmt = std::make_shared<sylar::LogFormatter>("%m%n");
auto appender = std::make_shared<sylar::StdoutLogAppender>();

#define LOG SYLAR_LOG_INFO(logger_test)

void init() {
    appender->setFormatter(fmt);
    logger_test->addAppender(appender);
}

void print_yaml(const YAML::Node& node, int level) {
    std::stringstream prefixss;
    for (int i = 0; i < level; ++i)
    {
        prefixss << "    ";
    }

    std::string prefix = prefixss.str();
    if (node.IsScalar())
    {
        LOG << prefix << node.Scalar() << "(S) - " 
            << node.Tag() << " - " << level;
    }
    else if (node.IsNull())
    {
        LOG << prefix << "NULL - " 
            << node.Tag() << " - " << level;
    }
    else if (node.IsMap())
    {
        // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "MAP - " 
        //     << node.Tag() << " - " << level;
        for (auto it = node.begin(); it != node.end(); ++it) {
            // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << it->first;
            // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << it->second;
            LOG << prefix << "[" << it->first << "] - "
                                             << it->second.Tag() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    }
    else if (node.IsSequence())
    {
        // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "SEQUENCE - " 
        //     << node.Tag() << " - " << level;
        for (size_t i = 0; i < node.size(); ++i) {
            LOG << prefix << "[" << i << "] - " 
                << node[i].Tag() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

void test_yaml() {
    try {
        YAML::Node root = YAML::LoadFile("conf/log.yml");

        // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << root;
        // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << root.Scalar();

        // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "\n" << root;
        // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << root["logs"].size();
        // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "\n" << root["logs"][0]["appender"];
        // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << root["logs"][0]["appender"].size();
        // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << root["logs"][0]["appender"][0]["file"].as<std::string>();
        // SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << root["logs"][0]["formatter"].Scalar();

        print_yaml(root, 0);
    }
    catch (std::exception &e)
    {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << e.what();
    }
}

void test_config() {
    sylar::ConfigVar<int>::ptr g_int_value_config =
        sylar::Config::Lookup("system.port", (int)8080, "system port");

    // test error
    sylar::ConfigVar<float>::ptr g_intx_value_config =
        sylar::Config::Lookup("system.port", (float)8080, "system port");

    sylar::ConfigVar<float>::ptr g_float_value_config =
        sylar::Config::Lookup("system.value", (float)10.2f, "system value");

    sylar::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config =
        sylar::Config::Lookup("system.int_vec", std::vector<int>{1, 2}, "system int vec");
        
    sylar::ConfigVar<std::list<int> >::ptr g_int_list_value_config =
        sylar::Config::Lookup("system.int_list", std::list<int>{1, 2}, "system int list");

    sylar::ConfigVar<std::set<int> >::ptr g_int_set_value_config =
        sylar::Config::Lookup("system.int_set", std::set<int>{1, 2}, "system int set");
        
    sylar::ConfigVar<std::unordered_set<int> >::ptr g_int_uset_value_config =
        sylar::Config::Lookup("system.int_uset", std::unordered_set<int>{1, 2}, "system int unordered_set");

    sylar::ConfigVar<std::map<std::string, int> >::ptr g_str_int_map_value_config =
        sylar::Config::Lookup("system.str_int_map", std::map<std::string, int>{{"k", 1}, {"p", 2}}, "system int str int map");
    
    sylar::ConfigVar<std::unordered_map<std::string, int> >::ptr g_str_int_umap_value_config =
        sylar::Config::Lookup("system.str_int_umap", std::unordered_map<std::string, int>{{"uk", 11}, {"up", 22}}, "system int str int umap");

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before:" << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before:" << g_float_value_config->getValue();

#define XX(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue();\
        for (auto &i : v) { \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ": " << i; \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

#define XX_M(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue();\
        for (auto &i : v) { \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name ": {" << i.first << " - " << i.second << "}"; \
        } \
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }

    XX(g_int_vec_value_config, int_vec, before);
    XX(g_int_list_value_config, int_list, before);
    XX(g_int_set_value_config, int_set, before);
    XX(g_int_uset_value_config, int_uset, before);
    XX_M(g_str_int_map_value_config, str_int_map, before);
    XX_M(g_str_int_umap_value_config, str_int_map, before);

    YAML::Node root = YAML::LoadFile("conf/log.yml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after:" << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after:" << g_float_value_config->getValue();

    
    XX(g_int_vec_value_config, int_vec, after);
    XX(g_int_list_value_config, int_list, after);
    XX(g_int_set_value_config, int_set, after);
    XX(g_int_uset_value_config, int_uset, after);
    XX_M(g_str_int_map_value_config, str_int_map, after);
    XX_M(g_str_int_umap_value_config, str_int_map, after);
#undef XX
#undef XX_M

    // auto a = sylar::Config::Lookup<int>("system.port");
    // if (a)
    //     SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << a->toString();

    // auto b = sylar::Config::Lookup<float>("system.value");
    // if (b)
    //     SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << b->toString();


}

// 示例如何自定义类型配置初始化
class Person {
public:
    std::string m_name = "";
    int m_age = 0;
    bool m_sex = 0;

    std::string toString() const {
        std::stringstream ss;
        ss << "[Person name = " << m_name
           << " age = " << m_age
           << " sex = " << m_sex
           << "]";
        return ss.str();
    }
};

namespace sylar {

template <>
class LexicalCast<std::string, Person > {
public:
    Person operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Person p;
        p.m_name = node["name"].as<std::string>();
        p.m_age = node["age"].as<int>();
        p.m_sex = node["sex"].as<bool>();

        return p;
    }
};

template <>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& p) {
        YAML::Node node;
        node["name"] = p.m_name;
        node["age"] = p.m_age;
        node["sex"] = p.m_sex;

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}

// 测试自定义类
void test_class() {
    sylar::ConfigVar<Person>::ptr g_person =
        sylar::Config::Lookup("class.person", Person(), "class person");
    
    sylar::ConfigVar<std::map<std::string, Person>>::ptr g_person_map =
        sylar::Config::Lookup("class.map", std::map<std::string, Person>{{"default", Person()}}, "class person");
    
    auto g_person_vec_map = 
        sylar::Config::Lookup("class.vec_map", std::map<std::string, std::vector<Person>>{
            {{"group1", {Person(), Person()}}}
        }, "class vec_map");

#define XX_PM(g_var, prefix) \
    {\
        auto m = g_var->getValue(); \
        for (auto& i : m) { \
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << prefix << ": " << i.first << " - " << i.second.toString(); \
        } \
    }

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.map before")
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before: " << g_person_vec_map->getValue().size() << '\n' << g_person_vec_map->toString();

    YAML::Node root = YAML::LoadFile("conf/log.yml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after : " << g_person->getValue().toString() << " - " << g_person->toString();
    XX_PM(g_person_map, "class.map after")
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after: " << g_person_vec_map->getValue().size() << '\n' << g_person_vec_map->toString();

#undef XX_PM
}

int main(int argc, char** argv) {
    init();


    std::cout << "----------test_yaml()----------\n";
    test_yaml();
    std::cout << "----------test_config()----------\n";
    test_config();
    std::cout << "----------test_class()----------\n";
    test_class();

    // test yaml


    return 0;
}