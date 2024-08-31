#pragma once

#include <iostream>

// 定制协议
class Request {

public:
    Request(int x, int y);
    ~Request();

private:
    int _data_x;// 参数x
    int _data_y;// 参数y
    char _oper;// 操作符 + - * / %
};

class Response {
private:
    int _result;// 结果
    int _code;// 状态码
};

