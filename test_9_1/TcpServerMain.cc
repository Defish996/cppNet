#include "Protocol.hpp"
#include "Socket.hpp"
#include "TcpServer.hpp"
#include <memory>
#include <unistd.h>
#include <iostream>

using namespace Net_Work;

void HandlerRequest(Socket *sockp)
{
    // 接收请求
    while(true)
    {
        struct Request req;
        recv(sockp->GetSocketFd(), &req, sizeof(req), 0);
        req.Debug();
    }
}

// ./TcpServer port
int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    uint16_t localport = std::stoi(argv[1]);
    std::unique_ptr<TcpServer> svr(new TcpServer(localport, HandlerRequest));
    svr->Loop();

    return 0;
}