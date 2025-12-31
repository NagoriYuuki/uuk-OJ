#pragma once
#include "base_entity.hpp"

#include <string>

using i64 = long long;

class Submission : public Entity
{
public:
    i64 id;
    int problem_id;
    int user_id;
    std::string language;
    std::string code_path;
    std::string status;
    std::string detail;
    std::string submit_time;
    int time_cost;
    int mem_cost;
    Submission() = default;
    Submission(const i64 id,
               const int problem_id,
               const int user_id,
               const std::string &language,
               const std::string &code_path,
               const std::string &status,
               const std::string &detail,
               const std::string &submit_time,
               const int time_cost,
               const int mem_cost)
        : id(id),
          problem_id(problem_id),
          user_id(user_id),
          language(language),
          code_path(code_path),
          status(status),
          detail(detail),
          submit_time(submit_time),
          time_cost(time_cost),
          mem_cost(mem_cost) {}
    ~Submission() override = default;
};