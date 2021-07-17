//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

// Hash-map of word to frequency.
typedef std::unordered_map<std::string, int> freq_table_t;

// Dumps a frequency table to ostream.
std::ostream &operator<<(std::ostream &os, const freq_table_t &ft) {
  for (auto &p : ft)
    os << p.first << " : " << p.second << std::endl;
  return os;
}

// Extracts words from a string, converts to lowercase and
// adds to its count in the frequency table.
void count_word_frequency(const std::string str, freq_table_t &ft) {
  std::string word;

  for (std::string::size_type i = 0; i < str.length(); ++i) {
    if (std::isalnum(str.at(i))) {
      word.push_back(static_cast<char>(tolower(str.at(i))));
    } else if (!word.empty()) { // Non-alpha-numeric; end of current word.
      ++ft[word];
      word.clear();
    }
  }

  // Last word
  if (!word.empty()) {
    ++ft[word];
    word.clear();
  }
}

// Reads a flie line-by-line, extracts words from each line and updates
// their frequency in frequency table.
void count_file_word_frequency(const std::string &fname, freq_table_t &ft) {
  std::ifstream infile(fname);
  std::string line;
  while (std::getline(infile, line)) {
    count_word_frequency(line, ft);
  }
}

// Inner product of two frequency tables.
double inner_product(const freq_table_t &f1, const freq_table_t &f2) {
  double sum = 0.0;
  for (const auto &p : f1) {
    const auto i = f2.find(p.first);
    if (i != f2.end()) {
      sum += (p.second * i->second);
    }
  }

  return sum;
}

// Angle of two frequency tables.
double vector_angle(const freq_table_t &f1, const freq_table_t &f2) {
  auto numerator = inner_product(f1, f2);
  auto denominator = std::sqrt(inner_product(f1, f1) * inner_product(f2, f2));
  // std::cout << numerator << "/" << denominator << std::endl;
  return std::acos(numerator / denominator);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " file1 file2" << std::endl;
    return 1;
  }

  freq_table_t ft1, ft2;
  count_file_word_frequency(argv[1], ft1);
  count_file_word_frequency(argv[2], ft2);
  // std::cout << "ft1: " << std::endl << ft1 << std::endl;
  // std::cout << "ft2: " << std::endl << ft2 << std::endl;

  double angle = vector_angle(ft1, ft2);

  std::cout << angle << std::endl;

  return 0;
}
