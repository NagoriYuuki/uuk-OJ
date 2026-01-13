#pragma once

#include <mariadb/conncpp.hpp>

#include "problem_mapper.hpp"

#include <memory>
#include <optional>
#include <vector>
#include <filesystem>

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
		bool judge = pm.remove(id);
		if (judge)
		{
			std::string tc_dir = "data/problems/p" + std::to_string(id);
			if (std::filesystem::exists(tc_dir))
				std::filesystem::remove_all(tc_dir);
			else
				std::cerr << "Testcase directory not found for problem " << id << std::endl;
		}
		return judge;
	}

	std::optional<std::vector<Problem>> listAll(int limit = 50, int offset = 0)
	{
		return pm.listAll(limit, offset);
	}

	// std::pair<i64, i64> countSub(const int id)
	// {
	// 	return pm.countSub(id);
	// }

	bool updateStat(const int id, const bool is_ac)
	{
		return pm.updateStat(id, is_ac);
	}

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