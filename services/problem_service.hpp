#pragma once

#include <mariadb/conncpp.hpp>
#include <sw/redis++/redis++.h>

#include "problem_mapper.hpp"

#include <memory>
#include <optional>
#include <vector>
#include <filesystem>

class ProblemService
{
public:
	explicit ProblemService(sql::Connection &con, std::shared_ptr<sw::redis::Redis> redis = nullptr)
		: con(con), pm(con), redis(redis)
	{
		if (redis)
		{
			is_redis_enabled = true;
			std::cerr << "Redis caching enabled for ProblemService" << std::endl;
		}
	}

	std::optional<Problem> getById(const int id)
	{
		std::string key = "problem:detail:" + std::to_string(id);
		if (is_redis_enabled)
		{
			try
			{
				auto val = redis->get(key);
				if (val.has_value())
				{
					auto json = crow::json::load(val.value());
					Problem problem(
						json["id"].i(),
						std::string(json["title"].s()),
						json["time_limit"].i(),
						json["mem_limit"].i(),
						std::string(json["description"].s()),
						std::string(json["sample_input"].s()),
						std::string(json["sample_output"].s()),
						std::string(json["created_time"].s()),
						std::string(json["tc_path"].s()),
						json["sub_count"].i(),
						json["ac_count"].i());
					return problem;
				}
			}
			catch (const std::exception &e)
			{
				std::cerr << "Failed to get problem detail from Redis: " << e.what() << std::endl;
			}
		}
		auto problem_opt = pm.findById(id);
		if (problem_opt.has_value() && is_redis_enabled)
		{
			crow::json::wvalue json;
			json["id"] = problem_opt->id;
			json["title"] = problem_opt->title;
			json["time_limit"] = problem_opt->time_limit;
			json["mem_limit"] = problem_opt->mem_limit;
			json["description"] = problem_opt->description;
			json["sample_input"] = problem_opt->sample_input;
			json["sample_output"] = problem_opt->sample_output;
			json["created_time"] = problem_opt->created_time;
			json["tc_path"] = problem_opt->tc_path;
			json["sub_count"] = problem_opt->sub_count;
			json["ac_count"] = problem_opt->ac_count;
			try
			{
				redis->set(key, json.dump(), std::chrono::seconds(30));
			}
			catch (const std::exception &e)
			{
				std::cerr << "Failed to cache problem detail in Redis: " << e.what() << std::endl;
			}
		}
		return problem_opt;
	}

	std::optional<int> create(const Problem &problem)
	{
		if (!pm.insert(problem))
			return std::nullopt;
		auto new_id_opt = lastInsertId();
		return new_id_opt;
	}

	bool update(const Problem &problem)
	{
		bool judge = pm.update(problem);
		if (judge && is_redis_enabled)
		{
			std::string key = "problem:detail:" + std::to_string(problem.id);
			try
			{
				redis->del(key);
			}
			catch (const std::exception &e)
			{
				std::cerr << "Failed to invalidate problem cache in Redis: " << e.what() << std::endl;
			}
		}
		return judge;
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
			if (is_redis_enabled)
			{
				try
				{
					std::string key = "problem:detail:" + std::to_string(id);
					redis->del(key);
				}
				catch (const std::exception &e)
				{
					std::cerr << "Failed to invalidate problem cache in Redis: " << e.what() << std::endl;
				}
			}
		}
		return judge;
	}

	std::optional<std::vector<Problem>> listAll(int limit = 50, int offset = 0)
	{
		std::string key = "problem:list:" + std::to_string(limit) + ":" + std::to_string(offset);
		if (is_redis_enabled)
		{
			try
			{
				auto val = redis->get(key);
				if (val.has_value())
				{
					auto json = crow::json::load(val.value());
					std::vector<Problem> problems;
					for (const auto &item : json)
					{
						problems.emplace_back(
							item["id"].i(),
							std::string(item["title"].s()),
							item["time_limit"].i(),
							item["mem_limit"].i(),
							"",
							"",
							"",
							std::string(item["created_time"].s()),
							"",
							item["sub_count"].i(),
							item["ac_count"].i());
					}
					return problems;
				}
			}
			catch (const std::exception &e)
			{
				std::cerr << "Failed to get problem list from Redis: " << e.what() << std::endl;
			}
		}
		auto problems_opt = pm.listAll(limit, offset);
		if (problems_opt.has_value() && is_redis_enabled)
		{
			try
			{
				crow::json::wvalue json;
				std::size_t idx = 0;
				for (const auto &problem : problems_opt.value())
				{
					crow::json::wvalue item;
					item["id"] = problem.id;
					item["title"] = problem.title;
					item["time_limit"] = problem.time_limit;
					item["mem_limit"] = problem.mem_limit;
					item["created_time"] = problem.created_time;
					item["sub_count"] = problem.sub_count;
					item["ac_count"] = problem.ac_count;
					json[idx++] = std::move(item);
				}

				redis->set(key, json.dump(), std::chrono::seconds(10));
			}
			catch (const std::exception &e)
			{
				std::cerr << "Failed to cache problem list in Redis: " << e.what() << std::endl;
			}
		}
		return problems_opt;
	}

	// std::pair<i64, i64> countSub(const int id)
	// {
	// 	return pm.countSub(id);
	// }

	bool updateStat(const int id, const bool is_ac)
	{
		// auto judge = pm.updateStat(id, is_ac);
		// if (is_redis_enabled && judge)
		// {
		// 	std::string key = "problem:detail:" + std::to_string(id);
		// 	try
		// 	{
		// 		redis->del(key);
		// 	}
		// 	catch (const std::exception &e)
		// 	{
		// 		std::cerr << "Failed to invalidate problem cache in Redis: " << e.what() << std::endl;
		// 	}
		// }
		// return judge;
		return pm.updateStat(id, is_ac);
	}

private:
	sql::Connection &con;
	ProblemMapper pm;
	std::shared_ptr<sw::redis::Redis> redis;
	bool is_redis_enabled = false;

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