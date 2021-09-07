//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>
#include <vector>

// Namespace providing utilities and definitions for manipulating the magnitude
// of large numbers.
namespace magn {

// Note: We can use more coarse data types to store the large numbers and
// operate on them, e.g. uint32_t as word_t and uint64_t as double_word_t.

// A vector of word_t is used to store the magnitude of the large numbers.
typedef uint8_t word_t;

// Results of some operations on word_t are of type double_word_t.
typedef uint16_t double_word_t;

// Data structure to store the magnitude of large numbers.
typedef std::vector<word_t> magnitude_t;

const auto BITS_PER_WORD = 8 * sizeof(word_t);

// Convert a letter specifying a hex-digit into a word_t.
word_t hexdigit2word(char hex_dig) {
  if (hex_dig >= '0' && hex_dig <= '9')
    return (hex_dig - '0');
  if (hex_dig >= 'A' && hex_dig <= 'F')
    return (hex_dig - 'A' + 10);
  if (hex_dig >= 'a' && hex_dig <= 'f')
    return (hex_dig - 'a' + 10);

  return 0;
}

// Convert the least significant nibble of a word_t into a hex character.
char word2hexdigit(word_t word) {
  word &= 0xF;
  return (word < 10) ? '0' + word : 'A' + word - 10;
}

// Append the hex representation of the magnitude_t into a string.
void magnitude2hexstr(const magnitude_t &magnitude, std::string &hexstr) {
  const auto digits_per_word = 2 * sizeof(word_t);
  for (auto it = magnitude.rbegin(); it != magnitude.rend(); ++it) {
    for (auto i = digits_per_word; i > 0; --i) {
      hexstr.push_back(word2hexdigit(*it >> (4 * (i - 1))));
    }
  }
}

// Parse a string of hex characters and store it into a magnitude_t.
void hexstr2magnitude(const std::string &hexstr, magnitude_t &magnitude) {
  const auto digits_per_word = 2 * sizeof(word_t);
  word_t word = 0;
  auto i = 0;

  for (auto itr = hexstr.rbegin(); itr != hexstr.rend(); ++itr) {
    word_t digit = hexdigit2word(*itr);
    word |= (digit << (4 * i));
    ++i;
    if (i == digits_per_word) {
      magnitude.push_back(word);
      word = 0;
      i = 0;
    }
  }

  if (word) {
    magnitude.push_back(word);
  }
}

// Convert the least significant bytes of magnitude_t into the biggest integer
// type supported by the platform.
intmax_t magnitude2int(const magnitude_t &magnitude) {
  intmax_t res = 0;
  const auto num_iter =
      std::min(magnitude.size(), sizeof(intmax_t) / sizeof(word_t));
  for (auto i = 0u; i < num_iter; ++i) {
    res |= (static_cast<intmax_t>(magnitude[i]) << (BITS_PER_WORD * i));
  }
  return res;
}

// Convert a value of the biggest integer type supported by the platform into a
// magnitude_t.
void int2magnitude(intmax_t val, magnitude_t &magnitude) {
  if (val < 0)
    val = -val;
  word_t mask = ~(word_t(0));
  for (; (val); val >>= BITS_PER_WORD) {
    magnitude.push_back(val & mask);
  }
}

// Compare two magnitude_t's.
// Return value:
// > 0 : if lhs > rhs
// < 0 : if lhs < rhs
// 0 : if lhs == rhs
int compare(const magnitude_t &lhs, const magnitude_t &rhs) {
  if (lhs.size() > rhs.size())
    return 2;

  if (lhs.size() < rhs.size())
    return -2;

  for (ssize_t i = lhs.size() - 1; i >= 0; --i) {
    if (lhs[i] > rhs[i]) {
      return 1;
    }

    if (lhs[i] < rhs[i]) {
      return -1;
    }
  }

  return 0;
}

// Word shift operators.

// Shift m left by n words.
void operator<<(magnitude_t &m, size_t n) {
  if ((m.size()) && (n > 0)) {
    m.insert(m.begin(), n, word_t(0));
  }
}

// Shift m right by n words.
void operator>>(magnitude_t &m, size_t n) {
  if (n >= m.size()) {
    m.clear();
  } else if (n > 0) {
    m.erase(m.begin(), m.begin() + n);
  }
}

// Addition helper: Adds the numbers formed by words (ll - lh) of lhs and
// words (rl - rh) of rhs.
magnitude_t add(const magnitude_t &lhs, const magnitude_t &rhs, ssize_t ll,
                ssize_t lh, ssize_t rl, ssize_t rh) {
  const auto mag_size = std::max(lh - ll + 1, rh - rl + 1);

  magnitude_t res;
  double_word_t carry = 0;
  const double_word_t lsword_mask = ~(word_t(0));
  for (ssize_t i = 0; i < mag_size; ++i) {
    double_word_t lword = (ll + i <= lh) ? lhs[ll + i] : 0;
    double_word_t rword = (rl + i <= rh) ? rhs[rl + i] : 0;
    double_word_t sum = lword + rword + carry;
    res.push_back(sum & lsword_mask);
    carry = (sum >> BITS_PER_WORD) & lsword_mask;
  }

  if (carry)
    res.push_back(carry & lsword_mask);

  return res;
}

// Adds two magnitude_t's.
magnitude_t operator+(const magnitude_t &lhs, const magnitude_t &rhs) {
  return add(lhs, rhs, 0, lhs.size() - 1, 0, rhs.size() - 1);
}

// Diff: Subtracts rhs from lhs.
// Beware: lhs is assumed to be greater than or equal to rhs.
magnitude_t operator-(const magnitude_t &lhs, const magnitude_t &rhs) {
  const auto mag_size = std::max(lhs.size(), rhs.size());

  magnitude_t res;
  double_word_t carry = 0;
  ssize_t mspos = -1;
  for (size_t i = 0; i < mag_size; ++i) {
    double_word_t lword = (i < lhs.size()) ? lhs[i] : 0;
    double_word_t rword = (i < rhs.size()) ? rhs[i] : 0;
    rword += carry;
    if (lword < rword) {
      lword |= (double_word_t(1) << BITS_PER_WORD);
      carry = 1;
    } else {
      carry = 0;
    }
    double_word_t sub = lword - rword;
    res.push_back(sub);
    if (sub)
      mspos = i;
  }

  // Remove leading zeroes.
  res.erase(res.begin() + (mspos + 1), res.end());

  return res;
}

// Naive multiplication of two magnitude_t's.
magnitude_t high_school_multiply(const magnitude_t &lhs,
                                 const magnitude_t &rhs) {
  magnitude_t res;
  const double_word_t lsword_mask = ~(word_t(0));

  for (size_t i = 0; i < rhs.size(); ++i) {
    double_word_t carry = 0;
    double_word_t rword = rhs[i];
    magnitude_t row;
    for (size_t j = 0; j < lhs.size(); ++j) {
      double_word_t lword = lhs[j];
      double_word_t mult = (lword * rword) + carry;
      row.push_back(mult & lsword_mask);
      carry = (mult >> BITS_PER_WORD) & lsword_mask;
    }

    if (carry)
      row.push_back(carry & lsword_mask);

    row << i;
    res = res + row;
  }

  return res;
}

magnitude_t dc_multiply_recurse(const magnitude_t &lhs, const magnitude_t &rhs,
                                size_t ll, size_t lh, size_t rl, size_t rh) {
  if (ll > lh || rl > rh || ll >= lhs.size() || rl >= rhs.size()) {
    return magnitude_t();
  }

  if (ll == lh && rl == rh) { // Base case: Single word multiplication.
    magnitude_t res;
    const double_word_t lsword_mask = ~(word_t(0));
    double_word_t lword = lhs[ll];
    double_word_t rword = rhs[rl];
    double_word_t mult = lword * rword;
    res.push_back(mult & lsword_mask);
    word_t carry = (mult >> BITS_PER_WORD) & lsword_mask;
    if (carry)
      res.push_back(carry);
    return res;
  }

  // Recurse: Calculate the multiplications of the magnitudes of
  // about half the sizes and combine them.
  const auto lm = (ll + lh) / 2;
  const auto rm = (rl + rh) / 2;

  const auto lshift = lm - ll + 1;
  const auto rshift = rm - rl + 1;

  // lhs = (base^lshift * a + b)
  // rhs = (base^lshift * c + d)
  // lhs * rhs = base^(lshift + rshift) * ac + base^(rshift) * bc +
  // base^(lshift) * ad + bd
  auto ac = dc_multiply_recurse(lhs, rhs, lm + 1, lh, rm + 1, rh);
  auto bc = dc_multiply_recurse(lhs, rhs, ll, lm, rm + 1, rh);
  auto ad = dc_multiply_recurse(lhs, rhs, lm + 1, lh, rl, rm);
  auto bd = dc_multiply_recurse(lhs, rhs, ll, lm, rl, rm);

  ac << (lshift + rshift);
  bc << rshift;
  ad << lshift;

  return ac + bc + ad + bd;
}

// Multiplication using divide-and-conquer of two magnitude_t's.
magnitude_t dc_multiply(const magnitude_t &lhs, const magnitude_t &rhs) {
  return dc_multiply_recurse(lhs, rhs, 0, lhs.size() - 1, 0, rhs.size() - 1);
}

magnitude_t karatsuba_multiply(const magnitude_t &lhs, const magnitude_t &rhs);

magnitude_t karatsuba_multiply_recurse(const magnitude_t &lhs,
                                       const magnitude_t &rhs, size_t ll,
                                       size_t lh, size_t rl, size_t rh) {
  if (ll > lh || rl > rh || ll >= lhs.size() || rl >= rhs.size()) {
    return magnitude_t();
  }

  if (ll == lh && rl == rh) { // Base case: Single word multiplication.
    magnitude_t res;
    const double_word_t lsword_mask = ~(word_t(0));
    double_word_t lword = lhs[ll];
    double_word_t rword = rhs[rl];
    double_word_t mult = lword * rword;
    res.push_back(mult & lsword_mask);
    word_t carry = (mult >> BITS_PER_WORD) & lsword_mask;
    if (carry)
      res.push_back(carry);
    return res;
  }

  // Recurse: Calculate the multiplications of the magnitudes of
  // half the sizes and combine them.
  const auto lm = (ll + lh) / 2;
  const auto rm = (rl + rh) / 2;

  // const auto lshift = lm - ll + 1;
  const auto shift = rm - rl + 1;

  // lhs = (base^shift * a + b)
  // rhs = (base^shift * c + d)
  // lhs * rhs = base^(2*shift) * ac + base^(shift) * (bc + ad) + bd
  // Karatsuba uses the following trick to reduce the number of multiplications
  // from 4 to 3.

  // Z0 = ac
  auto z0 = karatsuba_multiply_recurse(lhs, rhs, lm + 1, lh, rm + 1, rh);
  // Z2 = bd
  auto z2 = karatsuba_multiply_recurse(lhs, rhs, ll, lm, rl, rm);

  // Z1 = bc + ad = (a + b) (c + d) - Z0 - Z2
  auto a_plus_b = add(lhs, lhs, lm + 1, lh, ll, lm);
  auto c_plus_d = add(rhs, rhs, rm + 1, rh, rl, rm);

  // Note: a_plus_b and c_plus_d may not have sizes that are exact
  // powers of 2. Directly calling karatsuba_multiply_recurse() may
  // not work.
  auto a_plus_b_X_c_plus_d = karatsuba_multiply(a_plus_b, c_plus_d);
  auto z0_plus_z2 = z0 + z2;
  auto z1 = a_plus_b_X_c_plus_d - z0_plus_z2;

  z0 << (2 * shift);
  z1 << shift;

  return z0 + z1 + z2;
}

// Get the least number which is a power of 2 and is greater than or equal to n.
size_t power_of_two_aligned(size_t n) {
  size_t i = 1;
  while (i < n)
    i = i << 1;
  return i;
}

// Karatsuba multiplication
magnitude_t karatsuba_multiply(const magnitude_t &lhs, const magnitude_t &rhs) {
  // Align size to power of 2. It is essential for the Karatsuba multiplication
  // algorithm that the operands in the subproblems at each recursion of the
  // divide and conquer have the same size.
  auto magsz = power_of_two_aligned(std::max(lhs.size(), rhs.size()));
  auto lshift = magsz - lhs.size();
  auto rshift = magsz - rhs.size();
  auto ll = lhs;
  ll << lshift;
  auto rr = rhs;
  rr << rshift;

  auto res =
      karatsuba_multiply_recurse(ll, rr, 0, ll.size() - 1, 0, rr.size() - 1);
  res >> (lshift + rshift);
  return res;
}

magnitude_t operator*(const magnitude_t &lhs, const magnitude_t &rhs) {
  // return high_school_multiply(lhs, rhs);
  // return dc_multiply(lhs, rhs);
  return karatsuba_multiply(lhs, rhs);
}
} // namespace magn

// A class encapsulating the sign and magnitude of a large number.
class large_num_t {

private:
  enum sign_t { POSITIVE, NEGATIVE };
  sign_t mSign;

  magn::magnitude_t mMagnitude;

  static sign_t flip(sign_t s) { return (s == POSITIVE) ? NEGATIVE : POSITIVE; }

  large_num_t add_sub(const large_num_t &rhs, bool add) const {
    large_num_t res;
    const sign_t rsign = (add) ? rhs.mSign : flip(rhs.mSign);
    if (mSign == rsign) {
      using magn::operator+;
      auto mres = this->mMagnitude + rhs.mMagnitude;
      res.mMagnitude.swap(mres);
      res.mSign = mSign;
    } else {
      const auto cmp = magn::compare(mMagnitude, rhs.mMagnitude);
      const auto &ll = (cmp > 0) ? *this : rhs;
      const auto &rr = (cmp > 0) ? rhs : *this;
      using magn::operator-;
      auto mres = ll.mMagnitude - rr.mMagnitude;
      res.mMagnitude.swap(mres);
      res.mSign = (cmp > 0) ? mSign : rsign;
    }
    return res;
  }

public:
  large_num_t() : mSign(POSITIVE) {}

  large_num_t(const std::string &hexstr) {
    magn::hexstr2magnitude(hexstr, mMagnitude);
    mSign = (!hexstr.empty() && hexstr.at(0) == '-') ? NEGATIVE : POSITIVE;
  }

  large_num_t(intmax_t val) {
    magn::int2magnitude(val, mMagnitude);
    mSign = (val < 0) ? NEGATIVE : POSITIVE;
  }

  size_t size() const { return mMagnitude.size(); }

  std::string hex_str() const {
    std::string hexstr;
    if (mSign == NEGATIVE)
      hexstr = "-";
    magn::magnitude2hexstr(mMagnitude, hexstr);
    return hexstr;
  }

  intmax_t to_int() const {
    auto val = magn::magnitude2int(mMagnitude);
    return (mSign == NEGATIVE) ? -val : val;
  }

  large_num_t operator+(const large_num_t &rhs) const {
    return add_sub(rhs, true);
  }

  large_num_t operator-(const large_num_t &rhs) const {
    return add_sub(rhs, false);
  }

  large_num_t operator*(const large_num_t &rhs) const {
    large_num_t res;
    using magn::operator*;
    auto mres = mMagnitude * rhs.mMagnitude;
    res.mMagnitude.swap(mres);
    res.mSign = (mSign == rhs.mSign) ? POSITIVE : NEGATIVE;
    return res;
  }
};

std::ostream &operator<<(std::ostream &os, const large_num_t &num) {
  if (num.size() == 0) {
    os << 0;
  } else {
    os << num.hex_str();
  }
  return os;
}

int main() {
  // Multiply a few random numbers.
  srand(time(0));
  for (auto i = 0; i < 1000; ++i) {
    intmax_t vm1 = rand();
    intmax_t vm2 = rand();
    intmax_t vm = vm1 * vm2;
    large_num_t m1(vm1);
    large_num_t m2(vm2);
    large_num_t m = (m1 * m2);
    if (m.to_int() != vm) {
      std::cout << i << ": " << m1 << " X " << m2 << " = " << m << std::endl;
      std::cout << std::hex << std::endl
                << "FAIL: " << m << " != " << vm << std::dec << std::endl;
      std::cout << "vm1: " << vm1 << " vm2: " << vm2 << std::endl;
      std::cout << m.to_int() << " != " << vm << std::endl;
      break;
    } else {
      std::cout << i << ": " << m1 << " X " << m2 << " = " << m << std::endl;
    }
  }

  // Multiply a couple of very big numbers.
  large_num_t n1("111111111111111111111111111111111111111111111111");
  large_num_t n2("123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0");

  std::cout << std::endl << n1 << " X " << n2 << " = " << std::endl;
  std::cout << n1 * n2 << std::endl;

  return 0;
}
