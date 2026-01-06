#pragma once
#include <filesystem>
#include <string>
#include <iostream>
#include <regex>
#include <set>
#include <unordered_map>
#include <random>
#include <chrono>

#include <crow.h>

class ZipCheck
{
public:
    struct Result
    {
        bool judge;
        std::string message;
        std::string tc_path = "";
    };

    ZipCheck() = default;
    ~ZipCheck() = default;

    Result zipcheck(const crow::request &req, int problem_id)
    {
        crow::multipart::message filemsg(req);
        auto file = filemsg.get_part_by_name("file");
        if (!file.body.size())
            return {false, "No testcase file uploaded."};
        std::mt19937 rng(static_cast<unsigned long>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
        int rand_val = std::uniform_int_distribution<int>(100000, 999999)(rng);
        std::string final_path = "data/problems/p" + std::to_string(problem_id);
        std::filesystem::create_directories(final_path);
        std::string temp_path = final_path + "/temp" + std::to_string(rand_val);
        std::filesystem::create_directories(temp_path);
        std::string zip_path = temp_path + "/testcase.zip";
        std::ofstream fout(zip_path, std::ios::binary);
        fout << file.body;
        fout.close();
        std::string cmd = "unzip -o \"" + zip_path + "\" -d \"" + temp_path + "\"";

        int ret = system(cmd.c_str());

        if (ret != 0)
        {
            std::filesystem::remove(zip_path);
            return {false, "Failed to unzip."};
        }

        cleanDir(zip_path);
        std::error_code ec;
        std::filesystem::path temp_canon;
        try
        {
            temp_canon = std::filesystem::canonical(temp_path);
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::filesystem::remove_all(temp_path, ec);
            std::cerr << "Filesystem error in canonicalizing temp path: " << e.what() << std::endl;
            return {false, "Internal error resolving temp path."};
        }

        std::regex re(R"(^data(\d+)\.(in|out)$)");
        std::unordered_map<std::string, std::set<std::string>> mp;
        for (auto &i : std::filesystem::directory_iterator(temp_path))
        {
            if (i.is_directory())
            {
                cleanDir(temp_path);
                return {false, "Testcase zip should not contain directories."};
            }
            if (!i.is_regular_file())
            {
                cleanDir(temp_path);
                return {false, "Testcase zip contains invalid files."};
            }

            auto st = i.symlink_status(ec);
            if (std::filesystem::is_symlink(st))
            {
                cleanDir(temp_path);
                return {false, "Symbolic links are not allowed."};
            }

            std::string fname = i.path().filename().string();
            std::smatch match;
            if (!std::regex_match(fname, match, re))
            {
                cleanDir(temp_path);
                return {false, "Invalid filename found: " + fname + ". Invalid format."};
            }
            std::string num = match[1].str();
            std::string ext = match[2].str();
            mp[num].insert(ext);
            std::filesystem::path p;
            try
            {
                p = std::filesystem::canonical(i.path());
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                std::cerr << "Filesystem error in canonicalizing extracted file path: " << e.what() << std::endl;
                std::filesystem::remove_all(temp_path, ec);

                return {false, std::string("Invalid extracted entry: ") + i.path().string()};
            }
            auto temp_canon_str = temp_canon.string();
            auto p_str = p.string();
            if (p_str.rfind(temp_canon_str, 0) != 0)
            {
                std::filesystem::remove_all(temp_path, ec);
                return {false, "Extracted file is outside temp directory."};
            }
        }
        for (const auto &[val, st] : mp)
        {
            if (!(st.count("in") && st.count("out")))
            {
                cleanDir(temp_path);
                return {false, "Mismatched input/output file pair for data" + val + "."};
            }
        }
        if (mp.empty())
        {
            cleanDir(temp_path);
            return {false, "No valid testcase files found."};
        }

        for (auto &i : std::filesystem::directory_iterator(final_path))
        {
            if (i.is_directory() && i.path().filename().string() == std::string("temp" + std::to_string(rand_val)))
                continue;
            cleanDir(i.path());
        }
        for (auto &i : std::filesystem::directory_iterator(temp_path))
            std::filesystem::copy(i.path(), std::filesystem::path(final_path) / i.path().filename(), std::filesystem::copy_options::overwrite_existing);
        cleanDir(temp_path);
        return {true, "Testcases uploaded successfully.", final_path};
    }

private:
    static void cleanDir(const std::string &path)
    {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }
};