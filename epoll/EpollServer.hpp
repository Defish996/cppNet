#pragma once

#include <iostream>
#include <memory>
#include "Socket.hpp"
#include "Log.hpp" 

const static int gbacklog = 32;

class EpollServer
{
public:
    EpollServer(int port): _port(port), _listensocket(new Net_Work::TcpSocket())
    {}
    bool InitServer()
    {
        _listensocket->BuildListenSocketMethod((uint16_t)_port, gbacklog);
    }
    void Loop()
    {
        _isrunning = true;
        while(_isrunning == true)
        {   
             
        }
        _isrunning = false;
    }
    ~EpollServer();
private:
    int _port;
    std::unique_ptr<Net_Work::Socket> _listensocket;
    bool _isrunning;
};