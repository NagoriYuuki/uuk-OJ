#pragma once

#include <kafka/KafkaConsumer.h>
#include "IPCprotocol.hpp"
#include "crow.h"
#include "../include/ans_checker.hpp"
#include "../entities/task_info.hpp"
#include "../include/overlayFS.hpp"
#include "container.hpp"
#include <string>

extern std::atomic<bool> judger_running;

class Worker
{
public:
    Worker(const std::string &_brokers, const int _ipc_fd)
        : brokers(_brokers), ipc_fd(_ipc_fd) {}

    void run()
    {
        kafka::Properties props({{"bootstrap.servers", {brokers}},
                                 {"group.id", {"worker_group"}},
                                 {"enable.auto.commit", {"false"}}});
        kafka::clients::consumer::KafkaConsumer consumer(props);
        consumer.subscribe({"submission_queue"});
        std::cerr << "[Worker] Started worker, waiting for msg Nya~" << std::endl;
        try
        {
            while (judger_running)
            {
                auto records = consumer.poll(std::chrono::milliseconds(100));
                // std::cerr << "[Worker] get task" << std::endl;
                if (records.empty())
                    continue;
                for (const auto &i : records)
                {
                    if (!i.value().size())
                        continue;
                    TaskInfo task_info = parse_task(i.value().toString());
                    IPCMessage req;
                    std::memset(&req, 0, sizeof(req));
                    req.type = IPCType::REQ_DATA;
                    req.info.problem_id = task_info.problem_id;
                    std::cerr << "[Worker] write to daeon" << req.info.problem_id << std::endl;
                    write(ipc_fd, &req, sizeof(req));
                    IPCMessage rep;
                    read(ipc_fd, &rep, sizeof(rep));
                    if (rep.type != IPCType::RDY_DATA)
                    {
                        std::cerr << "Unexpected IPC response type!" << std::endl;
                        continue;
                    }
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
                    result.submission_id = task_info.submission_id;
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
                        result.set_status("Compile Error");
                        result.set_detail(compile_msg);
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
                        // std::thread([this, sid = task_info.submission_id, idx = i, total = testcases]()
                        //             { report_progress(sid, idx, total); })
                        //     .detach();
                        report_progress(task_info.submission_id, i, testcases);
                        // std::cerr << "check2!" << std::endl;
                        const std::string host_input = data_path + "/data" + std::to_string(i) + ".in";
                        const std::string host_output = data_path + "/data" + std::to_string(i) + ".out";

                        copy_testcase(host_input, ofs.upper_path + "/data.in");

                        if (!std::filesystem::exists(host_input))
                        {
                            result.set_status("System Error");
                            result.set_detail("Input file missing for Test #" + std::to_string(i));
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
                            result.set_status("System Error");
                            result.set_detail("Failed to start container for Test #" + std::to_string(i));
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
                                result.set_status("Time Limit Exceeded");
                                result.set_detail("Time Limit Exceeded on Test #" + std::to_string(i));
                            }
                            else if (run_result.status == 111)
                            {
                                result.set_status("Memory Limit Exceeded");
                                result.set_detail("Memory Limit Exceeded on Test #" + std::to_string(i));
                            }
                            else if (run_result.status == 31)
                            {
                                result.set_status("Runtime Error");
                                result.set_detail("Dangerous System Call Blocked (Seccomp) on Test #" + std::to_string(i));
                                std::cerr << "Process killed by Seccomp (SIGSYS 31)!" << std::endl;
                            }
                            else
                            {
                                std::cerr << "exit_code: " << run_result.exit_code << std::endl;
                                result.set_status("Runtime Error");
                                result.set_detail("Runtime Error on Test #" + std::to_string(i));
                            }
                            break;
                        }

                        bool judge_res = AnsChecker::check(ofs.upper_path + "/user.out", host_output);
                        if (!judge_res)
                        {
                            result.set_status("Wrong Answer");
                            result.set_detail("Wrong Answer on Test #" + std::to_string(i));
                            break;
                        }
                        if (i == testcases)
                        {
                            result.set_status("Accepted");
                            result.set_detail("All test cases passed.");
                        }
                    }
                    callback_final_res(result);
                    overlay.remove_workspace();
                }
                consumer.commitSync();
            }
            consumer.close();
        }
        catch (const std::exception &e)
        {
            consumer.close();
            std::cerr << "Worker encountered an error: " << e.what() << std::endl;
        }
    }

private:
    TaskInfo parse_task(const std::string &task_str)
    {
        using i64 = long long;
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

    void report_compiling(const i64 &sub_id)
    {
        IPCMessage rep;
        std::memset(&rep, 0, sizeof(rep));
        rep.type = IPCType::REP_STATUS;
        rep.info.submission_id = sub_id;
        rep.info.set_status("Compiling");
        write(ipc_fd, &rep, sizeof(rep));
        IPCMessage ack;
        read(ipc_fd, &ack, sizeof(ack));
        if (ack.type != IPCType::NYA_PONG)
            std::cerr << "Unexpected IPC ack type for compiling report!" << std::endl;
    }

    void report_progress(const i64 &sub_id, const int &tc_id, const int &tc_total)
    {
        IPCMessage rep;
        rep.type = IPCType::REP_STATUS;
        rep.info.submission_id = sub_id;
        rep.info.set_status("Running");
        if (tc_total > 0)
        {
            std::string detail = "Running on Test# " + std::to_string(tc_id) + "/" + std::to_string(tc_total);
            rep.info.set_detail(detail);
        }
        write(ipc_fd, &rep, sizeof(rep));
        IPCMessage ack;
        if (!read(ipc_fd, &ack, sizeof(ack)))
            return;
        if (ack.type != IPCType::NYA_PONG)
            std::cerr << "Unexpected IPC ack type for progress report!" << std::endl;
    }

    void callback_final_res(const JudgeResult &result)
    {
        IPCMessage rep;
        rep.type = IPCType::REP_STATUS;
        rep.info = result;
        write(ipc_fd, &rep, sizeof(rep));
        IPCMessage ack;
        read(ipc_fd, &ack, sizeof(ack));
        if (ack.type != IPCType::NYA_PONG)
            std::cerr << "Unexpected IPC ack type for final result callback!" << std::endl;
    }

    void copy_testcase(const std::string &src, const std::string &dir)
    {
        std::ifstream fin(src, std::ios::binary);
        std::ofstream fout(dir, std::ios::binary);
        fout << fin.rdbuf();
        fin.close();
        fout.close();
    }

    int ipc_fd;
    std::string brokers;
};