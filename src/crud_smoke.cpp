#include <mariadb/conncpp.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include <unistd.h>

#include "db.hpp"
#include "problem_mapper.hpp"
#include "submission_mapper.hpp"
#include "user_mapper.hpp"

namespace
{

    std::optional<int> findUserIdByUsername(sql::Connection &con, const std::string &username)
    {
        auto stmt = std::unique_ptr<sql::PreparedStatement>(
            con.prepareStatement("SELECT id FROM users WHERE username = ?"));
        stmt->setString(1, username);
        auto rs = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
        if (!rs->next())
            return std::nullopt;
        return rs->getInt("id");
    }

    std::optional<int> findProblemIdByTitle(sql::Connection &con, const std::string &title)
    {
        auto stmt = std::unique_ptr<sql::PreparedStatement>(
            con.prepareStatement("SELECT id FROM problems WHERE title = ?"));
        stmt->setString(1, title);
        auto rs = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
        if (!rs->next())
            return std::nullopt;
        return rs->getInt("id");
    }

    bool expect(bool cond, const char *msg)
    {
        if (!cond)
        {
            std::cerr << "[FAIL] " << msg << "\n";
            return false;
        }
        std::cout << "[OK] " << msg << "\n";
        return true;
    }

} // namespace

int main()
{
    try
    {
        auto &db = DB::instance();
        auto &con = db.getConnection();

        bool ok = true;

        // ----------------------
        // UserMapper CRUD
        // ----------------------
        {
            std::cout << "\n== UserMapper CRUD ==\n";
            UserMapper userMapper(con);

            const std::string username = "smoke_user_" + std::to_string(::getpid());
            const std::string passwordHash = "hash";

            User userToInsert(0, username, passwordHash);
            ok &= expect(userMapper.insert(userToInsert), "insert user");

            const auto userIdOpt = findUserIdByUsername(con, username);
            ok &= expect(userIdOpt.has_value(), "find inserted user id by username");
            if (userIdOpt)
            {
                const int userId = *userIdOpt;
                const auto userOpt = userMapper.findById(userId);
                ok &= expect(userOpt.has_value(), "findById user");
                if (userOpt)
                {
                    ok &= expect(userOpt->username == username, "username matches");
                    ok &= expect(userOpt->password_hash == passwordHash, "password_hash matches");
                }

                User userToUpdate(userId, username + "_u", passwordHash + "_u");
                ok &= expect(userMapper.update(userToUpdate), "update user");

                const auto userOpt2 = userMapper.findById(userId);
                ok &= expect(userOpt2.has_value(), "findById updated user");
                if (userOpt2)
                {
                    ok &= expect(userOpt2->username == userToUpdate.username, "updated username matches");
                    ok &= expect(userOpt2->password_hash == userToUpdate.password_hash, "updated password_hash matches");
                }

                ok &= expect(userMapper.remove(userId), "remove user");
                ok &= expect(!userMapper.findById(userId).has_value(), "removed user not found");
            }
        }

        // ----------------------
        // ProblemMapper CRUD
        // ----------------------
        {
            std::cout << "\n== ProblemMapper CRUD ==\n";
            std::cout << "Note: this will FAIL until your SQL schema matches mapper fields.\n";
            std::cout << "- init.sql uses column create_time, but mapper selects created_time\n";

            ProblemMapper problemMapper(con);
            const std::string title = "smoke_problem_" + std::to_string(::getpid());

            Problem problemToInsert(
                0,
                title,
                1000,
                65536,
                "desc",
                "1 2",
                "3",
                "", // created_time handled by DB
                "/tmp/tc");

            ok &= expect(problemMapper.insert(problemToInsert), "insert problem");

            const auto problemIdOpt = findProblemIdByTitle(con, title);
            ok &= expect(problemIdOpt.has_value(), "find inserted problem id by title");
            if (problemIdOpt)
            {
                const int problemId = *problemIdOpt;
                const auto pOpt = problemMapper.findById(problemId);
                ok &= expect(pOpt.has_value(), "findById problem");

                Problem problemToUpdate = problemToInsert;
                problemToUpdate.id = problemId;
                problemToUpdate.title = title + "_u";
                ok &= expect(problemMapper.update(problemToUpdate), "update problem");

                ok &= expect(problemMapper.remove(problemId), "remove problem");
                ok &= expect(!problemMapper.findById(problemId).has_value(), "removed problem not found");
            }
        }

        // ----------------------
        // SubmissionMapper CRUD
        // ----------------------
        {
            std::cout << "\n== SubmissionMapper CRUD ==\n";
            std::cout << "Note: submit_time uses DB DEFAULT when empty.\n";

            SubmissionMapper submissionMapper(con);

            Submission s(
                0,
                1,
                1,
                "cpp",
                "/tmp/code.cpp",
                "Pending",
                "",
                "", // submit_time handled by DB
                0,
                0);

            ok &= expect(submissionMapper.insert(s), "insert submission");
            // Without a stable unique key, we only validate that insert doesn't throw.
        }

        return ok ? 0 : 2;
    }
    catch (const sql::SQLException &e)
    {
        std::cerr << "[SQL EXCEPTION] " << e.what() << "\n";
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[EXCEPTION] " << e.what() << "\n";
        return 1;
    }
}
