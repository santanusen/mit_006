//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>
#include <vector>

// Linearly examine each elemnt in 'a' if it is a peak.
int linear_peak_finder(const std::vector<int> &a) {
  int N = a.size();
  for (int i = 0; i < N; ++i) {
    if ((i > 0 && a[i] < a[i - 1]) || (i < N - 1 && a[i] < a[i + 1])) {
      continue;
    }
    return i;
  }

  return -1;
}

int main() {
  std::vector<int> a1{0, 1, 2, 3, 4, 5, 6, 7};
  std::cout << linear_peak_finder(a1) << std::endl;

  std::vector<int> a2{0, 1, 2, 3, 4, 3, 2, 1};
  std::cout << linear_peak_finder(a2) << std::endl;

  std::vector<int> a3{7, 6, 5, 4, 3, 2, 1, 0};
  std::cout << linear_peak_finder(a3) << std::endl;

  return 0;
}
