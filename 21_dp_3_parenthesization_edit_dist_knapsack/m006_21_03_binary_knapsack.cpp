//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iomanip>
#include <iostream>
#include <string>

#define MIN_WEIGHT 1
#define MAX_WEIGHT 10

#define MIN_PROFIT 1
#define MAX_PROFIT 100

struct item_t {
  size_t weight;
  size_t profit;
};

void knapsack_dp(size_t S, const item_t *items, size_t N) {
  size_t max_profit[N + 1][S + 1];

  for (auto n = 0u; n <= N; ++n) {   // Evaluate each item.
    for (auto w = 0u; w <= S; ++w) { // Remainig Capacity.
      if (n == 0 || w == 0) {
        max_profit[n][w] = 0;
      } else {
        // if item n is not included
        max_profit[n][w] = max_profit[n - 1][w];
        if (w >= items[n - 1].weight) {
          max_profit[n][w] =
              std::max(max_profit[n - 1][w],
                       max_profit[n - 1][w - items[n - 1].weight] +
                           items[n - 1].profit); // if item n is included
        }
      }
    }
  }

#if 0
  for (auto n = 0u; n <= N; ++n) {
    for (auto w = 0u; w <= S; ++w) {
      std::cout << std::setw(5) << max_profit[n][w] << " ";
    }
    std::cout << std::endl;
  }
#endif

  // Reconstruct solution
    auto n = N;
    auto w = S;
    while (w > 0 && n > 0) {
      if (max_profit[n][w] != max_profit[n - 1][w]) {
        std::cout << "Include: " << n - 1 << std::endl;
        w -= items[n - 1].weight;
      }
      --n;
    }

}

size_t rand_num(size_t from, size_t to) { return from + rand() % (to - from); }

int main() {

  constexpr size_t N = 10; // Number of items.
  item_t items[N];

  // Knapsack capacity.
  size_t S = (N * (MAX_WEIGHT + MIN_WEIGHT)) / 4;

  // Create a list of random items.
  srand(time(0));
  for (auto i = 0u; i < N; ++i) {
    items[i].weight = rand_num(MIN_WEIGHT, MAX_WEIGHT);
    items[i].profit = rand_num(MIN_PROFIT, MAX_PROFIT);
  }

  std::cout << "Knapsack Capacity: " << S << std::endl;
  std::cout << "Items:" << std::endl;

  std::cout << "Index : ";
  for (auto i = 0u; i < N; ++i)
    std::cout << std::setw(4) << i << " ";

  std::cout << std::endl << "Weight: ";
  for (auto i = 0u; i < N; ++i)
    std::cout << std::setw(4) << items[i].weight << " ";

  std::cout << std::endl << "Profit: ";
  for (auto i = 0u; i < N; ++i)
    std::cout << std::setw(4) << items[i].profit << " ";

  std::cout << std::endl << std::endl;

  knapsack_dp(S, items, N);

  return 0;
}
