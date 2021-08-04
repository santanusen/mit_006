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

// Hash Function Interface:
// The concrete implementation of the function opertaor should
// map an input 'key' into an integer  value [0 ... M) where
// M is the hash table size.
class HashFunction {

public:
  explicit HashFunction(uint32_t m) : M(m) {}

  virtual ~HashFunction() {}

  virtual uint32_t operator()(uint32_t key) const = 0;

  void UpdateHashSize(uint32_t m) {
    M = m;
    OnHashSizeChange();
  }

protected:
  virtual void OnHashSizeChange() {}

  uint32_t M;
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

// Implements hashing with chaining
template <typename HASHFUNC> class HashTable {

protected:
  typedef std::list<uint32_t> hash_chain_t;

  // Internal utility function
  static typename HashTable<HASHFUNC>::hash_chain_t::iterator
  find(hash_chain_t &chain, uint32_t key) {
    for (auto itr = chain.begin(); itr != chain.end(); ++itr) {
      if (*itr == key) {
        return itr;
      }
    }
    return chain.end();
  }

public:
  HashTable()
      : length(MIN_LENGTH), num_entries(0), hashFunc(length),
        hashTable(new hash_chain_t[length]) {}

  ~HashTable() { delete[] hashTable; }

  // Insert key to hash table.
  void insert(uint32_t key) {
    if (length == num_entries) {
      // Hash table too dense: expand.
      rehash(2 * length);
    }

    hashTable[hashFunc(key)].push_back(key);
    ++num_entries;
  }

  // Find a key in hash table. Return INVALID_KEY if not present.
  uint32_t find(uint32_t key) const {
    auto hkey = hashFunc(key);
    auto &chain = hashTable[hkey];

    auto itr = find(chain, key);
    return (itr == chain.end()) ? INVALID_KEY : *itr;
  }

  // Remove a key from the hash table.
  void remove(uint32_t key) {
    auto hkey = hashFunc(key);
    auto &chain = hashTable[hkey];

    auto itr = find(chain, key);
    if (itr != chain.end()) {
      chain.erase(itr);
    }

    --num_entries;

    if (length > MIN_LENGTH && num_entries <= length / 4) {
      // Hash table too sparse: shrink.
      rehash(length / 2);
    }
  }

  void dump(std::ostream &os) {
    for (size_t i = 0; i < length; ++i) {
      if (!hashTable[i].empty()) {
        os << "[" << i << "] : ";
        for (auto k : hashTable[i])
          os << k << " ";
        os << std::endl;
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
    hashTable = new hash_chain_t[new_length];
    length = new_length;
    num_entries = 0;

    // Adjust the hash function to the new length.
    hashFunc.UpdateHashSize(new_length);

    // Rehash the entries from the old table to the new.
    for (size_t i = 0; i < old_length; ++i)
      for (auto key : old_hashTable[i])
        insert(key);

    // Not to forget to free up the old table.
    delete[] old_hashTable;
  }

public:
  static const uint32_t INVALID_KEY = 0xFFFFFFFF;

protected:
  static const uint32_t MIN_LENGTH = 8;

  // Hash table length.
  uint32_t length;

  uint32_t num_entries;

  HASHFUNC hashFunc;

  hash_chain_t *hashTable;
};

// Run some tests on the hash table.
template <typename HASHFUNC>
void run_test(const char *msg, uint32_t *nums, uint32_t N) {
  std::cout << msg << ":" << std::endl;
  HashTable<HASHFUNC> ht;

  // Insert N numbers into hash table.
  for (uint32_t i = 0; i < N; ++i)
    ht.insert(nums[i]);

  // All the N numbers should be found in the hash table.
  for (uint32_t i = 0; i < N; ++i)
    if (nums[i] != ht.find(nums[i]))
      std::cout << msg << ": Error: Not found: " << i << ":" << nums[i]
                << std::endl;

  // Remove all numbers from the hash table except the last 2.
  for (uint32_t i = 0; i < N - 2; ++i)
    ht.remove(nums[i]);

  // The removed numbers should not be present in the hash table.
  for (uint32_t i = 0; i < N - 2; ++i)
    if (HashTable<HASHFUNC>::INVALID_KEY != ht.find(nums[i]))
      std::cout << msg << ": Error: found: " << i << ":" << nums[i]
                << std::endl;

  // Print the hash table, it should be shrunk back to min length.
  ht.dump(std::cout);
  std::cout << std::endl;
}

int main() {
  const uint32_t N = 1000000; // A million
  srand(A_BIG_PRIME_NUMBER);
  uint32_t nums[N];
  for (uint32_t i = 0; i < N; ++i)
    nums[i] = rand();

  run_test<DivisionHashFunction>("Division", nums, N);
  run_test<MultiplicationHashFunction>("Multiplication", nums, N);
  run_test<UniversalHashFunction>("Universal", nums, N);

  return 0;
}
