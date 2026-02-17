#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <atomic>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include <sstream>
#include <seccomp.h>

using i64 = long long;



struct RunResult
{
    int status;
    int exit_code;
    int signal;
    int time_cost;
    i64 mem_cost;
    RunResult()
        : status(0),
          exit_code(0),
          signal(0),
          time_cost(0),
          mem_cost(0) {}
    RunResult(int s, int ec, int sig, int tc, i64 mc)
        : status(s),
          exit_code(ec),
          signal(sig),
          time_cost(tc),
          mem_cost(mc) {}
};

class Container
{
public:
    Container() = default;
    ~Container() = default;
    explicit Container(const Config &cfg) : config(cfg)
    {
        stack = std::make_unique<char[]>(STACK_SIZE);
    }

    struct ProcessInfo
    {
        pid_t pid;
        std::shared_ptr<std::atomic<bool>> finished;
        ProcessInfo() : pid(-1), finished(std::make_shared<std::atomic<bool>>(false)) {}
        ProcessInfo(pid_t p, std::shared_ptr<std::atomic<bool>> f) : pid(p), finished(f) {}
    };

    struct CloneArgs
    {
        Container *container;
        int read_fd;
    };

    ProcessInfo start()
    {
        int sync_pipe[2];
        if (pipe(sync_pipe) == -1)
        {
            perror("pipe failed");
            return {-1, std::make_shared<std::atomic<bool>>(false)};
        }

        int flags = SIGCHLD | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNET | CLONE_NEWNS | CLONE_NEWIPC;

        char *stack_top = stack.get() + STACK_SIZE;

        auto args = std::make_unique<CloneArgs>();
        args->container = this;
        args->read_fd = sync_pipe[0];

        auto pid = clone([](void *arg)
                         {
                            CloneArgs *ca = static_cast<CloneArgs *>(arg);
                            int read_fd = ca->read_fd;
                            Container *container = ca->container;

                            char ch;
                            read(read_fd,&ch,1);
                            close(read_fd);
                            delete ca;
                            container->run_container(); 
                            return 0; },
                         stack_top, flags, args.get());
        auto finished = std::make_shared<std::atomic<bool>>(false);

        if (pid == -1)
        {
            perror("clone");
            close(sync_pipe[0]);
            close(sync_pipe[1]);
            return {-1, finished};
        }

        args.release();

        try
        {
            setup_cgroup(pid);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Setup cgroup failed: " << e.what() << std::endl;
            kill(pid, SIGKILL);
            close(sync_pipe[1]);
            return {-1, finished};
        }
        write(sync_pipe[1], "Nya~", 1);
        close(sync_pipe[1]);
        close(sync_pipe[0]);


        std::thread watch_thread(
            [pid, limit = config.time_limit, finished]()
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(limit + 500));
                if (!finished->load())
                    kill(pid, SIGKILL);
            });
        watch_thread.detach();
        return {pid, finished};
    }

    RunResult wait(const pid_t &pid, std::shared_ptr<std::atomic<bool>> finished)
    {
        int status;
        rusage usage;
        if (wait4(pid, &status, 0, &usage) == -1)
        {
            perror("wait4 failed");
            return RunResult(-1, -1, -1, -1, -1);
        }
        finished->store(true);

        int time_cost = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000 +
                        (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000;
        i64 mem_cost = get_mem_cgroup(pid);
        bool check_oom_res = check_oom(pid);
        clean_cgroup(pid);

        RunResult result;
        result.time_cost = time_cost;
        result.mem_cost = mem_cost;
        if (check_oom_res || mem_cost > config.mem_limit)
        {
            status = 111;
            result.status = status;
            result.mem_cost = config.mem_limit + 1;
            result.signal = SIGKILL;
        }
        else if (WIFEXITED(status))
        {
            result.exit_code = WEXITSTATUS(status);
            result.status = !result.exit_code ? 0 : 1;
            result.signal = 0;
        }
        else if (WIFSIGNALED(status))
        {
            result.exit_code = -1;
            result.signal = WTERMSIG(status);
            result.status = result.signal;
        }
        else
        {
            result.exit_code = -1;
            result.signal = -1;
            result.status = -1;
        }

        return result;
    }

private:
    Config config;
    static constexpr int STACK_SIZE = 1024 * 1024;
    std::unique_ptr<char[]> stack;

    const std::string CG_ROOT = "/sys/fs/cgroup/";
    const std::string CG_NAME = "uuk-judger";

    void run_container()
    {
        sethostname("ch_container", 12);
        setup_mounts();
        setup_chroot();
        setup_io();
        setup_rlim();

        if (setgid(65534) == -1 || setuid(65534) == -1)
        {
            perror("setuid/gid failed");
            _exit(1);
        }

        load_seccomp();

        execute_code();
    }

    void setup_mounts()
    {
        if (mount(NULL, "/", NULL, MS_PRIVATE | MS_REC, NULL) == -1)
        {
            perror("mount Failed");
            _exit(1);
        }

        if (mount("proc", "/proc", "proc", 0, NULL) == -1)
        {
            perror("mount proc failed");
            _exit(1);
        }
    }

    void setup_io()
    {

        if (!config.input_path.empty())
        {
            int fd = open(config.input_path.c_str(), O_RDONLY);
            if (fd == -1)
            {
                perror("open input file failed");
                _exit(1);
            }
            if (dup2(fd, STDIN_FILENO) == -1)
            {
                perror("dup2 input failed");
                _exit(1);
            }
            close(fd);
        }

        if (!config.output_path.empty())
        {
            int fd = open(config.output_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1)
            {
                perror("open output file failed");
                _exit(1);
            }
            if (dup2(fd, STDOUT_FILENO) == -1)
            {
                perror("dup2 output failed");
                _exit(1);
            }
            close(fd);
        }
    }

    void setup_chroot()
    {
        if (chdir(config.work_dir.c_str()))
        {
            perror("chdir failed");
            _exit(1);
        }
        if (chroot("."))
        {
            perror("chroot failed");
            _exit(1);
        }
        if (chdir("/") == -1)
        {
            perror("chdir / failed");
            _exit(1);
        }
        if (mount("proc", "/proc", "proc", 0, NULL) == -1)
        {
            perror("mount proc failed");
            _exit(1);
        }
    }

    void setup_rlim()
    {
        rlimit rl_cpu;
        rl_cpu.rlim_cur = (config.time_limit + 999) / 1000;
        rl_cpu.rlim_max = rl_cpu.rlim_cur + 1;
        if (setrlimit(RLIMIT_CPU, &rl_cpu) == -1)
        {
            perror("setrlimit CPU failed");
            _exit(1);
        }

        rlimit rl_st;
        rl_st.rlim_cur = RLIM_INFINITY;
        rl_st.rlim_max = RLIM_INFINITY;
        if (setrlimit(RLIMIT_STACK, &rl_st) == -1)
        {
            perror("setrlimit STACK failed");
            _exit(1);
        }

        rlimit rl_fsize;
        rl_fsize.rlim_cur = 64 * 1024 * 1024;
        rl_fsize.rlim_max = rl_fsize.rlim_cur;
        if (setrlimit(RLIMIT_FSIZE, &rl_fsize) == -1)
        {
            perror("setrlimit FSIZE failed");
            _exit(1);
        }

        rlimit rl_stack;
        rl_stack.rlim_cur = 4 * 1024 * 1024;
        rl_stack.rlim_max = rl_stack.rlim_cur;
        if (setrlimit(RLIMIT_STACK, &rl_stack) == -1)
        {
            perror("setrlimit STACK failed");
            _exit(1);
        }
    }

    void execute_code()
    {
        std::vector<char *> argv;
        argv.push_back(const_cast<char *>(config.code_path.c_str()));
        for (const auto &i : config.args)
            argv.push_back(const_cast<char *>(i.c_str()));
        argv.push_back(nullptr);

        std::vector<std::string> env = {
            "PATH=/bin",
            "LANG=C",
            "TZ=UTC"};

        std::vector<char *> envp;
        envp.reserve(env.size() + 1);
        for (auto &s : env)
            envp.push_back(strdup(s.c_str()));
        envp.push_back(nullptr);
        execve(config.code_path.c_str(), argv.data(), envp.data());
        perror("execve failed");
        _exit(1);
    }

    void setup_cgroup(const pid_t &pid)
    {
        std::string cg_path = CG_ROOT + "/" + CG_NAME;
        if (access(cg_path.c_str(), F_OK) != 0)
        {
            mkdir(cg_path.c_str(), 0755);
            write_into(cg_path + "/cgroup.subtree_control", "+memory +pids");
        }

        std::string task_path = cg_path + "/" + std::to_string(pid);
        if (mkdir(task_path.c_str(), 0755) != 0 && errno != EEXIST)
        {
            perror("mkdir cgroup task failed");
            throw std::runtime_error("mkdir cgroup task failed");
        }

        write_into(task_path + "/memory.max", std::to_string(config.mem_limit * 1024));
        write_into(task_path + "/memory.swap.max", "0");
        write_into(task_path + "/pids.max", "1");
        write_into(task_path + "/cgroup.procs", std::to_string(pid));
    }

    i64 get_mem_cgroup(const pid_t &pid)
    {
        std::string path = CG_ROOT + "/" + CG_NAME + "/" + std::to_string(pid) + "/memory.peak";
        std::ifstream fin(path);
        i64 peak_bytes = 0;
        if (fin >> peak_bytes)
            return peak_bytes / 1024;
        return 0;
    }

    void load_seccomp()
    {
        scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);
        if (ctx == NULL)
        {
            perror("seccomp_init failed");
            _exit(1);
        }

        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fstat), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0);

        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(readlinkat), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(readlink), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(set_robust_list), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(prlimit64), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getrandom), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rseq), 0);

        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(arch_prctl), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(set_tid_address), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(uname), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(sysinfo), 0);

        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(lseek), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mremap), 0);

        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(newfstatat), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(access), 0);

        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(readv), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(writev), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(ioctl), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fcntl), 0);

        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 0);

        if (seccomp_load(ctx) != 0)
        {
            perror("seccomp_load failed");
            seccomp_release(ctx);
            _exit(1);
        }

        seccomp_release(ctx);
    }

    void clean_cgroup(const pid_t &pid)
    {
        std::string task_path = CG_ROOT + "/" + CG_NAME + "/" + std::to_string(pid);

        std::ofstream fout(task_path + "/cgroup.kill");
        if (fout.is_open())
        {
            fout << "1";
            fout.close();
        }

        rmdir(task_path.c_str());
    }

    bool check_oom(const pid_t &pid)
    {
        std::string path = CG_ROOT + "/" + CG_NAME + "/" + std::to_string(pid) + "/memory.events";
        std::ifstream fin(path);
        if (!fin.is_open())
            return false;
        std::string line;
        i64 val = 0;
        while (fin >> line >> val)
            if (line == "oom")
            {
                fin.close();
                return val > 0;
            }
        fin.close();
        return false;
    }

    void write_into(const std::string &path, const std::string &content)
    {
        std::ofstream fout(path);
        if (!fout.is_open())
            throw std::runtime_error("open cgroup file failed: " + path + " errno=" + std::to_string(errno));
        fout << content;
        if (!fout)
            throw std::runtime_error("write cgroup file failed: " + path);

        fout.close();
    }
};