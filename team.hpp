
#ifndef CONTEST_TEAM_INCLUDED
#define CONTEST_TEAM_INCLUDED 1

#include <map>
#include <vector>
#include <cassert>

#include "contest.hpp"

class team {
private:
  int pcnt[2] = {-1, -1};
  std::vector<std::vector<std::pair<std::string, std::string>>> ranks;
  
  void calc_pcnt(Contest_type type) {
    pcnt[type] = 0;
    std::string pname(1, 'a');
    int f = 0;
    for (const auto& [x, y] : ranks[type]) {
      if (f) {
        if (!((int) x.size() == 2 && x[0] + 1 == pname[0] && x[1] == '1')) {
          debug(x);
          assert(0);
        }
        if (y != "-") {
          for (char c : y) assert(c >= '0' && c <= '9');
          pcnt[type]++;
        }
        f = 0;
      }
      if (x == pname) {
        f = 1;
        pname[0]++;
      }
    }
  }
public:
  team() : ranks(2) {}

  void update_rank(Contest_type type,
                   const std::vector<std::pair<std::string, std::string>>& rank) {
    ranks[type] = rank;
    calc_pcnt(type);
  }

  std::string get(Contest_type type, const std::string& key) const {
    for (const auto& [x, y] : ranks[type]) {
      if (x == key) {
        return y;
      }
    }
    assert(0);
  }

  double eval_score(Contest_type type, double score_per_ac = 3000, bool percent = false) const {
    assert(pcnt[type] != -1);
    double score = stoi(get(type, "score")) * score_per_ac;
    double fastscore = score_per_ac - stoi(get(type, "penalty"));
    assert(fastscore > 0.0);
    score += fastscore;
    if (!percent) {
      return score;
    }
    double total = score_per_ac * pcnt[type];
    return score / total;
  }
};

#endif

