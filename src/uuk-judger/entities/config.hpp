#pragma once

#include <string>
#include <vector>

class Config
{
public:
    using i64 = long long;
    std::string code_path;
    std::vector<std::string> args;
    int time_limit;
    i64 mem_limit;
    std::string input_path;
    std::string output_path;
    std::string work_dir;

    Config()
        : code_path(""),
          args({}),
          time_limit(1000),
          mem_limit(65536),
          input_path(""),
          output_path(""),
          work_dir("") {}
    Config(const std::string &cp,
           const std::vector<std::string> &a,
           const int tl,
           const i64 ml,
           const std::string &ip,
           const std::string &op, const std::string &wd)
        : code_path(cp),
          args(a),
          time_limit(tl),
          mem_limit(ml),
          input_path(ip),
          output_path(op),
          work_dir(wd) {}
};