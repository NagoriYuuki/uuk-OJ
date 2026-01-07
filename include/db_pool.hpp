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

    DBPool(int n)
    {
        try
        {
            driver = sql::mariadb::get_driver_instance();
            sql::Properties props({{"hostName", "localhost:3307"},
                                   {"userName", "uuk"},
                                   {"password", "qweasd"},
                                   {"schema", "uuk_oj"}});
            for (int i = 0; i < n; i++)
            {
                std::shared_ptr<sql::Connection> con(driver->connect(props));
                if (!con->isValid())
                    throw sql::SQLException("Connection is not valid");
                pool.push(con);
            }
        }
        catch (sql::SQLException &e)
        {
            std::cerr << "SQL Error in DBPool initialization: " << e.what() << std::endl;
            exit(1);
        }
    }
};