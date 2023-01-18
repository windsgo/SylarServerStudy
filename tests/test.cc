#include "sylar/log.h"
#include "sylar/util.h"

#include <iostream>
#include <unistd.h>
#include <cstdio>

int main(int argc, char* argv[]) {
    sylar::Logger::ptr logger = std::make_shared<sylar::Logger>("root");

    auto stdout_logappender = std::make_shared<sylar::StdoutLogAppender>();
    auto stdout_formatter = std::make_shared<sylar::LogFormatter>("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%P]%T[%c]%T%f:%l%T%m%n");
    stdout_logappender->setFormatter(stdout_formatter);
    logger->addAppender(stdout_logappender);

    auto file_appender = std::make_shared<sylar::FileLogAppender>("aa.log");
    logger->addAppender(file_appender);

    auto fmt = std::make_shared<sylar::LogFormatter>("[%p]%T%d:%T%m%n");
    file_appender->setFormatter(fmt);
    file_appender->setLevel(sylar::LogLevel::ERROR);

    // auto event = std::make_shared<sylar::LogEvent>(__FILE__, __LINE__, 0, sylar::GetThreadId(), sylar::GetFiberId(), time(0));

    // logger->log(sylar::LogLevel::Level::DEBUG, event);

    SYLAR_LOG_INFO(logger) << "test macro info";
    SYLAR_LOG_DEBUG(logger) << "test macro debug";
    SYLAR_LOG_WARN(logger) << "test macro warn";
    SYLAR_LOG_ERROR(logger) << "test macro error";
    SYLAR_LOG_FATAL(logger) << "test macro fatal";

    SYLAR_LOG_FMT_DEBUG(logger, "test fmt, %s", "abc");

    auto l = sylar::LoggerMgr::GetInstance()->getLogger("xx");
    SYLAR_LOG_INFO(l) << "xxx";

    return 0;
}