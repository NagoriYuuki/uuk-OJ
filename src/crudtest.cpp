#include <mariadb/conncpp.hpp>

#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "db.hpp"
#include "../mapper/user_mapper.hpp"
#include "../entities/user.hpp"
#include "../mapper/problem_mapper.hpp"
#include "../entities/problem.hpp"
#include "../mapper/submission_mapper.hpp"
#include "../entities/submission.hpp"

void checkUser(sql::Connection &con)
{
    try
    {
        User user = User(0, "testuser2", "testhash");
        UserMapper um(con);

        auto checkreset = [&]()
        {
            std::string resetsql = "TRUNCATE TABLE users;";
            auto stmt = std::unique_ptr<sql::Statement>(con.createStatement());
            stmt->execute(resetsql);
        };

        // checkreset();

        if (um.insert(user))
        {
            std::cerr << "User inserted successfully." << std::endl;
        }
        else
        {
            std::cerr << "Failed to insert user." << std::endl;
        }
        auto res = um.findById(1);
        if (res)
        {
            std::cerr << "User found: " << res->username << std::endl;
        }
        else
        {
            std::cerr << "Failed to find user." << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void checkProblem(sql::Connection &con)
{
    try
    {
        Problem problem = Problem(0, "Sample Problem", 1000, 65536, "Description", "Input", "Output", "", "/path/to/tc");
        ProblemMapper pm(con);

        auto checkreset = [&]()
        {
            std::string resetsql = "TRUNCATE TABLE problems;";
            auto stmt = std::unique_ptr<sql::Statement>(con.createStatement());
            stmt->execute(resetsql);
        };

        // checkreset();

        if (pm.insert(problem))
        {
            std::cerr << "Problem inserted successfully." << std::endl;
        }
        else
        {
            std::cerr << "Failed to insert problem." << std::endl;
        }
        auto res = pm.findById(1);
        if (res)
        {
            std::cerr << "Problem found: " << res->id << " " << res->title << " " << res->created_time << std::endl;
        }
        else
        {
            std::cerr << "Failed to find problem." << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void checkSubmission(sql::Connection &con)
{
    try
    {
        Submission submission = Submission(0, 1, 1, "C++", "code", "Pending", "Detail", "", 0, 0);
        SubmissionMapper sm(con);

        auto checkreset = [&]()
        {
            std::string resetsql = "TRUNCATE TABLE submissions;";
            auto stmt = std::unique_ptr<sql::Statement>(con.createStatement());
            stmt->execute(resetsql);
        };

        // checkreset();

        if (sm.insert(submission))
        {
            std::cerr << "Submission inserted successfully." << std::endl;
        }
        else
        {
            std::cerr << "Failed to insert submission." << std::endl;
        }
        auto res = sm.findById(1);
        if (res)
        {
            std::cerr << "Submission found: " << res->id << " " << res->language << " " << res->submit_time << std::endl;
        }
        else
        {
            std::cerr << "Failed to find submission." << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

signed main(void)
{
    auto &con = DB::instance().getConnection();
    checkUser(con);
    checkProblem(con);
    checkSubmission(con);

    return 0;
}