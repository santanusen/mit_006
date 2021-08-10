//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <cstdint>
#include <iostream>
#include <string>

#define A_BIG_PRIME_NUMBER 2147483647 // srand seed

namespace karp_rabin_util {
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

// Generate a random number between min and max.
uint32_t random_number(uint32_t min, uint32_t max = RAND_MAX) {
  return rand() % (max - min + 1) + min;
}

// Generate a random prime number between min and max
uint32_t random_prime(uint32_t min, uint32_t max = RAND_MAX) {
  uint32_t p;
  while (!is_prime(p = random_number(min, max)))
    ;
  return p;
}

// Find multiplicative inverse 'ib' of 'b' : (b * ib) % P = 1
uint32_t mult_inverse(uint32_t b, uint32_t P) {
  for (uint32_t ib = 1; ib < P; ++ib) {
    if ((b * ib) % P == 1) {
      return ib;
    }
  }
  return P; // Invalid
}

} // namespace karp_rabin_util

// The interface for rolling hash
class rolling_hash {
public:
  virtual void append(uint32_t c) = 0;
  virtual void skip(uint32_t c) = 0;
  virtual void reset(){};
  virtual uint32_t operator()(void) const = 0;
  virtual ~rolling_hash() {}
};

// Karp-Rabin rolling hash
class karp_rabin_rolling_hash : public rolling_hash {
public:
  karp_rabin_rolling_hash(uint32_t b, uint32_t wl)
      : base(b), hash(0), mod_msb_pos(1) {
    // Keep trying till we find a prime number and a multiplicative inverse
    // combination for the given base.
    do {
      // Search for a random prime number in a confined range to avoid
      // choosing a prime number that is unnecessarily large.
      P = karp_rabin_util::random_prime(wl + 1, wl + 1024);
      ibase = karp_rabin_util::mult_inverse(b, P);
    } while (ibase >= P);
  }

  virtual void reset() override {
    hash = 0;
    mod_msb_pos = 1;
  }

  virtual void append(uint32_t c) override {
    mod_msb_pos = (mod_msb_pos * base) % P;
    hash = (hash * base + c) % P;
  }

  virtual void skip(uint32_t c) override {
    // mod_msb_pos = (mod_msb_pos / base) % P.
    // mod_msb_pos / base = 0 if mod_msb_pos < base. Need tp convert the
    // division into multiplication with inverse keeping the modulo result the
    // same. x % P = (x * base * ibase) % P.
    // Replacing x with (mod_msb_pos /base) :
    // (mod_msb_pos / base) % P = ((mod_msb_pos / base) * base * ibase) %P.
    mod_msb_pos = (mod_msb_pos * ibase) % P;

    // hash = (hash - c * (base ^ window_length)) % P
    //      = (hash - c * mod_msb_pos) % P
    // (hash - c * mod_msb_pos) can become negative. We can add a multiple of P
    // to keep it positive without changing the modulo result. c < base and
    // mod_msb_pos < P, thus (c * mod_msb_pos) < (P * base).
    hash = (hash - c * mod_msb_pos + P * base) % P;
  }

  virtual uint32_t operator()(void) const override { return hash; }

protected:
  uint32_t base;

  uint32_t P; // A prime number > window_length.

  uint32_t hash; // The rolling hash.

  uint32_t mod_msb_pos; // (base ^ window_length) % P.

  uint32_t ibase; // Multiplicative inverse of base. (base * ibase) % P = 1.
};

// Rolling hash using xor
class poor_mans_rolling_hash : public rolling_hash {
public:
  poor_mans_rolling_hash() : hash(0) {}

  virtual void append(uint32_t c) override { hash ^= c; }

  virtual void skip(uint32_t c) override { hash ^= c; }

  virtual void reset() override { hash = 0; }

  virtual uint32_t operator()(void) const override { return hash; }

protected:
  uint32_t hash; // The rolling hash.
};

// Search for needle in the haystack using Karp-Rabin.
int32_t karp_rabin_strstr(const std::string &needle,
                          const std::string &haystack) {
  if (needle.empty() || haystack.length() < needle.length()) {
    return -1;
  }

#ifdef USE_TRIVIAL_HASHING
  poor_mans_rolling_hash rh;
#else
  karp_rabin_rolling_hash rh(255, needle.length());
#endif

  for (auto i = 0u; i < needle.length(); ++i) {
    rh.append(needle.at(i));
  }
  auto nh = rh();

  rh.reset();
  for (auto i = 0u; i < needle.length(); ++i) {
    rh.append(haystack.at(i));
  }

  auto idx = -1;
  auto false_positives = 0;
  for (auto j = needle.length();; ++j) {
    if (rh() == nh) {
      // Hashes match, compare the actual characters to confirm.
      if (haystack.compare(j - needle.length(), needle.length(), needle) == 0) {
        idx = j - needle.length();
        break;
      } else {
        ++false_positives;
      }
    }

    if (j == haystack.length())
      break;

    rh.skip(haystack.at(j - needle.length()));
    rh.append(haystack.at(j));
  }

  if (false_positives)
    std::cout << "flase_positives: " << false_positives << std::endl;
  return idx;
}

int main() {
  srand(A_BIG_PRIME_NUMBER);

  std::string haystack = "Twinkle, twinkle, little bat!"
                         "How I wonder what you're at!"
                         "Up above the world you fly,"
                         "Like a tea-tray in the sky.";

  std::string needle = "tray";

  auto i = karp_rabin_strstr(needle, haystack);

  if (i >= 0) {
    std::cout << "Found \"" << haystack.substr(i, needle.length())
              << "\" at index " << i << std::endl;
  } else {
    std::cout << "Not found" << std::endl;
  }

  return 0;
}
