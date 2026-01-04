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

    CROW_ROUTE(app, "/")([]()
                         { return "Hello, uuk-OJ!"; });

    CROW_ROUTE(app, "/problems")([]()
                                 { auto page = crow::mustache::load_text("problemslist.html"); });

    app.port(18080).multithreaded().run();

    return 0;
}