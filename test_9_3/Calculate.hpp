#pragma once

#include <iostream>
#include <string>
#include <memory>
#include "Protocol.hpp"

// 业务逻辑层
namespace CalculateNS
{
    enum
    {
        Success = 0,
        DivZeroErr,
        ModZeroErr,
        UnknownOper
    };
    class Calculate
    {
    public:
        Calculate()
        {
        }
        std::shared_ptr<Protocol::Response> Cal(std::shared_ptr<Protocol::Request> req) // 把请求中的数据进行计算, 并返回一个响应
        {
            std::shared_ptr<Protocol::Response> resp = std::make_shared<Protocol::Response>();
            resp->SetCode(Success);
            switch (req->GetOper())
            {
            case '+':
                resp->SetResult(req->GetX() + req->GetY());
                break;
            case '-':
                resp->SetResult(req->GetX() - req->GetY());
                break;
            case '*':
                resp->SetResult(req->GetX() * req->GetY());
                break;
            case '/':
            {
                if (req->GetY() == 0)
                {
                    resp->SetCode(DivZeroErr);
                }
                else
                {
                    resp->SetResult(req->GetX() / req->GetY());
                }

            }
            break;
            case '%':
            {
                if (req->GetY() == 0)
                {
                    resp->SetCode(DivZeroErr);
                }
                else
                {
                    resp->SetResult(req->GetX() % req->GetY());
                }
            }
            break;
            default:
                resp->SetCode(UnknownOper);
                break;
            }
            return resp;
        }
        ~Calculate()
        {
        }

        private:
            Protocol::Request req;
        };
    }
