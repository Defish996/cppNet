#pragma once
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

// 该部分代码用于对HTTP请求进行解析

const std::string HttpSep = "\r\n";

// 未来是可以进行配置的
const std::string homepage = "index.html";
const std::string wwwroot = "./wwwroot";


class HttpRequest
{
public:
    HttpRequest() : _req_line(HttpSep), _path(wwwroot)
    {
    }
    bool GetLine(std::string &request, std::string *line)
    {
        auto pos = request.find(HttpSep);
        if (pos == std::string::npos)
        {
            return false;
        }
        *line = request.substr(0, pos);
        request.erase(0, pos + HttpSep.size()); // 删除掉已经处理过的部分
        return true;
    }
    bool Deserialize(std::string &request) // 请求只需要反序列化即可, 响应是浏览器自动解析的
    {
        std::string line;
        bool ok = GetLine(request, &line); // 按照/r/n截取一行
        if (!ok)                           // 读取失败, 说明请求不完整
        {
            return false;
        }
        _req_line = line; // 请求行
        while (true)
        {
            ok = GetLine(request, &line); // 按照/r/n截取一行
            if (ok && line.empty())       // 如果读取到空行, 说明头部读取完毕
            {
                _req_content = request;
                break;
            }
            else if (ok && !line.empty()) // 读取成功且不是空行, 说明是头部信息
            {
                _req_header.push_back(line);
            }
            else
            {
                break;
            }
        }
        return true;
    }
    std::string GetFileContentHelper(const std::string &path) // 获取正文内容
    {
        // 二进制读取代码
        std::ifstream in(path, std::ios::binary); // 以二进制的方式先打开文件
        if (!in.is_open())
        {
            return "";
        }
        // std::cout << "path: " << path << std::endl;// 解析之后的实际要访问的path
        // std::cout << "_path: " << _path << std::endl;// 网页得到的path
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
        // std::cout << "--------------------"<< std::endl;
        // std::cout << "content: " << content << std::endl;
        // std::cout << "--------------------"<< std::endl;
        return content;
    }

    void DebugHttp()
    {
        std::cout << "_req_line: " << _req_line << std::endl;
        for (auto &line : _req_header)
        {
            std::cout << "--->" << line << std::endl;
        }
        std::cout << "_req_blank: " << _req_blank << std::endl;
        std::cout << "_req_content: " << _req_content << std::endl;

        std::cout << "Method: " << _method << std::endl;
        std::cout << "URL: " << _url << std::endl;
        std::cout << "HTTP Version: " << _http_version << std::endl;
    }
    void ParseReqLine() // 将字符串按照空格进行分割, 进行解析
    {
        std::stringstream ss(_req_line);
        ss >> _method >> _url >> _http_version;

        if (_url == "/") // 如果url为/, 则路径为访问首页
        {
            _path += _url;
            _path += homepage;
        }
        else
        { // 常规请求, 请求浏览器想访问的资源
            _path += _url;
        }
    }
    void ParseSuffix()
    {
        auto pos = _path.rfind(".");
        if (pos == std::string::npos)
        {
            _suffix = ".html";
        }
        else
        {
            _suffix = _path.substr(pos); // 推荐使用unordered_map构建一组映射关系的表
        }
    }
    void Parse()
    {
        // 1.分析请求行
        ParseReqLine();// _req_line: GET / HTTP/1.1
        // 2. 分析路径, 得到文件的后缀
        ParseSuffix();// _suffix: .html
    }
    std::string GetFileContent()
    {
        return GetFileContentHelper(_path);
    }
    std::string Get_404()
    {
        return GetFileContentHelper("./wwwroot/404.html");
    }
    std::string Url()
    {
        return _url;
    }
    std::string Path()
    {
        return _path;
    }
    std::string Suffix()
    {
        return _suffix;
    }
    ~HttpRequest() {}

private:
    // http报文自动
    std::string _req_line;                // method url http_version
    std::vector<std::string> _req_header; // 头部信息
    std::string _req_blank;               // 空行
    std::string _req_content;             // 内容

    // 请求行解析之后的内容
    std::string _method;
    std::string _url; // 可能的内容为 /   /dira/dirb/x.html     /dira/dirb/XX?usrname=100&&password=1234 /dira/dirb
    std::string _http_version;
    std::string _path;   // 默认为"./wwwroot"
    std::string _suffix; // 请求资源的后缀
};

const std::string LineSep = "\r\n";
const std::string BlankSep = " ";

class HttpResponse
{
public:
    HttpResponse() : _status_code(200), _status_code_desc("OK"), _http_version("HTTP/1.1"), _resp_blank(LineSep)
    {
    }
    void SetCode(int code)
    {
        _status_code = code;
    }
    void SetDesc(const std::string &desc)
    {
        _status_code_desc = desc;
    }
    void AddHeader(const std::string &header)
    {
        _resp_header.push_back(header);
    }
    void AddContent(const std::string &content)
    {
        _resp_content = content;
    }
    void MakeStatusLine()
    {
        _status_line = _http_version + BlankSep + std::to_string(_status_code) + BlankSep + _status_code_desc + LineSep;
    }
    std::string Serialize()
    {
        std::string response_str = _status_line;
        for (auto &header : _resp_header)
        {
            response_str.append(header);
        }
        response_str.append(_resp_blank);
        response_str.append(_resp_content);

        return response_str;
    }
    ~HttpResponse() {}

private:
    std::string _status_line;              // 状态行
    std::vector<std::string> _resp_header; // 头部信息
    std::string _resp_blank;               // 空行
    std::string _resp_content;             // 正文

    // 响应行解析之后的内容描述
    int _status_code;
    std::string _status_code_desc;
    std::string _http_version;
};