#include <crow.h>
#include <bits/stdc++.h>
#include <mariadb/conncpp.hpp>

#include "db.hpp"
#include "zipcheck.hpp"
#include "db_pool.hpp"

#include "user_mapper.hpp"

#pragma push_macro("DELETE")
#undef DELETE
#include "httplib.h"
#pragma pop_macro("DELETE")

#include "../services/problem_service.hpp"
#include "../entities/problem.hpp"
#include "../services/auth_service.hpp"
#include "../include/auth_mw.hpp"
#include "../services/user_service.hpp"
#include "../services/submission_service.hpp"

using i64 = long long;

signed main(void)
{
    crow::App<AuthMiddleware> app;

    CROW_ROUTE(app, "/")
    ([]()
     { return "Hello, uuk-OJ!"; });

    CROW_ROUTE(app, "/api/auth/login")
        .methods(crow::HTTPMethod::POST)(
            [&](const crow::request &req)
            {
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
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
                std::string pw = json["password"].s();
                std::string pwhash = Encode::gethash(pw);
                user.password_hash = pwhash;
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
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
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
                user.password_hash = Encode::gethash(password);
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
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
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
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
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
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
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
                std::cerr << "asd: " << problem.ac_count << " " << problem.sub_count << std::endl;
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
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
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
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
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

    CROW_ROUTE(app, "/api/admin/problems/<int>/testcases")
        .methods(crow::HTTPMethod::POST)(
            [&](const crow::request &req, int problem_id)
            {
                auto &ctx = app.get_context<AuthMiddleware>(req);
                crow::json::wvalue res;
                if (!ctx.userid.has_value() || ctx.role != 1)
                {
                    res["code"] = 403;
                    res["message"] = "Admin access required nya~";
                    return crow::response(403, res);
                }

                ZipCheck zc;
                auto result = zc.zipcheck(req, problem_id);
                if (!result.judge)
                {
                    res["code"] = 400;
                    res["message"] = result.message;
                    return crow::response(400, res);
                }
                res["code"] = 200;
                res["message"] = "Testcases uploaded successfully nya~";
                res["tc_path"] = result.tc_path;
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/admin/users")
        .methods(crow::HTTPMethod::GET)(
            [&](const crow::request &req)
            {
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
                auto &ctx = app.get_context<AuthMiddleware>(req);
                crow::json::wvalue res;
                if (!ctx.userid.has_value() || ctx.role != 1)
                {
                    res["code"] = 403;
                    res["message"] = "Admin access required nya~";
                    return crow::response(403, res);
                }
                UserService us(con);
                auto vec = us.listAll(100, 0);
                if (!vec.has_value())
                {
                    res["code"] = 500;
                    res["message"] = "No users found nya~";
                    return crow::response(500, res);
                }
                crow::json::wvalue list = crow::json::wvalue::list();
                std::size_t idx = 0;
                for (const auto &u : vec.value())
                {
                    crow::json::wvalue item;
                    item["id"] = u.id;
                    item["username"] = u.username;
                    item["role"] = u.role;
                    list[idx++] = std::move(item);
                }
                return crow::response(200, list);
            });

    CROW_ROUTE(app, "/api/admin/users")
        .methods(crow::HTTPMethod::POST)(
            [&](const crow::request &req)
            {
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
                auto &ctx = app.get_context<AuthMiddleware>(req);
                crow::json::wvalue res;
                if (!ctx.userid.has_value() || ctx.role != 1)
                {
                    res["code"] = 403;
                    res["message"] = "Admin access required nya~";
                    return crow::response(403, res);
                }
                auto json = crow::json::load(req.body);
                if (!json || !json.has("username") || !json.has("password"))
                {
                    res["code"] = 400;
                    res["message"] = "Username and password required nya~";
                    return crow::response(400, res);
                }
                User user;
                user.username = json["username"].s();
                // user.password_hash = json["password"].s();
                auto pw = json["password"].s();
                user.password_hash = Encode::gethash(pw);
                user.role = json.has("role") ? json["role"].i() : 0;

                UserService us(con);
                if (!us.checkrep(user.username))
                {
                    res["code"] = 409;
                    res["message"] = "Username already exists nya~";
                    return crow::response(409, res);
                }
                auto new_id = us.create(user);
                if (!new_id.has_value())
                {
                    res["code"] = 500;
                    res["message"] = "Failed to create user nya~";
                    return crow::response(500, res);
                }
                res["code"] = 201;
                res["new_id"] = new_id.value();
                res["message"] = "User created successfully nya~";
                return crow::response(201, res);
            });

    CROW_ROUTE(app, "/api/admin/users/<int>")
        .methods(crow::HTTPMethod::PUT)(
            [&](const crow::request &req, int id)
            {
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
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
                UserService us(con);
                auto temp = us.getById(id);
                if (!temp.has_value())
                {
                    res["code"] = 404;
                    res["message"] = "User not found nya~";
                    return crow::response(404, res);
                }
                User u = temp.value();
                if (json.has("username"))
                    u.username = json["username"].s();
                if (json.has("password"))
                    u.password_hash = Encode::gethash(json["password"].s());
                if (json.has("role"))
                    u.role = json["role"].i();

                if (!us.update(u))
                {
                    res["code"] = 500;
                    res["message"] = "Failed to update user nya~";
                    return crow::response(500, res);
                }
                res["code"] = 200;
                res["message"] = "User updated successfully nya~";
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/admin/users/<int>")
        .methods(crow::HTTPMethod::DELETE)(
            [&](const crow::request &req, int id)
            {
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
                auto &ctx = app.get_context<AuthMiddleware>(req);
                crow::json::wvalue res;
                if (!ctx.userid.has_value() || ctx.role != 1)
                {
                    res["code"] = 403;
                    res["message"] = "Admin access required nya~";
                    return crow::response(403, res);
                }
                UserService us(con);
                if (id == ctx.userid.value())
                {
                    res["code"] = 400;
                    res["message"] = "Cannot delete yourself nya~";
                    return crow::response(400, res);
                }
                if (!us.remove(id))
                {
                    res["code"] = 500;
                    res["message"] = "Failed to delete user nya~";
                    return crow::response(500, res);
                }
                res["code"] = 200;
                res["message"] = "User deleted successfully nya~";
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/submit/all")
        .methods(crow::HTTPMethod::GET)(
            [&](const crow::request &req)
            {
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
                SubmissionService ss(con);
                auto vec = ss.listAll(50, 0);
                if (!vec.has_value())
                {
                    crow::json::wvalue res;
                    res["code"] = 500;
                    res["message"] = "No submissions found nya~";
                    return crow::response(500, res);
                }
                crow::json::wvalue res = crow::json::wvalue::list();
                std::size_t idx = 0;
                for (const auto &i : vec.value())
                {
                    crow::json::wvalue item;
                    item["id"] = i.id;
                    item["problem_id"] = i.problem_id;
                    item["user_id"] = i.user_id;
                    item["language"] = i.language;
                    item["status"] = i.status;
                    item["submit_time"] = i.submit_time;
                    item["time_cost"] = i.time_cost;
                    item["mem_cost"] = i.mem_cost;
                    res[idx++] = std::move(item);
                }
                return crow::response(200, res);
            });

    auto send_judge_request = [](const Submission &sub, const Problem &problem)
    {
        std::thread([sub, problem]()
                    {
        httplib::Client main_client("localhost", 18081);

        main_client.set_connection_timeout(3);

        crow::json::wvalue req_json;
        req_json["submission_id"] = sub.id;
        req_json["problem_id"] = sub.problem_id;
        req_json["language"] = sub.language;
        req_json["code"] = sub.code;
        req_json["time_limit"] = problem.time_limit;
        req_json["mem_limit"] = problem.mem_limit;
        req_json["tc_path"] = problem.tc_path;
        auto res = main_client.Post("/rpc/judge", req_json.dump(), "application/json");

        if (!res || res->status != 200)
            std::cerr << "Failed to send judge request for submission" << " " << sub.id << std::endl; })
            .detach();
    };

    CROW_ROUTE(app, "/api/submit/new")
        .methods(crow::HTTPMethod::POST)(
            [&](const crow::request &req)
            {
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
                auto &ctx = app.get_context<AuthMiddleware>(req);
                crow::json::wvalue res;
                if (!ctx.userid.has_value())
                {
                    res["code"] = 401;
                    res["message"] = "Unlogged nya~";
                    return crow::response(401, res);
                }
                auto json = crow::json::load(req.body);
                if (!json || !json.has("problem_id") || !json.has("language") || !json.has("code") || !json.has("user_id"))
                {
                    res["code"] = 400;
                    res["message"] = "Invalid submission data nya~";
                    return crow::response(400, res);
                }

                Submission submission;

                submission.problem_id = json["problem_id"].i();
                submission.user_id = ctx.userid.value();
                // submission.user_id = json["user_id"].i();
                submission.language = std::string(json["language"].s());
                submission.code = std::string(json["code"].s());
                submission.status = "Submitted";
                submission.detail = "";
                submission.submit_time = "";
                submission.time_cost = 0;
                submission.mem_cost = 0;
                SubmissionService ss(con);
                auto new_id = ss.create(submission);

                ProblemService ps(con);
                ps.updateStat(submission.problem_id, false);

                if (!new_id.has_value())
                {
                    res["code"] = 500;
                    res["message"] = "Failed to submit nya~";
                    return crow::response(500, res);
                }
                submission.id = new_id.value();

                auto problem = ps.getById(submission.problem_id);
                if (!problem.has_value())
                {
                    res["code"] = 404;
                    res["message"] = "Problem not found nya~";
                    return crow::response(404, res);
                }

                send_judge_request(submission, problem.value());

                res["code"] = 200;
                res["new_id"] = new_id.value();
                res["message"] = "Submitted successful nya~";
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/rpc/callback")
        .methods(crow::HTTPMethod::POST)(
            [&](const crow::request &req)
            {
                auto json = crow::json::load(req.body);
                if (!json)
                    return crow::response(400, "Invalid callback data nya~");
                i64 submission_id = static_cast<i64>(json["submission_id"].i());
                std::string status = std::string(json["status"].s());
                std::string detail = std::string(json["detail"].s());
                int time_cost = static_cast<int>(json.has("time_cost") ? json["time_cost"].i() : 0);
                i64 mem_cost = static_cast<i64>(json.has("mem_cost") ? json["mem_cost"].i() : 0);

                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
                SubmissionService ss(con);
                Submission submission(
                    submission_id,
                    0,
                    0,
                    "",
                    "",
                    status,
                    detail,
                    "",
                    static_cast<int>(time_cost),
                    static_cast<int>(mem_cost));
                if (status == "Accepted")
                {
                    auto sub_opt = ss.getById(submission_id);
                    if (sub_opt.has_value())
                    {
                        ProblemService ps(con);
                        ps.updateStat(sub_opt->problem_id, true);
                    }
                }
                if (!ss.updateStatus(submission))
                {
                    std::cerr << "Callback update failed for submission " << submission_id << std::endl;
                    return crow::response(500, "Failed to update submission nya~");
                }
                return crow::response(200, "Callback processed successfully nya~");
            });

    CROW_ROUTE(app, "/api/submit/<int>")
        .methods(crow::HTTPMethod::GET)(
            [&](const crow::request &req, int submit_id)
            {
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
                auto &ctx = app.get_context<AuthMiddleware>(req);
                crow::json::wvalue res;
                if (!ctx.userid.has_value())
                {
                    res["code"] = 401;
                    res["message"] = "Unlogged nya~";
                    return crow::response(401, res);
                }
                SubmissionService ss(con);
                auto submission = ss.getById(submit_id);
                if (!submission.has_value())
                {
                    res["code"] = 404;
                    res["message"] = "Submission not found nya~";
                    return crow::response(404, res);
                }
                int uid = submission->user_id;
                if (ctx.role != 1 && ctx.userid.value() != uid)
                {
                    res["code"] = 403;
                    res["message"] = "Forbidden nya~";
                    return crow::response(403, res);
                }
                res["id"] = submission->id;
                res["problem_id"] = submission->problem_id;
                res["user_id"] = submission->user_id;
                res["language"] = submission->language;
                res["code"] = submission->code;
                res["status"] = submission->status;
                res["detail"] = submission->detail;
                res["submit_time"] = submission->submit_time;
                res["time_cost"] = submission->time_cost;
                res["mem_cost"] = submission->mem_cost;
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/submit/problem/<int>/all")
        .methods(crow::HTTPMethod::GET)(
            [&](const crow::request &req, int problem_id)
            {
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
                SubmissionService ss(con);
                auto vec = ss.findByProblemId(problem_id);
                if (!vec.has_value())
                {
                    crow::json::wvalue res;
                    res["code"] = 500;
                    res["message"] = "No submissions found nya~";
                    return crow::response(500, res);
                }
                crow::json::wvalue res = crow::json::wvalue::list();
                std::size_t idx = 0;
                auto arr = vec.value();
                for (const auto [id, problem_id, user_id, language, code, status, detail, submit_time, time_cost, mem_cost] : arr)
                {
                    crow::json::wvalue item;
                    item["id"] = id;
                    item["problem_id"] = problem_id;
                    item["user_id"] = user_id;
                    item["language"] = language;
                    item["status"] = status;
                    item["submit_time"] = submit_time;
                    item["time_cost"] = time_cost;
                    item["mem_cost"] = mem_cost;
                    res[idx++] = std::move(item);
                }

                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/submit/problem/<int>/user/<int>")
        .methods(crow::HTTPMethod::GET)(
            [&](const crow::request &req, int problem_id, int user_id)
            {
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
                SubmissionService ss(con);
                auto vec = ss.findByProblemIdAndUserId(problem_id, user_id);
                if (!vec.has_value())
                {
                    crow::json::wvalue res;
                    res["code"] = 500;
                    res["message"] = "No submissions found nya~";
                    return crow::response(500, res);
                }

                crow::json::wvalue res = crow::json::wvalue::list();
                std::size_t idx = 0;

                auto arr = vec.value();
                for (const auto [id, problem_id, user_id, language, code, status, detail, submit_time, time_cost, mem_cost] : arr)
                {
                    crow::json::wvalue item;
                    item["id"] = id;
                    item["problem_id"] = problem_id;
                    item["user_id"] = user_id;
                    item["language"] = language;
                    item["status"] = status;
                    item["submit_time"] = submit_time;
                    item["time_cost"] = time_cost;
                    item["mem_cost"] = mem_cost;
                    res[idx++] = std::move(item);
                }
                return crow::response(200, res);
            });

    CROW_ROUTE(app, "/api/user/<int>")
        .methods(crow::HTTPMethod::GET)(
            [&](const crow::request &req, int user_id)
            {
                auto conn = DBPool::instance().getConnection();
                auto &con = *conn;
                UserService us(con);
                auto user = us.getById(user_id);
                if (!user.has_value())
                {
                    crow::json::wvalue res;
                    res["code"] = 404;
                    res["message"] = "User not found nya~";
                    return crow::response(404, res);
                }

                SubmissionService ss(con);
                auto result = ss.findByUserId(user_id);
                if (!result.has_value())
                {
                    crow::json::wvalue res;
                    res["code"] = 500;
                    res["message"] = "Failed to query submissions nya~";
                    return crow::response(500, res);
                }
                auto [sub, ac] = result.value();
                crow::json::wvalue res;
                res["id"] = user->id;
                res["username"] = user->username;
                res["role"] = user->role;
                res["sub"] = sub.size();
                res["ac_count"] = ac.size();
                crow::json::wvalue sub_list = crow::json::wvalue::list();
                std::size_t idx = 0;
                for (const auto &[id, problem_id, user_id, language, code, status, detail, submit_time, time_cost, mem_cost] : sub)
                {
                    crow::json::wvalue item;
                    item["id"] = id;
                    item["problem_id"] = problem_id;
                    item["user_id"] = user_id;
                    item["language"] = language;
                    item["status"] = status;
                    item["submit_time"] = submit_time;
                    item["time_cost"] = time_cost;
                    item["mem_cost"] = mem_cost;
                    sub_list[idx++] = std::move(item);
                }
                res["submissions"] = std::move(sub_list);
                res["ac_problems"] = crow::json::wvalue::list();
                idx = 0;
                for (const auto &i : ac)
                    res["ac_problems"][idx++] = i;
                return crow::response(200, res);
            });

    app.port(18080)
        .multithreaded()
        .run();

    return 0;
}