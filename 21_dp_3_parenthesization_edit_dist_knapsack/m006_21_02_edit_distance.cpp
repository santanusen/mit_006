//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>
#include <string>

#define INFINITE 0x1FFFFFFF

// Types of operations.
enum transform_op_t { NOOP, INSERT, DELETE, REPLACE };

std::ostream &operator<<(std::ostream &os, const transform_op_t &op) {
  switch (op) {
  case INSERT:
    os << 'I';
    break;
  case DELETE:
    os << 'D';
    break;
  case NOOP:
    os << '.';
    break;
  case REPLACE:
    os << 'R';
    break;
  default:
    os << 'X';
    break;
  }

  return os;
}

typedef size_t (&cost_func_t)(char, char, transform_op_t);

// Least cost operations to transform x into y.
void edit_distance_dp(const std::string &x, const std::string &y,
                      cost_func_t costfunc) {
  const ssize_t M = x.length();
  const ssize_t N = y.length();

  // DP Table: least_cost[i][j] = Least cost to transform x.suffix(i ... M) into
  // y.suffix(j ... N)
  size_t least_cost[M + 1][N + 1];

  // operation[i][j] = Least cost operation to transform x.suffix(i ... M) into
  // y.suffix(j ... N)
  transform_op_t operation[M + 1][N + 1];

  // Operations.
  transform_op_t ops[] = {NOOP, INSERT, DELETE, REPLACE};
  const auto O = sizeof(ops) / sizeof(ops[0]);
  // Whether to advance x or y if operation at same index in ops is performed.
  ssize_t xopshift[O] = {1, 0, 1, 1};
  ssize_t yopshift[O] = {1, 1, 0, 1};

  // Topological Order: Each cell depends on the cell at right (i, j + 1), down
  // (i + 1, j) or diagonal (i + 1, j + 1)
  for (ssize_t i = M; i >= 0; --i) {
    for (ssize_t j = N; j >= 0; --j) {
      if (i == M && j == N) {
        // Seed Value: No cost to transform from empty suffix to empty suffix.
        least_cost[M][N] = 0;
        operation[M][N] = NOOP;
        continue;
      }

      // Remember the operation that produces the least cost.
      least_cost[i][j] = INFINITE;
      for (size_t o = 0; o < O; ++o) {
        if ((i + xopshift[o]) > M || (j + yopshift[o]) > N)
          continue;

        size_t cost = costfunc(((i < M) ? x.at(i) : '\0'),
                               ((j < N) ? y.at(j) : '\0'), ops[o]) +
                      least_cost[i + xopshift[o]][j + yopshift[o]];

        if (cost < least_cost[i][j]) {
          least_cost[i][j] = cost;
          operation[i][j] = ops[o];
        }
      }
    }
  }

#if 0
        for (ssize_t i = 0; i <= M; ++i) {
                for (ssize_t j = 0; j <= N; ++j)
                        std::cout << operation[i][j] << " ";
                std::cout << std::endl;
        }

        std::cout << std::endl;

        for (ssize_t i = 0; i <= M; ++i) {
                for (ssize_t j = 0; j <= N; ++j)
                        std::cout << least_cost[i][j] << " ";
                std::cout << std::endl;
        }
#endif

  std::cout << "Edit Distance: " << least_cost[0][0] << std::endl;

  // Construct the solution back by following the operation table.
  ssize_t i = 0, j = 0;
  while (i < M || j < N) {
    std::cout << "[" << i << "," << j << "] : ";
    auto op = operation[i][j];
    switch (op) {
    case NOOP:
      std::cout << "NOOP " << x[i] << std::endl;
      ++i;
      ++j;
      break;

    case REPLACE:
      std::cout << "REPLACE " << x[i] << " -> " << y[j] << std::endl;
      ++i;
      ++j;
      break;

    case INSERT:
      std::cout << "INSERT " << y[j] << std::endl;
      ++j;
      break;

    case DELETE:
      std::cout << "DELETE " << x[i] << std::endl;
      ++i;
      break;

    default:
      std::cout << "INVALID " << std::endl;
      return;
    }
  }
}

size_t edit_cost(char c1, char c2, transform_op_t op) {
  switch (op) {
  case INSERT:
  case DELETE:
    return 1;
  case NOOP:
    return (c1 == c2) ? 0 : INFINITE;
  case REPLACE:
    return (c1 == c2) ? INFINITE : 1;
  default:
    break;
  }

  return INFINITE;
}

size_t edit_cost_noreplace(char c1, char c2, transform_op_t op) {
  switch (op) {
  case INSERT:
  case DELETE:
    return 1;
  case NOOP:
    return (c1 == c2) ? 0 : INFINITE;
  case REPLACE:
    return INFINITE;
  default:
    break;
  }

  return INFINITE;
}

int main() {

  std::string x = "HELLO", y = "YELLOW";

  std::cout << "x: " << x << std::endl << "y: " << y << std::endl << std::endl;

  edit_distance_dp(x, y, edit_cost);

  std::cout << std::endl << "Edit without replace: " << std::endl;
  edit_distance_dp(x, y, edit_cost_noreplace);

  return 0;
}
