#pragma once

#include <iostream>
#include <queue>
#include <vector>
#include <pthread.h>
#include <functional>
#include "LockGuard.hpp"
#include "Thread.hpp"
#include "Log.hpp"

namespace ThreadNs
{
    static const int defaultnum = 5;
    class ThreadData
    {
    public:
        ThreadData(const std::string &name)
            : threadname(name)
        {
        }
        ~ThreadData() {}

    public:
        std::string threadname;
    };

    template <class T>
    class ThreadPool
    {
    private:
        ThreadPool(int thread_num = defaultnum)
            : _thread_num(thread_num)
        {
            pthread_mutex_init(&_mutex, nullptr);
            pthread_cond_init(&_cond, nullptr);
            // 构建指定个数个线程
            for (int i = 0; i < _thread_num; i++)
            {
                // 待优化
                std::string threadname = "thread-";
                threadname += std::to_string(i + 1);
                ThreadData td(threadname);
                // 参数二bind表示, 将当前对象和函数调用的实参的第一个参数传给这个函数
                // Thread<ThreadData> t(threadname, std::bind(&ThreadPool<T>::ThreadRun, this, std::placeholders::_1), td);
                // _threads.push_back(t);
                _threads.emplace_back(threadname, std::bind(&ThreadPool<T>::ThreadRun, this, std::placeholders::_1), td);
                lg.LogMessage(Info, "%s is created...\n", threadname.c_str());
            }
        }
        ThreadPool(const ThreadPool<T> &tp) = delete;
        ThreadPool<T> &operator=(const ThreadPool<T> &) = delete;

    public:
        // 单例方法
        // 存在线程安全
        static ThreadPool<T> *GetInstance()
        {
            if (instance == nullptr)
            {

                LockGuard lock_guard(&sig_lock);
                if (instance == nullptr)
                {
                    lg.LogMessage(Info, "创建单例成功...\n");
                    instance = new ThreadPool<T>();
                }
            }
            return instance;
        }
        bool Start()
        {
            // 启动
            for (auto &thread : _threads)
            {
                thread.Start();
                lg.LogMessage(Info, "%s is running ...\n", thread.ThreadName().c_str());
            }
            return true;
        }
        void ThreadWait(ThreadData &td)
        {
            lg.LogMessage(Debug, "no task, %s is sleep...\n", td.threadname.c_str());
            pthread_cond_wait(&_cond, &_mutex);
        }
        void ThreadWakeup()
        {
            pthread_cond_signal(&_cond);
        }

        // 线程执行的方法
        void ThreadRun(ThreadData &td) // 进行处理任务
        {
            while (true)
            {
                //  取任务
                T t;
                {
                    LockGuard lock_guard(&_mutex);
                    while (_q.empty())
                    {
                        ThreadWait(td);
                        lg.LogMessage(Debug, "have task, %s is wakeup...\n", td.threadname.c_str());
                    }
                    t = _q.front();
                    _q.pop();
                    // std::cout << "thread is running ..." << std::endl;
                    // sleep(1);
                }
                // 处理任务
                // 处理任务属于私有,不被锁包含
                t();
            }
        }
        void Push(T &in)
        {
            LockGuard lg(&_mutex);
            _q.push(in);
            ThreadWakeup();
        }

        ~ThreadPool()
        {
            pthread_mutex_destroy(&_mutex);
            pthread_cond_destroy(&_cond);
        }
        // for DeBug
        void Wait()
        {
            for (auto &thread : _threads)
            {
                thread.join();
            }
        }

    private:
        std::queue<T> _q;
        std::vector<Thread<ThreadData>> _threads;
        int _thread_num;
        pthread_mutex_t _mutex;
        pthread_cond_t _cond;
        static ThreadPool<T> *instance; // 单例指针

        static pthread_mutex_t sig_lock;
    };
    template <class T>
    ThreadPool<T> *ThreadPool<T>::instance = nullptr;

    template <class T>
    pthread_mutex_t ThreadPool<T>::sig_lock = PTHREAD_MUTEX_INITIALIZER;
}