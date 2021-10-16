//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>
#include <string>

// Cost of multiplying 2 matrices of dimensions d1 X d2 and d2 X d3.
size_t mult_cost(size_t d1, size_t d2, size_t d3) {
  // In brute force matrix multiplication, calculation of each element of the
  // product requires d2 multiplications and the product is of d1 X d3
  // dimension.
  return (d1 * d2 * d3);
}

// Recursively build the parenthesized string following the pivots.
std::string parenthesize(size_t *pivot, size_t N, int i, int j) {
  // Base cases.
  if (i > j)
    return "";
  if (i == j)
    return std::string("A") + std::to_string(i);

  // Recurse.
  auto k = pivot[i * N + j];
  return std::string("(") + parenthesize(pivot, N, i, k - 1) + " " +
         parenthesize(pivot, N, k, j) + ")";
}

// Determine least cost matrix chain multiplication order using DP.
std::string dp_matrix_chain_mult_order(const size_t *dimension, size_t N) {
  // DP Table: Least cost of multiplying matrix chain from row index to column
  // index.
  size_t least_cost[N][N];

  // Pivot points to parenthesize. Serialized to 1D for ease of passing around.
  // If pivot[i][j] = k say, then parenthesize (A_i . A_k-1) (A_k . A_j).
  size_t pivot[N * N];

  // Seed values, no cost for multiplying a single matrix.
  for (auto i = 0U; i < N; ++i)
    least_cost[i][i] = 0;

  // Topological order: i ... j while distance 'd' between i and j increases
  // from 0 to N.
  for (auto d = 1u; d < N; ++d) {
    for (auto i = 0u; i < N - d; ++i) {
      auto j = i + d;
      // Try parenthesizing at all the points between i and j and remember what
      // gives the least cost.
      least_cost[i][j] = UINT64_MAX;
      pivot[i * N + j] = N;
      for (auto k = i + 1; k <= j; ++k) {
        auto cost = least_cost[i][k - 1] + least_cost[k][j] +
                    mult_cost(dimension[i], dimension[k], dimension[j + 1]);
        if (cost < least_cost[i][j]) {
          least_cost[i][j] = cost;
          pivot[i * N + j] = k;
        }
      }
    }
  }

  // Build the solution by following the pivot points.
  return parenthesize(pivot, N, 0, N - 1);
}

int main() {

  constexpr size_t N = 10; // Number of matrices.
  size_t dimension[N + 1]; // Matrix A_n is of dimension[n] X dimension[n + 1]

  // Create a list of random numbers.
  srand(time(0));
  for (auto i = 0u; i <= N; ++i)
    dimension[i] = 1 + rand() % 20;

  std::cout << "Matrix Dimensions: " << std::endl;
  for (auto i = 0u; i < N; ++i)
    std::cout << "[" << dimension[i] << " X " << dimension[i + 1] << "] ";
  std::cout << std::endl << std::endl;

  std::cout << "Optimal Multiplication Order: " << std::endl
            << dp_matrix_chain_mult_order(dimension, N) << std::endl;

  return 0;
}
