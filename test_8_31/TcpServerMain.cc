#include "Protocol.hpp"
#include "Socket.hpp"
#include <unistd.h>
#include <iostream>

// ./TcpServer port
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    uint16_t localport = std::stoi(argv[1]);

    Socket* listensock = new TcpSocket();
    listensock->BuildListenSocketMethod(localport, backlog);

    while(1)
    {
        std::string peerip;
        uint16_t peerport;
        Socket* newsock = listensock->AcceptSocketOrDie(&peerip, &peerport);
        std::cout << "Get a new connection, sockfd is : " << newsock->GetSocketFd() << ", peer ip: " << peerip << ", peer port: " << peerport << std::endl;
        sleep(5);
        newsock->CloseSocket();
    }
    return 0;
}