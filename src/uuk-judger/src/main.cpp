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
#include "../entities/task_info.hpp"
#include "../include/overlayFS.hpp"

using i64 = long long;

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

void copy_testcase(const std::string &src, const std::string &dir)
{
    std::ifstream fin(src, std::ios::binary);
    std::ofstream fout(dir, std::ios::binary);
    fout << fin.rdbuf();
    fin.close();
    fout.close();
}

void report_progress(const i64 &sub_id, const int &tc_id, const int &tc_total)
{
    std::string str = "Running on Test# " + std::to_string(tc_id);
    if (tc_total > 0)
        str += "/" + std::to_string(tc_total);

    crow::json::wvalue json;
    json["submission_id"] = sub_id;
    json["status"] = "Running";
    json["detail"] = str;
    json["time_cost"] = 0;
    json["mem_cost"] = 0;

    httplib::Client client("localhost", 18080);
    client.set_connection_timeout(3);
    client.Post("/rpc/callback", json.dump(), "application/json");
}

void report_compiling(const i64 &sub_id)
{
    crow::json::wvalue json;
    json["submission_id"] = sub_id;
    json["status"] = "Compiling";
    json["detail"] = "Compiling...";
    json["time_cost"] = 0;
    json["mem_cost"] = 0;

    httplib::Client client("localhost", 18080);
    client.set_connection_timeout(3);
    client.Post("/rpc/callback", json.dump(), "application/json");
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
        std::string work_path = std::filesystem::absolute("workspace/task" + std::to_string(task_info.submission_id)).string();

        OverlayFS overlay(task_info);
        auto ofs = overlay.prepare_workspace();
        if (ofs.status)
        {
            std::cerr << "Failed to init workspace for subid: " << task_info.submission_id << std::endl;
            overlay.remove_workspace();
            continue;
        }
        // std::cerr << "Ready to complie!" << std::endl;
        JudgeResult result;
        result.id = task_info.submission_id;
        result.problem_id = task_info.problem_id;

        // ========
        // Compilation Nya~
        // ========

        Config base_config;
        base_config.overlay_lower = ofs.lower_path;
        base_config.overlay_upper = ofs.upper_path;
        base_config.overlay_work = ofs.work_path;
        base_config.overlay_merge = ofs.merge_path;
        base_config.work_dir = ofs.merge_path;

        bool compile_success = true;
        std::string compile_msg;

        if (task_info.lang != "Python")
        {
            report_compiling(task_info.submission_id);
            Config compile_cfg = base_config;
            compile_cfg.is_judging = false;
            compile_cfg.time_limit = 10000;
            compile_cfg.mem_limit = 512 * 1024;
            compile_cfg.output_path = "/compile.log";
            // std::cerr << "task_info.lang: " << task_info.lang << std::endl;
            if (task_info.lang == "C++")
            {
                compile_cfg.code_path = "/usr/bin/g++";
                compile_cfg.args = {"/main.cpp", "-o", "/exe", "-O2", "-static"};
            }
            else if (task_info.lang == "Java")
            {
                compile_cfg.code_path = "/usr/bin/javac";
                compile_cfg.args = {"-d", ".", "/Main.java"};
            }
            // std::cerr << "Ready to in container!!" << std::endl;

            Container compile_container(compile_cfg);
            auto proc = compile_container.start();
            auto compile_res = compile_container.wait(proc.pid, proc.finished);
            if (compile_res.exit_code || compile_res.status)
            {
                std::cerr << "compile info: " << compile_res.exit_code << " " << compile_res.status << std::endl;
                compile_success = false;
                std::ifstream fin(ofs.upper_path + "/compile.log");
                std::stringstream buffer;
                buffer << fin.rdbuf();
                compile_msg = buffer.str();
                if (compile_msg.empty())
                    compile_msg = "Compilation Failed Exit Code " + std::to_string(compile_res.exit_code);
            }
        }
        // std::cerr << "compile_success: " << compile_success << std::endl;
        if (!compile_success)
        {
            result.status = "Compile Error";
            result.detail = compile_msg;
            std::cerr << "CE msg: " << compile_msg << std::endl;
            callback_final_res(result);
            overlay.remove_workspace();
            continue;
        }

        // ==========
        // Execution Nya~
        // ==========

        std::string data_path = "data/problems/p" + std::to_string(task_info.problem_id);
        int testcases = 0;
        const std::filesystem::path data_dir(data_path);
        for (auto &i : std::filesystem::directory_iterator(data_dir))
            if (i.path().extension() == ".in")
                testcases++;
        result.time_cost = 0;
        result.mem_cost = 0;
        bool all_ac = false;
        for (int i = 1; i <= testcases; i++)
        {
            std::thread([sid = task_info.submission_id, idx = i, total = testcases]()
                        { report_progress(sid, idx, total); })
                .detach();
            // std::cerr << "check2!" << std::endl;
            const std::string host_input = data_path + "/data" + std::to_string(i) + ".in";
            const std::string host_output = data_path + "/data" + std::to_string(i) + ".out";

            // copy_testcase(input_file, work_path + "/data.in");
            copy_testcase(host_input, ofs.upper_path + "/data.in");

            if (!std::filesystem::exists(host_input))
            {
                result.status = "System Error";
                result.detail = "Input file missing for Test #" + std::to_string(i);
                break;
            }
            // std::cerr << "Compiled!!" << std::endl;
            Config run_cfg = base_config;
            run_cfg.time_limit = task_info.time_limit;
            run_cfg.mem_limit = task_info.mem_limit;
            run_cfg.is_judging = true;
            run_cfg.input_path = "/data.in";
            run_cfg.output_path = "/user.out";

            if (task_info.lang == "C++")
            {
                run_cfg.code_path = "/exe";
                run_cfg.args = {"/exe"};
            }
            else if (task_info.lang == "Python")
            {
                run_cfg.code_path = "/usr/bin/python3";
                run_cfg.args = {"/main.py"};
            }
            else if (task_info.lang == "Java")
            {
                run_cfg.code_path = "/usr/bin/java";
                run_cfg.args = {"-cp", ".", "Main"};
            }

            Container run_container(run_cfg);

            auto process_info = run_container.start();
            if (process_info.pid == -1)
            {
                result.status = "System Error";
                result.detail = "Failed to start container for Test #" + std::to_string(i);
                break;
            }
            auto run_result = run_container.wait(process_info.pid, process_info.finished);
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
                else if (run_result.status == 31)
                {
                    result.status = "Runtime Error";
                    result.detail = "Dangerous System Call Blocked (Seccomp) on Test #" + std::to_string(i);
                    std::cerr << "Process killed by Seccomp (SIGSYS 31)!" << std::endl;
                }
                else
                {
                    std::cerr << "exit_code: " << run_result.exit_code << std::endl;
                    result.status = "Runtime Error";
                    result.detail = "Runtime Error on Test #" + std::to_string(i);
                }
                break;
            }

            bool judge_res = AnsChecker::check(ofs.upper_path + "/user.out", host_output);
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
        overlay.remove_workspace();
    }
}

signed main(void)
{
    std::filesystem::create_directory("workspace");

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