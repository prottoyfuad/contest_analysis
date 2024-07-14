
#ifndef DATA_LOADER_INCLUDED
#define DATA_LOADER_INCLUDED 1

#include <string>
#include <vector>
#include <cassert>
#include <sstream>

#include "csv.hpp"
#include "contest.hpp"
#include "debug.hpp"

enum Institute_type {
  PRIVATE = 0,
  PUBLIC = 1,
  NATIONAL = 2,
  POLYTECH = 3,
  IOI = 4,
  ABROAD = 5,
  NOT_FOUND = 6
};

namespace data {

int years, begin_year = -1;
std::vector<Contest> contests;

bool loaded() {
  return begin_year != -1;
}

const int ins_type = 7;
std::vector<std::string> all_insts;
std::vector<std::vector<std::string>> insts;
std::map<std::string, std::string> insts_tag;
std::map<std::string, int> insts_type;

void read_institutions() {
  insts.clear();
  insts_tag.clear();
  insts_type.clear();
  all_insts.clear();
  insts.resize(ins_type);
  std::ifstream fin("res/out/insraw.txt");
  assert(fin.is_open());
  int N;
  fin >> N;
  for (int i = 0; i < N; i++) {
    std::string a, b, c;
    fin >> a >> b >> c;
    assert((int) c.length() == 1 && c[0] >= '0' && c[0] < '0' + ins_type);
    int x = c[0] - '0';
    insts[x].push_back(b);
    all_insts.push_back(b);
    if (insts_tag.find(a) == insts_tag.end()) {
      insts_tag[a] = b;
    } else {
      assert(insts_tag[a] == b);
    }
    if (insts_type.find(b) == insts_type.end()) {
      insts_type[b] = x;
    } else {
      assert(insts_type[b] == x);
    }
  }
  fin.close();
  for (int i = 0; i < ins_type; i++) {
    sort(insts[i].begin(), insts[i].end());
    insts[i].resize(unique(insts[i].begin(), insts[i].end()) - insts[i].begin());
  }
  sort(all_insts.begin(), all_insts.end());
  all_insts.resize(unique(all_insts.begin(), all_insts.end()) - all_insts.begin());
  debug(all_insts.size());
  for (int i = 0; i < ins_type; i++) {
    debug(insts[i].size());
  }
}

std::vector<std::string> all_tags;
std::vector<std::vector<std::vector<int>>> con_tags;

int tag_id(const std::string& s) {
  int x = lower_bound(all_tags.begin(), all_tags.end(), s) - all_tags.begin();
  if (x >= (int) all_tags.size()) {
    debug(s);
    debug("!tag");
    assert(0);
  }
  return x;
}

void read_tags() {
  std::ifstream fin("res/tags/all.txt");
  assert(fin.is_open());
  int N;
  fin >> N;
  all_tags.resize(N);
  for (int i = 0; i < N; i++) {
    fin >> all_tags[i];
  }
  fin.close();
  debug(all_tags);
  assert(is_sorted(all_tags.begin(), all_tags.end()));
  con_tags.clear();
  con_tags.resize(years * 2);
  for (int y = 0; y < years; y++) {
    for (int t = 0; t < 2; t++) {
      std::string fname = "res/tags/" + std::to_string(y + begin_year) + 
                       ((Contest_type) t == PRELI ? "p" : "r") +  ".txt";
      fin.open(fname);
      int pc;
      fin >> pc;
      // debug(fname, pc);
      if (pc > 0) con_tags[y * 2 + t].resize(pc);
      for (int i = 0; i < pc; i++) {
        std::string s;
        while (s.empty()) getline(fin, s);
        std::stringstream strin(s);
        strin >> s;
        if (!((int) s.length() == 1 && s[0] == ('a' + i))) {
          debug(s);
          debug("bad");
          assert(0);
        }
        while (strin >> s) {
          con_tags[y * 2 + t][i].push_back(tag_id(s));
        }
        // debug(i, con_tags[y * 2 + t][i]);
        assert(!con_tags[y * 2 + t][i].empty());
      }
      assert(fin.is_open());
      fin.close();
    }
  }
}

void fix_institution_names(CSV& rank) {
  int K = (int) rank.get_column("name").size();
  assert(K == (int) rank.get_column("institution").size());
  for (int i = 0; i < K; i++) {
    std::string tname = rank.get_cell_by_index(i, "name");
    std::string tinst = rank.get_cell_by_index(i, "institution");
    for (char& c : tname) if (c >= 'A' && c <= 'Z') c += 32;
    for (char& c : tinst) {
      if (c >= 'A' && c <= 'Z') c += 32;
      if (c == ' ') c = '_';
    }
    auto ioi_at = tname.find("ioi");
    int err = 0;
    std::string ins, ins1, ins2;
    if (ioi_at != std::string::npos) {
      ins1 = "ioi";
    } else {
      int n = tname.size();
      int open = -1, close = -1;
      for (int j = 0; j < n; j++) {
        if (tname[j] == '[' && open == -1) {
          open = j;
        }
        if (tname[j] == ']' && open >= 0 && open < j - 2) {
          close = j;
          break;
        }
      }
      if (close == -1) {
        ins1 = "$prottoy_error";
        err += 1;
      } else {
        ins1 = tname.substr(open + 1, close - open - 1);
        for (char& c : ins1) if (c == ' ') c = '_';
      }
    }
    if ((int) tinst.size() < 2) {
      assert(tinst.empty() || tinst == "-");
      ins2 = "$prottoy_error";
      err += 1;
    } else {
      ins2 = tinst;
    }
    assert(err < 2);
    if (err == 1) {
      ins = (ins1 == "$prottoy_error" ? ins2 : ins1);
    } else {
      if (ins1 == "ioi") {
        ins = "ioi";
        if (ins2 != "ioi") {
          // debug(err, ins_fname, i, ins1, ins2);
          // debug_("write", ins1);
        }
      } else if (ins1 == ins2) {
        ins = ins1;
      } else {
        ins = ins2;
        // debug(err, ins_fname, i, ins1, ins2);
        // debug_("write", ins2);
      }
    }
    rank.update_cell_by_index(i, "institution", ins);
    assert(insts_tag.find(ins) != insts_tag.end());
  }
}

void read(int from, int to) {
  years = to - from + 1;
  begin_year = from;
  assert(years >= 0);
  contests.assign(years * 2, Contest());
  read_institutions();
  read_tags();
  for (int y = 0; y < years; y++) {
    for (int t = 0; t < 2; t++) {
      std::string rank_file = "./res/stats/";
      rank_file += std::to_string(begin_year + y);
      if (t == PRELI) {
        rank_file += "_preli";
      }
      std::string verdict_file = rank_file;
      rank_file += "_standings.csv";                   
      verdict_file += "_submissions.csv";
      CSV rank_csv = CSV(rank_file);
      CSV verdict_csv = CSV(verdict_file);
      fix_institution_names(rank_csv);
      contests[y * 2 + t].assign(begin_year + y, (Contest_type) t);
      contests[y * 2 + t].update_ranks(rank_csv);
      contests[y * 2 + t].update_verdicts(verdict_csv);
    }
  }
}

int contest_id(int year, Contest_type type) {
  assert(loaded());
  return (year - begin_year) * 2 + type;
}

} // End of namespace Data

#endif

