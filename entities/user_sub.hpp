#pragma once

#include <string>
#include <vector>

class UserSub
{
public:

    UserSub() = default;
    UserSub(const int user_id,
            const std::vector<int> &ac_problems,
            const std::vector<int> &submitted_problems)
        : user_id(user_id),
          ac_problems(ac_problems),
          submitted_problems(submitted_problems) {}
    ~UserSub() = default;


    int user_id;
    std::vector<int> ac_problems;
    std::vector<int> submitted_problems;

};
