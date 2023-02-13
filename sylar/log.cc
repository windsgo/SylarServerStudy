#include "log.h"

#include <map>
#include <unordered_map>
#include <functional>

#include <cassert>
#include <ctime>
#include <cstring>

#include <boost/filesystem.hpp>

#include "unistd.h"


#include "config.h"

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
        return "UNKNOW";
    }
    return "UNKNOW";
}

LogLevel::Level LogLevel::FromString(const std::string& str) {
    std::string ucstr = str;
    std::transform(ucstr.begin(), ucstr.end(), ucstr.begin(), ::toupper);
#define XX(name) \
    if (ucstr == #name) { \
        return LogLevel::name; \
    }

    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    return LogLevel::UNKNOW;
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem([[maybe_unused]] const std::string& str = "") {}
    void format(std::ostream& os, LogEvent::ptr event) override {
#ifdef TEST_LOG_MUTEX_USLEEP
        // sleep here to test mutex
        ::usleep(1000);
#endif // TEST_LOG_MUTEX_USLEEP
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
    FilenameFormatItem([[maybe_unused]] const std::string& str = "") : m_str(str) {}
    void format(std::ostream& os, LogEvent::ptr event) override {
        if (m_str == "s") {
            // 简化目录层级输出
            boost::filesystem::path p(std::string(event->getFile()));
            std::string filename = p.filename().string();
            std::stringstream ss;
            p.remove_filename();
            for (auto& s : p) {
                ss << s.string()[0];
                if (s.string() != "/") {
                    ss << "/";
                }
            }
            ss << filename;

            os << ss.str();
        } else if (m_str == "r") {
            os << boost::filesystem::relative(std::string(event->getFile())).string();
        } else {
            os << event->getFile();
        }
    }
private:
    std::string m_str;
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
#ifdef TEST_LOG_MUTEX_USLEEP
        // sleep here to test mutex
        ::usleep(10000);
#endif // TEST_LOG_MUTEX_USLEEP
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

void LogAppender::setFormatter(LogFormatter::ptr val) {
    MutexType::Lock lock(m_mutex);
    m_formatter = val; 
    m_has_formatter = true;
}

LogFormatter::ptr LogAppender::getFormatter() const {
    MutexType::Lock lock(m_mutex);
    return m_formatter;
}

Logger::Logger(const std::string& name) 
    : m_name(name) 
    , m_level(LogLevel::DEBUG){
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));

    // if (name == "root") {
    //     m_appenders.push_back(std::make_shared<StdoutLogAppender>());
    // }
}

void Logger::addAppender(LogAppender::ptr appender) {
    MutexType::Lock lock(m_mutex);
    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    MutexType::Lock lock(m_mutex);
    for (auto it = m_appenders.begin();
            it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders() {
    m_appenders.clear();
}

void Logger::setFormatter(LogFormatter::ptr val) {
    MutexType::Lock lock(m_mutex);
    if (val->isError())
        return;
    
    m_formatter = val;

    for (auto& i : m_appenders) {
        if (!i->hasFormatter()) {
            i->setFormatter(m_formatter);
        }
    }
}

void Logger::setFormatter(const std::string& val) {
    auto new_val = std::make_shared<LogFormatter>(val);
    if (new_val->isError()) {
        std::cout << "Logger::setFormatter name=" << m_name
                  << " value=" << val << " invalid formatter"
                  << std::endl;
        return;
    }

    // m_formatter = new_val;
    setFormatter(new_val);
}

LogFormatter::ptr Logger::getFormatter() const {
    MutexType::Lock lock(m_mutex);
    return m_formatter;
}

std::string Logger::toYamlString() const {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["name"] = m_name;
    node["level"] = LogLevel::ToString(m_level);
    if (m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }

    for (auto& i : m_appenders) {
        node["appenders"].push_back(YAML::Load(i->toYamlString()));
    }

    std::stringstream ss;
    ss << node;
    return ss.str();
}

void Logger::log(LogEvent::ptr event) {
    if (event->getLevel() >= m_level) {
        MutexType::Lock lock(m_mutex);
        if (!m_appenders.empty()) {
            for (auto& i : m_appenders) {
                i->log(event);
            }
        } else if (m_root) {
            m_root->log(event);
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
        MutexType::Lock lock(m_mutex);
        std::cout << m_formatter->format(event);
    }
}

std::string StdoutLogAppender::toYamlString() const {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "StdoutLogAppender";
    if (m_level != LogLevel::Level::UNKNOW)
        node["level"] = LogLevel::ToString(m_level);
    if (m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

FileLogAppender::FileLogAppender(const std::string& filename) 
    :m_filename(filename), m_lastTime(time(0)) {

    if (
#ifdef SYLAR_LOG_FILE_APPEND
        !reopen(std::ios::app)
#else
        !reopen(std::ios::out)
#endif // SYLAR_LOG_FILE_APPEND
        ) {
        std::cout << "open log file failed:" << filename << std::endl;
    }
}

std::string FileLogAppender::toYamlString() const {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "FileLogAppender";
    if (m_level != LogLevel::Level::UNKNOW)
        node["level"] = LogLevel::ToString(m_level);
    node["file"] = m_filename;
    if (m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

FileLogAppender::~FileLogAppender() {
    m_filestream.close();
}

bool FileLogAppender::reopen(std::ios::openmode openmode/*=std::ios::out*/) {
    MutexType::Lock lock(m_mutex);
    if (m_filestream) {
        m_filestream.close();
    }

    m_filestream.open(m_filename, openmode);
    // m_filestream.open(m_filename);
    return !!m_filestream;
}

void FileLogAppender::log(LogEvent::ptr event)  {
    if (event->getLevel() >= m_level) {
        uint64_t now = time(0);
        if (now != m_lastTime) {
            reopen(std::ios::app); //每隔一秒重新打开，app方式
            m_lastTime = now;
        }

        MutexType::Lock lock(m_mutex);
        m_filestream << m_formatter->format(event);
        m_filestream.flush();
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

                    m_error = true;
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

                    m_error = true;
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
        m_error = true;
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
                m_error = true;
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

    // root日志器的初始化
    auto appender = std::make_shared<StdoutLogAppender>();
    auto formatter = std::make_shared<LogFormatter>("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%P]%T[%c](dft)%T%f{}:%l%T%m%n");
    appender->setFormatter(formatter);
    m_root->addAppender(appender);

    auto file_appender = std::make_shared<FileLogAppender>("root.log");
    auto file_formatter = std::make_shared<LogFormatter>("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c](dft)%T%f{}:%l%T%m%n");
    file_appender->setFormatter(file_formatter);
    m_root->addAppender(file_appender);

    m_loggers[m_root->getName()] = m_root;

    init();
}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) {
        return it->second;
    }

    // 如果没有查询到，则创建一个新的logger，并使其拥有一个指向root日志器的指针
    Logger::ptr logger = std::make_shared<Logger>(name);
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}

struct LogAppenderDefine {
    enum Type {
        TypeUNKNOW = 0,
        TypeFileLogAppender = 1,
        TypeStdoutLogAppender = 2
    };
    Type type = TypeUNKNOW; // 1 File, 2 Stdout
    LogLevel::Level level = LogLevel::Level::UNKNOW;
    std::string formatter;
    std::string file;

    bool operator==(const LogAppenderDefine& rhs) const {
        return type == rhs.type 
            && level == rhs.level
            && formatter == rhs.formatter
            && file == rhs.file;
    }

    static Type StringToType(const std::string& str) {
        std::string ucstr = str;
        std::transform(ucstr.begin(), ucstr.end(), ucstr.begin(), ::toupper);
        
        // std::cout << ucstr << std::endl;
        if (ucstr == "FILELOGAPPENDER") {
            return Type::TypeFileLogAppender;
        } else if (ucstr == "STDOUTLOGAPPENDER") {
            return Type::TypeStdoutLogAppender;
        }

        return Type::TypeUNKNOW;
    }

    static const char* TypeToString(Type type) {
        switch (type) {
#define XX(name) \
            case Type::Type##name: \
                return #name; \
                break;

            XX(FileLogAppender);
            XX(StdoutLogAppender);
#undef XX
            default:
                return "UNKNOW";
        }
    }
};

struct LogDefine {
    std::string name;
    LogLevel::Level level = LogLevel::Level::UNKNOW;
    std::string formatter;

    std::vector<LogAppenderDefine> appenders;

    bool operator==(const LogDefine& rhs) const {
        return name == rhs.name 
            && level == rhs.level
            && formatter == rhs.formatter
            && appenders == rhs.appenders;
    }

    bool operator!=(const LogDefine& rhs) const {
        return !(*this == rhs);
    }

    bool operator<(const LogDefine& rhs) const {
        return name < rhs.name;
    }
};

static sylar::ConfigVar<std::set<LogDefine> >::ptr g_log_defines = 
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

template <>
class LexicalCast<std::string, LogDefine> {
public:
    LogDefine operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        LogDefine ld;
        
        if (!_read_name(ld, node)) return ld;
        if (!_read_level(ld, node)) return ld;
        if (!_read_formatter(ld, node)) return ld;
        if (!_read_appenders(ld, node)) return ld;
        
        return ld;
    }
private:
    bool _read_name(LogDefine& ld, const YAML::Node& node) {
        if (!node["name"].IsDefined()) {
            std::cout << "log config error: name not defined\n" << node << std::endl;
            return false;
        }

        if (!node["name"].IsScalar()) {
            std::cout << "log config error: name not scalar\n" << node << std::endl;
            return false;
        }
        
        ld.name = node["name"].as<std::string>();
        return true;
    }

    bool _read_level(LogDefine& ld, const YAML::Node& node) {
        if (!node["level"].IsDefined()) {
            std::cout << "log config error: level not defined\n" << node << std::endl;
            return false;
        }

        if (!node["level"].IsScalar()) {
            std::cout << "log config error: level not scalar\n" << node << std::endl;
            return false;
        }
        
        ld.level = LogLevel::FromString(node["level"].as<std::string>());
        return true;
    }

    bool _read_formatter(LogDefine& ld, const YAML::Node& node) {
        if (!node["formatter"].IsDefined()) {
            std::cout << "log config warn: formatter not defined\n" << node << std::endl;
            return true;
        }

        if (!node["formatter"].IsScalar()) {
            std::cout << "log config error: formatter not scalar\n" << node << std::endl;
            return false;
        }
        
        ld.formatter = node["formatter"].as<std::string>();
        return true;
    }

    bool _read_appenders(LogDefine& ld, const YAML::Node& node) {
        if (!node["appenders"].IsDefined()) {
            std::cout << "log config warn: appenders not defined\n" << node << std::endl;
            return true;
        }

        if (!node["appenders"].IsSequence()) {
            std::cout << "log config error: appenders not sequence\n" << node << std::endl;
            return false;
        }
        
        for (size_t x = 0; x < node["appenders"].size(); ++x) {
            auto a = node["appenders"][x];
            LogAppenderDefine lad;
            if (!_read_an_appender(lad, a)) continue;
            ld.appenders.push_back(lad);
        }
        return true;
    }

    bool _read_an_appender(LogAppenderDefine& lad, const YAML::Node& appender_node) {
        // type
        if (!appender_node["type"].IsDefined()) {
            std::cout << "logappender config error: type not defined\n" << appender_node << std::endl;
            return false;
        }

        if (!appender_node["type"].IsScalar()) {
            std::cout << "logappender config error: type not scalar\n" << appender_node << std::endl;
            return false;
        }
        
        lad.type = LogAppenderDefine::StringToType(appender_node["type"].as<std::string>());
        if (lad.type == LogAppenderDefine::Type::TypeUNKNOW) {
            std::cout << "logappender config error: type unknown\n" << appender_node << std::endl;
            return false;
        }

        // file iff FileLogAppender
        if (lad.type == LogAppenderDefine::Type::TypeFileLogAppender) {
            if (!appender_node["file"].IsDefined() || !appender_node["file"].IsScalar()) {
                std::cout << "logappender config error: file not defined or not scalar\n" << appender_node << std::endl;
                return false;
            }
            lad.file = appender_node["file"].as<std::string>();
        }

        // level
        if (!appender_node["level"].IsDefined()) {
            std::cout << "logappender config warn: level not defined\n" << appender_node << std::endl;
            // lad.level = LogLevel::Level::DEBUG; 
            // 保留UNKNOW
        } else {
            if (!appender_node["level"].IsScalar()) {
                std::cout << "logappender config error: level not scalar\n" << appender_node << std::endl;
                return false;
            } else {
                lad.level = LogLevel::FromString(appender_node["level"].as<std::string>());
            }
        }

        // formatter
        if (!appender_node["formatter"].IsDefined()) {
            std::cout << "logappender config warn: formatter not defined\n" << appender_node << std::endl;
        } else {
            if (!appender_node["formatter"].IsScalar()) {
                std::cout << "logappender config error: formatter not scalar\n" << appender_node << std::endl;
                return false;
            } else {
                lad.formatter = appender_node["formatter"].as<std::string>();
            }
        }
        
        return true;
    }
};

template <>
class LexicalCast<LogDefine, std::string> {
public:
    std::string operator()(const LogDefine& ld) {
        YAML::Node node;
        node["name"] = ld.name;
        node["level"] = LogLevel::ToString(ld.level);
        node["formatter"] = ld.formatter;
        
        YAML::Node appenders_node;
        for (size_t n = 0; n < ld.appenders.size(); ++n) {
            auto a = ld.appenders[n];
            if (!a.file.empty())
                appenders_node[n]["file"] = a.file;
            if (!a.formatter.empty())
                appenders_node[n]["formatter"] = a.formatter;
            if (a.level != LogLevel::Level::UNKNOW)
                appenders_node[n]["level"] = LogLevel::ToString(a.level);
            appenders_node[n]["type"] = LogAppenderDefine::TypeToString(a.type);
        }
        node["appenders"] = appenders_node;

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

struct LogIniter {
    LogIniter() {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "New Launch\
            \n===============================================================\n";
        SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "LogIniter() construction";

        auto key = g_log_defines->addListener([](const std::set<LogDefine>& old_value, const std::set<LogDefine>& new_value)
        {
            SYLAR_LOG_DEBUG(SYLAR_LOG_ROOT()) << "on_logger_conf_changed!";
            // 对于std::set严格弱序的排列规则
            // find查找时会使用operator<进行判断相等
            // 所以只要用于查找的LogDefine对象的name与set容器中的某个对象中相同、
            // set的查找就会查找到该对象并返回其迭代器
            for (auto& i : new_value) {
                auto it = old_value.find(i); // 寻找old_value容器中name相同的LogDefine对象
                sylar::Logger::ptr logger;

                if (it == old_value.end()) {
                    // 新增
                    logger = SYLAR_LOG_NAME(i.name); // 返回一个新构造的logger的指针
                } else {
                    if (i != *it) {
                        // 修改
                        logger = SYLAR_LOG_NAME(i.name);
                    } else {
                        continue;
                    }
                }

                logger->setLevel(i.level);
                if (!i.formatter.empty()) {
                    logger->setFormatter(i.formatter);
                }

                logger->clearAppenders();
                for (auto &a : i.appenders) {
                    sylar::LogAppender::ptr ap;
                    if (a.type == LogAppenderDefine::Type::TypeFileLogAppender) {
                        ap.reset(new FileLogAppender(a.file));
                    } else if (a.type == LogAppenderDefine::Type::TypeStdoutLogAppender) {
                        ap.reset(new StdoutLogAppender);
                    } else {
                        std::cout << "Unknown Appender Type" << std::endl;
                        break;
                    }

                    ap->setLevel(a.level);
                    if (!a.formatter.empty())
                        ap->setFormatter(std::make_shared<LogFormatter>(a.formatter));
                    
                    logger->addAppender(ap);
                }

            }

            for (auto& i : old_value) {
                auto it = new_value.find(i);

                if (it == new_value.end()) {
                    // 删除
                    auto logger = SYLAR_LOG_NAME(i.name);
                    logger->setLevel(static_cast<LogLevel::Level>(100)); // 通过设置高的level来使其不输出
                    logger->clearAppenders();
                }
            }

        });

        std::cout << "LogIniter g_log_defines listener cb id:" << key << std::endl;
    }
};

// 用于在全局main函数之前执行log初始化
static LogIniter __log_init;

void LoggerManager::init() {

}

std::string LoggerManager::toYamlString() const {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for (auto& i : m_loggers) {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }

    std::stringstream ss;
    ss << node;
    return ss.str();
}

void print_log_config_var() {
    std::cout << g_log_defines->toString() << std::endl;
}


}