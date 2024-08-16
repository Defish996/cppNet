#include "TcpServer.hpp"
#include "Comm.hpp"
#include "Translate.hpp"
#include <memory>
#include <cctype>
#include <algorithm>

void Usage(std::string proc)
{
    std::cout << "Usage : \n\t" << proc << " local_port\n"
              << std::endl;
}

Translate trans;

int Interact(int sockfd, std::string &out, const std::string &in) // 交互
{
    char buf[1024];
    ssize_t n = read(sockfd, buf, sizeof(buf) - 1);
    if (n > 0)
    {
        buf[n] = 0;

        write(sockfd, in.c_str(), in.size());
    }
    else if (n == 0)
    {
        lg.LogMessage(Info, "client quit... \n");
    }
    else
    {
        lg.LogMessage(Error, "read error : %s\n", strerror(errno));
    }
    return 0;
}
void Ping(int sockfd, InetAddr addr) // 执行ping指令的内容, 也就是之前tcp_server里面servre方法要做的事
{
    lg.LogMessage(Debug, "%s select %s success, fd : %d\n", addr.PrintDebug().c_str(), "ping", sockfd);
    std::string message;
    Interact(sockfd, message, "pong");
}
void Transform(int sockfd, InetAddr addr)
{
    lg.LogMessage(Debug, "Transform\n");
    char message[128];
    int n = read(sockfd, message, sizeof(message) - 1);
    if (n > 0)
    {
        message[n] = '\0';
    }
    std::string messagebuf(message);
    std::transform(messagebuf.begin(), messagebuf.end(), messagebuf.begin(), [](unsigned char c) { return std::toupper(c); });
    write(sockfd, messagebuf.c_str(), messagebuf.size());                  
}
void Translate(int sockfd, InetAddr addr)
{
    lg.LogMessage(Debug, "%s select %s success, fd : %d\n", addr.PrintDebug().c_str(), "translate", sockfd);
    char wordbuf[128];
    int n = read(sockfd, wordbuf, sizeof(wordbuf) - 1);
    if (n > 0)
    {
        wordbuf[n] = '\0';
    }
    std::string chinese = trans.Excute(wordbuf);
    write(sockfd, chinese.c_str(), chinese.size());

    lg.LogMessage(Debug, "%s Translate service, %s->%s\n", addr.PrintDebug().c_str(), wordbuf, chinese.c_str());
}

// ./tcp_server 8888
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]); // 输出使用手册
        return Usage_Err;
    }
    int16_t port = std::stoi(argv[1]);
    std::unique_ptr<TcpServer> tsvr = std::make_unique<TcpServer>(port);
    tsvr->Resgister("ping", Ping);
    tsvr->Resgister("transform", Transform);
    tsvr->Resgister("translate", Translate);
    tsvr->Init();
    tsvr->Start();
    return 0;
}