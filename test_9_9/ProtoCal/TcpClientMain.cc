#include "Protocol.hpp"
#include "Socket.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <ctime>
#include <cstdlib>

using namespace Protocol;

// ./TcpClient Serverip Serverport
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <Serverip> <Serverport>" << std::endl;
        return 1;
    }
    std::string serverip = argv[1];
    uint16_t serverport = std::stoi(argv[2]);

    Net_Work::Socket *conn = new Net_Work::TcpSocket();
    if (!conn->BuildConnectSocketMethod(serverip, serverport))// 连接socket的方法
    {
        std::cerr << "Connect " << serverip << ":" << serverport << " failed" << std::endl;
        return 1;
    }
    std::cout << "Connect " << serverip << ":" << serverport << " success" << std::endl;

    std::unique_ptr<Factory> factory = std::make_unique<Factory>();// 创建一个工厂对象

    srand(time(nullptr) ^ getpid());
    const std::string opers = "+-*/%^=&";
    // 发送请求
    while (true)
    {
        // 1. 构建一个请求, 遵守协议规则
        int x = rand() % 100; // [0, 99]
        usleep(rand() % 7777);
        int y = rand() % 100; // [0, 99]
        char oper = opers[rand() % opers.size()];
        std::shared_ptr<Request> req = factory->BuildRequest(x, y, oper);// 创建请求对象

        // 2. 对请求进行序列化
        std::string requeststr;
        req->Serialize(&requeststr);

        // for test
        std::string testreq = requeststr;
        testreq += " ";
        testreq += "= ";

        // 3. 添加自描述报头
        requeststr = Encode(requeststr);

        // 4. 发送请求
        conn->Send(requeststr);

        std::string responsestr;

        while (true)
        {
            // 5. 读取响应
            if (!conn->Recv(&responsestr, 1024))
            {
                std::cout << "recv failed" << std::endl;
                break;
            }
            sleep(1);
            // 6. 对响应报文进行解码
            std::string response;
            if (!Decode(responsestr, &response))
                continue; // 不再进行连续处理

            // 7. 对响应进行反序列化 "result code"
            std::shared_ptr<Response> resp = factory->BuildResponse();
            resp->Deserialize(response);

            // 8. 打印响应
            std::cout << testreq << resp->GetResult() << "[" << resp->GetCode() << "]" << std::endl;
            break;
        }
        sleep(1);
    }

    conn->CloseSocket();
    return 0;
}