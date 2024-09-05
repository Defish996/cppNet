#pragma once

#include <iostream>
#include <memory>

namespace Protocol// 制定协议
{

    // 协议分隔符
    const std::string ProtSep = " ";
    const std::string LineBreakSep = "\n";

    std::string Encode(const std::string &message)//编码方法, 将"x op y"格式的字符串, 变成"len\nx op y\n"形式
    {
        std::string len = std::to_string(message.size());
        std::string package = len + LineBreakSep + message + LineBreakSep;
        return package;
    }

    // 解析的时候, 无法保证传进来的package一定是一个完整的协议, 所以需要传入一个指针, 指向一个字符串, 将解析出来的协议内容放入这个字符串中
    bool Decode(std::string &package, std::string *message)// 解码方法, 将"len\nx op y\n"形式的字符串变成"x op y"形式的字符串
    {
        auto pos = package.find(LineBreakSep);// 拿取第一个"\n"的位置
        if (pos == std::string::npos)
        {
            std::cerr << "Decode failed! pos == std::string::npos" << std::endl;
            return false;
        }
        std::string lens = package.substr(0, pos);// 拿到协议"x op y"的长度, 长度为"len\n"
        int message_len = std::stoi(lens);// 将长度转换为整数
        int total_len = message_len + lens.size() + 2 * LineBreakSep.size();// 确保长度是完整的协议的长度
        if (package.size() < total_len)// 如果长度不够, 返回false
        {
            std::cerr << "Decode failed! package.size() < message_len + pos + LineBreakSep.size()" << std::endl;
            return false;
        }
        // 走到这里, 说明长度至少是一个完整的协议的长度
        *message = package.substr(pos + LineBreakSep.size(), message_len);// 将协议"x op y"的内容赋值给message
        package.erase(0, total_len);// 删除已经解析出来的协议所包含的这个整个字段, 保证下一次解析的时候, 不会重复解析
        return true;
    }

    // 请求类
    class Request
    {
    public:
        Request():_data_x(0), _data_y(0), _oper(0)
        {
        }
        Request(int x, int y, char op) : _data_x(x), _data_y(y), _oper(op)
        {
        }
        void Debug()
        {
            std::cout << "x: " << _data_x << " y: " << _data_y << " op: " << _oper << std::endl;
        }
        void Inc()
        {
            _data_x++;
            _data_y++;
        }
        // 序列化 --- 将对象序列化成字符串
        bool Serialize(std::string *out) // 把对象序列化成"x op y"的字符串格式
        {
            out->append(std::to_string(_data_x));
            out->append(ProtSep);
            out->append(1, _oper);
            out->append(ProtSep);
            out->append(std::to_string(_data_y));
            return true;
        }
        // 反序列化 --- 将字符串反序列化成对象
        bool Deserialize(std::string &in) // 将"x op y"格式的字符串反序列化成对象
        {
            auto left = in.find(ProtSep);
            if (left == std::string::npos)
            {
                std::cerr << "Request::Deserialize failed! left == std::string::npos" << std::endl;
                return false;
            }
            auto right = in.rfind(ProtSep);
            if (right == std::string::npos)
            {
                std::cerr << "Request::Deserialize failed! right == std::string::npos" << std::endl;
                return false;
            }
            _data_x = std::stoi(in.substr(0, left));
            _data_y = std::stoi(in.substr(right + ProtSep.size())); // 从当前位置开始截取到ProtSep.size()个字符, 截到结尾
            std::string oper = in.substr(left + ProtSep.size(), right - left - ProtSep.size());
            if (oper.size() != 1)
            {
                std::cerr << "Request::Deserialize failed! oper.size() != 1" << std::endl;
                return false;
            }
            _oper = oper[0];
            return true;
        }
        int GetX()
        {
            return _data_x;
        }
        int GetY()
        {
            return _data_y;
        }
        char GetOper()
        {
            return _oper;
        }
    private:
        int _data_x; // 参数x
        int _data_y; // 参数y
        char _oper;  // 操作符 + - * / %
    };

    class Response
    {
    public:
        Response():_result(0), _code(0)
        {
        }
        Response(int result, int code) : _result(result), _code(code)
        {
        }
        bool Serialize(std::string *out)
        {
            out->append(std::to_string(_result));
            out->append(ProtSep);
            out->append(std::to_string(_code));
            return true;
        }
        bool Deserialize(std::string &in) // 将"_result _code" 反序列化成对象
        {
            auto pos = in.find(ProtSep);
            if (pos == std::string::npos)
            {
                std::cerr << "Response::Deserialize failed! pos == std::string::npos" << std::endl;
                return false;
            }
            _result = std::stoi(in.substr(0, pos)); // 刚好是[)区间
            _code = std::stoi(in.substr(pos + ProtSep.size()));
            return true;
        }
        void SetResult(int result)
        {
            _result = result;
        }
        void SetCode(int code)
        {
            _code = code;
        }
        int GetResult()
        {
            return _result;
        }
        int GetCode()
        {
            return _code;
        }
    private:
        int _result; // 结果
        int _code;   // 状态码
    };

    // 简单的工厂模式, 建造类设计模式
    class Factory
    {
    public:
        std::shared_ptr<Request> BuildRequest()
        {
            std::shared_ptr<Request> req = std::make_shared<Request>();
            return req;
        }
        std::shared_ptr<Request> BuildRequest(int x, int y, char op)
        {
            std::shared_ptr<Request> req = std::make_shared<Request>(x, y, op);
            return req;
        }
        std::shared_ptr<Response> BuildResponse()
        {
            std::shared_ptr<Response> resp = std::make_shared<Response>();
            return resp;
        }
        std::shared_ptr<Response> BuildResponse(int result, int code)
        {
            std::shared_ptr<Response> resp = std::make_shared<Response>(result, code);
            return resp;
        }
    };
}
