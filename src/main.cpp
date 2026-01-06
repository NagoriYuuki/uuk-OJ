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
                crow::json::wvalue res;
                if (!json || !json.has("username") || !json.has("password"))
                {
                    res["code"] = 400;
                    res["message"] = "No Username or Password nya~";
                    return crow::response(400, res);
                }
                AuthService auths(con);
                User user;
                user.username = json["username"].s();
                user.password_hash = json["password"].s();
                auto token = auths.login(user);
                if (!token.has_value())
                {
                    res["code"] = 401;
                    res["message"] = "Invalid Username or Password nya~";
                    return crow::response(401, res);
                }
                res["token"] = token.value();
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/auth/register")
        .methods(crow::HTTPMethod::POST)(
            [&](const crow::request &req)
            {
                auto json = crow::json::load(req.body);
                crow::json::wvalue res;
                if (!json || !json.has("username") || !json.has("password"))
                {
                    res["code"] = 400;
                    res["message"] = "No Username or Password nya~";
                    return crow::response(400, res);
                }
                auto username = json["username"].s();
                auto password = json["password"].s();
                for (auto &i : username)
                    if (!std::isalnum(i) && i != '_' && i != '-')
                    {
                        res["code"] = 400;
                        res["message"] = "Invalid characters in username nya~";
                        return crow::response(400, res);
                    }
                if (password.size() < 6)
                {
                    res["code"] = 400;
                    res["message"] = "Password too short nya~";
                    return crow::response(400, res);
                }
                User user;
                user.username = username;
                user.password_hash = password;
                AuthService auths(con);
                if (!auths.registerUser(user))
                {
                    res["code"] = 409;
                    res["message"] = "Username already exists nya~";
                    return crow::response(409, res);
                }
                res["code"] = 200;
                res["message"] = "Register successful nya~";
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/auth/userinfo")
        .methods(crow::HTTPMethod::GET)(
            [&](const crow::request &req)
            {
                auto &ctx = app.get_context<AuthMiddleware>(req);
                crow::json::wvalue res;
                if (!ctx.userid.has_value())
                {
                    res["code"] = 401;
                    res["message"] = "Unauthorized nya~";
                    return crow::response(401, res);
                }
                res["id"] = ctx.userid.value();
                res["role"] = ctx.role;
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/problems")
        .methods(crow::HTTPMethod::GET)(
            [&]()
            {
                ProblemService ps(con);
                auto vec = ps.listAll(50, 0);

                if (!vec.has_value())
                {
                    crow::json::wvalue res;
                    res["code"] = 500;
                    res["message"] = "No problems found nya~";
                    return crow::response(500, res);
                }
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
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/problems/<int>")
        .methods(crow::HTTPMethod::GET)(
            [&](int problem_id)
            {
                ProblemService ps(con);
                auto problem = ps.getById(problem_id);
                crow::json::wvalue item;
                if (!problem.has_value())
                {
                    item["code"] = 404;
                    item["message"] = "Problem not found nya~";
                    return crow::response(404, item);
                }
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
                return crow::response(200, item);
            });

    CROW_ROUTE(app, "/api/admin/problems")
        .methods(crow::HTTPMethod::POST)(
            [&](const crow::request &req)
            {
                auto &ctx = app.get_context<AuthMiddleware>(req);
                crow::json::wvalue res;
                if (!ctx.userid.has_value() || ctx.role != 1)
                {
                    res["code"] = 403;
                    res["message"] = "Admin access required nya~";
                    return crow::response(403, res);
                }
                auto json = crow::json::load(req.body);
                if (!json)
                {
                    res["code"] = 400;
                    res["message"] = "Invalid JSON nya~";
                    return crow::response(400, res);
                }
                if (!json.has("title"))
                {
                    res["code"] = 400;
                    res["message"] = "Title is required nya~";
                    return crow::response(400, res);
                }
                Problem problem;
                problem.title = std::string(json["title"].s());
                problem.time_limit = json.has("time_limit") ? json["time_limit"].i() : 1000;
                problem.mem_limit = json.has("mem_limit") ? json["mem_limit"].i() : 65536;
                problem.description = json.has("description") ? std::string(json["description"].s()) : std::string{};
                problem.sample_input = json.has("sample_input") ? std::string(json["sample_input"].s()) : std::string{};
                problem.sample_output = json.has("sample_output") ? std::string(json["sample_output"].s()) : std::string{};
                problem.tc_path = json.has("tc_path") ? std::string(json["tc_path"].s()) : std::string{};
                ProblemService ps(con);
                auto new_id = ps.create(problem);
                if (!new_id.has_value())
                {
                    res["code"] = 500;
                    res["message"] = "Failed to create problem nya~";
                    return crow::response(500, res);
                }
                res["new_id"] = new_id.value();
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/admin/problems/<int>")
        .methods(crow::HTTPMethod::PUT)(
            [&](const crow::request &req, int id)
            {
                crow::json::wvalue res;
                auto &ctx = app.get_context<AuthMiddleware>(req);
                if (!ctx.userid.has_value() || ctx.role != 1)
                {
                    res["code"] = 403;
                    res["message"] = "Admin access required nya~";
                    return crow::response(403, res);
                }
                auto json = crow::json::load(req.body);
                if (!json)
                {
                    res["code"] = 400;
                    res["message"] = "Invalid JSON nya~";
                    return crow::response(400, res);
                }
                Problem problem;
                problem.id = id;
                problem.title = std::string(json["title"].s());
                problem.time_limit = json.has("time_limit") ? json["time_limit"].i() : 1000;
                problem.mem_limit = json.has("mem_limit") ? json["mem_limit"].i() : 65536;
                problem.description = json.has("description") ? std::string(json["description"].s()) : std::string{};
                problem.sample_input = json.has("sample_input") ? std::string(json["sample_input"].s()) : std::string{};
                problem.sample_output = json.has("sample_output") ? std::string(json["sample_output"].s()) : std::string{};
                problem.tc_path = json.has("tc_path") ? std::string(json["tc_path"].s()) : std::string{};
                ProblemService ps(con);
                ps.update(problem);
                res["code"] = 200;
                res["message"] = "Problem updated successfully nya~";
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/admin/problems/<int>")
        .methods(crow::HTTPMethod::DELETE)(
            [&](const crow::request &req, int problem_id)
            {
                crow::json::wvalue res;
                auto &ctx = app.get_context<AuthMiddleware>(req);
                if (!ctx.userid.has_value() || ctx.role != 1)
                {
                    res["code"] = 403;
                    res["message"] = "Admin access required nya~";
                    return crow::response(403, res);
                }
                ProblemService ps(con);
                if (!ps.getById(problem_id).has_value())
                {
                    res["code"] = 404;
                    res["message"] = "Problem not found nya~";
                    return crow::response(404, res);
                }
                if (!ps.remove(problem_id))
                {
                    res["code"] = 500;
                    res["message"] = "Failed to delete problem nya~";
                    return crow::response(500, res);
                }
                res["code"] = 200;
                res["message"] = "Problem deleted successfully nya~";
                return crow::response(200, res);
            });

    app.port(18080)
        .run();

    return 0;
}