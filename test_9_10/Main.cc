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
    // 文本读取代码
    // std::ifstream in(path, std::ios::binary); // 以二进制的方式来进行读取
    // if (!in.is_open())
    // {
    //     return "";
    // }
    // std::string content;
    // std::string line;
    // while(std::getline(in, line))// BUG
    // {
    //     content += line;
    // }
    // in.close();
    // return content;

    // 二进制读取代码
    std::ifstream in(path, std::ios::binary); // 以二进制的方式来进行读取
    if (!in.is_open())
    {
        return "";
    }
    // 如果是图片文件, 就不能按照string类的方式来读取了
    // 需要使用char[] 来读取
    in.seekg(0, in.end);       // 将文件指针移动到文件末尾
    size_t fsize = in.tellg(); // 获取文件偏移量
    in.seekg(0, in.beg);       // 将文件指针移动到文件开头以便重新读取

    std::string content;
    content.resize(fsize);                   // 调整字符串的大小
    in.read((char *)content.c_str(), fsize); // 将文件内容全部放到字符数组中, 参数分别是: 字符数组, 字符数组的大小, 然后返回

    // // 更推荐使用vector这样的方式来进行读取
    // std::vector<char> content;
    // content.resize(fsize); // 调整字符串的大小
    // in.read(content.data(), fsize); // 将文件内容读取到字符数组中, 参数分别是: 字符数组, 字符数组的大小

    in.close(); // 关闭文件
    return content;
}

std::string SuffixToType(const std::string &suffix)
{
    if (suffix == ".html" || suffix == ".html")
        return "text/html";
    else if (suffix == ".png")
        return "image/png";
    else if (suffix == ".jpg")
        return "image/jpeg";
    else
    {
        return "text/html";
    }
}

std::string HandlerHTTPRequest(std::string &request) // 接受的参数假设我们已经读到了一个完整的请求
{
    HttpRequest req;
    // 进行反序列化操作
    req.Deserialize(request);
    req.Parse();
    req.DebugHttp();

    std::string content;
    content = GetFileContent(req.Path());

    // std::cout << "content: " << content << std::endl;

    std::cout << "suffix: " << req.Suffix() << "Type: " << SuffixToType(req.Suffix()) << std::endl;
    if (!content.empty())
    {
        // 要响应的内容, 需要知道请求的资源是什么
        std::string httpstatusline = "HTTP/1.1 200 OK\r\n";                                    // 状态行 状态行, 状态码, 状态描述
        std::string httpheader = "Content-length: " + std::to_string(content.size()) + "\r\n"; // 内容长度
        httpheader += "Content-Type:" + SuffixToType(req.Suffix()) + "\r\n";                   // 正文的内容类型

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