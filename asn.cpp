 
#define _GLIBCXX_DEBUG

#include <cassert>
#include <iostream>

#include "data_loader.hpp"
#include "analyze.hpp"

#include "debug.hpp"

constexpr int beg_year = 2014;
constexpr int end_year = 2020;
constexpr int tot_year = end_year - beg_year + 1;

void midterm() {
  std::vector<double> cor = analyze::main_preli_correlation();
  std::vector<std::vector<std::tuple<std::string, double, double>>> sus = analyze::get_outliers(2);
  assert((int) cor.size() == tot_year);
  assert((int) sus.size() == tot_year);

  int pc = 0, tc = 0, sc = 0;
  for (int i = 0; i < tot_year; i++) {
    const Contest& p = data::contests[data::contest_id(beg_year + i, PRELI)];
    const Contest& r = data::contests[data::contest_id(beg_year + i, MAIN)];
    int tt = 0;
    for (auto& c : {p, r}) {
      tt ^= 1;
      /*
      debug_(
        c.name(), 
        c.problem_count(), 
        c.team_count(),
        c.submission_count(),
        c.get_ranks().eval_column("penalty", "0",
                                  [](const std::string& a, const std::string& b) {
                                    return std::to_string(std::max(stoi(a), stoi(b)));
                                  })
      ); 
      */
      pc += c.problem_count();
      tc += c.team_count() * tt;
      sc += c.submission_count();
    }
    /*
    debug(sus[i].size());
    debug_("Outliers :");
    if (sus[i].empty()) debug_("\tnone!");
    else {
      for (auto& [name, psc, msc] : sus[i]) {
        debug_("\t" + name, "preli:", psc, "main :", msc, "diff :", psc - msc);
      }
    }
    debug_(beg_year + i, "correlation", cor[i]);
    */
  }
  analyze::attempts_and_ranks();
  debug(pc, tc, sc);
}

void finalex() {
  analyze::private_vs_public();
}
 
int main() {
  std::ios::sync_with_stdio(false);    
  
  data::read(beg_year, end_year);
  midterm();
  finalex();

  return 0;
}

