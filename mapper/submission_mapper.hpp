#pragma once
#include "base_mapper.hpp"
#include "../entities/submission.hpp"
#include "../include/db.hpp"

#include <memory>
#include <vector>
#include <set>

class SubmissionMapper : public BaseMapper<Submission>
{
public:
    explicit SubmissionMapper(sql::Connection &con) : BaseMapper<Submission>(con) {}

    bool insert(const Submission &submission) override
    {
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "INSERT INTO submissions (problem_id, user_id, language, code, status, detail, time_cost, mem_cost) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
            stmtPtr->setInt(1, submission.problem_id);
            stmtPtr->setInt(2, submission.user_id);
            stmtPtr->setString(3, submission.language);
            stmtPtr->setString(4, submission.code);
            stmtPtr->setString(5, submission.status);
            stmtPtr->setString(6, submission.detail);
            stmtPtr->setInt(7, submission.time_cost);
            stmtPtr->setInt(8, submission.mem_cost);
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
                "UPDATE submissions SET problem_id = ?, user_id = ?, language = ?, code = ?, status = ?, detail = ?, time_cost = ?, mem_cost = ? WHERE id = ?"));
            stmPtr->setInt(1, submission.problem_id);
            stmPtr->setInt(2, submission.user_id);
            stmPtr->setString(3, submission.language);
            stmPtr->setString(4, submission.code);
            stmPtr->setString(5, submission.status);
            stmPtr->setString(6, submission.detail);
            stmPtr->setInt(7, submission.time_cost);
            stmPtr->setInt(8, submission.mem_cost);
            stmPtr->setInt(9, submission.id);
            stmPtr->executeUpdate();
            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in update Submission: " << e.what() << std::endl;
            return false;
        }
    }

    bool updateStatus(const Submission &sub)
    {
        try
        {
            auto stmPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "UPDATE submissions SET status = ?, detail = ?, time_cost = ?, mem_cost = ? WHERE id = ?"));
            stmPtr->setString(1, sub.status);
            stmPtr->setString(2, sub.detail);
            stmPtr->setInt(3, sub.time_cost);
            stmPtr->setInt(4, sub.mem_cost);
            stmPtr->setInt(5, sub.id);
            stmPtr->executeUpdate();
            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in updateStatus Submission: " << e.what() << std::endl;
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
                "SELECT id, problem_id, user_id, language, code, status, detail, submit_time, time_cost, mem_cost FROM submissions WHERE id = ?"));
            stmtPtr->setInt(1, id);
            auto resPtr = std::unique_ptr<sql::ResultSet>(stmtPtr->executeQuery());
            if (resPtr->next())
            {
                Submission submission(
                    resPtr->getInt64("id"),
                    resPtr->getInt("problem_id"),
                    resPtr->getInt("user_id"),
                    std::string(resPtr->getString("language").c_str()),
                    std::string(resPtr->getString("code").c_str()),
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

    std::optional<std::vector<Submission>> listAll(int limit = 50, int offset = 0)
    {
        std::vector<Submission> vec;
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "SELECT id, problem_id, user_id, language, code, status, detail, submit_time, time_cost, mem_cost FROM submissions ORDER BY id DESC LIMIT ? OFFSET ?"));
            stmtPtr->setInt(1, limit);
            stmtPtr->setInt(2, offset);
            auto resPtr = std::unique_ptr<sql::ResultSet>(stmtPtr->executeQuery());
            while (resPtr->next())
            {
                Submission submission(
                    resPtr->getInt64("id"),
                    resPtr->getInt("problem_id"),
                    resPtr->getInt("user_id"),
                    std::string(resPtr->getString("language").c_str()),
                    std::string(resPtr->getString("code").c_str()),
                    std::string(resPtr->getString("status").c_str()),
                    std::string(resPtr->getString("detail").c_str()),
                    std::string(resPtr->getString("submit_time").c_str()),
                    resPtr->getInt("time_cost"),
                    resPtr->getInt("mem_cost"));
                vec.push_back(submission);
            }
            return vec;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in listAll Submission: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::optional<std::vector<Submission>> findByProblemId(const int problem_id, int limit = 50, int offset = 0)
    {
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "SELECT id, problem_id, user_id, language, code, status, detail, submit_time, time_cost, mem_cost FROM submissions WHERE problem_id = ? ORDER BY id DESC LIMIT ? OFFSET ?"));
            stmtPtr->setInt(1, problem_id);
            stmtPtr->setInt(2, limit);
            stmtPtr->setInt(3, offset);
            std::vector<Submission> vec;
            auto resPtr = std::unique_ptr<sql::ResultSet>(stmtPtr->executeQuery());
            while (resPtr->next())
            {
                Submission submission(
                    resPtr->getInt64("id"),
                    resPtr->getInt("problem_id"),
                    resPtr->getInt("user_id"),
                    std::string(resPtr->getString("language").c_str()),
                    std::string(resPtr->getString("code").c_str()),
                    std::string(resPtr->getString("status").c_str()),
                    std::string(resPtr->getString("detail").c_str()),
                    std::string(resPtr->getString("submit_time").c_str()),
                    resPtr->getInt("time_cost"),
                    resPtr->getInt("mem_cost"));
                vec.push_back(submission);
            }
            return vec;
        }
        catch (const sql::SQLException &e)
        {
            std::cerr << "SQL Error in findByProblemId Submission: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::optional<std::pair<std::vector<Submission>, std::vector<int>>> findByUserId(const int user_id)
    {
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "SELECT id, problem_id, user_id, language, code, status, detail, submit_time, time_cost, mem_cost FROM submissions WHERE user_id = ? ORDER BY id DESC"));
            stmtPtr->setInt(1, user_id);
            std::vector<Submission> vec;
            std::vector<int> arr;

            auto resPtr = std::unique_ptr<sql::ResultSet>(stmtPtr->executeQuery());
            while (resPtr->next())
            {
                Submission submission(
                    resPtr->getInt64("id"),
                    resPtr->getInt("problem_id"),
                    resPtr->getInt("user_id"),
                    std::string(resPtr->getString("language").c_str()),
                    std::string(resPtr->getString("code").c_str()),
                    std::string(resPtr->getString("status").c_str()),
                    std::string(resPtr->getString("detail").c_str()),
                    std::string(resPtr->getString("submit_time").c_str()),
                    resPtr->getInt("time_cost"),
                    resPtr->getInt("mem_cost"));
                vec.push_back(submission);
                if (submission.status == "Accepted")
                    arr.push_back(resPtr->getInt("problem_id"));
            }
            std::set<int> st(arr.begin(), arr.end());
            arr.assign(st.begin(), st.end());
            return std::make_pair(vec, arr);
        }
        catch (const sql::SQLException &e)
        {
            std::cerr << "SQL Error in findByUserId Submission: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

    std::optional<std::vector<Submission>> findByProblemIdAndUserId(const int problem_id, const int user_id, int limit = 50, int offset = 0)
    {
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "SELECT id, problem_id, user_id, language, code, status, detail, submit_time, time_cost, mem_cost FROM submissions WHERE problem_id = ? AND user_id = ? ORDER BY id DESC LIMIT ? OFFSET ?"));
            stmtPtr->setInt(1, problem_id);
            stmtPtr->setInt(2, user_id);
            stmtPtr->setInt(3, limit);
            stmtPtr->setInt(4, offset);
            std::vector<Submission> vec;
            auto resPtr = std::unique_ptr<sql::ResultSet>(stmtPtr->executeQuery());
            while (resPtr->next())
            {
                Submission submission(
                    resPtr->getInt64("id"),
                    resPtr->getInt("problem_id"),
                    resPtr->getInt("user_id"),
                    std::string(resPtr->getString("language").c_str()),
                    std::string(resPtr->getString("code").c_str()),
                    std::string(resPtr->getString("status").c_str()),
                    std::string(resPtr->getString("detail").c_str()),
                    std::string(resPtr->getString("submit_time").c_str()),
                    resPtr->getInt("time_cost"),
                    resPtr->getInt("mem_cost"));
                vec.push_back(submission);
            }
            return vec;
        }
        catch (const sql::SQLException &e)
        {
            std::cerr << "SQL Error in findByProblemIdAndUserId Submission: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
};