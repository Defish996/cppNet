#include "Daemon.hpp"
#include <unistd.h>
int main()
{
    // 守护进程
    Deamon(true, false);// CWD为根目录, 使用重定向到null文件的方式
    // 要执行的核心代码
    while(1)
    {
        sleep(1);
    }


    return 0;
}