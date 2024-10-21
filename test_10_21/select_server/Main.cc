#include <iostream>
#include <memory>
#include "selectServer.hpp"


// ./select_server 8080
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        std::cout << "Usage : " << argv[0] << " port " << std::endl;
        return 1;
    }
    uint16_t port = atoi(argv[1]);
    std::unique_ptr<SelectServer> svr = std::make_unique<SelectServer>(port);
    svr->Init();
    svr->Loop();



    return 0;
}