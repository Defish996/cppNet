#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>

#include "Comm.hpp"
#define ReTry_count 5
using namespace std;
void Usage(std::string proc)
{
    std::cout << "Usage : \n\t" << proc << " local_port\n"
              << std::endl;
}
bool visitServer(string serverip, int16_t serverport, int &count)
{
    string message;
    char service_list[1024];
    ssize_t m;
    ssize_t n;
    bool ret = true;
    // 1. 创建socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cerr << "socket error" << endl;
        count = 1;
        return false;
    }

    // 2. client必须要ip和port, 但是不需要显示绑定, client 系统随机端口
    // tcp发起链接的时候, 被OS自动进行本地绑定
    // 建立连接connect
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(serverport);
    // server.sin_addr.s_addr = inet_addr(serverip.c_str());
    // 换一个接口 ipv4转二进制 p当成process n是网络 -- 不太准确, 但是好记
    inet_pton(AF_INET, serverip.c_str(), &server.sin_addr); // 1. IPV4--> 四字节ip 2. 进程转网络序列

    n = connect(sockfd, CONV(&server), sizeof(server)); // 这里自动进行bind
    if (n < 0)
    {
        cerr << "connect error" << endl;
        ret = false;
        goto END;
    }

    m = read(sockfd, service_list, sizeof(service_list) - 1);
    if (m > 0)
    {
        service_list[m] = 0;
        cout << "服务器提供的服务列表是: " << service_list << endl;
    }

    cout << "请选择服务: ";
    getline(cin, message);
    write(sockfd, message.c_str(), message.size());

    // 3. 发送数据
    // 与服务器进行持续通信的循环

    // 用户输入的消息
    // 提示用户输入信息
    cout << "Enter> ";
    // 读取用户输入的整行文本
    getline(cin, message);
    if (message == "quit")
        return true;

    // 尝试向服务器发送消息
    n = write(sockfd, message.c_str(), message.size());
    // 检查消息是否成功发送
    if (n > 0)
    {
        // 用于接收服务器响应的缓冲区
        char buffer[1024];
        // 从服务器读取响应, sockfd读写都用, 更加说明是全双工
        ssize_t m = read(sockfd, buffer, sizeof(buffer) - 1);
        // 检查是否接收到响应
        if (m > 0)
        {
            // 确保响应字符串以空字符结尾
            buffer[m] = 0;
            // 打印服务器的响应
            std::cout << buffer << std::endl;
        }
        // 检查服务器是否关闭连接
        else if (m == 0)
        {
            // 服务器关闭时打印信息并退出循环
            std::cout << "server close!" << std::endl;
            return true;
        }
        else // 处理读取响应时的错误情况
        {
            count = 1;
            ret = false;
            goto END;
        }
    }
    // 发送消息时发生错误
    else
    {
        // 打印错误信息并退出循环
        cerr << "write error" << endl;
        count = 1;
        ret = false;
        goto END;
    }

END:
    close(sockfd);
    return ret;
}
// ./tcp_client 127.0.0.1 8888
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        Usage(argv[0]); // 输出使用手册
        return Usage_Err;
    }
    string serverip = argv[1];
    int16_t serverport = stoi(argv[2]);

    int count = 1;
    while (count <= ReTry_count) // 断线重连实现
    {
        bool result = visitServer(serverip, serverport, count);
        if (result)
            break;
        else
        {
            // 重连操作
            sleep(1);
            cout << "reconnecting..., count: " << count++ << endl;
        }
    }
    if (count > ReTry_count)
    {
        cout << "reconnect failed" << endl;
    }
    return 0;
}