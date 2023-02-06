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

    sylar::ConfigVar<float>::ptr g_float_value_config =
        sylar::Config::Lookup("system.value", (float)10.2f, "system value");

    sylar::ConfigVar<std::vector<int> >::ptr g_int_vec_value_config =
        sylar::Config::Lookup("system.int_vec", std::vector<int>{1, 2}, "system value");

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before:" << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before:" << g_float_value_config->getValue();

    auto v = g_int_vec_value_config->getValue();
    for (auto &i : v) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "before int_vec: " << i;
    }

    YAML::Node root = YAML::LoadFile("conf/log.yml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after:" << g_int_value_config->getValue();
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after:" << g_float_value_config->getValue();

    v = g_int_vec_value_config->getValue();
    for (auto &i : v) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "after int_vec: " << i;
    }
    // auto a = sylar::Config::Lookup<int>("system.port");
    // if (a)
    //     SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << a->toString();

    // auto b = sylar::Config::Lookup<float>("system.value");
    // if (b)
    //     SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << b->toString();


}

int main(int argc, char** argv) {
    init();


    test_yaml();
    std::cout << "----------\n";
    test_config();

    // test yaml


    return 0;
}