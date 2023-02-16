#pragma once

#include <string>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>

#include <cstdint>
#include <cstdarg>

#include "singleton.h"
#include "util.h"
#include "thread.h" // 线程锁

#define SYLAR_LOG_LEVEL(logger, level)                                                              \
    if (logger->getLevel() <= level)                                                                \
        sylar::LogEventWrap(std::make_shared<sylar::LogEvent>(logger, level, __FILE__, __LINE__, 0, \
            sylar::GetThreadId(), sylar::GetFiberId(), time(0),                                     \
            sylar::Thread::GetName())).getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger)  SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...)                                                \
    if (logger->getLevel() <= level)                                                                \
        sylar::LogEventWrap(std::make_shared<sylar::LogEvent>(logger, level, __FILE__, __LINE__, 0, \
            sylar::GetThreadId(), sylar::GetFiberId(), time(0),                                     \
            sylar::Thread::GetName()))                                                               \
        .getEvent()                                                                                 \
        ->format(fmt, __VA_ARGS__)

#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...)  SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO,  fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...)  SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN,  fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()
#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)

namespace sylar {

// #define TEST_LOG_NO_MUTEX
// #define TEST_LOG_MUTEX_USLEEP

// #define SYLAR_LOG_SPINLOCK
// #define SYLAR_LOG_MUTEX

#ifdef TEST_LOG_NO_MUTEX
using LogMutex = NullMutex;
#else // not TEST_LOG_NO_MUTEX
    #if defined(SYLAR_LOG_MUTEX) 
    using LogMutex = Mutex;
    #elif defined(SYLAR_LOG_SPINLOCK)
    using LogMutex = Spinlock;
    #elif defined(SYLAR_LOG_CASLOCK)
    using LogMutex = CASLock;
    #else // not defined SYLAR_LOG_SPINLOCK and SYLAR_LOG_MUTEX and SYLAR_LOG_CASLOCK
    static_assert(false);
    #endif // SYLAR_LOG_SPINLOCK
#endif // TEST_LOG_NO_MUTEX

class Logger;
class LoggerManager;

class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const char* ToString(LogLevel::Level level);
    static LogLevel::Level FromString(const std::string& str);
};

class LogEvent {
public:
    using ptr = std::shared_ptr<LogEvent>;
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, 
            const char* file, int32_t line, uint32_t elapse, 
            uint32_t thread_id, uint32_t fiber_id, uint64_t time,
            const std::string& thread_name);
    ~LogEvent();

    const char* getFile() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    std::string getContent() const { return m_ss.str(); }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }
    LogLevel::Level getLevel() const { return m_level; }
    const std::string& getThreadName() const { return m_threadName; }

    std::stringstream& getSS() { return m_ss; }
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);
private:
    const char* m_file = nullptr;
    int32_t m_line = 0;
    uint32_t m_elapse = 0;
    uint32_t m_threadId = 0;
    uint32_t m_fiberId = 0;
    uint64_t m_time = 0;
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
    std::string m_threadName;

    std::stringstream m_ss;
};

class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr event);
    ~LogEventWrap();

    std::stringstream& getSS();
    LogEvent::ptr getEvent() { return m_event; }
private:
    LogEvent::ptr m_event;
};

/*
%m -- message
%p -- level
%P -- colored level(use in stdout)
%r -- time after launch
%c -- name of log
%t -- thread id
%d -- time
%f -- file name
%l -- line number
%T -- Tab
%F -- Fiber id
%N -- Thread name
*/
class LogFormatter {
public:
    using ptr = std::shared_ptr<LogFormatter>;
    LogFormatter(const std::string& pattern);

    std::string format(LogEvent::ptr event);

    class FormatItem {
    public:
        using ptr = std::shared_ptr<FormatItem>;
        // FormatItem(/*const std::string& fmt = ""*/) {}
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, LogEvent::ptr event) = 0;
    };

    void init();

    bool isError() const { return m_error; }

    const std::string& getPattern() const { return m_pattern; }
private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
    bool m_error = false;

};


class LogAppender {
public:
    using ptr = std::shared_ptr<LogAppender>;
    using MutexType = LogMutex;
    virtual ~LogAppender() {}

    virtual void log(LogEvent::ptr event) = 0;
    virtual std::string toYamlString() const = 0;

    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatter() const;

    void setLevel(LogLevel::Level level) { m_level = level; }
    LogLevel::Level getLevel() const { return m_level; }

    bool hasFormatter() const { MutexType::Lock lock(m_mutex); return m_has_formatter; }
protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
	mutable MutexType m_mutex;
    bool m_has_formatter = false;
};


class Logger : public std::enable_shared_from_this<Logger> {
    friend class LoggerManager;
public:
    using ptr = std::shared_ptr<Logger>;
    using MutexType = LogMutex;
    Logger(const std::string& name = "root");

    void log(LogEvent::ptr event);

    // void log(LogLevel::Level level, LogEvent::ptr event);

    // void debug(LogEvent::ptr event);
    // void info(LogEvent::ptr event);
    // void warn(LogEvent::ptr event);
    // void error(LogEvent::ptr event);
    // void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();
    std::list<LogAppender::ptr> getAppenders() { return m_appenders; }

    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level level) { m_level = level; }

    const std::string& getName() const { return m_name; }

    void setFormatter(LogFormatter::ptr val);
    void setFormatter(const std::string& val);
    LogFormatter::ptr getFormatter() const;

    std::string toYamlString() const;
private:
    std::string m_name;
    LogLevel::Level m_level; // when level >= m_level, log
    std::list<LogAppender::ptr> m_appenders;
    LogFormatter::ptr m_formatter; 
    mutable MutexType m_mutex;
    Logger::ptr m_root;
};

class StdoutLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    void log(LogEvent::ptr event) override;
    std::string toYamlString() const override; 
private:
};

class FileLogAppender : public LogAppender {
public:
    using ptr = std::shared_ptr<FileLogAppender>;
    FileLogAppender(const std::string& filename);
    ~FileLogAppender();
    void log(LogEvent::ptr event) override;
    std::string toYamlString() const override; 

    bool reopen(std::ios_base::openmode openmode = std::ios::out);

    const std::string& getFilename() const { return m_filename; }
private:
    std::string m_filename;
    std::ofstream m_filestream;
    uint64_t m_lastTime;
};

class LoggerManager {
public:
    LoggerManager();
    using MutexType = LogMutex;
    Logger::ptr getLogger(const std::string& name);
    
    void init();
    Logger::ptr getRoot() const { return m_root; }

    // void addLogger(Logger::ptr logger) {
    //     m_loggers[logger->getName()] = logger;
    // }

    std::string toYamlString() const;

private:
    mutable MutexType m_mutex;
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

using LoggerMgr = sylar::Singleton<LoggerManager>; // 提供单例构造器

void print_log_config_var(); // 打印logs信息
}
