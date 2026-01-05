#include <crow.h>
#include <bits/stdc++.h>
#include <mariadb/conncpp.hpp>

#include "db.hpp"
#include "../services/problem_service.hpp"
#include "../entities/problem.hpp"
#include "../services/auth_service.hpp"
#include "../include/auth_mw.hpp"
#include "../services/user_service.hpp"

using i64 = long long;

signed main(void)
{
    auto &con = DB::instance().getConnection();

    crow::App<AuthMiddleware> app{AuthMiddleware{con}};

    CROW_ROUTE(app, "/")
    ([]()
     { return "Hello, uuk-OJ!"; });

    CROW_ROUTE(app, "/api/auth/login")
        .methods(crow::HTTPMethod::POST)(
            [&](const crow::request &req)
            {
                auto json = crow::json::load(req.body);
                if (!json || !json.has("username") || !json.has("password"))
                    return crow::response(400, "No Username or Password nya~");
                AuthService auths(con);
                User user;
                user.username = json["username"].s();
                user.password_hash = json["password"].s();
                auto token = auths.login(user);
                if (!token.has_value())
                    return crow::response(401, "Invalid Username or Password nya~");
                auto res = crow::json::wvalue();
                res["token"] = token.value();
                return crow::response(200, res);
            });
    CROW_ROUTE(app, "/api/auth/register")
        .methods(crow::HTTPMethod::POST)(
            [&](const crow::request &req)
            {
                auto json = crow::json::load(req.body);
                if (!json || !json.has("username") || !json.has("password"))
                    return crow::response(400, "No Username or Password nya~");
                auto username = json["username"].s();
                auto password = json["password"].s();
                for (auto &i : username)
                    if (!std::isalnum(i) && i != '_' && i != '-')
                        return crow::response(400, "Invalid characters in username nya~");
                if (password.size() < 6)
                    return crow::response(400, "Password too short nya~");
                User user;
                user.username = username;
                user.password_hash = password;
                AuthService auths(con);
                if (!auths.registerUser(user))
                    return crow::response(409, "Username already exists nya~");
                return crow::response(200, "User registered successfully nya~");
            });

    CROW_ROUTE(app, "/api/problems")([&]()
                                     {
        ProblemService ps(con);
        auto vec = ps.listAll(50, 0);
        if(!vec.has_value())
            return crow::response(500, "No problems found nya~");
        crow::json::wvalue res = crow::json::wvalue::list();
        std::size_t idx = 0;
        for (const auto &i : vec.value())
        {
            crow::json::wvalue item;
            item["id"] = i.id;
            item["title"] = i.title;
            item["time_limit"] = i.time_limit;
            item["mem_limit"] = i.mem_limit;
            item["ac_count"] = i.ac_count;
            item["sub_count"] = i.sub_count;
            res[idx++] = std::move(item);
        }
        return crow::response(200, res); });

    CROW_ROUTE(app, "/api/problems/<int>")
    ([&](int problem_id)
     {
        ProblemService ps(con);
        auto problem = ps.getById(problem_id);
        if (!problem.has_value())
            return crow::response(404, "Problem not found nya~");
        crow::json::wvalue item;
        item["id"] = problem->id;
        item["title"] = problem->title;
        item["time_limit"] = problem->time_limit;
        item["mem_limit"] = problem->mem_limit;
        item["description"] = problem->description;
        item["sample_input"] = problem->sample_input;
        item["sample_output"] = problem->sample_output;
        item["created_time"] = problem->created_time;
        item["tc_path"] = problem->tc_path;
        item["sub_count"] = problem->sub_count;
        item["ac_count"] = problem->ac_count;
        return crow::response(200, item); });

    app.port(18080)
        .run();

    return 0;
}