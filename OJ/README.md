项目核心是三个模块
1. comm: 公共模块
2. compile_server: 编译与运行模块
3. oj_server: 获取题目列表，查看题目编写题目界面，负载均衡，其他功能

1. comm: 存放一些公共的函数，如数据库连接，文件操作等
2. compile_server: 存放编译与运行代码的函数，如客户端上传的代码在本地进行分析实现结果的验证
3.oj_server部分是对题目信息的描述 判题的标准和数据库链接的相关操作,实现对服务器主逻辑的实现操作