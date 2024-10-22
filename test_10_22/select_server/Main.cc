#include <iostream>
#include <memory>
#include "PollServer.hpp"


// ./select_server 8080
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        std::cout << "Usage : " << argv[0] << " port " << std::endl;
        return 1;
    }
    uint16_t port = atoi(argv[1]);
    std::unique_ptr<PollServer> svr = std::make_unique<PollServer>(port);
    svr->Init();
    svr->Loop();



    return 0;
}