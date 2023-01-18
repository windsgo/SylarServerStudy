#include "log.h"

#include <map>
#include <unordered_map>
#include <functional>

#include <cassert>
#include <ctime>
#include <cstring>

namespace sylar{

const char* LogLevel::ToString(LogLevel::Level level) {
    switch (level) {
#define XX(name) \
    case LogLevel::name: \
        return #name; \
        break;
    
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << LogLevel::ToString(event->getLevel());
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLogger()->getName();
    }
};

class ThreadIDFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIDFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class FiberIDFormatItem : public LogFormatter::FormatItem {
public:
    FiberIDFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        : m_format(format) {
        if (m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str) 
        : m_string(str) {

    }
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        os << '\t';
    }
};

static std::unordered_map<LogLevel::Level, const char*> stdout_colors = {
    {LogLevel::DEBUG, "\033[32m\033[1m"},
    {LogLevel::INFO,  "\033[0m\033[1m "},
    {LogLevel::WARN,  "\033[33m\033[1m "},
    {LogLevel::ERROR, "\033[31m\033[1m"},
    {LogLevel::FATAL, "\033[35m\033[1m"}
};

static const char *stdout_clearfmt = "\033[0m";

// 带颜色的Level输出, 终端输出时用%P代替%p输出level颜色，给StdoutAppender设置
class ColorLevelFormatItem : public LogFormatter::FormatItem {
public:
    ColorLevelFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        auto level = event->getLevel();
        os << stdout_colors[level] << LogLevel::ToString(level) << stdout_clearfmt;
    }
};

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level, const char* file, int32_t line, uint32_t elapse
    , uint32_t thread_id, uint32_t fiber_id, uint64_t time) 
    : m_file(file), m_line(line), m_elapse(elapse)
    , m_threadId(thread_id), m_fiberId(fiber_id), m_time(time)
    , m_logger(logger), m_level(level) {

}

LogEvent::~LogEvent() {

}

void LogEvent::format(const char* fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

LogEventWrap::LogEventWrap(LogEvent::ptr event) 
    :m_event(event) {

}

LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event);
}

std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}


Logger::Logger(const std::string& name) 
    : m_name(name) 
    , m_level(LogLevel::DEBUG){
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
}

void Logger::addAppender(LogAppender::ptr appender) {
    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    for (auto it = m_appenders.begin();
            it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::log(LogEvent::ptr event) {
    if (event->getLevel() >= m_level) {
        // auto self = shared_from_this();
        for (auto& i : m_appenders) {
            i->log(event);
        }
    }
}

/*
void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        // auto self = shared_from_this();
        for (auto& i : m_appenders) {
            i->log(event);
        }
    }
}

void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}

void Logger::info(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}

void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARN, event);
}

void Logger::error(LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
}
*/

void StdoutLogAppender::log(LogEvent::ptr event)  {
    if (event->getLevel() >= m_level) {
        std::cout << m_formatter->format(event);
    }
}

FileLogAppender::FileLogAppender(const std::string& filename) 
    :m_filename(filename) {
    if (!reopen()) {
        std::cout << "open log file failed:" << filename << std::endl;
    }
}

bool FileLogAppender::reopen() {
    if (m_filestream) {
        m_filestream.close();
    }

    // m_filestream.open(m_filename, std::ios::app);
    m_filestream.open(m_filename);
    return !!m_filestream;
}

void FileLogAppender::log(LogEvent::ptr event)  {
    if (event->getLevel() >= m_level) {
        m_filestream << m_formatter->format(event);
    }
}

LogFormatter::LogFormatter(const std::string& pattern) 
    : m_pattern(pattern) {
    init();
}

std::string LogFormatter::format(LogEvent::ptr event) {
    std::stringstream ss;
    for (auto& item : m_items) {
        item->format(ss, event);
    }

    return ss.str();
}

// %xxx %xxx{xxx} %%
void LogFormatter::init() {
    // str format type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    std::string pstr;
    std::string pfmt;

    enum ParseStatus {
        PS_NORMAL_STRING = 0,
        PS_PARSING_STR,
        PS_PARSING_FMT
    } status = PS_NORMAL_STRING;

    for (size_t i = 0; i < m_pattern.size(); ++i) {
        switch (status) {
            case PS_NORMAL_STRING: {
                if (m_pattern[i] == '%') {
                    if ((i + 1) < m_pattern.size() && m_pattern[i + 1] == '%') {
                        nstr.append(1, '%');
                        ++i;
                        break; // switch
                    }
                    status = PS_PARSING_STR;
                    if (!nstr.empty()) {
                        vec.emplace_back(nstr, "", 0);
                        nstr.clear();
                    }
                    break; // switch
                }

                nstr.append(1, m_pattern[i]);
                break; // switch
            }
            case PS_PARSING_STR: {
                if (m_pattern[i] == '{') {
                    status = PS_PARSING_FMT;
                    break; // switch
                }

                if (m_pattern[i] == '}') {
                    status = PS_NORMAL_STRING;
                    vec.emplace_back("<<error_pattern>>", "", 0);
                    pstr.clear();
                    pfmt.clear();
                    break;
                }

                if (m_pattern[i] == '%') {

                    vec.emplace_back(pstr, pfmt, 1);
                    pstr.clear();
                    pfmt.clear();

                    if ((i + 1) < m_pattern.size() && m_pattern[i + 1] == '%') {
                        status = PS_NORMAL_STRING;
                        nstr.append(1, '%');
                        ++i;
                    } else {
                        status = PS_PARSING_STR;
                    }

                    break; // switch
                }

                if (!isalpha(m_pattern[i])) {
                    status = PS_NORMAL_STRING;
                    vec.emplace_back(pstr, pfmt, 1);
                    nstr.append(1, m_pattern[i]);
                    pstr.clear();
                    pfmt.clear();
                    break; // switch
                }

                pstr.append(1, m_pattern[i]);
                break; // switch
            }
            case PS_PARSING_FMT: {
                if (m_pattern[i] == '}') {
                    status = PS_NORMAL_STRING;
                    vec.emplace_back(pstr, pfmt, 1);
                    pstr.clear();
                    pfmt.clear();
                    break; // switch
                }

                if (m_pattern[i] == '{' /*|| isspace(m_pattern[i]) || m_pattern[i] == '%'*/) {
                    status = PS_NORMAL_STRING;
                    vec.emplace_back("<<error_pattern>>", "", 0);
                    pstr.clear();
                    pfmt.clear();
                    break;
                }

                pfmt.append(1, m_pattern[i]);
                break; // switch
            }
            default:
                assert(false);
                break; // switch
        }
    }

    // parse end check
    if (status == PS_NORMAL_STRING && !nstr.empty()) {
        vec.emplace_back(nstr, "", 0);
    } else if (status == PS_PARSING_STR) {
        vec.emplace_back(pstr, pfmt, 1);
    } else {
        vec.emplace_back("<<error_pattern>>", "", 0);
    }
/*
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        // m_pattern[i] == '%'
        // met a '%', start to parse
        // process m_pattern[i + 1] , [i + 2], ...
        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str; // %___
        std::string fmt; // %xxx{___}
        while (n < m_pattern.size()) {
            if (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}') {
                break;
            }

            // haven't met a space, continue to parse

            if (fmt_status == 0) {
                if (m_pattern[n] == '{') {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    fmt_status = 1;
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            }

            else if (fmt_status == 1) {
                if (m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 2;
                    break;
                }
            }
            
            ++n;
        }

        // met a space without having met a couple of '{}'
        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, "", 0));
                nstr.clear();
            }
            str = m_pattern.substr(i + 1, n - i - 1);
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n;
        } 
        // pattern error
        else if (fmt_status == 1) {
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        } 
        // parse over
        else if (fmt_status == 2) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, "", 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n;
        }

        nstr.clear();
    }

    if (!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }
*/
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
#define XX(str, C) \
        {#str, [](const std::string& fmt) {return FormatItem::ptr(new C(fmt));}}

        XX(m, MessageFormatItem),
        XX(p, LevelFormatItem),
        XX(r, ElapseFormatItem),
        XX(c, NameFormatItem),
        XX(t, ThreadIDFormatItem),
        XX(n, NewLineFormatItem),
        XX(d, DateTimeFormatItem),
        XX(f, FilenameFormatItem),
        XX(l, LineFormatItem),
        XX(T, TabFormatItem),
        XX(F, FiberIDFormatItem),
        XX(P, ColorLevelFormatItem)

#undef XX
    };

    for (auto& i : vec) {
        if (std::get<2>(i) == 0) {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }

        // std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }

    // std::cout << m_items.size() << std::endl;

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
    */

}

LoggerManager::LoggerManager() {
    m_root.reset(new Logger("root"));

    auto appender = std::make_shared<StdoutLogAppender>();
    auto formatter = std::make_shared<LogFormatter>("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%P]%T[%c]%T%f:%l%T%m%n");
    appender->setFormatter(formatter);
    m_root->addAppender(appender);
}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    auto it = m_loggers.find(name);
    return it == m_loggers.end() ? m_root : it->second;
}
    
void LoggerManager::init() {

}


}