#pragma once
#include "base_mapper.hpp"
#include "../entities/problem.hpp"
#include "../include/db.hpp"

#include <memory>
#include <vector>

using i64 = long long;

class ProblemMapper : public BaseMapper<Problem>
{
public:
    explicit ProblemMapper(sql::Connection &con) : BaseMapper<Problem>(con) {}

    bool insert(const Problem &problem) override
    {
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(
                con.prepareStatement(
                    "INSERT INTO problems (title, time_limit, mem_limit, description, sample_input, sample_output, tc_path, sub_count, ac_count) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"));

            stmtPtr->setString(1, problem.title);
            stmtPtr->setInt(2, problem.time_limit);
            stmtPtr->setInt(3, problem.mem_limit);
            stmtPtr->setString(4, problem.description);
            stmtPtr->setString(5, problem.sample_input);
            stmtPtr->setString(6, problem.sample_output);
            stmtPtr->setString(7, problem.tc_path);
            stmtPtr->setInt64(8, problem.sub_count);
            stmtPtr->setInt64(9, problem.ac_count);

            stmtPtr->executeUpdate();

            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in insert Problem: " << e.what() << std::endl;
            return false;
        }
    }
    bool update(const Problem &problem) override
    {
        try
        {
            auto stmPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "UPDATE problems SET (title, time_limit, mem_limit, description, sample_input, sample_output, tc_path, sub_count, ac_count) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?) WHERE id = ?"));
            stmPtr->setString(1, problem.title);
            stmPtr->setInt(2, problem.time_limit);
            stmPtr->setInt(3, problem.mem_limit);
            stmPtr->setString(4, problem.description);
            stmPtr->setString(5, problem.sample_input);
            stmPtr->setString(6, problem.sample_output);
            stmPtr->setString(7, problem.tc_path);
            stmPtr->setInt64(8, problem.sub_count);
            stmPtr->setInt64(9, problem.ac_count);
            stmPtr->setInt(10, problem.id);
            stmPtr->executeUpdate();
            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in update Problem: " << e.what() << std::endl;
            return false;
        }
    }
    bool remove(int id) override
    {
        try
        {
            auto stmPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "DELETE FROM problems WHERE id = ?"));
            stmPtr->setInt(1, id);
            stmPtr->executeUpdate();
            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in remove Problem: " << e.what() << std::endl;
            return false;
        }
    }

    std::optional<Problem> findById(const int id) override
    {
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "SELECT id, title, time_limit, mem_limit, description, sample_input, sample_output, created_time, tc_path, sub_count, ac_count FROM problems WHERE id= ?"));
            stmtPtr->setInt(1, id);
            auto resPtr = std::unique_ptr<sql::ResultSet>(stmtPtr->executeQuery());
            if (!resPtr->next())
                return std::nullopt;
            Problem problem(
                resPtr->getInt("id"),
                std::string(resPtr->getString("title").c_str()),
                resPtr->getInt("time_limit"),
                resPtr->getInt("mem_limit"),
                std::string(resPtr->getString("description").c_str()),
                std::string(resPtr->getString("sample_input").c_str()),
                std::string(resPtr->getString("sample_output").c_str()),
                std::string(resPtr->getString("created_time").c_str()),
                std::string(resPtr->getString("tc_path").c_str()));
            resPtr->getInt64("sub_count");
            resPtr->getInt64("ac_count");
            return problem;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in findById Problem: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::vector<Problem> listAll(int limit = 50, int offset = 0)
    {
        std::vector<Problem> vec;
        try
        {
            auto stmPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement("SELECT id,title,time_limit,mem_limit,description,sample_input,sample_output,created_time,tc_path, sub_count, ac_count FROM problems ORDER BY id DESC LIMIT ? OFFSET ?"));
            stmPtr->setInt(1, limit);
            stmPtr->setInt(2, offset);
            auto resPtr = std::unique_ptr<sql::ResultSet>(stmPtr->executeQuery());
            while (resPtr->next())
            {
                Problem problem(
                    resPtr->getInt("id"),
                    std::string(resPtr->getString("title").c_str()),
                    resPtr->getInt("time_limit"),
                    resPtr->getInt("mem_limit"),
                    std::string(resPtr->getString("description").c_str()),
                    std::string(resPtr->getString("sample_input").c_str()),
                    std::string(resPtr->getString("sample_output").c_str()),
                    std::string(resPtr->getString("created_time").c_str()),
                    std::string(resPtr->getString("tc_path").c_str()));
                resPtr->getInt64("sub_count");
                resPtr->getInt64("ac_count");
                vec.push_back(problem);
            }
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in listAll Problems: " << e.what() << std::endl;
        }
        return vec;
    }

    // std::pair<i64, i64> countSub(int id)
    // {
    //     try
    //     {
    //         auto stmPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
    //             "SELECT COUNT(*) AS total, SUM(CASE WHEN status = 'Accepted' THEN 1 ELSE 0 END) AS accepted FROM submissions WHERE problem_id = ?"));
    //         stmPtr->setInt(1, id);
    //         auto resPtr = std::unique_ptr<sql::ResultSet>(stmPtr->executeQuery());
    //         if (resPtr->next())
    //         {
    //             i64 total = resPtr->getInt64("total");
    //             i64 accepted = resPtr->getInt64("accepted");
    //             return {total, accepted};
    //         }
    //     }
    //     catch (sql::SQLException &e)
    //     {
    //         std::cerr << "SQL Error in countSub Problem: " << e.what() << std::endl;
    //     }
    // }
};