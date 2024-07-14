
#ifndef MATH_UTILS_INCLUDED
#define MATH_UTILS_INCLUDED 1

#include <cmath>
#include <vector>
#include <cassert>

template <typename T>
double mean(const std::vector<T>& a) {
  double ans = 0;
  for (const T& x : a) {
    ans += x;
  }
  return ans / (double) a.size();
}

template <typename T>
double standard_deviation(const std::vector<T>& a) {
  double m = mean(a);
  double ans = 0;
  for (const T& x : a) {
    double y = m - (double) x;
    ans += y * y;
  }
  return sqrt(ans);
}
  
template <typename T, typename U>
double covariance(const std::vector<T>& a, const std::vector<U>& b) {
  assert(a.size() == b.size());
  int n = (int) a.size();
  double ma = mean(a);
  double mb = mean(b);
  double ans = 0;
  for (int i = 0; i < n; i++) {
    double x = ma - (double) a[i];
    double y = mb - (double) b[i];
    ans += x * y;
  }
  return ans;
}

template <typename T, typename U>
double correlation(const std::vector<T>& a, const std::vector<U>& b) {
  double ans = covariance(a, b);
  ans /= standard_deviation(a);
  ans /= standard_deviation(b);
  return ans;
}

#endif

