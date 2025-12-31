#include <crow.h>
#include <bits/stdc++.h>
#include <mariadb/conncpp.hpp>
#include "db.hpp"

using i64 = long long;

signed main(void)
{
    auto &db = DB::instance();
    auto &con = db.getConnection();

    crow::SimpleApp app;
    CROW_ROUTE(app, "/")([]()
                         { return "Hello, uuk-OJ!"; });
    app.port(18080).multithreaded().run();

    return 0;
}