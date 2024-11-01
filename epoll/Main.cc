#include "EpollServer.hpp"

#include <iostream>
#include <string>
#include <memory>

inline void Usage(std::string process)
{
    std::cout << "Please input ./" << process << " port" << std::endl;
}

//./epoll_server 8888
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        return -1;
    }

    int port = std::stoi(argv[1]);
    std::unique_ptr<EpollServer> svr = std::make_unique<EpollServer>(port);    

    svr->InitServer();
    svr->Loop();
    return 0;
}