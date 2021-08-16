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

#define A_BIG_PRIME_NUMBER 2147483647 // srand seed

// Common Base for plain hash functions and probing
// hash functions.
class HashFunctionBase {

public:
  explicit HashFunctionBase(uint32_t m) : M(m) {}

  virtual ~HashFunctionBase() {}

  void UpdateHashSize(uint32_t m) {
    M = m;
    OnHashSizeChange();
  }

protected:
  virtual void OnHashSizeChange() {}

  uint32_t M;
};

// Hash Function Interface:
// The concrete implementation of the function opertaor should
// map an input 'key' into an integer  value [0 ... M) where
// M is the hash table size.
class HashFunction : public HashFunctionBase {

public:
  explicit HashFunction(uint32_t m) : HashFunctionBase(m) {}

  virtual uint32_t operator()(uint32_t key) const = 0;
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
    MultiplicationHashFunction::OnHashSizeChange();
  }

  virtual uint32_t operator()(uint32_t key) const override {
    return (A * key) >> (W - R);
  }

protected:
  virtual void OnHashSizeChange() override {
    // M = 2 ^ R
    uint32_t m = M;
    for (R = 0; (m = (m >> 1)); ++R)
      ;
  }

protected:
  // Word size
  const static uint32_t W = sizeof(uint32_t) * 8;

  // Fibonacci hashing: Multiplier = 2 ^ W / phi
  const static uint32_t A = (static_cast<uint64_t>(1) << W) / 1.6180339;
  ;

  // Bit width of table size
  uint32_t R;
};

// Universal Hashing.
class UniversalHashFunction : public HashFunction {

public:
  explicit UniversalHashFunction(uint32_t m) : HashFunction(m) {
    srand(A_BIG_PRIME_NUMBER);
    UniversalHashFunction::OnHashSizeChange();
  }

  virtual uint32_t operator()(uint32_t key) const override {
    return ((A * key + B) % P) % M;
  }

protected:
  virtual void OnHashSizeChange() override {
    P = least_prime_larger_than(M);
    A = rand() % P;
    B = rand() % P;
  }

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

// Probing Hash Function Interface:
// The concrete implementation of the function opertaor should
// map an input 'key' into an integer  value [0 ... M) where
// M is the hash table size for the 'trial_no'-th probe.
// The output of the function operator for trial_no : {0, ..., M-1}
// should be some permutation of {0, ..., M- 1}.
class ProbingHashFunction : public HashFunctionBase {

public:
  explicit ProbingHashFunction(uint32_t m) : HashFunctionBase(m) {}

  virtual uint32_t operator()(uint32_t key, uint32_t trial_no) const = 0;
};

class LinearProbingHashFunction : public ProbingHashFunction {
public:
  explicit LinearProbingHashFunction(uint32_t m, HashFunction &hf)
      : ProbingHashFunction(m), hFunc(hf) {
    hFunc.UpdateHashSize(m);
  }

  virtual uint32_t operator()(uint32_t key, uint32_t trial_no) const override {
    return (hFunc(key) + trial_no) % M;
  }

protected:
  virtual void OnHashSizeChange() override { hFunc.UpdateHashSize(M); }

  HashFunction &hFunc;
};

class DoubleHashFunction : public ProbingHashFunction {
public:
  explicit DoubleHashFunction(uint32_t m, HashFunction &hf1, HashFunction &hf2)
      : ProbingHashFunction(m), hFunc1(hf1), hFunc2(hf2) {
    hFunc1.UpdateHashSize(m);
    hFunc2.UpdateHashSize(m);
  }

  virtual uint32_t operator()(uint32_t key, uint32_t trial_no) const override {
    // To ensure (trial_no * x) % M is a permutation of {0, 1, ..., M -1} for
    // x = {0, 1, ..., M - 1}; x and M should be relatively prime to each
    // other. In this case M is a power of 2 and x is odd.
    return (hFunc1(key) + trial_no * Oddify(hFunc2(key))) % M;
  }

protected:
  virtual void OnHashSizeChange() override {
    hFunc1.UpdateHashSize(M);
    hFunc2.UpdateHashSize(M);
  }

  static uint32_t Oddify(uint32_t val) { return (val | 1); }

  HashFunction &hFunc1;

  HashFunction &hFunc2;
};

// Implements hashing with open addressing
class HashTable {
protected:
  static const uint32_t FREE_MARKER = UINT32_MAX;
  static const uint32_t DEL_MARKER = UINT32_MAX - 1;

  static uint32_t *new_hash_table(uint32_t length) {
    uint32_t *htable = new uint32_t[length];
    for (uint32_t i = 0; i < length; ++i)
      htable[i] = FREE_MARKER;
    return htable;
  }

  // Find index of a key in hash table. Return length if not present.
  uint32_t find_index(uint32_t key) const {
    uint32_t probe = 0;
    uint32_t index;
    do {
      index = prHashFunc(key, probe++);
      if (hashTable[index] == key) {
        return index;
      }
    } while (hashTable[index] != FREE_MARKER && probe < length);

    return length; // Invalid index.
  }

public:
  explicit HashTable(ProbingHashFunction &prh)
      : length(MIN_LENGTH), num_entries(0), prHashFunc(prh),
        hashTable(new_hash_table(length)) {
    prHashFunc.UpdateHashSize(length);
  }

  ~HashTable() { delete[] hashTable; }

  // Insert key to hash table.
  void insert(uint32_t key) {
    if (load_factor() >= 0.5) {
      // Hash table too dense: expand.
      rehash(2 * length);
    }

    uint32_t probe = 0;
    uint32_t index;
    do {
      index = prHashFunc(key, probe++);
      if (hashTable[index] == key) {
        return;
      }
    } while (hashTable[index] != FREE_MARKER);

    hashTable[index] = key;
    ++num_entries;
  }

  // Find a key in hash table. Return INVALID_KEY if not present.
  uint32_t find(uint32_t key) const {
    auto index = find_index(key);
    return (index < length) ? hashTable[index] : INVALID_KEY;
  }

  // Remove a key from the hash table.
  void remove(uint32_t key) {
    auto index = find_index(key);
    if (index >= length)
      return;

    hashTable[index] = DEL_MARKER;
    --num_entries;

    if (length / 4 >= MIN_LENGTH && load_factor() <= 0.125) {
      // Hash table too sparse: shrink.
      rehash(length / 4);
    }
  }

  void dump(std::ostream &os) {
    os << "length: " << length << std::endl;
    for (size_t i = 0; i < length; ++i) {
      if (hashTable[i] != FREE_MARKER && hashTable[i] != DEL_MARKER) {
        os << "[" << i << "] : " << hashTable[i] << std::endl;
      }
    }
  }

protected:
  // Rehash to a hash table with new length
  void rehash(uint32_t new_length) {
    // Save the old values.
    auto old_hashTable = hashTable;
    auto old_length = length;

    // Initialize with new values.
    hashTable = new_hash_table(new_length);
    length = new_length;
    num_entries = 0;

    // Adjust the hash function to the new length.
    prHashFunc.UpdateHashSize(new_length);

    // Rehash the entries from the old table to the new.
    for (size_t i = 0; i < old_length; ++i) {
      auto key = old_hashTable[i];
      if (key != FREE_MARKER && key != DEL_MARKER) {
        insert(key);
      }
    }

    // Not to forget to free up the old table.
    delete[] old_hashTable;
  }

public:
  static const uint32_t INVALID_KEY = FREE_MARKER;

protected:
  double load_factor() const {
    return (static_cast<double>(num_entries) / static_cast<double>(length));
  }

  static const uint32_t MIN_LENGTH = 8;

  // Hash table length.
  uint32_t length;

  uint32_t num_entries;

  ProbingHashFunction &prHashFunc;

  uint32_t *hashTable;
};

// Run some tests on the hash table.
void run_test(const char *msg, uint32_t *nums, uint32_t N,
              ProbingHashFunction &prh) {
  std::cout << msg << ":" << std::endl;
  HashTable ht(prh);

  // Insert N numbers into hash table.
  for (uint32_t i = 0; i < N; ++i)
    ht.insert(nums[i]);

  // All the N numbers should be found in the hash table.
  for (uint32_t i = 0; i < N; ++i)
    if (nums[i] != ht.find(nums[i]))
      std::cout << msg << ": Error: Not found: " << i << ":" << nums[i]
                << std::endl;

  // Remove all numbers from the hash table except the last 4.
  for (uint32_t i = 0; i < N - 4; ++i)
    ht.remove(nums[i]);

  // The removed numbers should not be present in the hash table.
  for (uint32_t i = 0; i < N - 4; ++i)
    if (HashTable::INVALID_KEY != ht.find(nums[i]))
      std::cout << msg << ": Error: found: " << i << ":" << nums[i]
                << std::endl;

  // Print the hash table, it should be shrunk back to min length.
  ht.dump(std::cout);
  std::cout << std::endl;
}

#define RUN_HASH_TEST(HF, PHF, nums, N)                                        \
  do {                                                                         \
    HF hf(N);                                                                  \
    PHF phf(N, hf);                                                            \
    run_test(#PHF " % " #HF, nums, N, phf);                                    \
  } while (0)

#define RUN_HASH_TEST2(HF1, HF2, PHF, nums, N)                                 \
  do {                                                                         \
    HF1 hf1(N);                                                                \
    HF1 hf2(N);                                                                \
    PHF phf(N, hf1, hf2);                                                      \
    run_test(#PHF " % " #HF1 " % " #HF2, nums, N, phf);                        \
  } while (0)

int main() {
  const uint32_t N = 1000000; // A Million
  srand(A_BIG_PRIME_NUMBER);
  uint32_t nums[N];
  for (uint32_t i = 0; i < N; ++i)
    nums[i] = rand();

  RUN_HASH_TEST(DivisionHashFunction, LinearProbingHashFunction, nums, N);
  RUN_HASH_TEST(MultiplicationHashFunction, LinearProbingHashFunction, nums, N);
  RUN_HASH_TEST(UniversalHashFunction, LinearProbingHashFunction, nums, N);

  RUN_HASH_TEST2(UniversalHashFunction, MultiplicationHashFunction, DoubleHashFunction, nums, N);

  return 0;
}
