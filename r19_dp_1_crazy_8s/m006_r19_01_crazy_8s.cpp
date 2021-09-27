//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>
#include <list>

#define NCVALS 13

// Represent a card using a card number (1 - 13) and a
// card type (CLUB, DIAMOND, HEARTS, SPADE).
enum card_type_t { CLUB, DIAMOND, HEARTS, SPADE, NCTYPES = 4 };
typedef std::pair<int, card_type_t> card_t;

std::ostream &operator<<(std::ostream &os, const card_t &c) {
  const char *card_val_table = "0A23456789TJQK";
  char vch = '#';
  if (c.first >= 1 && c.first <= NCVALS)
    vch = card_val_table[c.first];

  const char *card_type_table = "CDHS";
  char tch = 'X';
  if (c.second >= CLUB && c.second < NCTYPES)
    tch = card_type_table[c.second];

  os << "[" << vch << tch << "]";

  return os;
}

// Matching rule: Both the crads have same number or both the cards have the
// same type or one of the cards is an 8.
bool is_crazy_match(const card_t &c1, const card_t &c2) {
  return (c1.first == 8 || c2.first == 8 || c1.first == c2.first ||
          c1.second == c2.second);
}

// Calculate the longest crazy subsequence using DP.
// Backward scan from the last card to the first.
// For each card check if it is a crazy match for the previous cards in the
// deck. If it is a match, the longest crazy sequence starting from the previous
// card is the longer sequnce between the current longest sequence starting at
// the previous card and the longest crazy sequence starting from the current
// card plus the previous card.
void print_longest_craze_subseq(const card_t *deck, size_t DECKSZ) {
  // Index of the next card in the longest crazy subsequence starting from the
  // current card.
  size_t next[DECKSZ];
  for (auto i = 0u; i < DECKSZ; ++i)
    next[i] = DECKSZ;

  // Length of the longest crazy subsequence starting from the current card.
  size_t seqlen[DECKSZ];
  for (auto i = 0u; i < DECKSZ; ++i)
    seqlen[i] = 1;

  // Index of the first card in the longest crazy subsequnce.
  size_t smax = DECKSZ - 1;

  // Fill in the DP tables.
  for (auto i = DECKSZ - 1; i > 0; --i) {
    for (ssize_t j = i - 1; j >= 0; --j) {
      if (is_crazy_match(deck[i], deck[j])) {
        if (seqlen[i] + 1 > seqlen[j]) {
          seqlen[j] = seqlen[i] + 1;
          next[j] = i;

          if (seqlen[j] > seqlen[smax])
            smax = j;
        }
      }
    }
  }

  // Print the longest crazy subsequence.
  for (auto i = smax; i < DECKSZ; i = next[i])
    std::cout << deck[i] << " ";
  std::cout << std::endl;
}

// Similar logic as above, but using a forward pass through the deck of cards
// instead.
void print_longest_craze_subseq2(const card_t *deck, size_t DECKSZ) {
  size_t prev[DECKSZ];
  for (auto i = 0u; i < DECKSZ; ++i)
    prev[i] = DECKSZ;

  size_t seqlen[DECKSZ];
  for (auto i = 0u; i < DECKSZ; ++i)
    seqlen[i] = 1;

  size_t smax = 0;

  for (auto i = 0u; i < DECKSZ - 1; ++i) {
    for (auto j = i + 1; j < DECKSZ; ++j) {
      if (is_crazy_match(deck[i], deck[j])) {
        if (seqlen[i] + 1 > seqlen[j]) {
          seqlen[j] = seqlen[i] + 1;
          prev[j] = i;

          if (seqlen[j] > seqlen[smax])
            smax = j;
        }
      }
    }
  }

  std::list<size_t> iseq;
  for (auto i = smax; i < DECKSZ; i = prev[i])
    iseq.push_front(i);

  for (auto i : iseq)
    std::cout << deck[i] << " ";
  std::cout << std::endl;
}

int main() {

  const size_t DECKSZ = 15;
  card_t deck[DECKSZ];

  // Create a random deck of cards.
  srand(time(0));
  for (auto i = 0u; i < DECKSZ; ++i) {
    deck[i] = std::make_pair(1 + (rand() % NCVALS),
                             static_cast<card_type_t>(rand() % NCTYPES));
  }

  for (auto i = 0u; i < DECKSZ; ++i)
    std::cout << deck[i] << " ";
  std::cout << std::endl;

  // Find the longest crazy subsequence.
  std::cout << "Longest crazy sub sequence: " << std::endl;

  std::cout << "Method 1: ";
  print_longest_craze_subseq(deck, DECKSZ);

  std::cout << "Method 2: ";
  print_longest_craze_subseq2(deck, DECKSZ);

  return 0;
}
