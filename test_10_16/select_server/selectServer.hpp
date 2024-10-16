#pragma once

#include <iostream>
#include <sys/select.h>
#include "Socket.hpp"
#include "Log.hpp"

const static int defaultport = 8888;
const static int backlog = 8;

using namespace Net_Work;

class SelectServer{
public:
    SelectServer(int port = defaultport) 
        :_port(port)
        ,_listensocket(new TcpSocket)
        {}
    void Init()
    {
        _listensocket->BuildListenSocketMethod();
    }
    ~SelectServer(){}
private:
    std::unique_ptr<Socket> _listensocket;
    int _port;
};