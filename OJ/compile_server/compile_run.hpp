#pragma once // 适配用户请求 定制通信协议字段

#include "compiler.hpp"
#include "runner.hpp"
#include "../comm/log.hpp"
#include "../comm/util.hpp"

#include <signal.h>
#include <unistd.h>
#include <jsoncpp/json/json.h>

namespace ns_compile_and_run
{
    using namespace ns_log;
    using namespace ns_util;
    using namespace ns_compiler;
    using namespace ns_runner;

    class CompileAndRun // 他里面都是静态函数 只起到适配和正确连接的作用
    {
    public:
        static void RemoveTempFile(const std::string &file_name)
        {
            //清理文件的个数是不确定的，但是有哪些我们是知道的
            std::string _src = PathUtil::Src(file_name);
            if(FileUtil::IsFileExists(_src)) unlink(_src.c_str());

            std::string _compiler_error = PathUtil::CompilerError(file_name);
            if(FileUtil::IsFileExists(_compiler_error)) unlink(_compiler_error.c_str());

            std::string _execute = PathUtil::Exe(file_name);
            if(FileUtil::IsFileExists(_execute)) unlink(_execute.c_str());

            std::string _stdin = PathUtil::Stdin(file_name);
            if(FileUtil::IsFileExists(_stdin)) unlink(_stdin.c_str());

            std::string _stdout = PathUtil::Stdout(file_name);
            if(FileUtil::IsFileExists(_stdout)) unlink(_stdout.c_str());

            std::string _stderr = PathUtil::Stderr(file_name);
            if(FileUtil::IsFileExists(_stderr)) unlink(_stderr.c_str());
        }
        // code > 0 : 进程收到了信号导致异常奔溃
        // code < 0 : 整个过程非运行报错(代码为空，编译报错等)
        // code = 0 : 整个过程全部完成
        //待完善
        static std::string CodeToDesc(int code, const std::string &file_name)
        {
            std::string desc;
            switch (code)
            {
            case 0:
                desc = "编译运行成功";
                break;
            case -1:
                desc = "提交的代码是空";
                break;
            case -2:
                desc = "未知错误";
                break;
            case -3:
                // desc = "代码编译的时候发生了错误";
                FileUtil::ReadFile(PathUtil::CompilerError(file_name), &desc, true);
                break;
            case SIGABRT: // 6
                desc = "内存超过范围";
                break;
            case SIGXCPU: // 24
                desc = "CPU使用超时";
                break;
            case SIGFPE: // 8
                desc = "浮点数溢出";
                break;
            default:
                desc = "未知: " + std::to_string(code);
                break;
            }

            return desc;
        }
        // 通过http 将数据通过json传输给server
        /***************************************
         * 输入:
         * code： 用户提交的代码
         * input: 用户给自己提交的代码对应的输入，不做处理
         * cpu_limit: 时间要求
         * mem_limit: 空间要求
         *
         * 输出:
         * 必填
         * status: 状态码
         * reason: 请求结果
         * 选填：
         * stdout: 我的程序运行完的结果
         * stderr: 我的程序运行完的错误结果
         *
         * 参数：
         * in_json: {"code": "#include..."(主体代码部分), "input": ""(用户输入部分, 默认为空, 所以不做处理),"cpu_limit":1, "mem_limit":10240}
         * out_json: {"status":"0", "reason":"","stdout":"","stderr":"",}
         * ************************************/
        static void Start(const std::string &in_json, std::string *out_json)
        {
            // 进行反序列化
            Json::Value in_value; // 输入时的json值 
            Json::Reader reader;
            reader.parse(in_json, in_value); //最后在处理差错问题, 两个参数表示要解析谁, 解析到哪里

            std::string code = in_value["code"].asString(); // 进行提取值并保存,完成反序列化
            std::string input = in_value["input"].asString();
            int cpu_limit = in_value["cpu_limit"].asInt();
            int mem_limit = in_value["mem_limit"].asInt();

            int status_code = 0;
            Json::Value out_value;
            int run_result = 0;
            std::string file_name; //需要内部形成的唯一文件名

            if (code.size() == 0)
            {
                status_code = -1; //代码为空
                goto END;
            }
            // 形成的文件名只具有唯一性，没有目录没有后缀
            // 毫秒级时间戳+原子性递增唯一值: 来保证唯一性
            file_name = FileUtil::UniqFileName(); // 获得唯一性文件名
            LOG(INFO) << "11111111111111111111111" << "\n";
            
            //形成临时src文件
            if (!FileUtil::WriteFile(PathUtil::Src(file_name), code))
            {
                status_code = -2; //未知错误
                goto END;
            }

            if (!Compiler::Compile(file_name))
            {
                //编译失败
                status_code = -3; //代码编译的时候发生了错误
                goto END;
            }

            run_result = Runner::Run(file_name, cpu_limit, mem_limit);
            if (run_result < 0)
            {
                status_code = -2; //未知错误
            }
            else if (run_result > 0)
            {
                //程序运行崩溃了
                status_code = run_result;
            }
            else
            {
                //运行成功
                status_code = 0;
            }
        END:
            out_value["status"] = status_code;
            out_value["reason"] = CodeToDesc(status_code, file_name);
            if (status_code == 0)
            {
                // 整个过程全部成功
                std::string _stdout;
                FileUtil::ReadFile(PathUtil::Stdout(file_name), &_stdout, true);
                out_value["stdout"] = _stdout;

                std::string _stderr;
                FileUtil::ReadFile(PathUtil::Stderr(file_name), &_stderr, true);
                out_value["stderr"] = _stderr;
            }

            Json::StyledWriter writer;
            *out_json = writer.write(out_value);

            RemoveTempFile(file_name);
        }
    };
}