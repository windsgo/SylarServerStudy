#pragma once
#include <memory>

#define SYLAR_DISABLE_COPY(CLASS) \
private: \
    CLASS(const CLASS&) = delete; \
    CLASS(CLASS&&) = delete; \
    CLASS& operator=(const CLASS&) = delete; \
    CLASS& operator=(CLASS&&) = delete; \

namespace sylar {

template<class T, class X = void, int N = 0>
class Singleton {
public:
    static T* GetInstance() {
        static T v;
        return &v;
    }
private:
};

template<class T, class X = void, int N = 0>
class SingletonPtr {
public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v = std::make_shared<T>();
        return v;
    }
private:
};

}