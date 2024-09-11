#include "Socket.hpp"
#include "TcpServer.hpp"
#include <memory>
#include <string>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include "HttpProtocal.hpp"

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
std::string CodetoDesc(int code)
{
    switch (code)
    {
    case 200:
        return "OK";
    case 301:
        return "Moved Permanently";
    case 307:
        return "Temporary Redirect";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 500:
        return "Internal Server Error";
    default:
        return "Unknown";
    }
}
std::string HandlerHTTPRequest(std::string &request) // 接受的参数假设我们已经读到了一个完整的请求
{
    HttpRequest req;
    // 进行反序列化操作
    req.Deserialize(request);
    req.Parse(); // 解析url
    req.DebugHttp();
    int code = 200;

    std::string content = req.GetFileContent(); // 获取解析之后的网页或者是url指定的内容

    if (content.empty()) // 如果获取到的正文内容为空, 说明没有找到url对应的内容, 那就是为空, 返回404界面
    {
        code = 404;
        content = req.Get_404();
    }

    code = 307;// 临时重定向, 会重定向到新的url, 这个新的url放在location中


    // 构建响应
    HttpResponse resp;
    resp.SetCode(code);
    resp.SetDesc(CodetoDesc(code));                                                                // 将状态码转为状态码描述
    resp.MakeStatusLine();                                                                         // 构建响应
    std::string content_length_str = "Content-length: " + std::to_string(content.size()) + "\r\n"; // 内容长度
    resp.AddHeader(content_length_str);
    std::string content_type_str = "Content-Type:" + SuffixToType(req.Suffix()) + "\r\n"; // 正文的内容类型
    std::string location = "Location: https://www.qq.com/\r\n";
    resp.AddHeader(location);
    resp.AddHeader(content_type_str);
    resp.AddContent(content);

    // 响应构建完成, 进行序列化
    return resp.Serialize();
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
    std::unique_ptr<TcpServer> svr(new TcpServer(localport, HandlerHTTPRequest)); // 创建一个TcpServer对象, 参数是端口号和处理请求的函数
    svr->Loop();                                                                  // 开始循环

    return 0;
}