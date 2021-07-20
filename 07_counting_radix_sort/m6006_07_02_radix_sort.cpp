//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>

// Count Sort:
// Sorts the N elements in 'in' and place the sorted result
// into 'out'.
// Maintains stability. Does so without using array of lists.
// Instead, creates an array of positions into the output array.
template <typename T, typename KeyFunc>
void count_sort(const T *in, T *out, size_t N, KeyFunc get_key) {
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
  size_t pos[K];
  memset(pos, 0, K * sizeof(size_t));

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

template <typename UINT> void radix_sort_uint(UINT *arr, size_t N) {
  if (N < 2)
    return;

  // Count sort needs an auxiliary array
  UINT aux[N];

  // Start with the original array as input and the
  // auxiliary array as output.
  UINT *in = arr, *out = aux;

  // Use N as the base
  const UINT base = static_cast<UINT>(N);

  // Iterate till all the digits of the largest number are covered.
  const UINT max_num = *std::max_element(arr, arr + N);
  bool reached_msd = false;
  for (UINT div = 1; !reached_msd; div *= base) {
    count_sort(in, out, base, [div, base](const UINT &num) -> int {
      return ((num / div) % base);
    });

    // Check if MSD is reached.
    reached_msd = ((max_num / div) == 0);

    // Swap the input and the output arrays for the next iteration.
    UINT *tmp = in;
    in = out;
    out = tmp;
  }

  // Copy output back to the original array if necessary
  if (out != arr) {
    for (size_t i = 0; i < N; ++i)
      arr[i] = out[i];
  }
}

int main() {
  int arr[] = {14610, 21312, 16489, 200, 5093, 30301, 1999};
  constexpr size_t N = sizeof(arr) / sizeof(arr[0]);
  radix_sort_uint(arr, N);

  for (const auto &i : arr)
    std::cout << i << " ";
  std::cout << std::endl;

  return 0;
}
