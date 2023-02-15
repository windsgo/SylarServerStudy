#pragma once

#include <ucontext.h>
#include <memory>
#include <functional>

#include "thread.h"

namespace sylar
{

//! Do not create this class on stack
class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    using ptr = std::shared_ptr<Fiber>;
    
    enum class State {
        INIT,
        HOLD,
        EXEC,
        TERM,
        READY,
        EXCEP
    };

private:
    Fiber();

public:
    Fiber(std::function<void ()> cb, size_t stactsize = 0);
    ~Fiber();

    // 重置协程函数，并重置状态
    // INIT, TERM
    void reset(std::function<void ()> cb);

    // 切换到此协程
    void swapIn();

    // 切换到后台
    void swapOut();

    uint64_t getId() const { return m_id; }

public:
	// 设置当前协程
	static void SetThis(Fiber* f);

    // 返回当前执行点的协程
    static Fiber::ptr GetThis();

    // 协程切换到后台，并设置为Ready状态
    static void YieldToReady();
    
    // 协程切换到后台，并设置为Hold状态
    static void YieldToHold();

    // 总协程数
    static uint64_t TotalFibers();

   	static void MainFunc(); 
    
    static uint64_t GetFiberId();

private:
	uint64_t m_id = 0;
	uint32_t m_stacksize = 0;
	State m_state = State::INIT;

	ucontext_t m_ctx;
	void *m_stack = nullptr;

	std::function<void ()> m_cb;
};

} // namespace sylar
