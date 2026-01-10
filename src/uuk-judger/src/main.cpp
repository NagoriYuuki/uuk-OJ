#include "crow.h"

#include <iostream>
#include <string>
#include <filesystem>

#pragma push_macro("DELETE")
#undef DELETE
#include "httplib.h"
#pragma pop_macro("DELETE")

#include "../include/judge_result.hpp"
#include "../include/redis_queue.hpp"
#include "../include/container.hpp"
#include "../include/httplib.h"
#include "../include/ans_checker.hpp"

using i64 = long long;

struct TaskInfo
{
    i64 submission_id;
    int problem_id;
    std::string code;
    std::string lang;
    int time_limit;
    int mem_limit;
    std::string tc_path;

    TaskInfo() = default;
    TaskInfo(i64 sid, int pid, const std::string &c, const std::string &l, int tl, int ml, const std::string &tc)
        : submission_id(sid), problem_id(pid), code(c), lang(l), time_limit(tl), mem_limit(ml), tc_path(tc) {}
};

TaskInfo parse_task(const std::string &task_str)
{
    auto json = crow::json::load(task_str);
    return TaskInfo{
        static_cast<i64>(json["submission_id"].i()),
        static_cast<int>(json["problem_id"].i()),
        std::string(json["code"].s()),
        std::string(json["language"].s()),
        static_cast<int>(json.has("time_limit") ? json["time_limit"].i() : 1000),
        static_cast<int>(json.has("mem_limit") ? json["mem_limit"].i() : 65536),
        json.has("tc_path") ? std::string(json["tc_path"].s()) : std::string{}};
}

void report_progress(const i64 &sub_id, const int &tc_id)
{
    std::string str = "Running on Test# " + std::to_string(tc_id);
    crow::json::wvalue json;
    json["submission_id"] = sub_id;
    json["status"] = "Running";
    json["detail"] = str;
    json["time_cost"] = 0;
    json["mem_cost"] = 0;

    httplib::Client client("localhost", 18080);
    client.set_connection_timeout(3);
    auto res = client.Post("/rpc/callback", json.dump(), "application/json");
    // if (!res)
    //     std::cerr << "Failed to report progress for submission " << sub_id << "on Test# " << tc_id << std::endl;
}

void callback_final_res(const JudgeResult &result)
{
    crow::json::wvalue json;
    json["submission_id"] = result.id;
    json["problem_id"] = result.problem_id;
    json["status"] = result.status;
    json["detail"] = result.detail;
    json["time_cost"] = result.time_cost;
    json["mem_cost"] = result.mem_cost;
    httplib::Client client("localhost", 18080);
    client.set_connection_timeout(3);
    auto res = client.Post("/rpc/callback", json.dump(), "application/json");

    if (!res)
        std::cerr << "Failed to callback final result for submission " << result.id << std::endl;
}

void ch_proc_work(const std::string &redis_host, const int &redis_port)
{
    RedisQueue consumer(redis_host, redis_port);
    while (1)
    {
        auto task_opt = consumer.pop("judge_queue");
        if (!task_opt.has_value())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        auto task_info = parse_task(task_opt.value());
        std::string work_path = "../workspace/task" + std::to_string(task_info.submission_id);
        std::filesystem::remove_all(work_path);
        std::filesystem::create_directory(work_path);

        std::string code_path = work_path + "/code.cpp";
        std::string exe_path = work_path + "/exe";

        std::ofstream fout(code_path);
        fout << task_info.code;
        fout.close();
        std::string complie_log_path = work_path + "/compile_log.log";
        std::stringstream compile_cmd;
        compile_cmd << "g++ " << code_path << " -o " << exe_path << " -O2 -std=c++23 -Wall -lm -static -DONLINE_JUDGE" << " 2> " << complie_log_path;

        JudgeResult result(task_info.submission_id,
                           task_info.problem_id,
                           0,
                           task_info.lang,
                           task_info.code,
                           "Pending",
                           "",
                           "",
                           0,
                           0);

        int compile_res = system(compile_cmd.str().c_str());
        if (compile_res != 0)
        {
            std::cerr << "Compilation failed for submission " << task_info.submission_id << std::endl;
            result.status = "Compile Error";

            std::ifstream fin(complie_log_path);
            std::stringstream buffer;
            buffer << fin.rdbuf();
            std::string compile_log = buffer.str();
            if (compile_log.size() > 8192)
                compile_log = compile_log.substr(0, 8192) + "\n... (truncated)";
            result.detail = compile_log;

            callback_final_res(result);

            std::filesystem::remove_all(work_path);

            continue;
        }

        Config config;
        config.code_path = exe_path;
        config.time_limit = task_info.time_limit;
        config.mem_limit = task_info.mem_limit;
        std::string data_path = "data/problems/p" + std::to_string(task_info.problem_id);
        int testcases = 0;
        const std::filesystem::path data_dir(data_path);
        for (auto &i : std::filesystem::directory_iterator(data_dir))
            if (i.path().extension() == ".in")
                testcases++;
        result.time_cost = 0;
        result.mem_cost = 0;
        for (int i = 1; i <= testcases; i++)
        {
            // report_progress(task_info.submission_id, i);
            std::thread([sid = task_info.submission_id, idx = i]()
                        { report_progress(sid, idx); })
                .detach();

            const std::string input_file = data_path + "/data" + std::to_string(i) + ".in";
            const std::string output_file = data_path + "/data" + std::to_string(i) + ".out";

            if (!std::filesystem::exists(input_file))
            {
                result.status = "System Error";
                result.detail = "Input file missing for Test #" + std::to_string(i);
                break;
            }
            
            config.input_path = input_file;
            config.output_path = work_path + "/user_output.txt";
            Container container(config);

            auto process_info = container.start();
            if (process_info.pid == -1)
            {
                result.status = "System Error";
                result.detail = "Failed to start container for Test #" + std::to_string(i);
                break;
            }
            auto run_result = container.wait(process_info.pid, process_info.finished);
            if (run_result.time_cost > result.time_cost)
                result.time_cost = run_result.time_cost;
            if (run_result.mem_cost > result.mem_cost)
                result.mem_cost = run_result.mem_cost;
            if (run_result.status != 0)
            {
                if (run_result.status == SIGKILL)
                {
                    result.status = "Time Limit Exceeded";
                    result.detail = "Time Limit Exceeded on Test #" + std::to_string(i);
                }
                else if (run_result.status == 111)
                {
                    result.status = "Memory Limit Exceeded";
                    result.detail = "Memory Limit Exceeded on Test #" + std::to_string(i);
                }
                else
                {
                    result.status = "Runtime Error";
                    result.detail = "Runtime Error on Test #" + std::to_string(i);
                }
                break;
            }

            bool judge_res = AnsChecker::check(output_file, config.output_path);
            if (!judge_res)
            {
                result.status = "Wrong Answer";
                result.detail = "Wrong Answer on Test #" + std::to_string(i);
                break;
            }
            if (i == testcases)
            {
                result.status = "Accepted";
                result.detail = "All test cases passed.";
            }
        }
        callback_final_res(result);
        std::filesystem::remove_all(work_path);
    }
}

signed main(void)
{
    std::filesystem::create_directory("../workspace");

    crow::App app;

    std::string redis_host = "127.0.0.1";
    int redis_port = 6379;

    int proc_size = 4;

    std::vector<int> ch_pids;

    for (int i = 0; i < proc_size; i++)
    {
        auto pid = fork();
        if (!pid)
        {
            ch_proc_work(redis_host, redis_port);
            return 0;
        }
        else if (pid > 0)
        {
            ch_pids.push_back(pid);
            std::cerr << "Fork ch_process with pid: " << pid << std::endl;
        }
        else
            perror("Fork failed");
    }

    RedisQueue producer(redis_host, redis_port);

    CROW_ROUTE(app, "/rpc/judge")
        .methods(crow::HTTPMethod::POST)(
            [&](const crow::request &req)
            {
                auto json = crow::json::load(req.body);
                if (!json || !json.has("submission_id"))
                    return crow::response(400, "Invalid request: missing submission_id");
                try
                {
                    producer.push("judge_queue", req.body);
                    crow::json::wvalue res;
                    res["code"] = 200;
                    res["status"] = "Pending";
                    res["message"] = "Submission enqueued for judging";
                    return crow::response(200, res);
                }
                catch (const std::exception &e)
                {
                    std::cerr << "Error enqueuing submission: " << e.what() << std::endl;
                    return crow::response(500, e.what());
                }
            });

    app.port(18081).multithreaded().run();

    return 0;
}