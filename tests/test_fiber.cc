#include "../sylar/sylar.h"
#include <memory>
#include <string>
#include <vector>
#include <yaml-cpp/node/parse.h>

static auto g_logger = SYLAR_LOG_ROOT();

static void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber begin";
    sylar::Fiber::YieldToHold();
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber end";
}


void func1() {
    SYLAR_LOG_INFO(g_logger) << "func1 1";
    sylar::Fiber::YieldToReady();
    SYLAR_LOG_INFO(g_logger) << "func1 2";
}

static void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "test_fiber begin";
    {
        sylar::Fiber::ptr fiber = std::make_shared<sylar::Fiber>(run_in_fiber);
        
        fiber->swapIn();
        // SYLAR_LOG_DEBUG(g_logger) << "fiber1 ref count:" << fiber.use_count();
        SYLAR_LOG_INFO(g_logger) << "main after swapIn 1";
        
        fiber->swapIn();
        // SYLAR_LOG_DEBUG(g_logger) << "fiber1 ref count:" << fiber.use_count();
        SYLAR_LOG_INFO(g_logger) << "main after swapIn 2";

        // fiber->reset(func1);
        // fiber->swapIn();
        // SYLAR_LOG_INFO(g_logger) << "main after reset and swapIn";

        // fiber->swapIn();
        // SYLAR_LOG_INFO(g_logger) << "main after reset and swapIn 2";
    }
    SYLAR_LOG_INFO(g_logger) << "test_fiber end";
}

int main(int argc, char** argv) {
    sylar::Thread::SetName("main_t");
    // sylar::Fiber::GetThis();
    YAML::Node node = YAML::LoadFile(__ROOT_DIR__ "conf/log.yml");
    sylar::Config::LoadFromYaml(node);
       
    SYLAR_LOG_INFO(g_logger) << "main begin";
    // test_fiber();
    
    std::vector<sylar::Thread::ptr> thrs;
    for (int i = 0; i < 3; ++i) {
        thrs.push_back(std::make_shared<sylar::Thread>(&test_fiber, "name_" + std::to_string(i)));
    }

    for (auto& i : thrs) {
        i->join();
    }
     
    SYLAR_LOG_INFO(g_logger) << "main end";

    return 0;
}
