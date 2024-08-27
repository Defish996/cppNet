#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "Log.hpp"

const std::string unknown = "unknown";
const std::string mydict = "./resource/dict.txt";
const std::string sep = " ";

class Wold
{
    // 单词， 音节， 中文解释，类型， 相近
};

class Translate
{
public:
    Translate(std::string dict_path = mydict) : _dict_path(dict_path)
    {
        LoadDict();
        Parse();
    }
    void LoadDict()// 加载字典
    {
        std::ifstream in(_dict_path);
        std::string line;
        while(std::getline(in, line))// 按行读取, 读完加载到lines
        {
            lines.push_back(line);
        }
        in.close();
        //lg.LogMessage(Debug, "Load dict txt success, path: %s\n", _dict_path.c_str());
    }
    void Parse() // 进行分割
    {
        for(auto &line : lines)
        {
            auto pos = line.find(sep); // XXXX YYY
            if(pos == std::string::npos) continue;// 没找到就进行下一个
            else
            {
                std::string word = line.substr(0, pos);
                std::string chinese = line.substr(pos+sep.size());// 从sep之后的第一个字符开始截取，即跳过整个sep，直到line的末尾。
                _dict.insert(std::make_pair(word, chinese));
            }
        }
        lg.LogMessage(Debug, "Parse dict txt success, path: %s\n", _dict_path.c_str());
    }
    void debug()
    {
        // for(auto &line : lines)std::cout << line << std::endl;

        for(auto &elem : _dict)
        {
            std::cout << elem.first << " : " << elem.second << std::endl;
        }
    }
    std::string Excute(const std::string &word)// 翻译任务的执行
    {
        auto iter = _dict.find(word);
        if (iter == _dict.end())
            return unknown;
        else
            return _dict[word];
    }
    ~Translate()
    {
    }

private:
    std::string _dict_path; 
    std::unordered_map<std::string, std::string> _dict;
    std::vector<std::string> lines;
};

