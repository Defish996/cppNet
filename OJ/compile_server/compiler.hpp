#pragma once // 主要进行代码编译服务

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../comm/util.hpp"
#include "../comm/log.hpp"

// 只负责进行代码的编译

namespace ns_compiler
{
    // 引入路径拼接功能, 方法由util.hpp提供
    using namespace ns_util; // 引入工具类
    using namespace ns_log;

    class Compiler
    {
    public:
        Compiler()
        {}
        ~Compiler()
        {}
        //返回值：编译成功：true，否则：false
        //输入参数：编译的文件名
        //file_name: 1234
        //1234 -> ./temp/1234.cpp // 构建源文件   后期进行修改
        //1234 -> ./temp/1234.exe   // 构建可执行文件
        //1234 -> ./temp/1234.stderr // 构建标准错误文件
        static bool Compile(const std::string &file_name)// 因为是编译, 也不需要什么成员对象, 直接设置为static  && 给一个文件才能进行编译 && 完成来自浏览器的cpp文件的编译
        {
            // 为什么使用子进程?
            pid_t pid = fork();
            if(pid < 0)
            {
                LOG(ERROR) << "内部错误，创建子进程失败" << "\n";
                return false;
            }
            else if (pid == 0) // 主要目的是进行程序替换完成编译工作
            {
                umask(0);
                int _stderr = open(PathUtil::CompilerError(file_name).c_str(), O_CREAT | O_WRONLY, 0644);// 形成一个.compile_error文件用于记录编译出错的日志
                if(_stderr < 0){
                    LOG(WARNING) << "没有成功形成stderr文件" << "\n";
                    exit(1);// 打开文件失败, 直接退出
                }
                //重定向标准错误到_stderr
                dup2(_stderr, 2); // 将这个未来可能的标准错误重定向到_stderr文件 
                
                //程序替换，并不影响进程的文件描述符表
                //子进程: 调用编译器，完成对代码的编译工作
                //g++ -o target src -std=c++11
                execlp("g++", "g++", "-o", PathUtil::Exe(file_name).c_str(),\
                PathUtil::Src(file_name).c_str(), "-D", "COMPILER_ONLINE","-std=c++11",  nullptr/*不要忘记*/);// execlp的使用, 参数来自Util.hpp工具类
                LOG(ERROR) << "启动编译器g++失败，可能是参数错误" << "\n";
                exit(2);
            }
            else{
                waitpid(pid, nullptr, 0); // 等待子进程的功能
                //编译是否成功,就看有没有形成对应的可执行程序 && 有没有编译成功还可以使用g++的返回值进行判定, 不过这边就不进行了
                if(FileUtil::IsFileExists(PathUtil::Exe(file_name))) // 使用这样的方式判定文件是否存在
                {
                    LOG(INFO) << PathUtil::Src(file_name) << " 编译成功!" << "\n";
                    return true;
                }
            }
            LOG(ERROR) << "编译失败，没有形成可执行程序" << "\n";
            return false;
        }
    };
}