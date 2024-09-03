#include "Protocol.hpp"
#include "Socket.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
// ./TcpClient Serverip Serverport
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <Serverip> <Serverport>" << std::endl;
        return 1;
    }
    std::string serverip = argv[1];
    uint16_t serverport = std::stoi(argv[2]);

    Net_Work::Socket* sock = new Net_Work::TcpSocket();
    if(!sock->BuildConnectSocketMethod(serverip, serverport))
    {
        std::cerr << "Connect " << serverip << ":" << serverport << " failed" << std::endl;
        return 1;
    }
    std::cout << "Connect " << serverip << ":" << serverport << " success" << std::endl;
    
    std::unique_ptr<Factory> factory = std::make_unique<Factory>();
    std::shared_ptr<Request> req = factory->BuildRequest(10, 20, '+'); 
    
    // 发送请求
    while(true)
    {
        req->Inc();
        send(sock->GetSocketFd(), &(*req), sizeof(Request), 0);
        sleep(1);
    }

    sock->CloseSocket();
    return 0;
}