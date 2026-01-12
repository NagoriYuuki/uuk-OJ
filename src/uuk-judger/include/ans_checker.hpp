#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

class AnsChecker
{
public:
    AnsChecker() = default;
    ~AnsChecker() = default;

    static bool check(const std::string &res, const std::string &std)
    {
        std::ifstream res_file(res);
        std::ifstream std_file(std);

        std::string line_temp;

        std::string res_content;
        std::string std_content;

        while (std::getline(res_file, line_temp))
        {
            line_temp.erase(line_temp.find_last_not_of(" \n\r\t") + 1);
            // if (!line_temp.empty())
            res_content += line_temp + "\n";
        }
        if (!res_content.empty() && res_content.back() == '\n')
            res_content.pop_back();

        while (std::getline(std_file, line_temp))
        {
            line_temp.erase(line_temp.find_last_not_of(" \n\r\t") + 1);
            // if (!line_temp.empty())
            std_content += line_temp + "\n";
        }
        if (!std_content.empty() && std_content.back() == '\n')
            std_content.pop_back();

        return res_content == std_content;
    }
};