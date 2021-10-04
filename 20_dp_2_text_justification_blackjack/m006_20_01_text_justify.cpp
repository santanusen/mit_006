//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define INFINITE 0x7FFFFFFFFFFFFFFF

typedef std::vector<std::string> word_list_t;

// Extracts words from a string and adds it to the word list.
void extract_line_words(const std::string str, word_list_t &wl) {
  std::string word;

  for (std::string::size_type i = 0; i < str.length(); ++i) {
    if (std::isalnum(str.at(i)) || std::ispunct(str.at(i))) {
      word.push_back(static_cast<char>(str.at(i)));
    } else if (!word.empty()) { // Non-alpha-numeric; end of current word.
      wl.push_back(word);
      word.clear();
    }
  }

  // Last word
  if (!word.empty()) {
    wl.push_back(word);
    word.clear();
  }
}

// Reads a flie line-by-line, extracts words from each line and adds them
// to the word list.
void extract_file_words(const std::string &fname, word_list_t &wl) {
  std::ifstream infile(fname);
  std::string line;
  while (std::getline(infile, line)) {
    extract_line_words(line, wl);
  }
}

// Returns the width of list of words starting at 'from' upto, but not
// including, 'to' in the word list.
size_t word_sublist_width(const word_list_t &wl, size_t from, size_t to) {
  size_t nc = 0, nw = 0;
  for (auto i = from; i < to; ++i) {
    nc += wl[i].size();
    ++nw;
  }

  if (nw > 0)
    --nw; // No space after the last word.
  return nc + nw;
}

// Returns how bad is the look of a line that includes the words at 'from' upto,
// but not including, 'to' in the word list.
size_t badness(const word_list_t &wl, size_t from, size_t to, size_t width) {
  const auto slw = word_sublist_width(wl, from, to);
  if (slw > width) // Exceeding the width is prohibited.
    return INFINITE;

  const auto diff = width - slw;
  return (diff * diff * diff); // Why? Because LaTex uses the same.
}

// Justify text using DP.
void text_justify(const std::string &fname, size_t width) {
  word_list_t wl;
  extract_file_words(fname, wl);
  if (wl.empty())
    return;

  const ssize_t N = wl.size();

  // DP table. Stores the minimum badness of text that starts a line at the word
  // at the same index in the word table.
  size_t min_badness[N + 1];

  // Parent pointers to costruct the final solution.
  size_t next_line_start[N + 1];

  // Seed value. A line starts at the 'virtual' word past the last word.
  min_badness[N] = 0;
  next_line_start[N] = N;

  // Fill the DP table.
  for (auto i = N - 1; i >= 0; --i) {
    min_badness[i] = INFINITE;
    next_line_start[i] = N;
    for (auto j = i + 1; j <= N; ++j) {
      // Calculate the total badness at i if the next line starts at j.
      auto bad = badness(wl, i, j, width) + min_badness[j];
      if (bad < min_badness[i]) {
        min_badness[i] = bad;
        next_line_start[i] = j;
      }
    }
  }

  // Follow where the next line break is and print the text.
  for (auto ls = 0; ls < N; ls = next_line_start[ls]) {
    ssize_t nls = next_line_start[ls];
    std::string delim = "";
    for (auto i = ls; i < nls; ++i) {
      std::cout << delim << wl[i];
      delim = " ";
    }
    std::cout << std::endl;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <text_file_path> <width>"
              << std::endl;
    return 1;
  }

  auto width = std::atoi(argv[2]);
  if (width <= 0) {
    std::cerr << "Invalid width: " << argv[2] << std::endl;
    return 1;
  }

  text_justify(argv[1], width);

  return 0;
}
