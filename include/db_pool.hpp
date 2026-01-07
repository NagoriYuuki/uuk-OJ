#pragma once

#include <mariadb/conncpp.hpp>
#include <iostream>
#include <mutex>
#include <memory>
#include <string>
#include <queue>
#include <condition_variable>

class DBPool
{
public:
    DBPool(const DBPool &) = delete;
    DBPool &operator=(const DBPool &) = delete;

    static DBPool &instance()
    {
        static DBPool dbpool(10);
        return dbpool;
    }

    std::shared_ptr<sql::Connection> getConnection()
    {
        std::unique_lock<std::mutex> lock(mtx);

        if (pool.empty() && cur_size < max_size)
        {
            try
            {
                auto con = std::shared_ptr<sql::Connection>(driver->connect(props));
                if (!con->isValid())
                    throw sql::SQLException("Connection is not valid");
                cur_size++;
                return std::shared_ptr<sql::Connection>(
                    con.get(), [this, con](sql::Connection *ptr)
                    {
                        std::unique_lock<std::mutex> lock(this->mtx);
                        this->pool.push(con);
                        this->cv.notify_one(); });
            }
            catch (sql::SQLException &e)
            {
                std::cerr << "SQL Error in expanding new connection: " << e.what() << std::endl;
                throw;
            }
        }

        cv.wait(lock, [this]
                { return !pool.empty(); });
        auto con = pool.front();
        pool.pop();

        return std::shared_ptr<sql::Connection>(
            con.get(), [this, con](sql::Connection *ptr)
            {
                std::unique_lock<std::mutex> lock(this->mtx);
                this->pool.push(con);
                this->cv.notify_one(); });
    }

private:
    sql::Driver *driver;
    std::queue<std::shared_ptr<sql::Connection>> pool;
    std::mutex mtx;
    std::condition_variable cv;
    constexpr static int min_size = 10;
    constexpr static int max_size = 50;
    int cur_size;
    sql::Properties props{{"hostName", "localhost:3307"},
                          {"userName", "uuk"},
                          {"password", "qweasd"},
                          {"schema", "uuk_oj"}};

    DBPool(int min = min_size, int max = max_size)
    {
        try
        {
            cur_size = 0;
            driver = sql::mariadb::get_driver_instance();
            for (int i = 0; i < min; i++)
            {
                std::shared_ptr<sql::Connection> con(driver->connect(props));
                if (!con->isValid())
                    throw sql::SQLException("Connection is not valid");
                pool.push(con);
                cur_size++;
            }
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in DBPool initialization: " << e.what() << std::endl;
            exit(1);
        }
    }
};