#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
void SetNoBlock(int fd)
{
	int f1 = fcntl(fd, F_GETFL);// 获取指定文件描述符的状态标记, F_GETFL表示进行获取
	if(f1 < 0)// 说明获取失败了
	{
		std::cerr << "fcntl error" << std::endl;
		exit(0);
	}
	fcntl(fd, F_SETFL, f1 | O_NONBLOCK);// 将指定文件描述符的访问方式设置为非阻塞式的轮询访问, 其中fd表示指定是哪个文件描述符, F_SETFL表示进行设置
	// f1 | O_NONBLOCK表示将文件的标识方式在原有的基础上加上O_NONBLOCK, O_NONBLOCK表示非阻塞式进行访问
}
int main()
{
	SetNoBlock(0);
	while(1)
	{
		char buffer[1024];
		ssize_t s = read(0, buffer, sizeof(buffer) - 1);// 从标准输入进行读取, 读到buffer中, 读到的大小为缓冲区的最大大小-1, 像在语言中的接口会默认加/0, 但是在OS中没有字符串的概念, 全是二进制, 所以-1是为了到时候到了语言层面,可以给/0,不至于导致缓冲区溢出带来问题
		// 他的返回值是实际读取到的字符的大小, 大于0表示读取成功
		if(s > 0)
		{
			buffer[s] = 0;
			std::cout << "echo# " << buffer<< std::endl;
		}
		else if(s == 0)
		{
			std::cout << "end stdin" << std::endl;
			break;
		}
		else // 读取错误的处理方式
		{
			// // 非阻塞式的等待, 如果数据没有准备好, 返回值会按照出错进行返回
			// std::cout << "tets...." << std::endl;
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				std::cout << "OS还没有准备好数据, errno: " << errno << std::endl;
				// 做其他事
			}
			else if(errno == EINTR)
			{
				std::cout << "IO interrupted by signal, try again" << std::endl;
			}
			else
			{
				std::cout << "read error, errno:" << errno << std::endl;				
				break;
			}
		}
		sleep(1);
	}
	return 0;
}
