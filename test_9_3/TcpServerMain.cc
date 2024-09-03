#include "Protocol.hpp"
#include "Socket.hpp"
#include "TcpServer.hpp"
#include "Calculate.hpp"
#include <memory>
#include <unistd.h>
#include <iostream>

using namespace Net_Work;
using namespace Protocol;
using namespace CalculateNS;// 传进去的参数大部分都是指针, 内存比较小, 所以可以直接展开

void HandlerRequest(Socket *sockp)
{
    Calculate calculate;
    std::string inbufferstream;// 接收请求的缓冲区

    // 1.创建一个工厂类, 用于创建请求对象
    std::unique_ptr<Factory> factory = std::make_unique<Factory>();
    auto req = factory->BuildRequest();

    // 接收请求
    while(true)
    {
        // 1.读取报文
        if (!sockp->Recv(&inbufferstream, 1024))
        {
            break;// 接收失败
        }
        // 2.分析收到的字节流, 是否有一个完整的报文
        std::string message;
        if(!Decode(inbufferstream, &message))
        {
            continue;
        }
        // 3.处理请求, 走到这里, 说明已经有一个完整的报文了, 可以进行反序列化
        if(!req->Deserialize(message))
        {
            break;// 反序列化失败
        }
        // 4.业务处理, 计算器
        auto resp = calculate.Cal(req);
        // 5.将响应对象序列化

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