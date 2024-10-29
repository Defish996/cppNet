#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <sys/epoll.h>
#include "Log.hpp"
namespace EpollModule
{
    const static int defaultepfd = -1;
    const static int size = 128;
    class Epoller
    {
    public:
        Epoller(): _epfd(defaultepfd)
        {}    
        void InitEpoller()
        {
            _epfd = ::epoll_create(size);
            if(defaultepfd == _epfd)
            {
                lg.LogMessage(Fatal, "epoll_create error, %s : %d\n", strerror(errno), errno);
                exit(4);
            }
            lg.LogMessage(Info, "epoll_create success, epfd : %d\n", _epfd);
        }
        ~Epoller()
        {
            if(_epfd >= 0)
            {
                close(_epfd); 
            }
            lg.LogMessage(Info, "epoll close success\n"); 
        }    
    private:
        int _epfd;
    };
}