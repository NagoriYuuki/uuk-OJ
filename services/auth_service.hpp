#pragma once

#include <mariadb/conncpp.hpp>

#include <optional>
#include <string>
#include <iostream>
#include <memory>
#include <random>

#include <../entities/user.hpp>

class AuthService
{
public:
    explicit AuthService(sql::Connection &con) : con(con) {}

    std::optional<std::string> login(User &user)
    {
        try
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "SELECT id, password_hash FROM users WHERE username = ?"));
            stmt->setString(1, user.username);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (!res->next())
                return std::nullopt;
            std::string db_pw = res->getString("password_hash").c_str();
            if (db_pw != user.password_hash)
                return std::nullopt;
            std::string token = TokenGenerator();
            int user_id = res->getInt("id");
            stmt = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "INSERT INTO sessions (user_id, token) VALUES (?, ?)"));
            stmt->setInt(1, user_id);
            stmt->setString(2, token);
            stmt->executeUpdate();
            return token;
        }
        catch (const sql::SQLException &e)
        {
            std::cerr << "SQL Error in login: " << e.what() << std::endl;
        }
        return std::nullopt;
    }

private:
    sql::Connection &con;
    std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    constexpr static int token_len = 64;
    std::mt19937 rng{std::random_device{}()};
    std::string TokenGenerator()
    {
        std::string token;
        token.reserve(token_len);
        std::uniform_int_distribution<int> dist(0, static_cast<int>(charset.size()) - 1);
        for (int i = 0; i < token_len; i++)
            token += charset[dist(rng)];
        return token;
    }
};