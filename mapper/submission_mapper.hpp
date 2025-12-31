#pragma once
#include "base_mapper.hpp"
#include "../entities/submission.hpp"
#include "../include/db.hpp"

#include <memory>

class SubmissionMapper : public BaseMapper<Submission>
{
public:
    explicit SubmissionMapper(sql::Connection &con) : BaseMapper<Submission>(con) {}

    bool insert(const Submission &submission) override
    {
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "INSERT INTO submissions (problem_id, user_id, language, code_path, status, detail, submit_time, time_cost, mem_cost) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"));
            stmtPtr->setInt(1, submission.problem_id);
            stmtPtr->setInt(2, submission.user_id);
            stmtPtr->setString(3, submission.language);
            stmtPtr->setString(4, submission.code_path);
            stmtPtr->setString(5, submission.status);
            stmtPtr->setString(6, submission.detail);
            stmtPtr->setString(7, submission.submit_time);
            stmtPtr->setInt(8, submission.time_cost);
            stmtPtr->setInt(9, submission.mem_cost);
            stmtPtr->executeUpdate();
            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in insert Submission: " << e.what() << std::endl;
            return false;
        }
    }
    bool update(const Submission &submission) override
    {
        try
        {
            auto stmPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "UPDATE submissions SET problem_id = ?, user_id = ?, language = ?, code_path = ?, status = ?, detail = ?, submit_time = ?, time_cost = ?, mem_cost = ? WHERE id = ?"));
            stmPtr->setInt(1, submission.problem_id);
            stmPtr->setInt(2, submission.user_id);
            stmPtr->setString(3, submission.language);
            stmPtr->setString(4, submission.code_path);
            stmPtr->setString(5, submission.status);
            stmPtr->setString(6, submission.detail);
            stmPtr->setString(7, submission.submit_time);
            stmPtr->setInt(8, submission.time_cost);
            stmPtr->setInt(9, submission.mem_cost);
            stmPtr->setInt(10, submission.id);
            stmPtr->executeUpdate();
            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in update Submission: " << e.what() << std::endl;
            return false;
        }
    }
    bool remove(int id) override
    {
        try
        {
            auto stmPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "DELETE FROM submissions WHERE id = ?"));
            stmPtr->setInt(1, id);
            stmPtr->executeUpdate();
            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in remove Submission: " << e.what() << std::endl;
            return false;
        }
    }

    std::optional<Submission> findById(const int id) override
    {
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "SELECT id, problem_id, user_id, language, code_path, status, detail, submit_time, time_cost, mem_cost FROM submissions WHERE id = ?"));
            stmtPtr->setInt(1, id);
            auto resPtr = std::unique_ptr<sql::ResultSet>(stmtPtr->executeQuery());
            if (resPtr->next())
            {
                Submission submission(
                    resPtr->getInt64("id"),
                    resPtr->getInt("problem_id"),
                    resPtr->getInt("user_id"),
                    std::string(resPtr->getString("language").c_str()),
                    std::string(resPtr->getString("code_path").c_str()),
                    std::string(resPtr->getString("status").c_str()),
                    std::string(resPtr->getString("detail").c_str()),
                    std::string(resPtr->getString("submit_time").c_str()),
                    resPtr->getInt("time_cost"),
                    resPtr->getInt("mem_cost"));
                return submission;
            }
            return std::nullopt;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in findById Submission: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
};