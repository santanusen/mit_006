//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <cstdint>
#include <iostream>
#include <list>
#include <vector>

// Hash Function Interface:
// The concrete implementation of the function opertaor should
// map an input 'key' into an integer  value [0 ... M) where
// M is the hash table size.
class HashFunction {

public:
  explicit HashFunction(uint32_t m) : M(m) {}

  virtual ~HashFunction() {}

  virtual uint32_t operator()(uint32_t key) const = 0;

protected:
  const uint32_t M;
};

// Simple hashing using modulo division.
class DivisionHashFunction : public HashFunction {

public:
  explicit DivisionHashFunction(uint32_t m) : HashFunction(m) {}

  virtual uint32_t operator()(uint32_t key) const override { return key % M; }
};

// Hashing using multiplication.
class MultiplicationHashFunction : public HashFunction {

public:
  explicit MultiplicationHashFunction(uint32_t m) : HashFunction(m) {
    // M = 2 ^ R
    for (R = 0; (m = (m >> 1)); ++R)
      ;
  }

  virtual uint32_t operator()(uint32_t key) const override {
    /*
    // hash = ((A * key) mod 2 ^ W) / 2 ^ ( W - R)
    //      = ((A * key) mod 2 ^ W) >> ( W - R)
    return static_cast<uint32_t>(
        ((static_cast<uint64_t>(A) * static_cast<uint64_t>(key)) %
         (static_cast<uint64_t>(0x1) << W)) >> (W - R));

    or simply!!! */
    return (A * key) >> (W - R);
  }

protected:
  // Word size
  const static uint32_t W = sizeof(uint32_t) * 8;

  // Fibonacci hashing: Multiplier = 2 ^ W / phi
  const static uint32_t A = (static_cast<uint64_t>(1) << W) / 1.6180339;

  // Bit width of table size
  uint32_t R;
};

// Universal Hashing.
class UniversalHashFunction : public HashFunction {

public:
  explicit UniversalHashFunction(uint32_t m)
      : HashFunction(m), P(least_prime_larger_than(m)) {
    srand(P);
    A = rand() % P;
    B = rand() % P;
  }

  virtual uint32_t operator()(uint32_t key) const override {
    return ((A * key + B) % P) % M;
  }

protected:
  static bool is_prime(uint32_t n) {
    if (n % 2 == 0 || n % 3 == 0)
      return false;

    // n is a prime number if it does not have a prime factor
    // between 2 and sqrt(i).
    // The following loop is just an optimization of
    // for (i = 5; i * i <= n; i += 6) { ... }
    // avoiding the multiplication to compute i^2
    uint32_t i = 5, i_sq = 25, i_sq_step = 96;
    while (i_sq <= n) {
      // i = 5, 11, 17, 23, ...
      //   = 5 + 6 * k : k = 0, 1, 2, ...
      // i is odd; i + 1, i + 3, i + 5 are divisible by 2.
      // i + 4 = 9 + 6 * k is divisible by 3.
      // Only possible prime numbers between i and (i + 5) are
      // i and i + 2.
      if (n % i == 0 || n % (i + 2) == 0)
        return false;
      i += 6;
      i_sq += i_sq_step;
      i_sq_step += 72;
    }

    return true;
  }

  static uint32_t least_prime_larger_than(uint32_t n) {
    while (!is_prime(++n))
      ;
    return n;
  }

protected:
  uint32_t P; // Prime Number > M

  uint32_t A; // Random number between 0 and (P - 1)

  uint32_t B; // Random number between 0 and (P - 1)
};

// Implements hashing with chaining
template <typename HASHFUNC> class HashingWithChaining {
public:
  HashingWithChaining(uint32_t n)
      : length(power_of_two_aligned(n)), // table size = O(n)
        hashFunc(length), hashTable(length) {}

  void insert(uint32_t key) { hashTable[hashFunc(key)].push_back(key); }

  void dump(std::ostream &os) {
    for (size_t i = 0; i < hashTable.size(); ++i) {
      if (!hashTable[i].empty()) {
        os << "[" << i << "] : ";
        for (auto k : hashTable[i])
          os << k << " ";
        os << std::endl;
      }
    }
  }

protected:
  static uint32_t power_of_two_aligned(uint32_t n) {
    uint32_t i = 1;
    while (i < n)
      i = i << 1;
    return i;
  }

protected:
  uint32_t length;

  HASHFUNC hashFunc;

  typedef std::vector<std::list<uint32_t>> table_t;
  table_t hashTable;
};

int main() {
  const uint32_t N = 60;
  srand(2147483647);
  uint32_t nums[N];
  for (uint32_t i = 0; i < N; ++i)
    nums[i] = rand();

  std::cout << "Division: " << std::endl;
  HashingWithChaining<DivisionHashFunction> divHash(N);
  for (uint32_t i = 0; i < N; ++i)
    divHash.insert(nums[i]);
  divHash.dump(std::cout);

  std::cout << std::endl << "Multiplication: " << std::endl;
  HashingWithChaining<MultiplicationHashFunction> multHash(N);
  for (uint32_t i = 0; i < N; ++i)
    multHash.insert(nums[i]);
  multHash.dump(std::cout);

  std::cout << std::endl << "Universal: " << std::endl;
  HashingWithChaining<UniversalHashFunction> uHash(N);
  for (uint32_t i = 0; i < N; ++i)
    uHash.insert(nums[i]);
  uHash.dump(std::cout);

  return 0;
}
