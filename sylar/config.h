#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <typeinfo>
#include <functional>

#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

#include "log.h"
#include "thread.h" // 线程锁

namespace sylar {

#ifdef TEST_CONFIG_NO_RWMUTEX
using ConfigRWMutex = NullRWMutex;
#else // not TEST_CONFIG_NO_RWMUTEX
    #if defined(SYLAR_CONFIG_RWMUTEX) 
    using ConfigRWMutex = RWMutex;
    #else // not defined SYLAR_CONFIG_RWMUTEX
    static_assert(false);
    #endif // SYLAR_CONFIG_SPINLOCK
#endif // TEST_CONFIG_NO_RWMUTEX

class ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVarBase>;
    ConfigVarBase(const std::string& name, const std::string& description = "") 
        : m_name(name), m_description(description) {
        std::transform(name.begin(), name.end(), m_name.begin(), ::tolower);
    }
    virtual ~ConfigVarBase() {}

    const std::string& getName() const { return m_name; }
    const std::string& getDescription() const { return m_description; }

    virtual std::string toString() = 0;
    virtual bool fromString(const std::string &val) = 0;
    virtual std::string getTypeName() const = 0;

protected:
    std::string m_name;
    std::string m_description;
};

// 复杂类型支持
// F: from_type, T: to_type
template <typename F, typename T>
class LexicalCast {
public:
    T operator() (const F& v) noexcept(false) {
        return boost::lexical_cast<T>(v);
    }
};

// std::vector
template <typename T>
class LexicalCast<std::string, std::vector<T> > {
public:
    std::vector<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }

        return vec;
    }
};

template <typename T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// std::list
template <typename T>
class LexicalCast<std::string, std::list<T> > {
public:
    std::list<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::list<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }

        return vec;
    }
};

template <typename T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// std::set
template <typename T>
class LexicalCast<std::string, std::set<T> > {
public:
    std::set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::set<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }

        return vec;
    }
};

template <typename T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// std::unordered_set
template <typename T>
class LexicalCast<std::string, std::unordered_set<T> > {
public:
    std::unordered_set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }

        return vec;
    }
};

template <typename T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// std::map
template <typename T>
class LexicalCast<std::string, std::map<std::string, T> > {
public:
    std::map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::map<std::string, T> vec;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }

        return vec;
    }
};

template <typename T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// std::unordered_map
template <typename T>
class LexicalCast<std::string, std::unordered_map<std::string, T> > {
public:
    std::unordered_map<std::string, T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_map<std::string, T> vec;
        std::stringstream ss;
        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            vec.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }

        return vec;
    }
};

template <typename T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& v) {
        YAML::Node node;
        for (auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// T FromStr::operator()(const std::string&)
// std::string ToStr::operator()(const T&)
template <typename T, typename FromStr = LexicalCast<std::string, T>,
          typename ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase
{
public:
    using type = T;
    using ptr = std::shared_ptr<ConfigVar>;
    using on_change_cb = std::function<void (const T&old_value, const T& new_value)>;
    using RWMutexType = ConfigRWMutex;

    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "") 
        : ConfigVarBase(name, description), m_val(default_value) {
    }

    std::string toString() override {
        try {
            RWMutexType::ReadLock lock(m_mutex);
            // return boost::lexical_cast<std::string>(m_val);
            return ToStr()(m_val);
        }
        catch (std::exception& e)
        {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::toString exception " 
                << e.what() << " convert: " << typeid(m_val).name() << " to string";
        }
        return "..."; 
    }

    bool fromString(const std::string &val) override {
        try {
            // m_val = boost::lexical_cast<T>(val);
            setValue(FromStr()(val)); // 有锁
            return true;
        }
        catch (std::exception &e)
        {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception " 
                << e.what() << " convert: string to " << typeid(m_val).name() << val;
        }
        return false;
    }

    const T &getValue() const { 
        RWMutexType::ReadLock lock(m_mutex);
        return m_val; 
    }
    void setValue(const T &v) { 
        {
            RWMutexType::ReadLock lock(m_mutex);
            if (v == m_val) {
                return;
            }

            for (auto& i : m_cbs) {
                i.second(m_val, v); // 回调，通知观察者配置发生改变
            }
        }
        
        RWMutexType::WriteLock lock(m_mutex);
        m_val = v;
    }
    std::string getTypeName() const override { return typeid(T).name(); }

    uint64_t addListener(on_change_cb cb) {
        static uint64_t s_fun_id = 0;
        RWMutexType::WriteLock lock(m_mutex);
        ++s_fun_id; 
        m_cbs[s_fun_id] = cb;
        return s_fun_id;
    }

    void delListener(uint64_t key) {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.erase(key);
    }

    on_change_cb getListener(uint64_t key) const {
        RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    void clearListener() {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.clear();
    }

private:
    T m_val;
    // 变更回调函数组，uint64_t key，要求唯一，一般可以用hash
    std::map<uint64_t, on_change_cb> m_cbs;
    mutable RWMutexType m_mutex;
};

class Config {
public:
    using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::ptr>;
    using RWMutexType = ConfigRWMutex;
    
    template<typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,
            const T& default_value, const std::string& description = "") {
        auto tmp = Lookup<T>(name);

        if (tmp) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name=" << name << " exists";
            return tmp;
        }

        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")
                != std::string::npos) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name invalid: " << name;
            throw std::invalid_argument(name);
        }

        auto v = std::make_shared<ConfigVar<T>>(name, default_value, description);
        RWMutexType::WriteLock lock(GetMutex());
        GetDatas()[name] = v; // equal to the following line
        // s_datas[name] = std::dynamic_pointer_cast<ConfigVarBase>(v);

        return v;
    }

    template<typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if (it == GetDatas().end()) {
            return nullptr;
        }

        auto ret = std::dynamic_pointer_cast<ConfigVar<T>>(it->second); // 动态指针转换如果失败会返回nullptr

        // 增加这段，对同名的配置进行不同类型的覆盖时进行警告
        // 查找字段对应、但类型不正确时，dynamic_pointer_cast会转换失败返回nullptr
        if (ret == nullptr) {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "Lookup name=" << name << ",type=" << typeid(T).name()
                                             << " not correct, real type=" << it->second->getTypeName() 
                                             << ", value=" << it->second->toString();
        }

        return ret;
    }

    static void LoadFromYaml(const YAML::Node &root);

    static ConfigVarBase::ptr LookupBase(const std::string &name);

    static void Visit(std::function<void(ConfigVarBase::ptr)> vb);

private:
    Config() = default;
    Config(const Config &) = delete;
    Config(Config &&) = delete;
    
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }

    static RWMutexType& GetMutex() {
        static RWMutexType s_mutex;
        return s_mutex;
    }
};
}
