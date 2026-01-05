#pragma once

#include <mariadb/conncpp.hpp>

#include "problem_mapper.hpp"

#include <memory>
#include <optional>
#include <vector>

class ProblemService
{
public:
	explicit ProblemService(sql::Connection &con)
		: con(con), pm(con) {}

	std::optional<Problem> getById(const int id)
	{
		return pm.findById(id);
	}

	std::optional<int> create(const Problem &problem)
	{
		if (!pm.insert(problem))
			return std::nullopt;
		return lastInsertId();
	}

	bool update(const Problem &problem)
	{
		return pm.update(problem);
	}

	bool remove(const int id)
	{
		return pm.remove(id);
	}

	std::optional<std::vector<Problem>> listAll(int limit = 50, int offset = 0)
	{
		return pm.listAll(limit, offset);
	}

	// std::pair<i64, i64> countSub(const int id)
	// {
	// 	return pm.countSub(id);
	// }

private:
	sql::Connection &con;
	ProblemMapper pm;

	std::optional<int> lastInsertId()
	{
		try
		{
			auto stmt = std::unique_ptr<sql::Statement>(con.createStatement());
			auto res = std::unique_ptr<sql::ResultSet>(stmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
			if (!res->next())
				return std::nullopt;
			return res->getInt("id");
		}
		catch (sql::SQLException &e)
		{
			std::cerr << "SQL Error in lastInsertId: " << e.what() << std::endl;
			return std::nullopt;
		}
	}
};