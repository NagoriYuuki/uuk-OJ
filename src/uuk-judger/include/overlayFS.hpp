#pragma once

#include <string>
#include <filesystem>
#include <optional>
#include <iostream>

#include "./entities/task_info.hpp"

class OverlayFS
{
private:
    const std::string ROOTFS_PATH_ = "src/uuk-judger/rootfs";
    std::string base_path_;
    std::string upper_dir_;
    std::string work_dir_;
    std::string merge_dir_;
    TaskInfo task_info_;

    bool create_dir(const std::string &path, const std::string &name)
    {
        std::error_code ec;
        std::filesystem::create_directories(path, ec);

        if (ec)
        {
            std::cerr << "Failed to create " << name
                      << " for id: " << task_info_.submission_id
                      << "\nPath: " << path
                      << "\nError: " << ec.message()
                      << std::endl;
            return false;
        }
        return true;
    }

    bool create_new()
    {
        return create_dir(upper_dir_, "upper_dir") && create_dir(work_dir_, "work_dir") && create_dir(merge_dir_, "merge_dir");
    }

public:
    struct OFSPath
    {
        int status = 0;
        std::string upper_path;
        std::string lower_path;
        std::string work_path;
        std::string merge_path;
        std::string ROOTFS_PATH;

        OFSPath(int _status) : status(_status) {};

        OFSPath(const std::string &_lower_path, const std::string &_upper_path, const std::string &_work_path, const std::string &_merge_path, const std::string &_ROOTFS_PATH) : lower_path(_lower_path), upper_path(_upper_path), work_path(_work_path), merge_path(_merge_path), ROOTFS_PATH(_ROOTFS_PATH) {};
    };

    OFSPath prepare_workspace()
    {
        if (!create_new())
            return OFSPath(1);
        std::string code_file;
        if (task_info_.lang == "C++")
            code_file = "main.cpp";
        else if (task_info_.lang == "Python")
            code_file = "main.py";
        else if (task_info_.lang == "Java")
            code_file = "Main.Java";
        else
            return OFSPath(2);
        std::ofstream fout(upper_dir_ + "/" + code_file);
        if (!fout.is_open())
            return OFSPath(1);
        fout << task_info_.code;
        fout.close();
        return OFSPath(0, ROOTFS_PATH_, upper_dir_, work_dir_, merge_dir_);
    }

    void remove_workspace()
    {
        std::error_code ec;
        std::filesystem::remove_all(base_path_, ec);
    }

    std::string get_upper_path() const
    {
        return upper_dir_;
    }

    OverlayFS(const TaskInfo &task_info) : task_info_(task_info)
    {
        base_path_ = std::filesystem::absolute("../workspace/task" + std::to_string(task_info_.submission_id)).string();
        upper_dir_ = base_path_ + "/upper";
        work_dir_ = base_path_ + "/work";
        merge_dir_ = base_path_ + "/merge";
    }
};