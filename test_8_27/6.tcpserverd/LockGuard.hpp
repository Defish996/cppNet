#pragma once
#include <iostream>
//不定义锁,默认认为外部会给我们传入锁对象
class Mutex
{
public:
    Mutex(pthread_mutex_t *lock):_lock(lock)//包装加锁功能可以实现启动自定义锁时自定加锁,然后对应的函数功能结束自动解锁(利用构造函数和析构函数的性质实现)
    {}
    void Lock()
    {
        pthread_mutex_lock(_lock);
    }
    void Unlock()
    {
        pthread_mutex_unlock(_lock);
    }
    ~Mutex()
    {}
    
private:
    pthread_mutex_t *_lock;
};

class LockGuard
{
public:
    LockGuard(pthread_mutex_t *lock):_mutex(lock)//_mutex是Mutex的对象,该对象调用对应的方法
    {
        _mutex.Lock();//调用Mutex类的加锁方法
    }
    ~LockGuard ()
    {
        _mutex.Unlock();
    }

private:
    Mutex _mutex;
};