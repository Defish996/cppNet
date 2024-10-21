#pragma once

#include <iostream>
#include <sys/select.h>
#include "Socket.hpp"
#include "Log.hpp"

const static int gdefaultport = 8889;
const static int gbacklog = 8;
const static int num = sizeof(fd_set) * 8; // fd_set是以字节为单位的, 所以使用单位大小乘8的数组大小

using namespace Net_Work;

class SelectServer
{
private:
    void HandlerEvent(fd_set &rfds) // 如何处理这个listensock套接字呢? 将新的套接字添加到这个维护的数组中
    {
        // 我们不能只处理listen套接字, 要处理所有处于等待时间内的套接字
        for (int i = 0; i < num; ++i)
        {
            if (_rfds_array[i] == nullptr) // 为nullptr的就不合法(为空就是不需要关心这个文件描述符)跳过即可, 直接处理合法的fd
            {
                continue;
            }
            int fd = _rfds_array[i]->GetSocketFd();
            if (FD_ISSET(fd, &rfds)) // 是否存在(存在就说明是需要进行操作的文件描述符, 且已经就绪), 从业务层面来看, 就是判断这个fd_set中是否存在需要进行网络传输的sockfd, 若存在就进行连接传输
            {
                // 读事件就绪
                if (fd == _listensocket->GetSocketFd()) // 链接事件到来, 对listen套接字的处理
                {
                    lg.LogMessage(Info, "Get a new link");
                    // 获得一个新链接
                    //  谁来链接我?, 得到这个对端的信息
                    std::string clientip;
                    uint16_t clientport;

                    Socket *sock = _listensocket->AcceptSocketOrDie(&clientip, &clientport);
                    if (!sock)
                    {
                        lg.LogMessage(Error, "accept error...");
                        return;
                    }
                    lg.LogMessage(Info, "get a client, client info is #%s:%d, fd : %d\n", clientip.c_str(), clientport, sock->GetSocketFd());

                    // 现在进行read/write的IO等, 使用select处理
                    int pos = 0;
                    for (; pos < num; ++pos)
                    {
                        if (_rfds_array[pos] == nullptr) // 当前没有被使用
                        {
                            _rfds_array[pos] = sock;
                            lg.LogMessage(Info, "get a new link, fd is : %d\n", sock->GetSocketFd());
                            break;
                        }
                    }
                    if (pos == num) // 被写满了
                    {
                        sock->CloseSocket();
                        delete sock;
                        lg.LogMessage(Warning, "server is full......");
                    }
                }
                else
                { // 对普通套接字的处理
                    std::string buffer;
                    bool res = _rfds_array[i]->Recv(&buffer, 1024); // 期望读取1024, 但是实际读取跟协议有关, 会存在粘包, 序列化....等的相关问题, 这只是一种测试写法, select和poll不处理, epoll会对这个读取到的进行处理
                    if (res)
                    {
                        lg.LogMessage(Info, "client say# %s\n", buffer.c_str());
                        buffer.clear();
                        
                        // 进行写操作, 但是并不正确, 可以做演示使用
                        
                    }
                    else
                    {
                        // Recv失败, 或者是对方已经把连接关闭了, 同意当成一种情况来处理
                        // 则把这个socket先关闭, 然后释放对应的对象
                        // 未来当再次进行查找该位置的文件描述符需不需要进行select处理, 需不需要进行对应的操作时, 就会不考虑该位置, 直接continue
                        lg.LogMessage(Warning, "client quit, maybe close or error, close fd : %d\n", fd);
                        _rfds_array[i]->CloseSocket();
                        delete _rfds_array[i];
                        _rfds_array[i] = nullptr;

                    }
                }
            }
        }
    }

public:
    SelectServer(int port = gdefaultport)
        : _port(port), _listensocket(new TcpSocket)
    {
    }
    void Init()
    {
        _listensocket->BuildListenSocketMethod(_port, gbacklog);
        for (int i = 0; i < num; i++)
        {
            _rfds_array[i] = nullptr;
        }
        _rfds_array[0] = _listensocket.get(); // 得到这个智能指针对象的原始指针
        // 我们将新建的fd加到这个数组中, 然后找最大值, 这样就得到了最大文件描述符,
        // 然后下次循环又会添加新的fd,再找最大值
    }
    void Loop()
    {
        _isrunning = true;
        while (_isrunning)
        {
            // 不能直接使用accept创建新连接,  listensocket上面的新链接,相当于是读事件, 有新链接, 就等价于新数据要到来
            // 首先不能直接accept, 而是将拉特么socket交给select, 因为只有select有资格知道有没有IO时间就绪
            // 故意放在循环内部
            fd_set rfds;
            FD_ZERO(&rfds);
            int max_fd = _listensocket->GetSocketFd();
            for (int i = 0; i < num; ++i) // 更新看是否有新的连接
            {

                if (_rfds_array[i] != nullptr)
                {
                    int fd = _rfds_array[i]->GetSocketFd();
                    FD_SET(fd, &rfds); // 将要select的fd添加到这个数组
                    if (fd > max_fd)
                    {
                        max_fd = fd;
                    }
                }
            }

            // rfds本质是一个输入输出型参数, rfds在select调用返回的时候, 不断被修改, 所以要每次都进行重置
            //  这也就是 rfds写在循环内部的原因, 每次循环这也是他的缺点

            // 定义时间
            PrintDegug();
            struct timeval timeout = {0, 0};
            int n = select(max_fd + 1, &rfds, nullptr, nullptr, nullptr); // 第一个参数必须为最大fd + 1
            switch (n)
            {
            case 0:
                lg.LogMessage(Info, "select timeout..., last time : %u, %u\n", timeout.tv_sec, timeout.tv_usec); // 要加\n刷新缓冲区
                break;
            case -1:
                lg.LogMessage(Error, "select error!!!\n");
                break;
            default:
                // 正常就绪的fd
                lg.LogMessage(Info, "select success, begin event handler, last time : %u, %u\n", timeout.tv_sec, timeout.tv_usec);
                HandlerEvent(rfds);
                break;
            }
        }
    }
    void Stop()
    {
    }
    void PrintDegug()
    {
        std::cout << "current select rfds_array is : ";
        for (size_t i = 0; i < num; i++)
        {
            if (_rfds_array[i] != nullptr)
            {
                std::cout << _rfds_array[i]->GetSocketFd() << " ";
            }
        }
        std::cout << std::endl;
    }
    ~SelectServer() {}

private:
    std::unique_ptr<Socket> _listensocket;
    int _port;
    bool _isrunning;

    Socket *_rfds_array[num]; // 需要进行关心的文件描述符, 检查他是否是就绪的
};