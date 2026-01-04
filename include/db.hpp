#pragma once
#include <mariadb/conncpp.hpp>
#include <iostream>
#include <mutex>

class DB
{
public:
    DB(const DB &) = delete;
    DB &operator=(const DB &) = delete;
    static DB &instance()
    {
        static DB db;
        return db;
    }

    sql::Connection &getConnection()
    {
        return *con;
    }

    std::unique_ptr<sql::Statement> createStatement()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return std::unique_ptr<sql::Statement>(con->createStatement());
    }

private:
    sql::Driver *driver;
    std::unique_ptr<sql::Connection> con;
    std::mutex mtx;

    DB()
    {
        driver = sql::mariadb::get_driver_instance();
        sql::Properties props({{"hostName", "localhost:3307"},
                               {"userName", "uuk"},
                               {"password", "qweasd"},
                               {"schema", "uuk_oj"}});
        con.reset(driver->connect(props));
        std::cerr << "Database connected successfully!" << std::endl;
    }
    ~DB()
    {
        con->close();
        std::cerr << "Database connection closed." << std::endl;
    }
};