//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>
#include <list>

// Calculate the longest increasing subsequence using DP.
// Backward scan from the last number to the first.
// For each number check if it is greater than the previous numbers in the
// list. If it is, the longest increasing sequence starting from the previous
// number is the longer sequnce between the current longest sequence starting at
// the previous number and the longest increasing sequence starting from the
// current number plus the previous number.
void print_longest_increasing_subseq(const int *nums, size_t N) {
  // Index of the next number in the longest increasing subsequence starting
  // from the current number.
  size_t next[N];
  for (auto i = 0u; i < N; ++i)
    next[i] = N;

  // DP Table: Length of the longest increasing subsequence starting from the
  // current number.
  size_t seqlen[N];
  for (auto i = 0u; i < N; ++i)
    seqlen[i] = 1;

  // Index of the first number in the longest increasing subsequnce.
  size_t smax = N - 1;

  // Fill in the DP tables.
  for (auto i = N - 1; i > 0; --i) {
    for (ssize_t j = i - 1; j >= 0; --j) {
      if (nums[i] > nums[j]) {
        if (seqlen[i] + 1 > seqlen[j]) {
          seqlen[j] = seqlen[i] + 1;
          next[j] = i;

          if (seqlen[j] > seqlen[smax])
            smax = j;
        }
      }
    }
  }

  // Print the longest increasing subsequence.
  for (auto i = smax; i < N; i = next[i])
    std::cout << nums[i] << " ";
  std::cout << std::endl;
}

int main() {

  const size_t N = 15;
  int nums[N];

  // Create a list of random numbers.
  srand(time(0));
  for (auto i = 0u; i < N; ++i)
    nums[i] = rand() % 100;

  for (auto i = 0u; i < N; ++i)
    std::cout << nums[i] << " ";
  std::cout << std::endl;

  // Find the longest crazy subsequence.
  std::cout << "Longest increasing subsequence: " << std::endl;

  print_longest_increasing_subseq(nums, N);

  return 0;
}
