#pragma once

#include <mariadb/conncpp.hpp>

#include <memory>
#include <optional>
#include <string>

class UserService
{
public:
    explicit UserService(sql::Connection &con) : con(con), um(con) {}

    std::optional<User> getById(const int id)
    {
        return um.findById(id);
    }

    std::optional<int> create(const User &user)
    {
        if (!um.insert(user))
            return std::nullopt;
        return lastInsertId();
    }

    bool update(const User &user)
    {
        return um.update(user);
    }

    bool remove(const int id)
    {
        return um.remove(id);
    }

    std::optional<std::vector<User>> listAll(int limit = 50, int offset = 0)
    {
        return um.listAll(limit, offset);
    }

    
    bool checkrep(const std::string &username)
    {
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "SELECT id,username FROM users WHERE username = ?"));
            stmtPtr->setString(1, username);
            auto res = std::unique_ptr<sql::ResultSet>(stmtPtr->executeQuery());
            if (res->next())
                return false;
            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in checkrep: " << e.what() << std::endl;
            return false;
        }
    }

private:
    sql::Connection &con;
    UserMapper um;

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
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in lastInsertId: " << e.what() << std::endl;
            return std::nullopt;
        }
    }

};