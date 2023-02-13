#include "sylar.h"

#include <assert.h>


sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_assert() {
    SYLAR_LOG_INFO(g_logger) << "\n" << sylar::BacktraceToString(10, 2, "    ");
    
    SYLAR_ASSERT2(false, "my");
}

int main(int argc, char** argv) {
    SYLAR_LOG_INFO(g_logger) << __PRETTY_FUNCTION__;
    auto node = YAML::LoadFile(__ROOT_DIR__ "conf/log.yml");
    sylar::Config::LoadFromYaml(node);
    SYLAR_LOG_INFO(g_logger) << __PRETTY_FUNCTION__;

    std::cout << "--------------test_assert()--------------" << std::endl;
    test_assert();
    
    return 0;
}