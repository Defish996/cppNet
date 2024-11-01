#pragma once

#include <iostream>
#include <memory>
#include <unistd.h>
#include "Epoller.hpp"
#include "Socket.hpp"
#include "Log.hpp"

using namespace EpollModule;
using namespace Net_Work;
const static int gbacklog = 32;

// for debug
std::string EventToString(uint32_t events) // 看具体是哪个event
{
    std::string info;
    if (events & EPOLLIN)
        info += "EPOLLIN ";
    if (events & EPOLLOUT)
        info += "EPOLLOUT ";
    if (events & EPOLLET)
        info += "EPOLLET";

    return info;
}

class EpollServer
{
    static const int maxevents = 64;

public:
    EpollServer(int port) : _port(port),
                            _epoller(new Epoller()),
                            _listensocket(new Net_Work::TcpSocket())
    {
    }
    bool InitServer()
    {
        // 1. 创建listen 套接字
        _listensocket->BuildListenSocketMethod((uint16_t)_port, gbacklog);
        lg.LogMessage(Info, "listen socket create success, listensokcetfd is : %d\n", _listensocket->GetSocketFd());

        // 2. 创建epoll模型
        _epoller->InitEpoller();
        lg.LogMessage(Info, "epoll fd create success, epfd is : %d\n", _epoller->GetEpollFd());

        // 3.将listen套接字加入epoll模型
        _epoller->AddEvent(_listensocket->GetSocketFd(), EPOLLIN);
        return true;
    }
    bool Accepter(std::string *peerip, uint16_t *peerport) // 链接管理器
    {
        int sockfd = _listensocket->AcceptSocketOrDie(peerip, peerport)->GetSocketFd();
        if (sockfd < 0)
        {
            lg.LogMessage(Warning, "accept error\n");
            return false;
        }
        // 获取链接成功
        _epoller->AddEvent(sockfd, EPOLLIN); // 对新的listensocketfd进行epoll
        return true;
    }
    int Recver(int sockfd, std::string *out) // 将这个信息带出
    {
        char buffer[1024];
        int n = ::recv(sockfd, buffer, sizeof(buffer), 0); // 从sockfd套接字读取到buffer
        if (n > 0)
        {
            buffer[n] = 0;
            *out = buffer;
        }
        return n;
    }
    void HandlerEvent(int n)
    {
        lg.LogMessage(Info, "%d events has prepared\n", n);
        for (int i = 0; i < n; ++i)
        {
            lg.LogMessage(Debug, "epfd is : %d, event is : %s\n", _revs[i].data.fd, EventToString(_revs[i].events).c_str());
            int sockfd = _revs[i].data.fd;
            uint32_t events = _revs[i].events;
            if (events & EPOLLIN) // 若该事件events为EPOLLIN, 则与正确的EPOLLIN &的结果就是为真, EPOLLIN表示读事件
            {
                // 读事件分两种， 1. listen 2. normal sockfd
                if (sockfd == _listensocket->GetSocketFd())
                {
                    // listen事件就绪
                    std::string clientip;
                    uint16_t clientport;
                    if (!Accepter(&clientip, &clientport))
                    {
                        continue;
                    }
                    lg.LogMessage(Info, "accept client success, client[%s:%d]\n", clientip.c_str(), clientport);
                }
                else
                {
                    // normal sokcfd
                    std::string message;
                    int n = Recver(sockfd, &message);
                    if (n > 0) // 读取成功, 打印信息
                    {
                        std::cout << "client# " << message << std::endl;
                        // 直接写(不考虑异常的情况)
                        message.resize(message.size() - strlen("\r\n")); // 这个2是telnet构建的发送信息时的\r\n,将它去掉之后进行发送(向对端写入) 目的是去除\r\n对telnet信息显示的影响
                        std::string echo_message = "echo message: " + message + "\r\n";// 发送信息结束后便于下次消息的发送
                        send(sockfd, echo_message.c_str(), echo_message.size(), 0);// 这个send是向发送信息的一端进行消息的回显操作
                    }
                    else
                    {
                        if (n == 0)
                        {
                            lg.LogMessage(Info, "clinet %d close\n", sockfd);
                        }
                        else
                        {
                            lg.LogMessage(Info, "client recv %d error\n", sockfd);
                        }
                        // epoll_ctl_del的时候这个fd必须是合法的fd, 否则不能进行操作(具体看OS)
                        _epoller->DelEvent(sockfd);
                        ::close(sockfd);
                    }
                }
            }
        }
    }
    void Loop()
    {
        _isrunning = true;
        while (_isrunning == true)
        {
            int timeout = -1;
            int n = _epoller->Wait(_revs, maxevents, timeout);
            switch (n)
            {
            case 0:
                lg.LogMessage(Debug, "timeout.....\n");
                break;
            case -1:
                lg.LogMessage(Error, "epoll wait failed!\n");
                break;
            default:
                // 事件就绪
                lg.LogMessage(Info, "event happend ... \n");
                HandlerEvent(n);
                break;
            }
        }
        _isrunning = false;
    }
    ~EpollServer() {}

private:
    int _port;
    std::unique_ptr<Socket> _listensocket; // 这边创建的listen套接字和epoll的epfd两个fd是前一个fd小于后一个, 因为前一个的代码先执行
    std::unique_ptr<Epoller> _epoller;
    bool _isrunning;
    struct epoll_event _revs[maxevents]; // 将所有就绪的事件放在这个数组中
};