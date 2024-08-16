#pragma once
#include <iostream>
class nocopy
{
public:
    nocopy() = default;
    nocopy(const nocopy&) = delete;
    const nocopy& operator=(const nocopy&) = delete;
    //推荐禁用上面那个
    //nocopy& operator=(const nocopy&) = delete;
    ~nocopy() = default;

};