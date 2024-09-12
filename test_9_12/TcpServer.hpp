#pragma once

#include "Socket.hpp"
#include <iostream>
#include <unistd.h>
#include <string>
#include <pthread.h>
#include <functional>

// http协议底层采用的是tcp协议, 所以http是基于tcp协议的
using func_t = std::function<std::string(std::string &request)>; // &是必须的

class TcpServer;

class ThreadData
{
public:
    ThreadData(TcpServer *tcp_this, Net_Work::Socket *sockp) : _this(tcp_this), _sockp(sockp)
    {
    }

public:
    TcpServer *_this;
    Net_Work::Socket *_sockp;
};

class TcpServer
{
public:
    TcpServer(uint16_t port, func_t handler_request) : _port(port), _listenSock(new Net_Work::TcpSocket()), _handler_request(handler_request)
    {
        _listenSock->BuildListenSocketMethod(_port, Net_Work::backlog);
    }
    static void *ThreadRun(void *args)
    {
        pthread_detach(pthread_self()); // 分离线程
        ThreadData *td = static_cast<ThreadData *>(args);

        std::string http_request; // 接收请求的缓冲区

        // 读取数据, 不关心数据是什么, 只进行读取
        // 1.读取报文
        if (td->_sockp->Recv(&http_request, 4096))
        {
            // 2.报文处理
            std::string http_response = td->_this->_handler_request(http_request); // 注意: 回调不仅仅是调出去, 还会回来

            // 发送数据, 不关心数据是什么, 只进行发送
            // 3.发送报文
            if (!http_response.empty())
            {

                td->_sockp->Send(http_response);
            }
        }

        td->_sockp->CloseSocket(); // 关闭套接字
        delete td->_sockp;
        delete td;
        return nullptr;
    }
    void Loop() // 服务器的主循环
    {
        while (true)
        {
            std::string peerip;
            uint16_t peerport;
            // 获取套接字
            Net_Work::Socket *newsock = _listenSock->AcceptSocketOrDie(&peerip, &peerport); // 接受一个连接
            if (newsock == nullptr)                                                         // 如果接受失败, 则继续循环
            {
                std::cout << "AcceptSocketOrDie failed" << std::endl;
                continue;
            }
            std::cout << "Get a new connection, sockfd is : " << newsock->GetSocketFd() << ", peer ip: " << peerip << ", peer port: " << peerport << std::endl;
            // 使用线程来处理连接
            pthread_t tid;
            ThreadData *td = new ThreadData(this, newsock);
            pthread_create(&tid, nullptr, ThreadRun, (void *)td);
        }
    }
    ~TcpServer()
    {
        delete _listenSock;
    }

private:
    int _port;
    Net_Work::Socket *_listenSock; // 创建一个监听socket
public:
    func_t _handler_request; // 处理连接的函数
};

// 至此, 服务端的代码已经完成, 主要功能在HandlerRequest函数中, 该函数需要用户自己实现
// 服务端只是一个框架, 用户需要自己实现HandlerRequest函数来处理连接