//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <cstdint>
#include <iostream>

typedef uint64_t catalan_t;

// C_n = sum_{i = 0 ... n-1} (C_i * C_{n - i - 1})
void populate_catalan_numbers(catalan_t *memo, size_t N) {
  memo[0] = 1;
  if (N < 1)
    return;
  memo[1] = 1;

  for (size_t n = 2; n <= N; ++n) {
    catalan_t sum = 0;
    for (size_t i = 0; i < n; ++i) {
      sum += memo[i] * memo[n - i - 1];
    }
    memo[n] = sum;
  }
}

size_t populate_factorials(catalan_t *facts, size_t N) {
  facts[0] = 1;
  for (size_t i = 1; i <= N; ++i) {
    facts[i] = facts[i - 1] * i;
    if (facts[i] < facts[i - 1]) { // Overflow
      std::cout << "Factorial overflow at: " << i << std::endl;
      return i - 1;
    }
  }

  return N;
}

// C_n = (2n)! / ((n + 1)! n!)
void populate_catalan_numbers_method2(catalan_t *memo, size_t N) {
  catalan_t facts[2 * N + 1];
  auto nfact = populate_factorials(facts, 2 * N);
  size_t n;
  for (n = 0; 2 * n <= nfact; ++n) {
    memo[n] = facts[2 * n] / (facts[n + 1] * facts[n]);
  }

  if (n < N) { // Factorials overflowed: Can't compute further.
    memo[n] = 0;
  }
}

// C_n = 2C_{n-1} (2n - 1)/(n + 1) 
void populate_catalan_numbers_method3(catalan_t *memo, size_t N) {
  memo[0] = 1;

  for (size_t n = 1; n <= N; ++n) {
    if (memo[n - 1] % (n + 1) == 0) {
      memo[n] = 2 * (memo[n - 1] / (n + 1)) * (2 * n - 1);
    } else if ((2 * n - 1) % (n + 1) == 0) {
      memo[n] = 2 * memo[n - 1] * ((2 * n - 1) / (n + 1));
    } else {
      memo[n] = (2 * memo[n - 1] * (2 * n - 1)) / (n + 1);
    }
  }
}

void print_increasing_numbers(catalan_t *memo, size_t N) {
  for (size_t n = 0; n <= N; ++n) {
    if (n > 0 && memo[n] < memo[n - 1]) {
      std::cout << std::endl << "Overflow at index: " << n << std::endl;
      return;
    }
    std::cout << memo[n] << " ";
  }

  std::cout << std::endl;
}

int main() {
  const size_t N = 50;

  std::cout << "Method 1:" << std::endl;
  catalan_t memo[N + 1];
  populate_catalan_numbers(memo, N);
  print_increasing_numbers(memo, N);

  std::cout << "Method 2:" << std::endl;
  catalan_t memo2[N + 1];
  populate_catalan_numbers_method2(memo2, N);
  print_increasing_numbers(memo2, N);

  std::cout << "Method 3:" << std::endl;
  catalan_t memo3[N + 1];
  populate_catalan_numbers_method3(memo3, N);
  print_increasing_numbers(memo3, N);

  return 0;
}

