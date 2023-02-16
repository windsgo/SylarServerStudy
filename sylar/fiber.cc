#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "log.h"
#include "util.h"

#include <atomic>

namespace sylar
{

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id {0};
static std::atomic<uint64_t> s_fiber_count {0};

static thread_local Fiber* t_fiber = nullptr;
static thread_local std::shared_ptr<Fiber> t_threadFiber = nullptr;

static ConfigVar<uint32_t>::ptr g_fiber_stack_size 
	= Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");

class MallocStackAllocator {
public:
	static void* Alloc(size_t size) {
		return malloc(size);
	}

	static void Dealloc(void *vp, size_t size) {
		return free(vp);
	}
};

using StackAllocator = MallocStackAllocator;

Fiber::Fiber() {
	// 一定是main fiber 的构造
	m_state = State::EXEC;
    Fiber::SetThis(this);

	if (getcontext(&m_ctx)) {
	    SYLAR_ASSERT2(false, "getcontext");	
	}

    ++s_fiber_count;

    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber main_fiber id=" << m_id;
}

Fiber::Fiber(std::function<void ()> cb, size_t stactsize) 
    :m_id(++s_fiber_id)
     , m_cb(cb) {
    
    // 主协程不存在则创建主协程
    if (!t_threadFiber) {
        Fiber::GetThis();
    }

    // 此时主协程一定存在
    SYLAR_ASSERT(t_threadFiber);
        
    // 子协程构造一定在主协程完成，即当前协程一定是主协程
    SYLAR_ASSERT2(t_fiber == t_threadFiber.get(), "child fiber should be constructed in main fiber");

	++s_fiber_count;
    m_stacksize = stactsize ? stactsize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);
    if (getcontext(&m_ctx)) {
	    SYLAR_ASSERT2(false, "getcontext");	
    }

#ifdef SYLAR_FIBER_RETURN_USE_UCLINK
	m_ctx.uc_link = &t_threadFiber->m_ctx;
#else // not SYLAR_FIBER_RETURN_USE_UCLINK
    m_ctx.uc_link = nullptr;
#endif // SYLAR_FIBER_RETURN_USE_UCLINK
	m_ctx.uc_stack.ss_sp = m_stack;
	m_ctx.uc_stack.ss_size = m_stacksize;

	makecontext(&m_ctx, &Fiber::MainFunc, 0);

    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
}  

Fiber::~Fiber() {
	--s_fiber_count;
	if (m_stack) {
		SYLAR_ASSERT(m_state == State::TERM || m_state == State::INIT || m_state == State::EXCEP);

		StackAllocator::Dealloc(m_stack, m_stacksize);
	} else {
		// 主协程，无栈
		SYLAR_ASSERT(!m_cb);
		SYLAR_ASSERT(m_state == State::EXEC);

		Fiber* cur = t_fiber;
		if (cur == this) {
            Fiber::SetThis(nullptr);
		}
	}
    
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id;
    // std::cout << sylar::BacktraceToString(100) << std::endl;
}

void Fiber::reset(std::function<void ()> cb) {
    SYLAR_ASSERT(m_stack); // 要求一定有栈，即非主协程
	SYLAR_ASSERT(m_state == State::TERM || m_state == State::INIT || m_state == State::EXCEP);

    m_cb = cb;
    if (getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }

#ifdef SYLAR_FIBER_RETURN_USE_UCLINK
	m_ctx.uc_link = &t_threadFiber->m_ctx;
#else // not SYLAR_FIBER_RETURN_USE_UCLINK
    m_ctx.uc_link = nullptr;
#endif // SYLAR_FIBER_RETURN_USE_UCLINK
    m_ctx.uc_stack.ss_sp = m_stack;
	m_ctx.uc_stack.ss_size = m_stacksize;
    
    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = State::INIT;
}

void Fiber::swapIn() {
    Fiber::SetThis(this);
    SYLAR_ASSERT(m_state != State::EXEC);
    m_state = State::EXEC;
    if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

void Fiber::swapOut() {
    Fiber::SetThis(t_threadFiber.get());
    // m_state = State::HOLD;
    if (swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }    
}

void Fiber::SetThis(Fiber *f) {
    t_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }
    
    // Note: 这里因为Fiber的默认构造函数是private的
    // 所以不能使用make_shared来进行构造
    // Fiber::ptr main_fiber = std::make_shared<Fiber>();
    Fiber::ptr main_fiber(new Fiber());
    SYLAR_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;

    return t_fiber->shared_from_this();
}

void Fiber::YieldToReady() {
    Fiber::ptr cur = Fiber::GetThis();
    cur->m_state = State::READY;
    cur->swapOut();
}

void Fiber::YieldToHold() {
    Fiber::ptr cur = Fiber::GetThis();
    cur->m_state = State::HOLD;
    cur->swapOut();
}

uint64_t Fiber::TotalFibers() {
	return s_fiber_count;
}

void Fiber::MainFunc() {
    Fiber::ptr cur = Fiber::GetThis();
    SYLAR_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = State::TERM;
    } catch (std::exception& ex) {
        cur->m_state = State::EXCEP;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what();
    } catch (...) {
        cur->m_state = State::EXCEP;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except";
    }

#ifdef SYLAR_FIBER_RETURN_USE_UCLINK
    // by setting m_ctx->uc_link = &t_threadFiber->m_ctx at construction 
    // Way 1
    // do not need to do anything more here just
    SetThis(t_threadFiber.get());
    // let it returns
#else // not SYLAR_FIBER_RETURN_USE_UCLINK
    // Way 2
    // usr swapOut to force fiber to return to the main fiber
   
    auto raw_ptr = cur.get();
    // decrease the reference count of t_fiber 's shared_ptr, for auto destrcution later
    cur.reset(); 
    raw_ptr->swapOut();
    
    // won't return here again
    SYLAR_ASSERT2(false, "never reach after swapout");

    // this fiber should not be swapIn to here again
#endif
}

uint64_t Fiber::GetFiberId() {
    if (t_fiber) {
        return t_fiber->getId();
    }

    return 0;
}


} // namespace sylar
