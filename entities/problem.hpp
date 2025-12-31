#pragma once
#include "base_entity.hpp"
#include <string>

class Problem : public Entity
{
public:
    int id;
    std::string title;
    int time_limit;
    int mem_limit;
    std::string description;
    std::string sample_input;
    std::string sample_output;
    std::string created_time;
    std::string tc_path;

    Problem() = default;
    Problem(const int id,
            const std::string &title,
            const int time_limit,
            const int mem_limit,
            const std::string &description,
            const std::string &sample_input,
            const std::string &sample_output,
            const std::string &created_time,
            const std::string &tc_path)
        : id(id),
          title(title),
          time_limit(time_limit),
          mem_limit(mem_limit),
          description(description),
          sample_input(sample_input),
          sample_output(sample_output),
          created_time(created_time),
          tc_path(tc_path) {};
    ~Problem() override = default;
};
