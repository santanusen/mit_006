//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <functional>
#include <iostream>
#include <string>

// Count Sort:
// Sorts the N elements in 'in' and place the sorted result
// into 'out'.
// Maintains stability. Does so without using array of lists.
// Instead, creates an array of positions into the output array.
template <typename T, size_t N, typename KeyFunc>
void count_sort(const T (&in)[N], T (&out)[N], KeyFunc get_key) {
  // Figure out the range of keys in input.
  int kmin = get_key(in[0]);
  int kmax = get_key(in[0]);
  for (size_t i = 1; i < N; ++i) {
    int key = get_key(in[i]);
    kmin = std::min(key, kmin);
    kmax = std::max(key, kmax);
  }

  // Allocate a position array to fit all keys
  // in input key range.
  const size_t K = kmax - kmin + 1;
  size_t pos[K] = {0};

  // First: update key frequencies
  for (size_t i = 0; i < N; ++i)
    ++pos[get_key(in[i]) - kmin];

  // Second: in-place calculate key positions from frequencies
  size_t sum = N;
  for (int k = K - 1; k >= 0; --k) {
    sum -= pos[k];
    pos[k] = sum;
  }

  // Place the elements in output array as per position
  for (size_t i = 0; i < N; ++i) {
    out[pos[get_key(in[i]) - kmin]++] = in[i];
  }
}

// A key-value pair.
typedef std::pair<int, std::string> kv_t;

// Extract key from a key-value pair.
int kv_key(const kv_t &kv) { return kv.first; }

int main() {
  // Input: Unsoreted key-value pairs.
  kv_t arr[] = {{10, "ten"}, {6, "six"}, {5, "five"}, {6, "VI"}, {4, "four"}};

  std::cout << "Input:" << std::endl;
  for (const auto &p : arr)
    std::cout << "(" << p.first << ", " << p.second << "), ";
  std::cout << std::endl;

  constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
  kv_t sorted_arr[N];
  count_sort(arr, sorted_arr, kv_key);

  // Output: Stable soreted key-value pairs.
  std::cout << "Output:" << std::endl;
  for (const auto &p : sorted_arr)
    std::cout << "(" << p.first << ", " << p.second << "), ";
  std::cout << std::endl;

  return 0;
}
