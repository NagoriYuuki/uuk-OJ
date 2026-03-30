#pragma once

#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <iostream>
#include <cstring>
#include <thread>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

#include "worker.hpp"
#include "IPCprotocol.hpp"

#pragma push_macro("DELETE")
#undef DELETE
#include "httplib.h"
#pragma pop_macro("DELETE")
#include "crow.h"

class Daemon;
extern Daemon *daemon_ptr;
void sigchld_handler_wrapper(int sig);

class Daemon
{
public:
    Daemon(const std::string &_brokers, const std::string &_oj_host, const int _oj_port, int _worker_count) : brokers(_brokers), oj_host(_oj_host), oj_port(_oj_port), worker_count(_worker_count)
    {
        epoll_fd = epoll_create1(EPOLL_CLOEXEC);
        if (epoll_fd == -1)
        {
            perror("Failed to create epoll instance");
            exit(1);
        }
        if (pipe(notify_pipe) == -1)
        {
            perror("Failed to create notify pipe");
            exit(1);
        }
        set_nonblocking(notify_pipe[0]);
        add_epoll(notify_pipe[0]);

        struct sigaction sa;
        std::memset(&sa, 0, sizeof(sa));
        sa.sa_handler = sigchld_handler_wrapper;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
        if (sigaction(SIGCHLD, &sa, nullptr) == -1)
            perror("Failed to set SIGCHLD handler");
        signal(SIGCHLD, sigchld_handler_wrapper);
        signal(SIGPIPE, SIG_IGN);

        daemon_ptr = this;
    }

    ~Daemon()
    {
        close(notify_pipe[0]);
        close(notify_pipe[1]);
        if (epoll_fd >= 0)
            close(epoll_fd);
    };

    void run()
    {
        for (int i = 1; i <= worker_count; i++)
            create_worker();

        struct epoll_event events[MAXN];
        while (judger_running || !worker_map.empty())
        {
            int nfds = epoll_wait(epoll_fd, events, MAXN, 500);
            if (nfds == -1)
            {
                if (errno == EINTR)
                    continue;
                perror("Failed to wait on epoll");
                break;
            }
            for (int i = 0; i < nfds; i++)
            {
                int fd = events[i].data.fd;
                if (fd == notify_pipe[0])
                    data_ready();
                else
                    handle_worker_msg(fd);
            }
        }
        std::cout << "[Daemon] Epoll stopped, waiting for remaining workers to cleanly exit..." << std::endl;
        int status;
        pid_t p;
        while ((p = waitpid(-1, &status, 0)) > 0)
            ;
        std::cout << "[Daemon] All workers have been shut down." << std::endl;
    }

    void handle_sigchld()
    {
        int status;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
            std::cerr << "[Daemon] Worker " << pid << " exited." << std::endl;
    }

private:
    std::string oj_host;
    int oj_port;
    std::string brokers;
    int worker_count;
    int epoll_fd;
    int notify_pipe[2];
    std::unordered_map<int, pid_t> worker_map;
    std::unordered_set<int> existed_data;
    std::unordered_map<int, std::vector<pid_t>> data_waiting;
    std::unordered_set<int> downloading_data;
    std::mutex downloading_mutex;
    static constexpr int MAXN = 128;

    void set_nonblocking(int fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
        {
            perror("Failed to get pipe flags");
            exit(1);
        }
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    void add_epoll(int fd)
    {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
        {
            perror("Failed to add fd to epoll");
            exit(1);
        }
    }

    void handle_worker_msg(int fd)
    {
        while (true)
        {
            IPCMessage msg;
            ssize_t n = read(fd, &msg, sizeof(msg));

            if (!n)
            {
                std::cerr << "[Daemon] Worker disconnected" << std::endl;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
                worker_map.erase(fd);
                create_worker();
                return;
            }
            else if (n == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return;
                else
                {
                    perror("Failed to read from worker IPC");
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                    pid_t old_pid = 0;
                    auto it = worker_map.find(fd);
                    if (it != worker_map.end())
                    {
                        old_pid = it->second;
                        worker_map.erase(it);
                    }
                    create_worker();
                    return;
                }
            }
            else if (static_cast<size_t>(n) != sizeof(msg))
            {
                std::cerr << "[Daemon] Partial read from worker IPC" << std::endl;
                return;
            }
            if (msg.type == IPCType::REQ_DATA)
            {
                int problem_id = msg.info.problem_id;
                if (existed_data.count(problem_id))
                {
                    IPCMessage rep;
                    rep.type = IPCType::RDY_DATA;
                    rep.info.problem_id = problem_id;
                    write(fd, &rep, sizeof(rep));
                }
                else
                {
                    data_waiting[problem_id].push_back(fd);
                    download_data(problem_id);
                }
            }
            else if (msg.type == IPCType::REP_STATUS)
            {
                progress_report(msg.info);
                IPCMessage rep;
                std::memset(&rep, 0, sizeof(rep));
                rep.type = IPCType::NYA_PONG;
                write(fd, &rep, sizeof(rep));
            }
        }
    }

    void download_data(int problem_id)
    {
        {
            std::lock_guard<std::mutex> lock(downloading_mutex);
            if (downloading_data.count(problem_id))
                return;
            downloading_data.insert(problem_id);
        }

        std::cerr << "[Daemon] Start downloading data for problem " << problem_id << std::endl;
        std::string host = this->oj_host;
        int port = this->oj_port;
        std::thread([this, host, port, pid = problem_id]()
                    {
                        httplib::Client client(host, port);
                        std::string data_path = "src/uuk-judger/data/p" + std::to_string(pid);
                        if (std::filesystem::exists(data_path))
                            std::filesystem::remove_all(data_path);
                        std::filesystem::create_directories(data_path);
                        client.set_connection_timeout(10);
                        client.set_read_timeout(60);
                        httplib::Headers headers = {{"Authorization", "is_Neko_Girl"}};
                        std::string url = "/api/internal/data/" + std::to_string(pid);
                        auto res = client.Get(url.c_str(), headers);
                        bool success = false;
                        if (res && res->status == 200)
                        {
                            std::string zipfile_name = "data_p" + std::to_string(pid) + ".zip";
                            std::ofstream fout(zipfile_name, std::ios::binary);
                            if (fout.is_open())
                            {
                                fout.write(res->body.data(), res->body.size());
                                fout.close();
                                std::string cmd = "unzip -q -o " + zipfile_name + " -d " + data_path;
                                int ret = system(cmd.c_str());
                                if (ret == 0)
                                {
                                    success = true;
                                    std::filesystem::remove(zipfile_name);
                                }
                                else
                                    std::cerr << "[Daemon] Failed to unzip data for problem " << pid << std::endl;
                            }
                            else
                                std::cerr << "[Daemon] Failed to save data for problem " << pid << std::endl;
                        }
                        else
                            std::cerr << "[Daemon] Failed to download data for problem " << pid << std::endl;
                        if (success)
                        {
                            if (write(notify_pipe[1], &pid, sizeof(pid)) == -1)
                                perror("Failed to write to notify pipe");
                        }
                        else
                        {
                            std::filesystem::remove_all(data_path);
                            int fail_pid = -pid;
                            write(notify_pipe[1], &fail_pid, sizeof(fail_pid));
                        }
                        {
                            std::lock_guard<std::mutex>lk(downloading_mutex);
                            downloading_data.erase(pid);
                        } })
            .detach();
    }

    void data_ready()
    {
        int problem_id;
        while (true)
        {
            ssize_t n = read(notify_pipe[0], &problem_id, sizeof(problem_id));
            if (n > 0)
            {
                int abs_id = std::abs(problem_id);
                if (problem_id > 0)
                    existed_data.insert(abs_id);
                if (data_waiting.count(abs_id))
                    {
                        for (const auto &i : data_waiting[abs_id])
                        {
                            IPCMessage rep;
                            rep.type = IPCType::RDY_DATA;
                            rep.info.problem_id = abs_id;
                            write(i, &rep, sizeof(rep));
                        }
                        data_waiting.erase(abs_id);
                    }
            }
            else if (n == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    break;
                else
                {
                    perror("Failed to read from notify pipe");
                    break;
                }
            }
            else
            {
                std::cerr << "[Daemon] Notify pipe closed" << std::endl;
                break;
            }
        }
    }

    void progress_report(const JudgeResult &result)
    {
        std::string host = this->oj_host;
        int port = this->oj_port;
        std::thread([host, port, result]()
                    {
            httplib::Client cli(host, port);
            cli.set_connection_timeout(5);

            crow::json::wvalue json;
            json["submission_id"] = result.submission_id;
            json["status"] = std::string(result.status);
            json["detail"] = result.detail;
            json["time_cost"] = result.time_cost;
            json["mem_cost"] = result.mem_cost;

            auto res = cli.Post("/rpc/callback", json.dump(), "application/json");
            if (!res || res->status != 200) {
                std::cerr << "[Daemon] callback failed for submission " << result.submission_id << std::endl;
            } })
            .detach();
    }

    void create_worker()
    {
        if (!judger_running)
            return;
        int sv[2];
        if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sv) == -1)
        {
            perror("Failed to create socket pair for worker IPC");
            return;
        }

        pid_t pid = fork();
        if (pid < 0)
        {
            close(sv[0]);
            close(sv[1]);
            perror("Failed to fork worker process");
            return;
        }
        if (!pid)
        {
            close(sv[0]);
            if (epoll_fd > 0)
                close(epoll_fd);
            Worker worker(brokers, sv[1]);
            worker.run();
            _exit(0);
        }
        else
        {
            close(sv[1]);
            set_nonblocking(sv[0]);
            add_epoll(sv[0]);
            worker_map[sv[0]] = pid;
        }
    }
};