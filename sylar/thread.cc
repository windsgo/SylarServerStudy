#include "thread.h"

#include "log.h"

namespace sylar
{

static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system"); // 静态初始化顺序问题

Thread::Thread(std::function<void()> cb, const std::string &name) 
    : m_cb(cb), m_name(name)
{
    // 注意，在函数中，仍然处于构造者所在线程
    // 只有在thread::run中才会切换到新建的线程中
    if (name.empty()) {
        m_name = "UNKNOW";
    }

    int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
    if (rt) {
        SYLAR_LOG_ERROR(g_logger) << "pthread_create thread failed, rt=" << rt
            << " name=" << name;
        throw std::logic_error("pthread_create error");
    }

    m_semapore.wait(); // wait pthread created
}

Thread::~Thread() {
    if (m_thread) {
        pthread_detach(m_thread); // or join, which will block
    }
}

void Thread::join() {
    if (m_thread) {
        int rt = pthread_join(m_thread, nullptr);
        if (rt) {
            SYLAR_LOG_ERROR(g_logger) << "pthread_join thread failed, rt=" << rt
                << " name=" << m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

Thread *Thread::GetThis() {
    return t_thread;
}

const std::string &Thread::GetName() {
    return t_thread_name;
    // return t_thread->getName();
}


void Thread::SetName(const std::string& name) {
    if (t_thread) {
        t_thread->m_name = name;
    }
    t_thread_name = name;
}

void *Thread::run(void *arg)
{
    Thread* thread = static_cast<Thread*>(arg);
    // Thread* thread = (Thread*)(arg);
    t_thread = thread;
    t_thread_name = thread->getName();
    thread->m_id = sylar::GetThreadId();
    pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

    std::function<void()> cb;
    cb.swap(thread->m_cb);

    thread->m_semapore.notify();

    cb();
    return 0;
}

Semaphore::Semaphore(uint32_t count)
{
    if (sem_init(&m_semaphore, 0, count)) {
        throw std::logic_error("sem_init error");
    }
}

Semaphore::~Semaphore()
{
    sem_destroy(&m_semaphore);
}

void Semaphore::wait()
{
    if (sem_wait(&m_semaphore)) {
        throw std::logic_error("sem_wait error");
    }
}

void Semaphore::notify()
{
    if (sem_post(&m_semaphore)) {
        throw std::logic_error("sem_post error");
    }
}

} // namespace sylar
