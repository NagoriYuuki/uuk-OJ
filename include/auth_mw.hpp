#pragma once

#include <crow.h>
#include <mariadb/conncpp.hpp>
#include <optional>
#include <string>
#include <iostream>
#include <memory>

class AuthMiddleware
{
public:
    struct context
    {
        std::optional<int> userid = std::nullopt;
        int role = 0;
    };

    AuthMiddleware() = default;
    explicit AuthMiddleware(sql::Connection &con) : con(con) {}
    ~AuthMiddleware() = default;

    void before_handle(crow::request &req, crow::response &res, context &ctx)
    {
        ctx.userid = std::nullopt;
        ctx.role = 0;

        const std::string auth = req.get_header_value("Authorization");
        if (static_cast<int>(auth.size()) < 7 || auth.substr(0, 7) != "Bearer ")
            return;

        const std::string token = auth.substr(7);
        if (token.empty())
            return;
        auto verify = getUserByToken(token);
        if (!verify.has_value())
            return;
        ctx.userid = verify->first;
        ctx.role = verify->second;
    }

    void after_handle(crow::request &, crow::response &, context &) {}

private:
    sql::Connection &con;

    std::optional<std::pair<int, int>> getUserByToken(const std::string &token)
    {
        try
        {
            auto stmt = std::unique_ptr<sql::PreparedStatement>(con.prepareStatement(
                "SELECT user_id, role FROM sessions WHERE token = ?"));
            stmt->setString(1, token);
            auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery());
            if (!res->next())
                return std::nullopt;
            return std::make_pair(res->getInt("user_id"), res->getInt("role"));
        }
        catch (const sql::SQLException &e)
        {
            std::cerr << "SQL Error in getUserByToken: " << e.what() << std::endl;
        }
        return std::nullopt;
    }
};
