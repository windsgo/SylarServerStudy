#pragma once

#include <functional>
#include <memory>
#include <iostream>

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <atomic>

#include "singleton.h" // disable copy

namespace sylar
{

class Semaphore {
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();

    void wait();
    void notify();

private:
    sem_t m_semaphore;

private:
    SYLAR_DISABLE_COPY(Semaphore)
};

template <typename T>
struct ScopedLockImpl {
public:
    ScopedLockImpl(T& mutex) 
        : m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }
    
    ~ScopedLockImpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked = false;
private:
    SYLAR_DISABLE_COPY(ScopedLockImpl)
};

template <typename T>
struct ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& mutex) 
        : m_mutex(mutex) {
        m_mutex.rdlock();
        m_locked = true;
    }
    
    ~ReadScopedLockImpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked = false;
private:
    SYLAR_DISABLE_COPY(ReadScopedLockImpl)
};

template <typename T>
struct WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex) 
        : m_mutex(mutex) {
        m_mutex.wrlock();
        m_locked = true;
    }
    
    ~WriteScopedLockImpl() {
        unlock();
    }

    void lock() {
        if (!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked = false;
private:
    SYLAR_DISABLE_COPY(WriteScopedLockImpl)
};

class Mutex {
public:
	using Lock = ScopedLockImpl<Mutex>;
    Mutex() {
        pthread_mutex_init(&m_lock, nullptr);
    }

    ~Mutex() {
        pthread_mutex_destroy(&m_lock);
	}

    void lock() {
        pthread_mutex_lock(&m_lock);
    }

    void unlock() {
        pthread_mutex_unlock(&m_lock);
    }

private:
    pthread_mutex_t m_lock;

private:
    SYLAR_DISABLE_COPY(Mutex)
};

class Spinlock {
public:
    using Lock = ScopedLockImpl<Spinlock>;
    Spinlock() {
        pthread_spin_init(&m_lock, 0);
    }

    ~Spinlock() {
        pthread_spin_destroy(&m_lock);
    }

    void lock() {
        pthread_spin_lock(&m_lock);
    }

    void unlock() {
        pthread_spin_unlock(&m_lock);
    }

private:    
    pthread_spinlock_t m_lock; 

private:
    SYLAR_DISABLE_COPY(Spinlock)
};

class CASLock {
public:
    using Lock = ScopedLockImpl<CASLock>;
    CASLock() {
        m_flag.clear();
    }

    ~CASLock() {

    }

    void lock() {
        while (std::atomic_flag_test_and_set_explicit(&m_flag, std::memory_order_acquire));
    }

    void unlock() {
        std::atomic_flag_clear_explicit(&m_flag, std::memory_order_release);
    }

private:
    volatile std::atomic_flag m_flag;

private:
    SYLAR_DISABLE_COPY(CASLock)
};

class NullMutex {
public:
    using Lock = ScopedLockImpl<NullMutex>;
    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
private:
    SYLAR_DISABLE_COPY(NullMutex)
};

class NullRWMutex {
public:
    using ReadLock = ReadScopedLockImpl<NullRWMutex>;
    using WriteLock = WriteScopedLockImpl<NullRWMutex>;
    NullRWMutex() {}
    ~NullRWMutex() {}
    void rdlock() {}
    void wrlock() {}
    void unlock() {}

private:
    SYLAR_DISABLE_COPY(NullRWMutex)
};

class RWMutex {
public:
    using ReadLock = ReadScopedLockImpl<RWMutex>;
    using WriteLock = WriteScopedLockImpl<RWMutex>;
    RWMutex() {
        pthread_rwlock_init(&m_lock, nullptr);
    }

    ~RWMutex() {
        pthread_rwlock_destroy(&m_lock);
    }

    void rdlock() {
        pthread_rwlock_rdlock(&m_lock);
    }

    void wrlock() {
        pthread_rwlock_wrlock(&m_lock);
    }

    void unlock() {
        pthread_rwlock_unlock(&m_lock);
    }

private:
    pthread_rwlock_t m_lock;

private:
    SYLAR_DISABLE_COPY(RWMutex)
};

class Thread {
public:
    using ptr = std::shared_ptr<Thread>;
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();

    const std::string& getName() const { return m_name; }
    pid_t getId() const { return m_id; }

    void join();

    static Thread* GetThis();
    static const std::string& GetName();
    static void SetName(const std::string& name);

private:
    static void* run(void* arg);

private:
    pid_t m_id = -1;
    pthread_t m_thread = 0;
    std::function<void()> m_cb;
    std::string m_name;

    Semaphore m_semapore;
private:
    SYLAR_DISABLE_COPY(Thread)
};
    
} // namespace sylar
