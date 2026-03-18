#include <iostream>
#include <thread>
#include <filesystem>
#include "../include/daemon.hpp"
#include "../include/worker.hpp"

std::atomic<bool> judger_running{true};

void signal_handler(int signum)
{
    std::cout << "[Judger] Received signal " << signum << ", initiating graceful shutdown..." << std::endl;
    judger_running = false;
}

std::string get_env(const std::string &key, const std::string &default_value)
{
    const char *val = std::getenv(key.c_str());
    return val ? std::string(val) : default_value;
}

signed main(void)
{

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    const std::string kafka_brokers = get_env("KAFKA_BROKERS", "127.0.0.1:9092");
    const std::string oj_host = get_env("OJ_HOST", "127.0.0.1");
    const int oj_port = std::stoi(get_env("OJ_PORT", "18080"));

    int worker_count = std::max(4, static_cast<int>(std::thread::hardware_concurrency()));
    try
    {
        if (!std::filesystem::exists("workspace"))
            std::filesystem::create_directory("workspace");
        if (!std::filesystem::exists("../data"))
            std::filesystem::create_directory("../data");
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "[Judger] Failed to create basic directories." << std::endl;
        return 1;
    }
    try
    {
        Daemon daemon(kafka_brokers, oj_host, oj_port, worker_count);
        daemon.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Judger] Failed to run daemon" << std::endl;
    }

    return 0;
}