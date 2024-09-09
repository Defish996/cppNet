#include <iostream>
#include <string>
#include <jsoncpp/json/json.h>

int main()
{
    Json::Value root;// 万能类型, 接受任何数据类型
    root["k1"] = 100;
    root["k2"] = 200;
    root["k3"] = 300;

    Json::Value v;
    v["hello"] = "world";
    v["world"] = "nihao";

    root["k4"] = v;

    Json::StyledWriter writer;
    std::string str = writer.write(root);// 序列化
    std::cout << str << std::endl;
    return 0;
}