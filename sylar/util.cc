#include "util.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <execinfo.h>

#include "log.h"

#ifdef BOOST_STACKTRACE_USE_BACKTRACE
#include <boost/stacktrace.hpp>
#endif

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

namespace sylar {

pid_t GetThreadId() {
#if defined(__linux__)
    // 返回linux系统的线程号
    return syscall(SYS_gettid);
#elif defined(_WIN32)
    // do not support windows now
    static_assert(false);
#endif
}

uint32_t GetFiberId() {
    return 0;
}

void Backtrace(std::vector<std::string> &bt, [[maybe_unused]] int size, int skip) {
#ifdef BOOST_STACKTRACE_USE_BACKTRACE
    auto vec = boost::stacktrace::stacktrace().as_vector();
    bt.reserve(vec.size() - skip);
    for (size_t i = skip; i < vec.size(); ++i) {
        std::stringstream ss;
        ss << vec[i];
        bt.push_back(ss.str());
    }
#else // BOOST_STACKTRACE_USE_BACKTRACE
    void **array = (void**)malloc((sizeof(void*) * size));
    size_t s = ::backtrace(array, size);

    char** strings = backtrace_symbols(array, s);
    if (strings == NULL) {
        SYLAR_LOG_ERROR(g_logger) << "backtrace_symbols error";
        free(strings);
        free(array);
        return;
    }

    bt.reserve(s - skip);
    for (size_t i = skip; i < s; ++i) {
        bt.push_back(strings[i]);
    }

    free(strings);
    free(array);
#endif // BOOST_STACKTRACE_USE_BACKTRACE
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::stringstream ss;
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    for (size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << i << "# " << bt[i] << std::endl;
    }
    return ss.str();
}

} // namespace sylar