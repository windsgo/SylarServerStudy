#include "sylar/sylar.h"
#include <unistd.h>

static auto g_logger = SYLAR_LOG_ROOT();

sylar::RWMutex s_mutex;

static void load_config(const std::string&& path) {
    std::cout << ">> loading config" << std::endl;
    YAML::Node root = YAML::LoadFile(path);
    sylar::Config::LoadFromYaml(root);  
}

int count = 0;

void fun1() {
    SYLAR_LOG_INFO(g_logger) << "name:" << sylar::Thread::GetName()
        << " this.name:" << sylar::Thread::GetThis()->getName()
        << " id:" << sylar::GetThreadId()
        << " this.id:" << sylar::Thread::GetThis()->getId();
    
    for (int i = 0; i < 100000; ++i) {
        sylar::RWMutex::WriteLock w(s_mutex);
        ++count;
    }
}

void fun2() {

}

int main(int argc, char** argv) {
    // std::cout << __ROOT_DIR__ << std::endl;
    load_config(__ROOT_DIR__ "/" "conf/log.yml");
    SYLAR_LOG_INFO(g_logger) << "thread test begin";    

    // sylar::Thread t([](){std::cout << "th\n";}, "test thread");
    // t.join();

    std::vector<sylar::Thread::ptr> thrs;
    const int thread_num = 5;
    for (int i = 0; i < thread_num; ++i) {
        auto thr = std::make_shared<sylar::Thread>(&fun1, "name_" + std::to_string(i));
        thrs.push_back(thr);
    }

    for (int i = 0; i < thread_num; ++i) {
        thrs[i]->join();
    }

    SYLAR_LOG_INFO(g_logger) << "count=" << count;
    SYLAR_LOG_INFO(g_logger) << "thread test end";
    return 0;
}