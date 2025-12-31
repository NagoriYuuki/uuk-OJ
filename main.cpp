#include <crow.h>

using i64 = long long;

signed main(void)
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);

    crow::SimpleApp app;
    CROW_ROUTE(app, "/")([]()
                         { return "Hello, uuk-OJ!"; });
    app.port(18080).multithreaded().run();

    return 0;
}