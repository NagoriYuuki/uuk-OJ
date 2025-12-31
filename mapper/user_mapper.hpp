#pragma once
#include "base_mapper.hpp"
#include "../entities/user.hpp"
#include "../include/db.hpp"
#include <memory>
#include <string>
#include <optional>

class UserMapper : public BaseMapper<User>
{
public:
    explicit UserMapper(sql::Connection &con) : BaseMapper<User>(con) {}
    bool insert(const User &user) override
    {
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "INSERT INTO users (username, password_hash) VALUES (?, ?)"));
            stmtPtr->setString(1, user.username);
            stmtPtr->setString(2, user.password_hash);
            stmtPtr->executeUpdate();
            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in insert User: " << e.what() << std::endl;
            return false;
        }
    }

    bool update(const User &user) override
    {
        try
        {
            auto stmPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "UPDATE users SET username = ?, password_hash = ? WHERE id = ?"));
            stmPtr->setString(1, user.username);
            stmPtr->setString(2, user.password_hash);
            stmPtr->setInt(3, user.id);
            stmPtr->executeUpdate();
            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in update User: " << e.what() << std::endl;
            return false;
        }
    }

    bool remove(const int id) override
    {
        try
        {
            auto stmPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "DELETE FROM users WHERE id = ?"));
            stmPtr->setInt(1, id);
            stmPtr->executeUpdate();
            return true;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in remove User: " << e.what() << std::endl;
            return false;
        }
    }

    std::optional<User> findById(const int id) override
    {
        try
        {
            auto stmtPtr = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "SELECT id, username, password_hash FROM users WHERE id = ?"));
            stmtPtr->setInt(1, id);
            auto rs = std::unique_ptr<sql::ResultSet>(stmtPtr->executeQuery());
            if (!rs->next())
            {
                return std::nullopt;
            }

            User user;
            user.id = rs->getInt("id");
            user.username = rs->getString("username");
            user.password_hash = rs->getString("password_hash");
            return user;
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in findById User: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
};