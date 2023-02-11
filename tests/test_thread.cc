#include "sylar/sylar.h"
#include <unistd.h>

#include <thread>

static auto g_logger = SYLAR_LOG_ROOT();

sylar::RWMutex s_mutex;
sylar::Mutex mutex;

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
        // sylar::RWMutex::WriteLock w(s_mutex);
       	sylar::Mutex::Lock w(mutex); 
		++count;
    }
}

void fun2() {
    while (true) {
        // printf("AAAA");
        SYLAR_LOG_DEBUG(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3() {
    while (true) {
        // printf("BBBB");
        SYLAR_LOG_DEBUG(g_logger) << "=================================================";
    }
}


int main(int argc, char** argv) {
    // std::cout << __ROOT_DIR__ << std::endl;
    load_config(__ROOT_DIR__ "/" "conf/log2.yml");
    SYLAR_LOG_INFO(g_logger) << "thread test begin";    

    // sylar::Thread t([](){std::cout << "th\n";}, "test thread");
    // t.join();

    std::vector<sylar::Thread::ptr> thrs;
    const int thread_num = 2;
    for (int i = 0; i < thread_num; ++i) {
        auto thr1 = std::make_shared<sylar::Thread>(&fun2, "name_" + std::to_string(i * 2));
        auto thr2 = std::make_shared<sylar::Thread>(&fun3, "name_" + std::to_string(i * 2 + 1));
        thrs.push_back(thr1);
        thrs.push_back(thr2);
    }

    // std::thread t1(&fun2);
    // std::thread t2(&fun2);
    // std::thread t3(&fun3);
    // std::thread t4(&fun3);

    // t1.join();
    // t2.join();
    // t3.join();
    // t4.join();

    for (size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }
    

    SYLAR_LOG_INFO(g_logger) << "count=" << count;
    SYLAR_LOG_INFO(g_logger) << "thread test end";
    return 0;
}
