#pragma once

#include <iostream>
#include <memory>

// 定制协议
class Request {
public:
    Request()
    {}
    Request(int x, int y, char op):_data_x(x), _data_y(y), _oper(op)
    {}
    void Debug()
    {
        std::cout << "x: " << _data_x << " y: " << _data_y << " op: " << _oper << std::endl;
    }
    void Inc()
    {
        _data_x++;
        _data_y++;
    }
private:
    int _data_x;// 参数x
    int _data_y;// 参数y
    char _oper;// 操作符 + - * / %
};

class Response {
public:
    Response()
    {}
    Response(int result, int code):_result(result), _code(code)
    {}
    ~Response();

private:
    int _result;// 结果
    int _code;// 状态码
};

// 简单的工厂模式, 建造类设计模式
class Factory {
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