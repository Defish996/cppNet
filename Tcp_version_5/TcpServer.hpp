#pragma once
#include <iostream>
#include <string>
#include <cerrno>
#include <cstring>
#include <unordered_map>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "InetAddr.hpp"
#include "Log.hpp"
#include "nocopy.hpp"
#include "Comm.hpp"
#include "ThreadPool.hpp"

// 后面的sockaddr_in的强转较多, 直接写为宏方便
const static int default_backlog = 5;
using task_t = std::function<void()>;
using callback_t = std::function<void(int, InetAddr &)>;

class TcpServer;

class TcpServer : public nocopy
{
public:
    TcpServer(int16_t port, bool isrunning = false)
        : _port(port), _isrunning(isrunning)
    {
    }
    void Init()
    {
        // 1. 创建socket, file fd, 本质是文件
        _listensock = socket(AF_INET, SOCK_STREAM, 0); // 创建tcp套接字, 第一个参数不变, 第二个参数是tcp, 面向字节流
        if (_listensock < 0)
        {
            lg.LogMessage(Fatal, "create socket error, errno code: %d, error string: %s\n", errno, strerror(errno));
            exit(Fatal);
        }
        lg.LogMessage(Debug, "create socket success, sockfd: %d\n", _listensock);

        // 固定写法, 解决一些少量的bind失败问题
        int opt = 1;
        setsockopt(_listensock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
        lg.LogMessage(Debug, "set socket option success, sockfd: %d\n", _listensock);
        // 2. 填充网络信息并绑定
        struct sockaddr_in local;
        memset(&local, 0, sizeof(local));
        local.sin_addr.s_addr = htonl(INADDR_ANY); // 必须是0, 表示任意
        local.sin_family = AF_INET;                // IPv4的网络套接字
        local.sin_port = htons(_port);
        if (bind(_listensock, CONV(&local), sizeof(local)) != 0)
        {
            lg.LogMessage(Fatal, "bind socket error, errno code: %d, error string: %s\n", errno, strerror(errno));
            exit(Bind_Err);
        }
        lg.LogMessage(Debug, "bind socket success, sockfd: %d\n", _listensock);
        // 3.设置socket为监听状态
        if (listen(_listensock, default_backlog) != 0)
        {
            lg.LogMessage(Fatal, "listen socket error, errno code: %d, error string: %s\n", errno, strerror(errno));
            exit(Listen_Err);
        }
        lg.LogMessage(Debug, "listen socket success, sockfd: %d\n", _listensock);
    
        ThreadNs::ThreadPool<task_t>::GetInstance()->Start();
        funcs.insert(std::make_pair("defaultService", std::bind(&TcpServer::DefaultService, this, std::placeholders::_1, std::placeholders::_2)));
        lg.LogMessage(Debug, "start thread pool success\n");
    }
    // 这个sockfd表示连接是全双工的

    void Service(int sockfd, InetAddr peer)
    {
        char buffer[1024];
        while (true)
        {
            ssize_t n = read(sockfd, buffer, sizeof(buffer) - 1);
            if (n > 0)
            {
                buffer[n] = 0;
                std::cout << peer.PrintDebug() << "# " << buffer << std::endl;

                std::string echo_string = "server echo# ";
                echo_string += buffer;
                write(sockfd, echo_string.c_str(), echo_string.size());
            }
            else if (n == 0) // read如果为0, 文件中表示读到了结尾, 这边表示对端关闭了连接(与管道的情况一模一样)
            {
                lg.LogMessage(Info, "client quit...\n");
                break;
            }
            else
            {
                lg.LogMessage(Error, "read error, errno code: %d, error string: %s\n", errno, strerror(errno));
                break;
            }
        }
    }

    void Start()
    {
        _isrunning = true;
        signal(SIGCHLD, SIG_IGN); // 在linux 中, 如果对SIGCHLD设置为SIG_IGN, 则会自动回收子进程, 不会wait阻塞住
        while (_isrunning)
        {
            // 4. 获取链接
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            int sockfd = accept(_listensock, CONV(&peer), &len); // 进行获取链接
            if (sockfd < 0)
            {
                lg.LogMessage(Warning, "accept socket error, errno code: %d, error string: %s\n", errno, strerror(errno));
                continue; // 获取失败则继续获取
            }
            lg.LogMessage(Debug, "accept socket success, get a new sockfd: %d\n", sockfd);
            
            // v4 多线程
            // ThreadData *td = new ThreadData(sockfd, this, peer);
            // pthread_t tid;
            // pthread_create(&tid, nullptr, HelderRequest, td);
            // 不等待新线程, 只需要设置为分离状态, 这样子线程被OS 回收, 不会阻塞住

            // 线程池版本
            //InetAddr addr(peer);// 任务与线程池绑定
            task_t task = std::bind(&TcpServer::Routine, this, sockfd, InetAddr(peer));
            ThreadNs::ThreadPool<task_t>::GetInstance()->Push(task);
            lg.LogMessage(Debug, "push task success\n");
        }
    }
    std::string Read(int sockfd)// 读信息, 获取用户需要的功能
    {
        char buf[1024] = {0};
        ssize_t n = read(sockfd, buf, sizeof(buf) - 1);
        if(n > 0)
        {
            buf[n] = 0;
        }
        else if(n == 0)
        {
            lg.LogMessage(Info, "client quit...");
        }
        else
        {
            lg.LogMessage(Error, "read error, error code %d, error string: %s", errno, strerror(errno));
        }
        return buf;
    }
    void Routine(int sockfd, InetAddr addr) // 实现对应的功能
    {
        funcs["defaultService"](sockfd, addr);
        std::string type = Read(sockfd);
        lg.LogMessage(Debug, "%s select : %s\n", addr.PrintDebug().c_str(), type.c_str()); // 哪个客户端选择了一个什么任务
        
        // 根据请求类型调用相应的处理函数
        if(type == "ping")// 请求ping
        {
            funcs[type](sockfd, addr);// 调用ping对应的处理函数
        }
        else if(type == "translate")// 同理
        {
            funcs[type](sockfd, addr);
        }
        else if(type == "transform")
        {
            funcs[type](sockfd, addr);
        }
        else{
        }
        close(sockfd);
    }
    void DefaultService(int sockfd, InetAddr &addr)
    {
        (void)addr;
        std::string service_string = " |";
        for(auto func : funcs)
        {
            service_string += func.first + "|";
        }
        write(sockfd, service_string.c_str(), service_string.size());
    }
    void Resgister(const std::string &name, callback_t func)
    {
        funcs[name] = func;
    }
    ~TcpServer() {}

private:
    int16_t _port; //
    int _listensock;
    bool _isrunning; // 是否运行

    // 业务逻辑
    unordered_map<std::string, callback_t> funcs;
};