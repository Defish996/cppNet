#include "Protocol.hpp"
#include "Socket.hpp"
#include "TcpServer.hpp"
#include "Calculate.hpp"
#include <memory>
#include <unistd.h>
#include <iostream>

using namespace Net_Work;
using namespace Protocol;
using namespace CalculateNS; // 传进去的参数大部分都是指针, 内存比较小, 所以可以直接展开

std::string HandlerRequest(std::string &inbufferstream, bool *error_code)
{
    *error_code = true;
    // 0.计算器对象
    Calculate calculate;

    // 1.创建一个工厂类, 用于创建请求对象, 用于响应
    std::unique_ptr<Factory> factory = std::make_unique<Factory>();
    auto req = factory->BuildRequest();

    // 接收请求
    // 2.分析收到的字节流, 是否有一个完整的报文
    std::string total_resp_string;
    std::string message;
    while (Decode(inbufferstream, &message))
    {
        // 3.处理请求, 走到这里, 说明已经有一个完整的报文了, 可以进行反序列化
        if (!req->Deserialize(message))
        {
            *error_code = false;
            std::cout << "Deserialize failed" << std::endl;
            return std::string();
        }
        // 4.业务处理, 计算器
        auto resp = calculate.Cal(req);
        // 5.将响应对象序列化
        std::string send_string;
        resp->Serialize(&send_string); // 序列化之后数据一定是"result code"
        // 6.添加报头(构建完整的字符串级别的响应报文)
        send_string = Encode(send_string);

        // 上述的操作, 可以封装成一个类, 因为序列化的整体流程就是这样的, 封装成一个类可以直接调用, 然后进行后续的工作, 更整体化, 更清晰
        // 7.发送响应, 这边就不进行发送, 进行拼接传输
        total_resp_string += send_string;
        break;
    }
    return total_resp_string;
}

// ./TcpServer port
int main(int argc, char *argv[])
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