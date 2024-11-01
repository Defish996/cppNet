#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <set>
#include <sys/epoll.h>
#include "Log.hpp"
namespace EpollModule
{
    const static int defaultepfd = -1;
    const static int size = 128;
    class Epoller
    {
    public:
        Epoller() : _epfd(defaultepfd)
        {
        }
        void InitEpoller()
        {
            _epfd = ::epoll_create(size);
            if (defaultepfd == _epfd)
            {
                lg.LogMessage(Fatal, "epoll_create error, %s : %d\n", strerror(errno), errno);
                exit(4);
            }
            lg.LogMessage(Info, "epoll_create success, epfd : %d\n", _epfd);
        }
        int GetEpollFd()
        {
            // std::cout << "return sockfd success ... " << std::endl;
            return _epfd;
        }
        void AddEvent(int sockfd, uint32_t events) // 为哪一个fd添加什么事件
        {
            // for test
            _fd_list.insert(sockfd);
            struct epoll_event ev;
            ev.data.fd = sockfd; // 不是给内核的
            ev.events = events;

            int n = ::epoll_ctl(_epfd, EPOLL_CTL_ADD, sockfd, &ev);
            if (n < 0)
            {
                lg.LogMessage(Error, "epoll_ctl add error, %s : %d\n", strerror(errno), errno);
            }
        }
        void DelEvent(int sockfd)
        {
            // for test
            std::cout << "fd list is :";
            for (auto &e : _fd_list)
            {
                std::cout << e;
            }
            std::cout << std::endl;
            int n = ::epoll_ctl(_epfd, EPOLL_CTL_DEL, sockfd, nullptr); // 删除这个事件, 那么只需要进行删除OP即可, 第四个参数事件可以为null
            if (n < 0)
            {
                lg.LogMessage(Error, "epoll_ctl_del error, %s : %d\n", strerror(errno), errno);
            }
        }
        int Wait(struct epoll_event *revs, int maxevents, int timeout)
        {
            int n = epoll_wait(_epfd, revs, maxevents, timeout);
            return n;
        }
        ~Epoller()
        {
            if (_epfd >= 0)
            {
                close(_epfd);
            }
            lg.LogMessage(Info, "epoll close success\n");
        }

    private:
        int _epfd;
        std::set<int> _fd_list;
    };
}