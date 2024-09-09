#pragma once
#include <iostream>
#include <string>
#include <time.h>
#include <cstdarg> //可变参数宏依赖
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

enum // 日志等级
{
    Debug = 0,
    Info,    // 常规信息
    Warning, // 警告信息
    Error,   // 错误信息,但还是可以运行
    Fatal    // 重大错误,代码终止

};
enum // 打印日志位置
{
    Screen = 10, // 向显示器打印
    OneFile,     // 向一个文件中打印
    ClassFile    // 按类型分为向多个文件打印
};

std::string LevelToString(int level)
{
    switch (level)
    {
    case Debug:
        return "Debug";
    case Info:
        return "Info";
    case Warning:
        return "Warning";
    case Error:
        return "Error";
    case Fatal:
        return "Fatal";
    default:
        return "Unknown";
    }
}
const int defaultstyle = Screen;             // 默认向显示器打印
const std::string default_filename = "log."; // 默认打印的文件名称
const std::string logdir = "log";

// 日志
class Log
{
public:
    Log() : style(defaultstyle), filename(default_filename)
    {
        mkdir(logdir.c_str(), 0775); // 没有就创建, 有就不用管
    }
    void Enable(int sty = defaultstyle) // 调整日志打印的位置
    {
        style = sty;
    }
    void WriteLogToClassFile(const std::string &LevelString, const std::string &message) // 向多个文件写入,枚举对应的操作,内部根据对应的内容变为对具体文件的写入
    {
        std::string filename = logdir;
        filename += "/";
        filename += default_filename;
        filename += LevelString;
        WriteLogToOneFile(filename, message);
    }
    void WriteLogToOneFile(const std::string &logname, const std::string &message) // 向一个文件中写入,文件操作
    {
        // std::ofstream out(logname, std::ios::app); // 打开一个名为logname的文件流,第二个参数是确保可以追加内容
        // if (!out.is_open())                        // 确保打开成功
        // {
        //     std::cout << "open log file failed" << std::endl;
        //     return;
        // }
        // out << message; // 向这个流中重定向,是否追加由打开方式决定
        // out.close();
        umask(0);
        int fd = open(logname.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
        if (fd < 0)
            return;
        write(fd, message.c_str(), message.size());
        close(fd);
    }
    void WriteLog(const std::string &LevelString, const std::string &message) // 向日志中写入信息
    {
        switch (style)
        {
        case Screen:
            std::cout << message;
            break;
        case OneFile:
            WriteLogToClassFile("all", message);
            break;
        case ClassFile:
            WriteLogToClassFile(LevelString, message); //
            break;
        default:
            break;
        }
    }
    // 设计成类似c语言形式的接口,方便像printf函数那样打印不同类别的信息,支持可变参数
    // LogMessage(LogLevel, "%s, %d, %f..."....);//日志等级 打印的信息
    void LogMessage(int level, const char *format, ...)
    {
        // // struct tm *tm = localtime((const time_t *)time(nullptr));
        // const time_t t = time(nullptr);//获取时间戳
        // std::string tm = GetTime(t);//将时间戳转化为年月日时分秒的string
        // char content[1024];
        // va_list args; // const char *, va_等的用来对付可变参数的函数
        // va_start(args, format);//初始化()
        // // args指向了可变参数部分
        // vsnprintf(content, sizeof(content), format, args); // 可变参数家族函数之一, 将可变参数内容输入到content中
        // va_end(args);                                      // args == nullptr

        // printf("[%s][%s]%s", LevelToString(level).c_str(), tm.c_str(), content);

        //[Debug][2024-7-10-14:23:19][3112]haha hehe, 3.140000, 7
        // 将上述的信息按照[]为left, 其他为right进行分开
        char rightbuffer[1024];
        va_list args;           // const char *, va_等的用来对付可变参数的函数
        va_start(args, format); // 初始化()
        // args指向了可变参数部分
        vsnprintf(rightbuffer, sizeof(rightbuffer), format, args); // 可变参数家族函数之一, 将可变参数内容输入到content中
        va_end(args);

        char leftbuffer[1024];
        const time_t t = time(nullptr); // 获取时间戳
        std::string tm = GetTime(t);    // 将时间戳转化为年月日时分秒的string
        std::string LevelString = LevelToString(level).c_str();
        std::string pid = std::to_string(getpid());
        snprintf(leftbuffer, sizeof(leftbuffer), "[%s][%s][%s]", LevelString.c_str(), tm.c_str(), pid.c_str());

        std::string loginfo = leftbuffer;
        loginfo += rightbuffer;
        WriteLog(LevelString, loginfo);
    }
    ~Log()
    {
    }

private:
    std::string GetTime(const time_t &t)
    {
        struct tm *tm = localtime(&t); // 将时间戳转为struct tm *类型,这个结构体内置的变量可以获取对应的时间
        // 年的计时从1900开始,所以计算当前要加1900, 月默认从0开始记,所以加1 这一点在man手册中也有说明
        return std::to_string(tm->tm_year + 1900) + "-" + std::to_string(tm->tm_mon + 1) + "-" + std::to_string(tm->tm_mday) + "-" + std::to_string(tm->tm_hour) + ":" + std::to_string(tm->tm_min) + ":" + std::to_string(tm->tm_sec);

        // 也可以使用这种方式
        /*
        char buf[256];
        snprintf(buf, sizeof(buf), "%d-%02d-%02d %02d:%02d:%02d",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                 tm->tm_hour, tm->tm_min, tm->tm_sec);
        return buf;
        */
    }

private:
    int style;            // 日志等级
    std::string filename; // 日志文件名称
};

//对这个日志添加配置中心
Log lg;
class Conf
{
public:
    Conf()
    {
        lg.Enable(OneFile);
    }
    ~Conf()
    {
    }
};

Conf conf;