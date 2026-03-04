#pragma once

#include <string>
#include <cstring>

enum class IPCType : int
{
    REQ_DATA = 0,
    RDY_DATA = 1,
    REP_STATUS = 2,
    NYA_PING = 3,
    NYA_PONG = 4,
};

struct IPCMessage
{
    using i64 = long long;
    IPCType type;
    JudgeResult info;
};

struct JudgeResult
{
    using i64 = long long;
    int problem_id;
    i64 submission_id;
    char status[32];
    char detail[2048];
    int time_cost;
    int mem_cost;
    void set_status(const std::string &str)
    {
        std::memset(status, 0, sizeof(status));
        std::strncpy(status, str.c_str(), sizeof(status) - 1);
        status[sizeof(status) - 1] = '\0';
    }
    void set_detail(const std::string &str)
    {
        std::memset(detail, 0, sizeof(detail));
        std::strncpy(detail, str.c_str(), sizeof(detail) - 1);
        detail[sizeof(detail) - 1] = '\0';
    }
};
