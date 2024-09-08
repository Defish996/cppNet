#include "Socket.hpp"
#include "Http.hpp"
#include <memory>
#include <string>
#include <unistd.h>
#include <iostream>

std::string HandlerHTTPRequest(std::string &request)
{
    std::cout << request << std::endl;
    std::string content = "<html><body><h1>Hello, World!</h1></body></html>";// 内容
    std::string httpstatusline = "HTTP/1.1 200 OK\r\n";// 状态行 状态行, 状态码, 状态描述
    std::string httpheader = "Content-length: " + std::to_string(content.size()) + "\r\n";// 内容长度
    httpheader += "\r\n";
    std::string httpresponse = httpstatusline + httpheader + content;// 内容为 状态行 + 头部 + 内容
    return httpresponse;
}

// ./HttpServer port
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }
    uint16_t localport = std::stoi(argv[1]);
    std::unique_ptr<HttpServer> svr(new HttpServer(localport, HandlerHTTPRequest));
    svr->Loop();

    return 0;
}  