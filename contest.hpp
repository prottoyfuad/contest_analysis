
#ifndef CONTEST_INCLUDED
#define CONTEST_INCLUDED 1

#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>

#include "csv.hpp"

// #include "debug.hpp"

enum Contest_type { 
  PRELI = 0,
  MAIN = 1
};

class Contest {
private:
  int year;
  Contest_type type;

  CSV ranks, verdicts;
  int practice_runs;
public:
  Contest() {}
  Contest(int y, Contest_type t) {
    assign(y, t);
  }

  void assign(int y, Contest_type t) {
    year = y;
    type = t;
  }

  std::function<std::string(const std::string&)> name_rule = [](const std::string& s) {
    int n = (int) s.length();
    std::string t;
    for (int i = 0; i < n; i++) {
      if (s[i] != '"') {
        t += s[i] == '_' ? char(1) : (s[i] >= 'A' && s[i] <= 'Z' ? s[i] + 32 : s[i]);
      }
    }
    for (int i = 0; i < 2; i++) {
      while (!t.empty() && t.back() == ' ') {
        t.pop_back();
      }
      if (t.back() == (i ? '[' : ']')) {
        for (int j = (int) t.length() - 1; j > 0; j--) {
          if (t[j] == (i ? ']' : '[')) {
            if (t[j - 1] == ' ') {
              t.erase(t.begin() + j, t.end());
            }
            break;
          }
        }
      }
      while (!t.empty() && t.back() == ' ') {
        t.pop_back();
      }
      std::reverse(t.begin(), t.end());
    }
    assert(!t.empty());
    for (char& c : t) {
      if (c == char(1)) {
        c = ' ';
      }
    }
    return t;
  };

  void update_ranks(const CSV& _csv) {
    ranks = _csv;
    ranks.update_column("name", name_rule);
  }

  void update_verdicts(const CSV& _csv) {
    verdicts = _csv;
    verdicts.update_column("author", name_rule);
    int tat = verdicts.column_stoi("time");
    int pat = verdicts.column_stoi("practice");
    assert(tat != -1 && pat != -1);
    practice_runs = 0;
    for (int i = 0; i < verdicts.row_count(); i++) {
      if (verdicts[i][pat] == "1") {
        practice_runs += 1;
      } else {
        assert(verdicts[i][pat] == "");
      }
      assert(verdicts[i][tat].length() == verdicts[0][tat].length());
    }
    verdicts.sort([&](const std::vector<std::string>& lhs, const std::vector<std::string>& rhs) {
      return stoll(lhs[tat]) < stoll(rhs[tat]);
    });
    for (int i = 0; i < practice_runs; i++) {
      assert(verdicts[verdicts.row_count() - 1 - i][pat] == "1");
    }
  }

  std::string name() const {
    return std::to_string(year) + (type ? "_main" : "_preli");
  }

  int problem_count() const {
    char c = ranks.column_name(ranks.column_count() - 1).front();
    assert('a' <= c && c <= 'z');
    return (int) (c - 'a' + 1);
  }

  int team_count() const {
    return ranks.row_count();
  }

  int submission_count() const {
    return verdicts.row_count() - practice_runs;
  }

  const CSV& get_ranks() const {
    return ranks;
  }

  const CSV& get_verdicts() const {
    return verdicts;
  }
};

#endif

