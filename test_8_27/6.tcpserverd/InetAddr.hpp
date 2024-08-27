#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class InetAddr
{
public:
    InetAddr(struct sockaddr_in &addr)
        :_addr(addr)
    {
        _port = ntohs(_addr.sin_port);      // 网络序列转主机序列
        //_ip = inet_ntoa(_addr.sin_addr); // 网络序列转主机序列
        // 这个转ip的函数返回值是一个char*, 未来的操作可能存在线程安全问题
        
        // 这样的操作与上述类似, 但是更安全, 更推荐
        char ipbuf[64];
        inet_ntop(AF_INET, &_addr.sin_addr, ipbuf, sizeof(ipbuf));
        _ip = ipbuf;
    }
    std::string Ip(){return _ip;}
    uint16_t Port(){return _port;}
    std::string PrintDebug()
    {
        return _ip + ":" + std::to_string(_port);
    }
    bool operator == (const InetAddr &addr)
    {
        return this->_ip == addr._ip && this->_port == addr._port;
    }
    const struct sockaddr_in &GetAddr()
    {
        return _addr;
    }
    ~InetAddr() {}

private:
    std::string _ip;
    uint16_t _port;
    struct sockaddr_in _addr;
};