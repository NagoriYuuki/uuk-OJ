#include <crow.h>
#include <bits/stdc++.h>
#include <mariadb/conncpp.hpp>

#include "db.hpp"
#include "../services/problem_service.hpp"

using i64 = long long;

signed main(void)
{
    auto &con = DB::instance().getConnection();

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
    ([]()
     { return "Hello, uuk-OJ!"; });

    CROW_ROUTE(app, "/api/problems")
    ([&]()
     {
        ProblemService ps(con);
        auto vec = ps.listAll(50, 0);
        crow::json::wvalue res = crow::json::wvalue::list();
        std::size_t idx = 0;
        for (const auto &i : vec)
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
        return crow::response(200,res); });

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