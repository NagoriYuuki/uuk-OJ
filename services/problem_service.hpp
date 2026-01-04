#pragma once

#include <mariadb/conncpp.hpp>

#include "problem_mapper.hpp"

#include <memory>
#include <optional>

class ProblemService
{
public:
	explicit ProblemService(sql::Connection &con)
		: con(con), mapper(con) {}

	std::optional<Problem> getById(const int id)
	{
		return mapper.findById(id);
	}

	std::optional<int> create(const Problem &problem)
	{
		if (!mapper.insert(problem))
			return std::nullopt;
		return lastInsertId();
	}

	bool update(const Problem &problem)
	{
		return mapper.update(problem);
	}

	bool remove(const int id)
	{
		return mapper.remove(id);
	}

private:
	sql::Connection &con;
	ProblemMapper mapper;

	std::optional<int> lastInsertId()
	{
		try
		{
			auto stmt = std::unique_ptr<sql::Statement>(con.createStatement());
			auto rs = std::unique_ptr<sql::ResultSet>(stmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
			if (!rs->next())
				return std::nullopt;
			
			return rs->getInt("id");
		}
		catch (sql::SQLException &)
		{
			return std::nullopt;
		}
	}

};