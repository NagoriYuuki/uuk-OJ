#include "httplib.h"
#include <crow.h>
#include <iostream>
#include <string>

signed main(void)
{
    httplib::Client client("localhost", 18080);
    client.set_read_timeout(5, 0);

    crow::json::wvalue json;

    json["title"] = "test";
    json["submission_id"] = 1;
    json["problem_id"] = 6;
    json["code"] = "#includ <bits/stdc++.h>\nusing i64 = long long;\n\nsigned main(void)\n{\n    std::vector<std::vector<int>> arr(1024*512);\n    for (auto &i : arr)\n    {\n        i.resize(1024, 0);\n        std::iota(i.begin(), i.end(), 0);\n    }\n}\n";
    json["language"] = "C++";
    json["user_id"] = 6;

    auto res = client.Post("/api/submit/new", json.dump(), "application/json");

    if (!res)
    {
        std::cerr << "Request failed: connection error or timeout\n";
        return 1;
    }

    std::cout << "HTTP status: " << res->status << std::endl;

    if (res->status != 200)
    {
        std::cerr << "Server returned error, body:\n"
                  << res->body << std::endl;
        return 1;
    }

    std::cout << "Response body:\n"
              << res->body << std::endl;
    return 0;
}
