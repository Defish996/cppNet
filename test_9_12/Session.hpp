#pragma once

#include <iostream>
#include <memory>
#include <unordered_map>

//先描述
class Session// Session类定义用于对象节点
{
private:
    std::string username;
    std::string passwd;
    int status;
    uint64_t login_time;
    //...
public:
    Session(/* args */);
    ~Session();
};

using session_ptr = std::unique_ptr<Session>;// 使用智能指针管理session


class SessionManager// 管理session
{
public:
    SessionManager(){}
    ~SessionManager(){}
    // 返回值就是sessionid
    std::string AddSession(std::string username, std::string passwd)
    {
        //
    }
    bool DelSession(std::string &sessionid)
    {}
    void ModSession(std::string &sessionid, session_ptr session)
    {}
    session_ptr SearchSession(std::string &sessionid)
    {}
private:
    // 在组织
    // std::string: sessionid
    // session_ptr: 用户登录信息
    std::unordered_map<std::string, session_ptr> sessions;
};