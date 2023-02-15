#include "../sylar/sylar.h"
#include <yaml-cpp/node/parse.h>

static auto g_logger = SYLAR_LOG_ROOT();

static void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber begin";
    sylar::Fiber::YieldToHold();
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber end";
    sylar::Fiber::YieldToHold();
}

int main(int argc, char** argv) {
    sylar::Fiber::GetThis();

    YAML::Node node = YAML::LoadFile(__ROOT_DIR__ "conf/log.yml");
    sylar::Config::LoadFromYaml(node);
     
    SYLAR_LOG_INFO(g_logger) << "main begin";
    sylar::Fiber::ptr fiber = std::make_shared<sylar::Fiber>(run_in_fiber);
    fiber->swapIn();

    SYLAR_LOG_DEBUG(g_logger) << "fiber1 ref count:" << fiber.use_count();

    SYLAR_LOG_INFO(g_logger) << "main after swapIn";
    fiber->swapIn();
    SYLAR_LOG_DEBUG(g_logger) << "fiber1 ref count:" << fiber.use_count();

    SYLAR_LOG_INFO(g_logger) << "main end";

    return 0;
}
