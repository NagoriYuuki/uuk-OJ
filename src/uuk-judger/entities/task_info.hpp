#pragma once

#include <string>

class TaskInfo
{
public:
    using i64 = long long;
    i64 submission_id;
    int problem_id;
    std::string code;
    std::string lang;
    int time_limit;
    int mem_limit;
    std::string tc_path;

    TaskInfo() = default;
    TaskInfo(i64 sid, int pid, const std::string &c, const std::string &l, int tl, int ml, const std::string &tc)
        : submission_id(sid), problem_id(pid), code(c), lang(l), time_limit(tl), mem_limit(ml), tc_path(tc) {}
};