#pragma once

#include <string>

class Encode
{
private:
    using i64 = long long;
    constexpr static i64 mod = 100000000000031LL;
    constexpr static i64 base = 13331;

public:
    static i64 hash(const std::string &str)
    {
        i64 val = 0;
        for (auto &i : str)
            val = (val * base + static_cast<i64>(i)) % mod;
        return val;
    }
};