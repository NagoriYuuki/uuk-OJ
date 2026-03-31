#pragma once
#include <sw/redis++/redis++.h>
#include <atomic>
#include <iostream>
#include<random>
#include<chrono>
#include<thread>
#include<string>

class RedisLock
{
public:
    using i64 = long long;
    RedisLock(sw::redis::Redis &_redis, const std::string &_key, i64 _ttl_ms = 30000)
        : redis(_redis), key(_key), ttl_ms(_ttl_ms)
    {
        value = generate();
    }
    ~RedisLock()
    {
        unlock();
    }

    void lock()
    {
        while (!try_lock())
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

        bool try_lock()
    {
        if (is_locked)
            return true;
        bool res = redis.set(key, value, std::chrono::milliseconds(ttl_ms), sw::redis::UpdateType::NOT_EXIST);
        if (res)
        {
            is_locked = true;
            is_watch = true;
            watchdog_thread = std::thread(&RedisLock::watch_dog, this);
        }
        return res;
    }

    void unlock()
    {
        is_watch = false;
        if (watchdog_thread.joinable())
            watchdog_thread.join();
        const std::string lua = R"(
            if redis.call("get",KEYS[1])==ARGV[1] then
                return redis.call("del",KEYS[1])
            else
                return 0
            end
        )";
        try
        {
            redis.eval<i64>(lua, {key}, {value});
        }
        catch (const sw::redis::Error &e)
        {
            std::cerr << "[RedisLock]: Unlock error " << e.what() << std::endl;
        }
        is_locked = false;
    }

private:
    sw::redis::Redis &redis;
    std::string key;
    std::string value;
    std::atomic<bool> is_locked = false;
    std::atomic<bool> is_watch = false;
    std::thread watchdog_thread;
    i64 ttl_ms;


    void watch_dog()
    {
        const std::string lua = R"(
            if redis.call("get",KEYS[1])==ARGV[1] then
                return redis.call("pexpire",KEYS[1],ARGV[2])
            else
                return 0
            end
        )";
        i64 sleep_time = ttl_ms >> 1;
        while (is_watch)
        {
            for (i64 i = 0; i <= sleep_time; i += 100)
            {
                if (!is_watch)
                    return;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            if (!is_watch)
                return;
            try
            {
                auto res = redis.eval<i64>(lua, {key}, {value, std::to_string(ttl_ms)});
                if (!res)
                    break;
            }
            catch (...)
            {
            }
        }
    }

    std::string generate()
    {
        thread_local static std::random_device rd;
        thread_local static std::mt19937 gen(rd());
        thread_local static std::uniform_int_distribution<int> dis(0, 35);
        static const std::string str = "0123456789abcdefghijklmnopqrstuvwxyz";
        std::string res;
        for (int i = 1; i <= 16; i++)
            res += str[dis(gen)];
        return res;
    }
};

class RedisLockGuard
{
public:
    explicit RedisLockGuard(RedisLock &_lock) : lock(_lock)
    {
        lock.lock();
    }
    ~RedisLockGuard()
    {
        lock.unlock();
    }

private:
    RedisLock &lock;
};