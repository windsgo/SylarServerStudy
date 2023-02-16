#pragma once

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "thread.h"
#include "fiber.h"

namespace sylar {

using SchedulerMutexType = Mutex;

class Scheduler {
public:
    using ptr = std::shared_ptr<Scheduler>;
    using MutexType = SchedulerMutexType;
    
    // usr_caller=true: caller will be taken into this thread pool
    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    virtual ~Scheduler();

    const std::string& getName() const { return m_name; }

    static Scheduler* GetThis();
    static Fiber* GetMainFiber();

    void start();
    void stop();

    template <typename FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1) {
        bool need_tickle = false;
        { 
            MutexType::Lock lock(m_mutex); 
            need_tickle = scheduleNoLock(fc, thread);
        }

        if (need_tickle) {
           tickle(); 
        }
    }

    template <typename InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while(begin != end) {
                need_tickle = scheduleNoLock(&*begin) || need_tickle; // use FiberAndThread(T * f) { ...; ... = swap(*f); ...; }
                ++begin;
            }
        }

        if (need_tickle) {
            tickle();
        }
    }
    
protected:
    virtual void tickle();

private:
    template<typename FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty(); // need to notify the threads or not
        FiberAndThread ft(fc, thread);
        if (ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }

        return need_tickle;
    }
private:
    struct FiberAndThread {
        Fiber::ptr fiber;
        std::function<void ()> cb;
        int thread;

        FiberAndThread(Fiber::ptr f, int thr)
            : fiber(f), thread(thr) {}

        FiberAndThread(Fiber::ptr* f, int thr)
            : thread(thr) {
            fiber.swap(*f);
        }

        FiberAndThread(std::function<void()> f, int thr) 
            : cb(f), thread(thr) {}
        
        FiberAndThread(std::function<void()>* f, int thr) 
            : thread(thr) {
            cb.swap(*f);
        }

        FiberAndThread() : thread(-1) {} // default cotr for being put in STL
        
        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };

private:
    mutable MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;
    std::list<FiberAndThread> m_fibers;
    std::string m_name;
};

}
