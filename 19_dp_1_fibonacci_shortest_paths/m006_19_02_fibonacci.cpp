//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include "exec_time.hpp"
#include <iostream>
#include <unordered_map>

// Vanilla Fibonacci.
long fib_rec(int n) {
  if (n < 2)
    return n;
  return fib_rec(n - 2) + fib_rec(n - 1);
}

// Fibonacci with memoization.
long fib_memoized(int n) {
  static std::unordered_map<int, long> memo;
  auto itr = memo.find(n);
  if (itr != memo.end())
    return itr->second;
  if (n < 2)
    return n;
  auto f = fib_memoized(n - 2) + fib_memoized(n - 1);
  memo[n] = f;
  return f;
}

int main() {

  auto n = 45;

  // Captures execution time.
  exec_time et;

  std::cout << "fib_rec(" << n << ")" << std::endl;
  std::cout << "Result = " << et(fib_rec, n)
            << ". Time = " << et.get() << " ms." << std::endl << std::endl;

  std::cout << "fib_memoized(" << n << ")" << std::endl;
  std::cout << "Result = "<< et(fib_memoized, n)
            << ". Time = " << et.get() << " ms." << std::endl;
  return 0;
}
