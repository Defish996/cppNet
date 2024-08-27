#pragma once
#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const char *root = "/";
const char *dev_null = "/dev/null";

void Deamon(bool ischdir, int isclosefd)
{
    // 1.忽略可能引起进程异常的信号(取决于应用场景)
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    // 2.让自己不要成为组长
    if (fork() > 0)
        exit(0); // 父进程退出

    // 3.设置让自己成为一个新的会话, 后面的代码是子进程在走
    setsid();

    // 4.每一个进程都有自己的CWD(PWD的路径), 是否将当前进程的CWD设置成根目录(好处是, 这样的顶级方式查找)
    // 1. 防止依赖临时文件
    // 守护进程通常不需要访问任何特定的文件或目录。如果守护进程在其启动时的工作目录中创建了临时文件或其他资源，可能会导致不必要的依赖。将 CWD 设置为根目录可以确保守护进程不会依赖于任何特定的工作目录。
    // 2. 避免权限问题
    // 如果守护进程在其启动时的工作目录中创建文件或修改文件，可能会遇到权限问题。例如，如果守护进程运行在非特权用户账户下，而该用户的家目录或工作目录可能不具备相应的写权限，这会导致守护进程无法正常工作。将 CWD 设置为根目录可以避免这些问题。
    // 3. 减少对系统的影响
    // 将 CWD 设置为根目录可以减少守护进程对其启动环境的依赖，从而减少其对系统的影响。这意味着守护进程不会意外地修改或依赖于其启动时的工作目录中的文件或目录。
    // 4. 便于管理和维护
    // 将 CWD 设置为根目录可以使守护进程的管理和维护更加简单。例如，如果守护进程需要在日志中记录其工作目录，将 CWD 设置为根目录可以确保日志的一致性和可预测性。
    // 5. 避免挂载点问题
    // 如果守护进程在其启动时的工作目录位于一个挂载点上（例如，一个可移动设备或网络文件系统），那么如果该挂载点变得不可用（例如，设备断开连接或网络中断），守护进程可能会出现问题。将 CWD 设置为根目录可以避免这些问题。
    if (ischdir)
        chdir(root); // 更改目录为根目录

    // 5.守护进程是一个独立的会话, 不需要和用户的输入输出进行关联, 关闭标准输入输出(不推荐, 万一真的存在从哪个文件读取输入到另一个文件, 此时就会出错)
    // 所以好的做法是使用 /dev/null, 向这个文件写入的任何东西都会丢弃, 读的时候读到文件结尾
    int fd = open(dev_null, O_RDWR);
    if (isclosefd) // 是否是直接关闭文件描述符的方式
    {
        close(0);
        close(1);
        close(2);
    }
    else // 重定向到null文件的形式
    {
        if (fd > 0)
        {
            // int dup2(int oldfd, int newfd);
            dup2(fd, 0);
            dup2(fd, 1);
            dup2(fd, 2);
            close(fd);
        }
    }
}