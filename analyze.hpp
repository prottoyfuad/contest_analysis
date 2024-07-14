
#ifndef ANALYZER_INCLUDED
#define ANALYZER_INCLUDED 1

#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <iomanip>
#include <algorithm>

#include "csv.hpp"
#include "math.hpp"
#include "team.hpp"
#include "data_loader.hpp"

// #include "debug.hpp"

namespace analyze {

double score_per_ac = 2500;
// should be strictly greater than the max penalty over all teams

std::vector<team> extract_regional_teams(int year) {
  assert(data::loaded());
  std::vector<std::string> names;
  {
    std::vector<std::string> pre = data::contests[data::contest_id(year, PRELI)]
                                   .get_ranks().get_column("name");
    std::vector<std::string> reg = data::contests[data::contest_id(year, MAIN)]
                                   .get_ranks().get_column("name");
    std::sort(pre.begin(), pre.end());
    for (const std::string& name : reg) {
      std::string school = data::contests[data::contest_id(year, MAIN)]
                                .get_ranks().get_cell("institution", "name", name);
      if (school == "IOI") {
        // debug_(name, school);
        continue;
      }
      int j = (int) (lower_bound(pre.begin(), pre.end(), name) - pre.begin());
      if (j >= (int) pre.size() || pre[j] != name) {
        // debug("bad", year, s);
        continue;
      }
      names.push_back(name);
    }
    // debug_(year, pre.size(), reg.size(), names.size());
  }
  int K = (int) names.size();
  std::vector<team> teams(K);
  for (int i = 0; i < K; i++) {
    teams[i].update_rank(PRELI, data::contests[data::contest_id(year, PRELI)]
                               .get_ranks().get_row("name", names[i]) );
    teams[i].update_rank(MAIN, data::contests[data::contest_id(year, MAIN)]
                               .get_ranks().get_row("name", names[i]) );
  }
  return teams;
}

std::vector<double> main_preli_correlation() {
  assert(data::loaded());
  std::vector<double> ans(data::years);
  for (int t = 0; t < data::years; t++) {
    int year = data::begin_year + t;
    std::vector<team> teams = extract_regional_teams(year);
    int K = (int) teams.size();
    std::vector<double> pre(K), reg(K);
    for (int i = 0; i < K; i++) {
      pre[i] = teams[i].eval_score(PRELI, score_per_ac);
      reg[i] = teams[i].eval_score(MAIN, score_per_ac);
    }
    assert(pre != reg);
    ans[t] = correlation(pre, reg);
    
    std::ofstream fout("res/out/" + std::to_string(year) + ".txt",
                      std::ofstream::out | std::ofstream::trunc);
    assert(fout.is_open());
    fout << std::fixed << std::setprecision(3) << ans[t] * 100 << ' ';
    fout << data::contests[data::contest_id(year, PRELI)].problem_count() * score_per_ac << ' ';
    fout << data::contests[data::contest_id(year, MAIN)].problem_count() * score_per_ac << '\n';
    for (int i = 0; i < K; i++) {
      std::string str = teams[i].get(MAIN, "name");
      assert(str == teams[i].get(PRELI, "name"));
      for (char& c : str) {
        if (c == ' ') {
          c = '_';
        }
      }
      fout << str << ' ' << pre[i] << ' ' << reg[i] << '\n';
    }
    fout.close();
  }
  return ans;
}

std::vector<std::vector<std::tuple<std::string, double, double>>> get_outliers(int cnt = 3) {
  assert(data::loaded());
  std::vector<std::vector<std::tuple<std::string, double, double>>> ans(data::years);
  for (int t = 0; t < data::years; t++) {
    int year = data::begin_year + t;
    std::vector<team> teams = extract_regional_teams(year);
    
    auto eval = [&](double d) {
      std::vector<std::tuple<std::string, double, double>> now;
      for (const team& te : teams) {
        double psc = te.eval_score(PRELI, score_per_ac);
        double rsc = te.eval_score(MAIN, score_per_ac);
        if (rsc + d < psc) {
          now.emplace_back(te.get(MAIN, "name"), psc, rsc);
        }
      }
      return now;
    };

    for (double l = 0, r = 25'000; l + 1 < r; ) {
      double d = 0.5 * (l + r);
      auto now = eval(d);
      if ((int) now.size() < cnt) {
        r = d;
      } else {
        if (!ans[t].empty()) assert(now.size() <= ans[t].size());
        ans[t] = now;
        l = d;
      }
    }
    std::sort(ans[t].begin(), ans[t].end(), 
        [](const std::tuple<std::string, double, double>& x, 
           const std::tuple<std::string, double, double>& y) {
      return std::get<1>(x) - std::get<2>(x) > std::get<1>(y) - std::get<2>(y);
    });
    
    std::ofstream fout("res/out/sus" + std::to_string(year) + ".txt",
                      std::ofstream::out | std::ofstream::trunc);
    assert(fout.is_open());
    for (auto [str, pre, reg] : ans[t]) {
      for (char& c : str) {
        if (c == ' ') {
          c = '_';
        }
      }
      fout << str << ' ' << pre << ' ' << reg << '\n';
    }
    fout.close();
  }
  return ans;
}

void attempts_and_ranks() {
  std::map<std::string, int> verdtype;
  std::vector<std::string> vnames;
  auto verd_id = [&](const std::string& s) {
    if (verdtype.find(s) == verdtype.end()) {
      verdtype[s] = vnames.size();
      vnames.push_back(s); 
    }
    return verdtype[s];
  };
  verd_id("Accepted");
  verd_id("Wrong answer");
  verd_id("Runtime error");
  verd_id("Compilation Error");
  std::vector<std::vector<std::map<int, int>>> all(data::years);
  std::vector<std::vector<std::vector<bool>>> all_ac(data::years);
  std::vector<std::vector<int>> ac_cnt(data::years);
  std::vector<std::vector<std::pair<long long, int>>> vtime(data::years);
  for (int t = 0; t < data::years; t++) {
    int year = data::begin_year + t;
    std::vector<team> teams = extract_regional_teams(year);
    int K = (int) teams.size();
    all[t].resize(K);
    all_ac[t].resize(K);
    ac_cnt[t].resize(K);
    int pcnt = data::contests[data::contest_id(year, MAIN)].problem_count();
    const CSV& verd = data::contests[data::contest_id(year, MAIN)].get_verdicts();
    for (int i = 0; i < K; i++) {
      all_ac[t][i].assign(pcnt, false);
      const team& te = teams[i];
      const std::string& auth = te.get(MAIN, "name");
      std::vector<std::vector<std::pair<std::string, std::string>>> runs = verd.get_all_rows("author", auth);
      for (const auto& run : runs) {
        int pos = 0;
        while (pos < (int) run.size() && run[pos].first != "practice") pos++;
        assert(pos < (int) run.size());
        if (run[pos].second[0] == '1') {
          continue;
        }
        pos = 0;
        while (pos < (int) run.size() && run[pos].first != "problem") pos++;
        assert(pos < (int) run.size());
        int p = run[pos].second[0] - 'A';
        assert(0 <= p && p < pcnt);
        pos = 0;
        while (pos < (int) run.size() && run[pos].first != "status") pos++;
        assert(pos < (int) run.size());
        int v = verd_id(run[pos].second.substr(1));
        if (v == 0) {
          all_ac[t][i][p] = 1;
        }
        all[t][i][v] += 1;
        pos = 0;
        while (pos < (int) run.size() && run[pos].first != "time") pos++;
        assert(pos < (int) run.size());
        long long at = stoll(run[pos].second);
        vtime[t].emplace_back(at, v);
      }
      std::sort(vtime[t].begin(), vtime[t].end());
      ac_cnt[t][i] = 0;
      for (int j = 0; j < pcnt; j++) {
        ac_cnt[t][i] += all_ac[t][i][j];
        if (all[t][i][0] > 0) {
          all[t][i][0]--;
        }
      }
    }
  }
  //debug(vnames);
  int V = (int) vnames.size();
  for (int t = 0; t < data::years; t++) {
    std::ofstream fout("res/out/vc" + std::to_string(data::begin_year + t) + ".txt",
                      std::ofstream::out | std::ofstream::trunc);
    assert(fout.is_open());
    int K = all[t].size();
    for (int i = 0; i < K; i++) {
      fout << i + 1 << ' ' << ac_cnt[t][i];
      for (int j = 0; j < V; j++) {
        int cc = 0;
        if (all[t][i].find(j) != all[t][i].end()) {
          cc = all[t][i][j];
        }
        fout << ' ' << cc;
      }
      fout << '\n';
    }
    fout.close();
  }
  const int T = 3'600'000, P = 5;
  for (int t = 0; t < data::years; t++) {
    std::vector phase(P, std::vector<int> (V));
    assert(!vtime[t].empty());
    long long beg = vtime[t].front().first, eds = vtime[t].back().first;
    if (!(eds < beg + T * P)) {
      // debug_(beg, eds, (eds - beg) / T);
      assert(false);
    }
    for (auto [w, v] : vtime[t]) {
      long long k = (w - beg) / T;
      assert(k < P && v < V);
      phase[k][v] += 1;
    }
    std::ofstream fout("res/out/ph" + std::to_string(data::begin_year + t) + ".txt",
                      std::ofstream::out | std::ofstream::trunc);
    assert(fout.is_open());
    for (const auto& p : phase) {
      for (int x : p) {
        fout << x << ' ';
      }
      fout << '\n';
    }
    fout.close();
  }
}

void private_vs_public() {
  std::vector got(2, std::vector<double> (data::all_tags.size()));
  std::vector all(2, std::vector<double> (data::all_tags.size()));
  for (int y = 0; y < data::years; y++) {
    for (int t = 0; t < 2; t++) {
      int pc = (int) data::con_tags[y * 2 + t].size();
      if (pc == 0) {
        continue;
      }
      assert(pc == data::contests[y * 2 + t].problem_count());
      int tc = data::contests[y * 2 + t].team_count();
      for (int i = 0; i < tc; i++) {
        const std::string& insname 
          = data::contests[y * 2 + t].get_ranks().get_cell_by_index(i, "institution");
        assert(data::insts_tag.find(insname) != data::insts_tag.end());
        const std::string& instag = data::insts_tag[insname];
        assert(data::insts_type.find(instag) != data::insts_type.end());
        int instype = data::insts_type[instag];
        if (instype != PUBLIC && instype != PRIVATE) {
          continue;
        }
        std::string pname(2, '1');
        for (int j = 0; j < pc; j++) {
          pname[0] = 'a' + j; 
          const std::string& status
            = data::contests[y * 2 + t].get_ranks().get_cell_by_index(i, pname);
          int ac = 0;
          if (status != "-") {
            for (char c : status) assert(c >= '0' && c <= '9');
            ac = 1;
          }
          for (int tag : data::con_tags[y * 2 + t][j]) {
            all[instype][tag] += 1;
            got[instype][tag] += ac;
          }
        }
      }
    }
  }
  std::vector<std::tuple<std::string, double, double>> res;
  for (int i = 0; i < (int) data::all_tags.size(); i++) {
    std::string s = data::all_tags[i];
    if (s == "giveaway") {
      continue;
    }
    double a = got[PRIVATE][i] / all[PRIVATE][i] * 100;
    double b = got[PUBLIC][i] / all[PUBLIC][i] * 100;
    if (std::min(a, b) < 1.0) continue;
    res.emplace_back(s, a, b);
  }
  std::sort(res.begin(), res.end(), [](const std::tuple<std::string, double, double>& a,
                                       const std::tuple<std::string, double, double>& b) {
      auto [s1, a1, b1] = a;
      auto [s2, a2, b2] = b;
      auto m1 = std::min(a1, b1), m2 = std::min(a2, b2);
      auto mx1 = std::max(a1, b1), mx2 = std::max(a2, b2);
      if (std::abs(m1 - m2) < 3.0 && std::abs(m1 - m2) < std::abs(mx1 - mx2)) return mx1 > mx2;
      return m1 > m2;
    }
  );
  {
    auto res2 = res;
    int k = res.size(), j = 0;
    for (int p = 0; p < 2; p++) {
      int i = p;
      while (i < k) res[j++] = res2[i], i += 2;
    }
    std::rotate(res.begin(), res.begin() + 3, res.end());
  }
  std::ofstream fout("res/tags/all_spider.txt", std::ofstream::out | std::ofstream::trunc);
  if(!(fout.is_open())) {
    debug("BBAD");
    assert(0);
  }
  for (const auto& [s, a, b] : res) fout << s << '\n';
  fout.close();
  fout.open("res/out/pri_tag_got.txt", std::ofstream::out | std::ofstream::trunc);
  assert(fout.is_open());
  for (const auto& [s, a, b] : res) fout << a << '\n';
  fout.close();
  fout.open("res/out/pub_tag_got.txt", std::ofstream::out | std::ofstream::trunc);
  assert(fout.is_open());
  for (const auto& [s, a, b] : res) fout << b << '\n';
  fout.close();
  for (int i = 0; i < (int) data::all_tags.size(); i++) {
    std::string s = data::all_tags[i];
    while ((int) s.length() < 15) s = " " + s;
    assert((int) s.length() == 15);
    debug_(s, "pub, pri, dif =", 
           got[PUBLIC][i] / all[PUBLIC][i] * 100,
           got[PRIVATE][i] / all[PRIVATE][i] * 100,
           got[PUBLIC][i] / all[PUBLIC][i] * 100 - got[PRIVATE][i] / all[PRIVATE][i] * 100
        );
  }
}

} // end of namespace analyze
#endif

