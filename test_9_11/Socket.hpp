#pragma once 
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
    
#define Convert(addrptr) ((struct sockaddr*)addrptr) // 这个宏用于将sockaddr_in 转换为struct sockaddr*

namespace Net_Work// 网络模块
{
    const static int defaultSockfd = -1;
    const static int backlog = 5;

    enum{
        Socket_Error = 1,
        Bind_Error,
        Listen_Error
    };

    // 封装一个基类, 用于表示一个socket接口类
    // 设计模式: 使用模板方法模式, 未来不管是什么socket, 都继承这个类, 实现这些接口, 这样就可以复用这些接口, 这个流程是固定的
    class Socket {
    public:
        virtual  ~Socket(){}
        virtual void CreateSocketOrDie() = 0;// 创建或者死亡
        virtual void BindSocketOrDie(uint16_t port) = 0;// 绑定或者死亡
        virtual void ListenSocketOrDie(int backlog) = 0;// 监听或者死亡 这个参数为listen函数的backlog参数,后续讲
        virtual Socket* AcceptSocketOrDie(std::string *peerip, uint16_t *peerport) = 0;// 接受或者死亡 输出型参数, 获得远端ip和端口
        virtual bool ConnectSocketOrDie(const std::string& serverip, uint16_t serverport) = 0;// 连接或者死亡
        virtual int GetSocketFd() const = 0;// 获取socket文件描述符 
        virtual void SetSocketFd(int sockfd) = 0;// 设置socket文件描述符
        virtual void CloseSocket() = 0;// 关闭socket
        virtual bool Recv(std::string *buffer, int size) = 0;// 从socket接收数据, 参数是期望接收的字节数
        virtual void Send(const std::string& send_str) = 0;// 从socket发送数据, 参数是发送的字符串
        virtual void ReUseAddr() = 0;// 重用地址
    public:
        void BuildListenSocketMethod(uint16_t port, int backlog)// 创建一个监听socket
        {
            CreateSocketOrDie();// 创建socket
            BindSocketOrDie(port);// 绑定端口
            ListenSocketOrDie(backlog);// 监听
        }
        bool BuildConnectSocketMethod(const std::string& serverip, uint16_t serverport)// 创建一个连接socket的方法
        {
            CreateSocketOrDie();// 创建socket
            return ConnectSocketOrDie(serverip, serverport);// 连接
        }
        void BuildNormalSocketMethod(int sockfd)// 创建一个普通socket
        {
            SetSocketFd(sockfd);
        }
    };

    // 继承类Socket, 用于表示一个tcp socket
    class TcpSocket : public Socket {
    public:
        TcpSocket(int sockfd = -1)
            :_sockfd(sockfd)
        {}
        
        ~TcpSocket()
        {}
        void CreateSocketOrDie() override
        {
            _sockfd = ::socket(AF_INET, SOCK_STREAM, 0); 
            if (_sockfd < 0)
            {
                exit(Socket_Error);
            }

        }
        void BindSocketOrDie(uint16_t port)
        {
            struct sockaddr_in local;
            memset(&local, 0, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(port);
            local.sin_addr.s_addr = INADDR_ANY;
            int n = ::bind(_sockfd, Convert(&local), sizeof(local));
            if (n < 0)
            {
                exit(Bind_Error);
            }
        }
        void ListenSocketOrDie(int backlog) override
        {
            int n = ::listen(_sockfd, backlog);
            if (n < 0)
            {
                exit(Listen_Error);
            }
        }
        Socket* AcceptSocketOrDie(std::string *peerip, uint16_t *peerport)
        {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            int newsockfd = ::accept(_sockfd, Convert(&peer), &len);
            if (newsockfd < 0)
            {
                return nullptr;
            }
            *peerport = ntohs(peer.sin_port);
            *peerip = inet_ntoa(peer.sin_addr);
            Socket* newSock = new TcpSocket(newsockfd);
            return newSock;
        }
        bool ConnectSocketOrDie(const std::string& serverip, uint16_t serverport) override
        {
            struct sockaddr_in server;
            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_port = htons(serverport);
            server.sin_addr.s_addr = inet_addr(serverip.c_str());
            int n = ::connect(_sockfd, Convert(&server), sizeof(server));
            if (n == 0)
            {
                return true;
            }
            return false;
        }

        int GetSocketFd() const override
        {
            return _sockfd;
        }
        void SetSocketFd(int sockfd) override
        {
            _sockfd = sockfd;
        }
        void CloseSocket() override
        {
            if (_sockfd > defaultSockfd)
            {
                ::close(_sockfd);
            }
        }
        bool Recv(std::string *buffer, int size) override // 从socket接收数据, 参数是期望接收的字节数
        {
            char inbuffer[size];  // 创建一个临时缓冲区来接收数据
            int n = recv(_sockfd, inbuffer, size - 1, 0);  // 从socket接收数据
            if (n > 0)  // 如果成功接收到数据
            {
                inbuffer[n] = 0;  // 在接收到的数据末尾添加字符串结束符
                *buffer += inbuffer;  // 将接收到的数据追加到输出缓冲区
                return true;  // 返回true表示成功接收数据
            }
            else if (n == 0)// 为0对方关闭连接 < 0出错   
            {
                return false;
            }
            else
            {
                return false;
            }
        }
        void Send(const std::string& send_str) override
        {
            // 多路转接再统一讲 
            ::send(_sockfd, send_str.c_str(), send_str.size(), 0);// 把数据从socket发送给对方
        }
        void ReUseAddr() override
        {
            int opt = 1;
            setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        }
    private:
        int _sockfd;
    };
}