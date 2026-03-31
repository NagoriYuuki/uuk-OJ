#pragma once

#include <sw/redis++/redis++.h>

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>

class RedisConnect
{
public:
    static RedisConnect &get_instance()
    {
        static RedisConnect instance;
        return instance;
    }

    sw::redis::Redis &client()
    {
        return *redis;
    }

    RedisConnect(const RedisConnect &) = delete;
    RedisConnect &operator=(const RedisConnect &) = delete;

private:
    std::unique_ptr<sw::redis::Redis> redis;

    RedisConnect()
    {
        sw::redis::ConnectionOptions opts;
        opts.host = get_env_or_default("REDIS_HOST", "127.0.0.1");
        opts.port = get_env_int_or_default("REDIS_PORT", 6379);
        opts.password = get_env_or_default("REDIS_PASSWORD", "");
        opts.db = get_env_int_or_default("REDIS_DB", 0);
        opts.keep_alive = true;
        opts.connect_timeout = std::chrono::milliseconds(get_env_int_or_default("REDIS_CONNECT_TIMEOUT_MS", 2000));
        opts.socket_timeout = std::chrono::milliseconds(get_env_int_or_default("REDIS_SOCKET_TIMEOUT_MS", 5000));

        sw::redis::ConnectionPoolOptions pool_opts;
        int pool_size = get_env_int_or_default("REDIS_POOL_SIZE", 4);
        if (pool_size < 1)
            pool_size = 1;
        pool_opts.size = static_cast<std::size_t>(pool_size);
        pool_opts.wait_timeout = std::chrono::milliseconds(get_env_int_or_default("REDIS_POOL_WAIT_TIMEOUT_MS", 1000));

        redis = std::make_unique<sw::redis::Redis>(opts, pool_opts);
    }

    ~RedisConnect() = default;

    static std::string get_env_or_default(const std::string &key, const std::string &default_value)
    {
        const char *val = std::getenv(key.c_str());
        return val ? std::string(val) : default_value;
    }

    static int get_env_int_or_default(const std::string &key, int default_value)
    {
        const char *val = std::getenv(key.c_str());
        if (!val)
            return default_value;
        try
        {
            return std::stoi(val);
        }
        catch (...)
        {
            return default_value;
        }
    }
};