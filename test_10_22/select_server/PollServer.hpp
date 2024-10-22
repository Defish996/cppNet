#pragma once

#include <iostream>
#include <poll.h>
#include "Socket.hpp"
#include "Log.hpp"

const static int gdefaultport = 8889;
const static int gbacklog = 8;
const int gnum = 1024; 

using namespace Net_Work;

class PollServer
{
private:
    void HandlerEvent() // 如何处理这个listensock套接字呢? 将新的套接字添加到这个维护的数组中
    {
        // 我们不能只处理listen套接字, 要处理所有处于等待时间内的套接字
        for (int i = 0; i < gnum; ++i)
        {
            if (_rfds[i].fd == -1) // 为nullptr的就不合法(为空就是不需要关心这个文件描述符)跳过即可, 直接处理合法的fd
            {
                continue;
            }
            int fd = _rfds[i].fd;
            short revent = _rfds[i].revents;
            if (revent & POLLIN) // 是否存在(存在就说明是需要进行操作的文件描述符, 且已经就绪), 从业务层面来看, 就是判断这个fd_set中是否存在需要进行网络传输的sockfd, 若存在就进行连接传输
            {
                // 新链接到来
                if (fd == _listensocket->GetSocketFd()) // 链接事件到来, 对listen套接字的处理
                {
                    lg.LogMessage(Info, "Get a new link");
                    // 获得一个新链接
                    //  谁来链接我?, 得到这个对端的信息
                    std::string clientip;
                    uint16_t clientport;

                    int sock = _listensocket->AcceptConnection(&clientip, &clientport);
                    if (!sock)
                    {
                        lg.LogMessage(Error, "accept error...");
                        continue;
                    }
                    lg.LogMessage(Info, "get a client, client info is #%s:%d, fd : %d\n", clientip.c_str(), clientport, sock);

                    // 现在进行read/write的IO等, 使用poll处理
                    int pos = 0;
                    for (; pos < _num; ++pos)
                    {
                        if (_rfds[i].fd == -1) // 当前没有被使用
                        {
                            _rfds[i].fd = sock;
                            _rfds[i].events = POLLIN;
                            lg.LogMessage(Info, "get a new link, fd is : %d\n", sock);
                            break;
                        }
                    }
                    if (pos == _num) // 被写满了
                    {
                        // 1.扩容 
                        // 2.关闭
                        close(sock);
                        lg.LogMessage(Warning, "server is full......");
                    }
                }
                else
                { // 对普通套接字的处理
                    char buffer[1024];
                    ssize_t n = recv(fd, buffer, sizeof(buffer) - 1, 0); // 这里不会阻塞, 因为
                    if (n > 0)
                    {
                        lg.LogMessage(Info, "client say# %s\n", buffer);
                        std::string message = "hello, how are you today?, ";
                        message += buffer;
                        send(fd, message.c_str(), message.size(), 0);
                        // 进行写操作, 但是并不正确, 可以做演示使用
                        
                    }
                    else
                    {
                        // Recv失败, 或者是对方已经把连接关闭了, 同意当成一种情况来处理
                        // 则把这个socket先关闭, 然后释放对应的对象
                        // 未来当再次进行查找该位置的文件描述符需不需要进行select处理, 需不需要进行对应的操作时, 就会不考虑该位置, 直接continue
                        lg.LogMessage(Warning, "client quit, maybe close or error, close fd : %d\n", fd);
                        close(fd);
                        // 关闭还要进行取消
                        _rfds[i].fd = -1;
                        _rfds[i].events = 0;
                        _rfds[i].revents = 0;
                    }
                }
            }
        }
    }

public:
    PollServer(int port = gdefaultport)
        : _port(port), _listensocket(new TcpSocket()), _isrunning(false), _num(gnum)
    {
    }
    void Init()
    {
        _listensocket->BuildListenSocketMethod(_port, gbacklog);
        _rfds = new struct pollfd[_num];
        for(int i = 0; i < _num; ++i)
        {
            _rfds[i].fd = -1;
            _rfds[i].events = 0;
            _rfds[i].revents = 0;
        }
        // 最开始只有一个文件描述符, 就是listensockfd
        _rfds[0].fd = _listensocket->GetSocketFd();
        _rfds[0].events |= POLLIN;
    }
    void Loop()
    {
        _isrunning = true;
        while (_isrunning)
        {

            // rfds本质是一个输入输出型参数, rfds在select调用返回的时候, 不断被修改, 所以要每次都进行重置
            //  这也就是 rfds写在循环内部的原因, 每次循环这也是他的缺点

            // 定义时间
            // PrintDegug();
            int timeout = 1000;
            // struct timeval timeout = {0, 0};
            PrintDegug();
            int n = poll(_rfds, _num, timeout); // 第一个参数必须为最大fd + 1, 每隔1s timeout一次
            switch (n)
            {
            case 0:
                lg.LogMessage(Info, "select timeout..."); // 要加\n刷新缓冲区
                break;
            case -1:
                lg.LogMessage(Error, "select error!!!\n");
                break;
            default:
                // 正常就绪的fd
                lg.LogMessage(Info, "select success, begin event handler\n");
                HandlerEvent();
                break;
            }
        }
    }
    void Stop()
    {
    }
    void PrintDegug()
    {
        std::cout << "current poll fd list is : ";
        for (size_t i = 0; i < _num; i++)
        {
            if (_rfds[i].fd != -1)
            {
                std::cout << _rfds[i].fd << " ";
            }
        }
        std::cout << std::endl;
    }
    ~PollServer() 
    {
        delete[] _rfds;
    }
private:
    std::unique_ptr<Socket> _listensocket;
    int _port; 
    bool _isrunning;

    struct pollfd *_rfds;
    int _num;
};
