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

    Socket* sock = new TcpSocket();
    if(!sock->BuildConnectSocketMethod(serverip, serverport))
    {
        std::cerr << "Connect " << serverip << ":" << serverport << " failed" << std::endl;
        return 1;
    }
    std::cout << "Connect " << serverip << ":" << serverport << " success" << std::endl;
    sock->CloseSocket();
    sleep(10);
    return 0;
}