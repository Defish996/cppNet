#include "Socket.hpp"
#include "TcpServer.hpp"
#include <memory>
#include <string>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "HttpProtocal.hpp"

std::string GetFileContent(const std::string &path) // 获取文件内容
{
    std::ifstream in(path, std::ios::binary); // 以二进制的方式来进行读取
    if (!in.is_open())
    {
        return "";
    }
    std::string content;
    std::string line;
    while(std::getline(in, line))// BUG
    {
        content += line;
    }
    in.close();
    return content;
}

std::string HandlerHTTPRequest(std::string &request) // 接受的参数假设我们已经读到了一个完整的请求
{
    HttpRequest req;
    // 进行反序列化操作
    req.Deserialize(request);
    req.ParseReqLine();
    req.DebugHttp();

    std::string content;
    content = GetFileContent(req.Path());

    std::cout << "content: " << content << std::endl;
    if (!content.empty()) 
    {
        // 要响应的内容, 需要知道请求的资源是什么
        std::string httpstatusline = "HTTP/1.1 200 OK\r\n";                                    // 状态行 状态行, 状态码, 状态描述
        std::string httpheader = "Content-length: " + std::to_string(content.size()) + "\r\n"; // 内容长度
        httpheader += "\r\n";
        std::string httpresponse = httpstatusline + httpheader + content; // 内容为 状态行 + 头部 + 内容
        return httpresponse;
    }
    return "";
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
    std::unique_ptr<TcpServer> svr(new TcpServer(localport, HandlerHTTPRequest));
    svr->Loop();

    return 0;
}