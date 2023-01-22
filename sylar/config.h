#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <map>
#include <typeinfo>

#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

#include "log.h"


namespace sylar {

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

// T FromStr::operator()(const std::string&)
// std::string ToStr::operator()(const T&)
template <typename T, typename FromStr = LexicalCast<std::string, T>,
          typename ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase
{
public:
    using type = T;
    using ptr = std::shared_ptr<ConfigVar>;
    ConfigVar(const std::string& name, const T& default_value, const std::string& description = "") 
        : ConfigVarBase(name, description), m_val(default_value) {
    }

    std::string toString() override {
        try {
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
            m_val = FromStr()(val);
            return true;
        }
        catch (std::exception &e)
        {
            SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ConfigVar::fromString exception " 
                << e.what() << " convert: string to " << typeid(m_val).name();
        }
        return false;
    }

    const T &getValue() const { return m_val; }
    void setValue(const T &v) { m_val = v; }

private:
    T m_val;
};

class Config {
public:
    using ConfigVarMap = std::map<std::string, ConfigVarBase::ptr>;
    
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
        s_datas[name] = v; // equal to the following line
        // s_datas[name] = std::dynamic_pointer_cast<ConfigVarBase>(v);

        return v;
    }

    template<typename T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        auto it = s_datas.find(name);
        if (it == s_datas.end()) {
            return nullptr;
        }

        auto ret = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);

        // 增加这段，对同名的配置进行不同类型的覆盖时进行警告
        // 查找字段对应、但类型不正确时，dynamic_pointer_cast会转换失败返回nullptr
        if (ret == nullptr) {
            SYLAR_LOG_WARN(SYLAR_LOG_ROOT()) << "Lookup name=" << name << ",typeid=" << typeid(T).name()
                                             << ", type not correct";
        }

        return ret;
    }

    static void LoadFromYaml(const YAML::Node &root);

    static ConfigVarBase::ptr LookupBase(const std::string &name);

private:
    Config() = default;
    Config(const Config &) = delete;
    Config(Config &&) = delete;
    static ConfigVarMap s_datas;
};
}