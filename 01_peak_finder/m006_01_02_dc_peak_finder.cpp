//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>
#include <vector>

// Peak finding using divide and conquer.
// Start at the middle.
// If we are on the way down peak is somewhere at left.
// If we are on the way up peak is somewhere at right.
int dc_peak_finder(const std::vector<int> &a) {
  int N = a.size();
  int l = 0, r = N - 1;
  while (l <= r) {
    auto m = (l + r) / 2;
    if ((m > 0 && a[m] < a[m - 1])) {
      r = m - 1;
    } else if (m < N - 1 && a[m] < a[m + 1]) {
      l = m + 1;
    } else {
      return m;
    }
  }

  return -1;
}

int main() {
  std::vector<int> a1{0, 1, 2, 3, 4, 5, 6, 7};
  std::cout << dc_peak_finder(a1) << std::endl;

  std::vector<int> a2{0, 1, 2, 3, 4, 3, 2, 1};
  std::cout << dc_peak_finder(a2) << std::endl;

  std::vector<int> a3{7, 6, 5, 4, 3, 2, 1, 0};
  std::cout << dc_peak_finder(a3) << std::endl;

  return 0;
}
