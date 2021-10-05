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
  const char *card_vals = "0A23456789TJQK";
  const char *card_types = "CDHS";
  os << "[" << ((c.first >= 1 && c.first <= NCVALS) ? card_vals[c.first] : '#')
     << ((c.second >= CLUB && c.second < NCTYPES) ? card_types[c.second] : 'X')
     << "]";

  return os;
}

size_t total_card_value(const card_t *deck, const std::list<size_t> &cards,
                        bool is_ace_11) {
  size_t sum = 0;
  for (auto c : cards) {
    if (deck[c].first == 1) // Ace: Either 1 or 11.
      sum += ((is_ace_11) ? 11 : 1);
    else if (deck[c].first > 1 && deck[c].first < 11)
      sum += deck[c].first;
    else // Picture Cards.
      sum += 10;
  }

  return sum;
}

size_t best_total_card_value(const card_t *deck,
                             const std::list<size_t> &cards) {
  auto value = total_card_value(deck, cards, true);
  return (value > 21) ? total_card_value(deck, cards, false) : value;
}

typedef std::pair<size_t /* cards_played */, int /* player_income */>
    round_res_t;

// Executes one round of play, starting with the card at index idx in the deck
// and returns the number of cards played and the income of the player. Player
// income returned =
//   1: player wins.
//  -1: dealer wins.
//   0: tie.
round_res_t round_outcome(const card_t *deck, size_t DECKSZ, size_t idx,
                          size_t hits, bool verbose = false) {
  size_t cards_played = 0;
  std::list<size_t> dealer_cards, player_cards;

  // Deal: Distribute one-one card each to player and dealer twice.
  for (auto i = 0; i < 2; ++i) {
    if (idx + cards_played < DECKSZ)
      player_cards.push_back(idx + cards_played++);
    else
      break;
    if (idx + cards_played < DECKSZ)
      dealer_cards.push_back(idx + cards_played++);
    else
      break;
  }

  // Player's hits:
  for (size_t h = 0; h < hits; ++h) {
    // If player has bust, stop.
    if (best_total_card_value(deck, player_cards) > 21)
      break;

    if (idx + cards_played < DECKSZ)
      player_cards.push_back(idx + cards_played++);
    else
      break;
  }

  auto player_value = best_total_card_value(deck, player_cards);

  int player_income = 0;

  if (player_value > 21) // Player bust
    player_income = -1;
  else {
    // Dealer's hits: Keep hitting till value < 17
    while (total_card_value(deck, dealer_cards, true) < 17) {
      // If dealer has won, stop.
      if (best_total_card_value(deck, dealer_cards) > player_value)
        break;

      if (idx + cards_played < DECKSZ)
        dealer_cards.push_back(idx + cards_played++);
      else
        break;
    }

    auto dealer_value = best_total_card_value(deck, dealer_cards);

    if (dealer_value > 21) // Dealer bust
      player_income = 1;
    else if (player_value > dealer_value)
      player_income = 1;
    else if (player_value < dealer_value)
      player_income = -1;
  }

  if (verbose) {
    std::cout << std::endl << "Player Cards: ";
    for (auto c : player_cards)
      std::cout << deck[c];
    std::cout << std::endl << "Player Value: " << player_value << std::endl;

    std::cout << "Dealer Cards: ";
    for (auto c : dealer_cards)
      std::cout << deck[c];
    std::cout << std::endl
              << "Dealer Value: " << best_total_card_value(deck, dealer_cards)
              << std::endl;
    std::cout << "Player Earnings: " << player_income << std::endl;
  }

  return std::make_pair(cards_played, player_income);
}

void blackjack_play_dp(const card_t *deck, size_t DECKSZ) {
  // DP Table: Stores the maximum possible profit possible if a round starts at
  // the same index in deck.
  int max_profit[DECKSZ + 1];
  // Number of hits that gives the maximum profit.
  size_t best_hit[DECKSZ + 1];

  // Seed value: If no card left in deck, no profit or loss.
  max_profit[DECKSZ] = 0;
  best_hit[DECKSZ] = 0;

  for (ssize_t i = DECKSZ - 1; i >= 0; --i) {
    max_profit[i] = -1 * DECKSZ; // Min possible profit.
    best_hit[i] = 0;
    // Save what number of hits 'h' give the maximum profit.
    for (size_t h = 0; h < DECKSZ - i; ++h) {
      auto rout = round_outcome(deck, DECKSZ, i, h);
      if (rout.second + max_profit[i + rout.first] > max_profit[i]) {
        max_profit[i] = rout.second + max_profit[i + rout.first];
        best_hit[i] = h;
      }
    }
  }

  // Now play as per calculated solution.
  size_t idx = 0;
  int profit = 0;
  while (idx < DECKSZ) {
    auto rout = round_outcome(deck, DECKSZ, idx, best_hit[idx], true);
    idx += rout.first;
    profit += rout.second;
  }

  std::cout << std::endl
            << "Profit prdicted: " << max_profit[0] << std::endl
            << "Profit earned: " << profit << std::endl;
}

int main() {

  constexpr size_t DECKSZ = NCVALS * NCTYPES;
  card_t deck[DECKSZ];

  for (int t = CLUB; t <= SPADE; ++t)
    for (auto n = 1; n <= NCVALS; ++n)
      deck[t * NCVALS + n - 1] = std::make_pair(n, static_cast<card_type_t>(t));

  // Shuffle.
  srand(time(0));
  for (auto i = 0u; i < DECKSZ; ++i)
    std::swap(deck[i], deck[rand() % DECKSZ]);

  for (auto i = 0u; i < DECKSZ; ++i)
    std::cout << deck[i] << " ";
  std::cout << std::endl;

  blackjack_play_dp(deck, DECKSZ);

  return 0;
}
