#pragma once

#include <sw/redis++/redis++.h>
#include <iostream>
#include <memory>
#include <optional>

class RedisQueue
{
public:
    RedisQueue(const RedisQueue &) = delete;
    RedisQueue &operator=(const RedisQueue &) = delete;
    ~RedisQueue() = default;

    RedisQueue(const std::string &host = "127.0.0.1", int port = 6379)
    {
        sw::redis::ConnectionOptions con_opts;
        con_opts.host = host;
        con_opts.port = port;
        con_opts.keep_alive = true;

        try
        {
            redis = std::make_unique<sw::redis::Redis>(con_opts);
        }
        catch (const sw::redis::Error &e)
        {
            std::cerr << "Failed to connect to Redis: " << e.what() << std::endl;
            throw;
        }
    }

    void push(const std::string &que, const std::string &val)
    {

        try
        {
            redis->lpush(que, val);
        }
        catch (const sw::redis::Error &e)
        {
            std::cerr << "Failed to push to Redis queue: " << e.what() << std::endl;
            throw;
        }
    }

    std::optional<std::string> pop(const std::string &que)
    {
        try
        {
            auto res = redis->rpop(que);
            if (res.has_value())
                return res.value();
            else
                return std::nullopt;
        }
        catch (const sw::redis::Error &e)
        {
            std::cerr << "Failed to pop from Redis queue: " << e.what() << std::endl;
            throw;
        }
    }

private:
    std::unique_ptr<sw::redis::Redis> redis;
};