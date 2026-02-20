#pragma once

#include <string>
#include <vector>

class Config
{
public:
  using i64 = long long;

  std::string code_path;
  std::vector<std::string> args;
  int time_limit;
  i64 mem_limit;
  std::string input_path;
  std::string output_path;
  std::string work_dir;
  std::string overlay_lower;
  std::string overlay_upper;
  std::string overlay_work;
  std::string overlay_merge;
  bool is_judging;

  Config()
      : code_path(""),
        args({}),
        time_limit(1000),
        mem_limit(65536),
        input_path(""),
        output_path(""),
        work_dir(""),
        overlay_lower(""),
        overlay_upper(""),
        overlay_work(""),
        overlay_merge(""), is_judging(true) {};

  Config(const std::string &cp,
         const std::vector<std::string> &a,
         const int tl,
         const i64 ml,
         const std::string &ip,
         const std::string &op, const std::string &wd, const std::string &overlay_lower, const std::string overlay_upper, const std::string overlay_work, const std::string &overlay_merge, const bool is_judging)
      : code_path(cp),
        args(a),
        time_limit(tl),
        mem_limit(ml),
        input_path(ip),
        output_path(op),
        work_dir(wd),
        overlay_lower(overlay_lower),
        overlay_upper(overlay_upper),
        overlay_work(overlay_work),
        overlay_merge(overlay_merge), is_judging(is_judging) {};
};