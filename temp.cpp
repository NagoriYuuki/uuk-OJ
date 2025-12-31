#include <bits/stdc++.h>
#include <mariadb/conncpp.hpp>

using i64 = long long;

signed main(void)
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    // std::cout << __cplusplus << '\n';

    try
    {
        sql::Driver *driver = sql::mariadb::get_driver_instance();
        sql::Properties props({{"hostName", "localhost"},
                               {"userName", "yuuuki"},
                               {"password", "qweasd"},
                               {"schema", "uuk_oj"}});
        auto conn(driver->connect(props));
        std::cout << "Connection Succeed!" << std::endl;
    }
    catch (sql::SQLException &e)
    {
        std::cout << "Error connecting to database: " << e.what() << std::endl;
    }

    std::cout << "Accepted!" << std::endl;

    return 0;
}