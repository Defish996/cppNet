#pragma once // 日志功能

#include <iostream>
#include <string>
#include "util.hpp"

namespace ns_log
{
    using namespace ns_util;

    // 日志等级
    enum
    {
        INFO, //0
        DEBUG,
        WARNING,
        ERROR, // 出错不能继续后续的运行
        FATAL
    };

    // 因为经常被用, 所以使用inline 
    inline std::ostream &Log(const std::string &level, const std::string &file_name, int line) // 日志等级, 是在哪个文件的哪一行出错了
    {
        // 添加日志等级
        std::string message = "[";
        message += level;
        message += "]";

        // 添加报错文件名称
        message += "[";
        message += file_name;
        message += "]";

        // 添加报错行
        message += "[";
        message += std::to_string(line);
        message += "]";

        // 日志时间戳
        message += "[";
        message += TimeUtil::GetTimeStamp();
        message += "]";

        // cout 本质 内部是包含缓冲区的
        std::cout << message; //不要endl进行刷新

        return std::cout; // 返回值为标准输出流, 输出到标准缓冲区, 然后与函数调用后的其他内容进行连接
    }
    // LOG(INFo) << "message" << "\n";
    // 开放式日志
    #define LOG(level) Log(#level, __FILE__, __LINE__) // 宏参数带#可以将参数转化为字符串直接使用, 使用当前源文件, 行号
}