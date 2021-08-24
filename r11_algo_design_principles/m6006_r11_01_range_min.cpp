//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>

#define INVALID_MIN UINT32_MAX

// Intarface class to calculate the minimum within a subrange of an array.
class range_min {

public:
  range_min(const uint32_t *_nums, uint32_t _N) : nums(_nums), N(_N) {}

  virtual ~range_min() {}

  virtual void pre_process() {}

  virtual uint32_t find_range_min(ssize_t low, ssize_t high) const = 0;

  uint32_t length() const { return N; }

protected:
  const uint32_t *nums;

  uint32_t N;
};

// Calculates the minimum within a subrange of an array using a
// divide-and-conquer method. It pre-calculates O(n*log(n)) subranges by
// recursively dividing the array into two subarray halves and saving, in a
// hash-map, the mins of the sub-ranges
// {(i, mid)} : start <= i <= mid; and {(mid+1, j)} mid+1 <= j <= end.
// While calculating the minimum for a sub-range (i, j), subsequent mid points
// of sub-arrays are calculated till a mid point is found such that i and j are
// at either side of mid. min(subrange_min(i, mid), subrange_min(mid+1, j))
// is the minimum for subrange (i, j).
class range_min_dc : public range_min {
protected:
  struct pair_hash {
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2> &p) const {
      return (std::hash<T1>{}(p.first) ^ std::hash<T2>{}(p.second));
    }
  };

  typedef std::unordered_map<std::pair<ssize_t, ssize_t>, uint32_t, pair_hash>
      range_min_table_t;

  range_min_table_t rtable;

  void populate_min_range_table_recurse(ssize_t low, ssize_t high) {
    // Base cases.
    if (low > high) {
      return;
    }
    if (low == high) {
      rtable[std::make_pair(low, high)] = nums[low];
      return;
    }

    ssize_t mid = (low + high) / 2;

    // Left half:
    // Pre-calculate min of sub-ranges (low, mid), (low+1, mid), ... (mid, mid)
    uint32_t lowest = nums[mid];
    for (auto i = mid; i >= low; --i) {
      lowest = std::min(lowest, nums[i]);
      rtable[std::make_pair(i, mid)] = lowest;
    }
    // Right half:
    // Pre-calculate min of sub-ranges (mid+1, mid+1), ... (mid+1, high-1),
    // (mid+1, high)
    lowest = nums[mid + 1];
    for (auto i = mid + 1; i <= high; ++i) {
      lowest = std::min(lowest, nums[i]);
      rtable[std::make_pair(mid + 1, i)] = lowest;
    }

    // Recurse.
    populate_min_range_table_recurse(low, mid);
    populate_min_range_table_recurse(mid + 1, high);
  }

public:
  range_min_dc(const uint32_t *_nums, uint32_t _N) : range_min(_nums, _N) {}

  virtual void pre_process() override {
    populate_min_range_table_recurse(0, N - 1);
  }

  virtual uint32_t find_range_min(ssize_t low, ssize_t high) const override {
    // Invalid ranges.
    if (low > high || low < 0 || high >= N) {
      return INVALID_MIN;
    }
    // Trivial range.
    if (low == high) {
      return nums[low];
    }

    ssize_t ll = 0, hh = N - 1;
    while (ll < hh) {
      auto mid = (ll + hh) / 2;
      if (low <= mid && high > mid) {
        // low and high are at either side of mid.
        // minimum of subrange mins of the subranges (low, mid) and
        // (mid+1, high) is the answer.
        auto itrl = rtable.find(std::make_pair(low, mid));
        if (itrl == rtable.end()) {
          assert(false);
          return INVALID_MIN;
        }
        auto itrr = rtable.find(std::make_pair(mid + 1, high));
        if (itrr == rtable.end()) {
          assert(false);
          return INVALID_MIN;
        }
        return std::min(itrl->second, itrr->second);
      } else if (high <= mid) {
        // Both low and high are on the left half; try there.
        hh = mid;
      } else {
        // Both low and high are on the right half; try there.
        ll = mid + 1;
      }
    }

    assert(false);
    return INVALID_MIN;
  }

  void dump_rtable(std::ostream &os) const {
    for (const auto &e : rtable)
      os << "[" << e.first.first << " : " << e.first.second
         << "] = " << e.second << std::endl;
  }
};

// Poor man's subrange min calculator.
class range_min_brute_force : public range_min {
public:
  range_min_brute_force(const uint32_t *_nums, uint32_t _N)
      : range_min(_nums, _N) {}

  virtual uint32_t find_range_min(ssize_t low, ssize_t high) const override {
    // Invalid ranges.
    if (low > high || low < 0 || high >= N) {
      return INVALID_MIN;
    }

    auto lowest = nums[low];
    for (auto i = low + 1; i <= high; ++i)
      lowest = std::min(lowest, nums[i]);

    return lowest;
  }
};

void print_nums(const uint32_t *nums, ssize_t N) {
  for (ssize_t i = 0; i < N; ++i)
    std::cout << nums[i] << " ";
  std::cout << std::endl;
}

#define A_BIG_PRIME_NUMBER 2147483647 // srand seed

double run_min_range_test(std::string msg, range_min &rmin) {
  std::cout << msg << ": length: " << rmin.length() << std::endl;

  auto t1 = std::chrono::high_resolution_clock::now();
  rmin.pre_process();

  auto t2 = std::chrono::high_resolution_clock::now();
  const auto N = rmin.length();
  for (ssize_t l = 0; l < N; ++l) {
    for (ssize_t h = l; h < N; ++h) {
      auto res = rmin.find_range_min(l, h);
      if (res == INVALID_MIN) {
        std::cout << "Invalid min for range (" << l << ", " << h << ") "
                  << std::endl;
      }
    }
  }
  auto t3 = std::chrono::high_resolution_clock::now();

  std::cout << "Preprocessing time: "
            << (std::chrono::duration<double, std::milli>(t2 - t1)).count()
            << std::endl;
  std::cout << "Test time: "
            << (std::chrono::duration<double, std::milli>(t3 - t2)).count()
            << std::endl;

  return std::chrono::duration<double, std::milli>(t3 - t1).count();
}

int main() {
  srand(A_BIG_PRIME_NUMBER);

  const ssize_t N = 10000;
  uint32_t nums[N];
  for (uint32_t i = 0; i < N; ++i)
    nums[i] = rand() % (4 * N);

  // print_nums(nums, N);

  // Test how much time finding min of all subranges take.
  // First test with N/10 elements and then with all N elements.
  // Check how dc and brute-force grow with this 10-fold growth
  // in the number of elements.

  range_min_dc rdc(nums, N / 10);
  auto t1_part = run_min_range_test("Divide-and-Conquer", rdc);
  range_min_dc rdc2(nums, N);
  auto t1_full = run_min_range_test("Divide-and-Conquer", rdc2);
  std::cout << "Growth: " << t1_full / t1_part << std::endl << std::endl;

  range_min_brute_force rbf(nums, N / 10);
  auto t2_part = run_min_range_test("Brute-Force", rbf);
  range_min_brute_force rbf2(nums, N);
  auto t2_full = run_min_range_test("Brute-Force", rbf2);
  std::cout << "Growth: " << t2_full / t2_part << std::endl;

  return 0;
}
