
#ifndef CSV_INCLUDED
#define CSV_INCLUDED 1

#include <map>
#include <vector>
#include <cassert>
#include <fstream>
#include <algorithm>
#include <functional>

#include "debug.hpp"

class CSV {
  int _N, _ptr;
  std::string _line, _word;

  bool new_line(std::fstream& fin) {
    if (!std::getline(fin, _line)) {
      return 0;
    }
    for (int i = 0; i < 2; i++) {
      while (!_line.empty() && _line.back() == ' ') {
        _line.pop_back();
      }
      std::reverse(_line.begin(), _line.end());
    }
    _N = (int) _line.length();
    _ptr = 0;
    return 1;
  }

  bool get() {
    if (_ptr >= _N) {
      return 0;
    }
    _word.clear();
    if (0 < _ptr && _ptr < _N) {
      assert(_line[_ptr++] == ',');
    }
    int quote = 0;
    while (_ptr < _N) {
      if (_line[_ptr] == ',' && quote % 2 == 0) {
        break;
      }
      quote += _line[_ptr] == '"';
      _word += _line[_ptr++];
    }
    assert(quote % 2 == 0);
    return 1;
  }

  std::vector<std::string> cols;
  std::vector<std::vector<std::string>> rows;
public:
  CSV() {}
  CSV(const std::string& file) {
    read(file);
  }
  void read(const std::string& file) {
    std::fstream fin;
    fin.open(file);
    assert(fin.is_open());
    assert(new_line(fin));
    while (get()) {
      for (char& c : _word) {
        if (c >= 'A' && c <= 'Z') {
          c += 32;
        }
      }
      cols.push_back(_word);
    }
    int m = (int) cols.size();
    for (int i = 0; i < m; i++) {
      if (cols[i].empty()) {
        assert(i > 0 && (int) cols[i - 1].length() == 1 
                     && cols[i - 1][0] >= 'a' && cols[i - 1][0] <= 'z');
        cols[i] = cols[i - 1] + "1";
      }
    }
    /*
     * if score == 0, then there is a problem with that row in the rank_csv
     * there are more entries than the number of columns
     * those might be some practice submissions or something else
     * which might be needed to be handled later
     */
    int score_at = column_stoi("score");
    for (int i = 0; new_line(fin); i++) {
      rows.emplace_back(m);
      int j;
      for (j = 0; j < m; j++) {
        if (!get()) {
          assert(score_at < j && rows[i][score_at] == "0");
          break;
        }
        rows[i][j] = _word;
      }
      if (get() || j < m) {
        assert(-1 < score_at && score_at < j);
        // assert(rows[i][score_at] == "0"); this one fails
        // debug(i, rows[i]);
        // debug("bad", file, i);
        if (j >= m) {
          int bad_cnt = 0;
          do {
            bad_cnt += 1;
            assert(_word == "0" || _word == "-");
          } while (get());
          assert(bad_cnt == 2);
        }
      }
      assert(!get());
    }
    fin.close();
  }

  int row_count() const {
    return (int) rows.size();
  }

  int column_count() const {
    return (int) cols.size();
  }

  const std::vector<std::string>& operator[] (int i) const {
    assert(0 <= i && i < (int) rows.size());
    return rows[i];
  }

  std::string column_name(int x) const {
    assert(0 <= x && x < (int) cols.size());
    return cols[x];
  }

  int column_stoi(const std::string& s) const {
    for (int i = 0; i < (int) cols.size(); i++) {
      if (cols[i] == s) {
        return i;
      }
    }
    return -1;
  }

  void sort(const std::function<bool(const std::vector<std::string>&, 
                                     const std::vector<std::string>&)>& fun) {
    std::sort(rows.begin(), rows.end(), fun);
  }

  std::vector<std::string> get_column(int j) const {
    if (!(0 <= j && j < (int) cols.size())) {
      debug("bad", j);
      assert(0);
    }
    std::vector<std::string> ans(row_count());
    for (int i = 0; i < row_count(); i++) {
      ans[i] = rows[i][j];
    }
    return ans;
  }

  std::vector<std::string> get_column(const std::string& s) const {
    return get_column(column_stoi(s));
  }

  std::vector<std::string> get_row_values(int i) const {
    assert(0 <= i && i < row_count());
    return rows[i];
  }

  std::vector<std::pair<std::string, std::string>> get_row(int i) const {
    assert(0 <= i && i < row_count());
    std::vector<std::pair<std::string, std::string>> ans(column_count());
    for (int j = 0; j < column_count(); j++) {
      ans[j] = std::make_pair(cols[j], rows[i][j]);
    }
    return ans;
  }

  std::vector<std::pair<std::string, std::string>> get_row(int cid, const std::string& cval) const {
    assert(0 <= cid && cid < (int) cols.size());
    for (int i = 0; i < row_count(); i++) {
      if (rows[i][cid] == cval) {
        return get_row(i);
      }
    }
    return {};
  }

  std::vector<std::pair<std::string, std::string>> get_row(const std::string& cname, 
                                             const std::string& cval) const {
    return get_row(column_stoi(cname), cval);
  }

  std::vector<std::vector<std::pair<std::string, std::string>>> get_all_rows(
                                                  int cid, const std::string& cval) const {
    assert(0 <= cid && cid < (int) cols.size());
    std::vector<std::vector<std::pair<std::string, std::string>>> ans;
    for (int i = 0; i < row_count(); i++) {
      if (rows[i][cid] == cval) {
        ans.emplace_back(get_row(i));
      }
    }
    return ans;
  }

  std::vector<std::vector<std::pair<std::string, std::string>>> get_all_rows(
                   const std::string& cname, const std::string& cval) const {
    return get_all_rows(column_stoi(cname), cval);
  }

  std::string get_cell(int q, int x, const std::string& cval) const {
    assert(0 <= std::min(x, q) && std::max(x, q) < (int) cols.size());
    for (int i = 0; i < row_count(); i++) {
      if (rows[i][x] == cval) {
        return rows[i][q];
      }
    }
    return "";
  }

  std::string get_cell(const std::string& qcname,
                       const std::string& cname, const std::string& cval) const {
    return get_cell(column_stoi(qcname), column_stoi(cname), cval);
  }

  std::string get_cell_by_index(int row_index, int col_index) const {
    assert(0 <= col_index && col_index < (int) cols.size());
    assert(0 <= row_index && row_index < row_count());
    return rows[row_index][col_index];
  }

  std::string get_cell_by_index(int row_index, const std::string& cname) const {
    return get_cell_by_index(row_index, column_stoi(cname));
  }

  void update_cell_by_index(int row_index, int col_index,
                                  const std::string& value) {
    assert(0 <= col_index && col_index < (int) cols.size());
    assert(0 <= row_index && row_index < row_count());
    rows[row_index][col_index] = value;
  }

  void update_cell_by_index(int row_index, const std::string& cname, 
                                  const std::string& value) {
    update_cell_by_index(row_index, column_stoi(cname), value);
  }

  void update_column(int j, const std::function<std::string(const std::string&)>& fun) {
    assert(0 <= j && j < (int) cols.size());
    for (std::vector<std::string>& row : rows) {
      row[j] = fun(row[j]);
    }
  }

  void update_column(const std::string& s, 
                     const std::function<std::string(const std::string&)>& fun) {
    update_column(column_stoi(s), fun);
  }
  
  std::string eval_column(int j, const std::string& def,
                          const std::function<std::string(const std::string&, 
                                                          const std::string&)>& fun) const {
    assert(0 <= j && j < (int) cols.size());
    std::string ans = def;
    for (const std::vector<std::string>& row : rows) {
      ans = fun(ans, row[j]);
    }
    return ans;
  }
  
  std::string eval_column(const std::string& s, const std::string& def,
                          const std::function<std::string(const std::string&, 
                                                          const std::string&)>& fun) const {
    return eval_column(column_stoi(s), def, fun);
  }
};

#endif

