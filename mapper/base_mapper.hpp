#pragma once
#include <mariadb/conncpp.hpp>
#include <optional>

template <typename ORMType>
class BaseMapper
{
public:
    virtual ~BaseMapper() = default;
    BaseMapper(sql::Connection &connection) : con(connection) {}
    virtual bool insert(const ORMType &obj) = 0;
    virtual bool update(const ORMType &obj) = 0;
    virtual bool remove(const int id) = 0;
    virtual std::optional<ORMType> findById(const int id) = 0;

protected:
    sql::Connection &con;
};