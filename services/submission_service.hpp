#pragma once

#include "../mapper/submission_mapper.hpp"

#include <memory>
#include <optional>
#include <vector>

class SubmissionService
{
public:
    explicit SubmissionService(sql::Connection &con)
        : con(con), sm(con) {}

    std::optional<Submission> getById(const int id)
    {
        return sm.findById(id);
    }

    std::optional<int> create(const Submission &submission)
    {
        if (!sm.insert(submission))
            return std::nullopt;
        return lastInsertId();
    }

    bool update(const Submission &submission)
    {
        return sm.update(submission);
    }

    bool updateStatus(const Submission&sub)
    {
        return sm.updateStatus(sub);
    }

    bool remove(const int id)
    {
        return sm.remove(id);
    }

    std::optional<std::vector<Submission>> listAll(int limit = 50, int offset = 0)
    {
        return sm.listAll(limit, offset);
    }

    std::optional<std::vector<Submission>> findByProblemId(const int problem_id, int limit = 50, int offset = 0)
    {
        return sm.findByProblemId(problem_id, limit, offset);
    }

private:
    sql::Connection &con;
    SubmissionMapper sm;

    std::optional<int> lastInsertId()
    {
        try
        {
            auto stmt = std::unique_ptr<sql::Statement>(con.createStatement());
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
            if (!res->next())
                return std::nullopt;
            return res->getInt("id");
        }
        catch (const sql::SQLException &e)
        {
            std::cerr << "SQL Error in lastInsertId: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
};