#include "log.h"

#include <iostream>
#include <unistd.h>
#include <cstdio>

int main(int argc, char* argv[]) {
    sylar::Logger::ptr logger = std::make_shared<sylar::Logger>("default");
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    logger->addAppender(sylar::LogAppender::ptr(new sylar::FileLogAppender("aa.log")));

    sylar::LogEvent::ptr event = std::make_shared<sylar::LogEvent>(__FILE__, __LINE__, 0, 1, 2, time(0));

    logger->log(sylar::LogLevel::Level::DEBUG, event);

    return 0;
}