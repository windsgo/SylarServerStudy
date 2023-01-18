#include <iostream>
#include "sylar/config.h"
#include "sylar/log.h"


int main(int argc, char** argv) {
    
    sylar::ConfigVar<int>::ptr g_int_value_config =
        sylar::Config::Lookup("system.port", (int)8080, "system port");
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_int_value_config->getValue();

    sylar::ConfigVar<float>::ptr g_float_value_config =
        sylar::Config::Lookup("system.port", (float)10.2f, "system port");
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << g_float_value_config->getValue();

    auto a = sylar::Config::Lookup<int>("system.port");
    if (a)
        SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << a->toString();

    auto b = sylar::Config::Lookup<float>("system.port");
    if (b)
        SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << b->toString();

    return 0;
}