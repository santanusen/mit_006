//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>

// Dance Dance Revolution.

// Game Overview:
//
// The dance board:
// The dance board is a board with the following tap-keys.
//
//         [TOP]
//
// [LEFT] [CENTER] [RIGHT]
//
//        [BOTTOM]
//
// Notes:
// A note is a combination of zero, one or two tap-key positions.
// Examples -
// Two position note:    TOP-LEFT
// Single position note: RIGHT
// A note with zero positions is also called a blank note.
//
// Constraints:
// When a note is displayed, the feet of the player must be on the tap-keys
// directed be the note. For example, when the note TOP-LEFT is displayed
// one of the player's feet should be on the top tap-key of the board while
// the other foot should be on the right tap-key. When the note LEFT is
// displayed, one foot of the player must be on the LEFT tap-key while the
// other foot is allowed to be at any tap-key. A player's feet is allowed
// to be on any two keys when a blank note is displayed.
//
// Objective:
// A sequence of notes with each alternate note being a blank note is displayed.
// The player has to move her/his feet so that the feet position constraints
// directed by the notes are satisfied.
//
// Problem Statement:
// Given a sequence of notes, figure out the set of steps that satisfy the
// feet position constraints while optimizing one of the objectives specified
// through a delta function. Examples of such objectives include -
// - Minimize foot movement.
// - Maximize entertainment (maximize calorie burn / entertainment).

#define INFINITE 0x1FFFFFFF

// Positions on the DDR board.
enum position_t { TOP, LEFT, CENTER, RIGHT, BOTTOM, POS_ANY };

std::ostream &operator<<(std::ostream &os, const position_t &pos) {
  const char *poschars = "TLCRB*";
  os << poschars[pos];
  return os;
}

// A note is a combination of maximum two positions.
typedef std::pair<position_t, position_t> note_t;

std::ostream &operator<<(std::ostream &os, const note_t &note) {
  os << "[" << note.first << note.second << "]";
  return os;
}

// Compose a note from the input positions. Avoid duplicates like LR and RL.
note_t compose_note(position_t p1, position_t p2) {
  return std::make_pair(std::max(p1, p2), std::min(p1, p2));
}

// Check if the feet positions satisfy the note constraints.
bool note_satisfied(note_t note, position_t f1_pos, position_t f2_pos) {
  auto p1 = std::max(f1_pos, f2_pos);
  auto p2 = std::min(f1_pos, f2_pos);

  if (note.first == POS_ANY)
    return note.second == POS_ANY || note.second == p1 || note.second == p2;

  return note.first == p1 && note.second == p2;
}

// Calculate the distance (squared) between two positions.
// Distance squared to avoid square-root calculations.
ssize_t distance(position_t p1, position_t p2) {
  typedef std::pair<ssize_t, ssize_t> coord_t;
  static const coord_t coords[] = {{0, 1}, {1, 0}, {1, 1}, {1, 2}, {2, 1}};

  if (p1 >= POS_ANY || p1 < 0 || p2 >= POS_ANY || p2 < 0) {
    return INFINITE;
  }

  return ((coords[p1].first - coords[p2].first) *
              (coords[p1].first - coords[p2].first) +
          (coords[p1].second - coords[p2].second) *
              (coords[p1].second - coords[p2].second));
}

// Returns the negative of the distance between two positions.
// Minimizing the negative distance maximizes the foot movement (a.k.a.
// entertainment.)
ssize_t negative_distance(position_t p1, position_t p2) {
  auto dist = distance(p1, p2);
  return (dist == INFINITE) ? dist : (-1 * dist);
}

// Generates a note of two random positions.
// Makes sure that the two positions are not the same; thus also avoiding
// generating a blank note.
note_t rand_note() {
  auto p1 = static_cast<position_t>(rand() % POS_ANY);
  auto p2 = p1;
  while (p2 == p1) {
    p2 = static_cast<position_t>(rand() % (POS_ANY + 1));
  }
  return compose_note(p1, p2);
}

// Given the input sequence of 'N' 'notes', calculate the sequence of steps to
// minimize the total 'delta' cost.
void ddr_dp(const note_t *notes, size_t N,
            ssize_t (&delta)(position_t, position_t)) {

  // DP table: The element at index [n][i][j] is the minimum total cost of steps
  // for notes (N - 1) to n with the left and right foot ending up at positions
  // i and j.

  ssize_t cost_matrix[N][POS_ANY][POS_ANY];
  // Links table to remeber the from which position we get to the current
  // position ([n][i][j]) with least cost. The elements at index [n][i][j][0]
  // and at index [n][i][j][1] specify the left and right foot position from
  // which we can arrive at the current position with least cost.
  position_t next_pos[N][POS_ANY][POS_ANY][2];

  // Seed values. Start from the last note with zero cost.
  for (auto i = 0; i < POS_ANY; ++i) {
    for (auto j = 0; j < POS_ANY; ++j) {
      cost_matrix[N - 1][i][j] = 0;
      next_pos[N - 1][i][j][0] = POS_ANY;
      next_pos[N - 1][i][j][1] = POS_ANY;
    }
  }

  // Topological order: From last note to the first.
  for (ssize_t n = N - 2; n >= 0; --n) {
    for (auto i = 0; i < POS_ANY; ++i) {
      for (auto j = 0; j < POS_ANY; ++j) {
        // Calculate minimum of: cost of a position till the previous note plus
        // the 'delta' cost of feet movement from the previous position to the
        // current.
        cost_matrix[n][i][j] = INFINITE;
        next_pos[n][i][j][0] = POS_ANY;
        next_pos[n][i][j][1] = POS_ANY;

        auto pi = static_cast<position_t>(i);
        auto pj = static_cast<position_t>(j);

        if (note_satisfied(notes[n], pi, pj)) {
          for (auto ii = 0; ii < POS_ANY; ++ii) {
            for (auto jj = 0; jj < POS_ANY; ++jj) {
              auto pii = static_cast<position_t>(ii);
              auto pjj = static_cast<position_t>(jj);
              auto cost =
                  cost_matrix[n + 1][ii][jj] + delta(pii, pi) + delta(pjj, pj);
              if (cost < cost_matrix[n][i][j]) {
                // Update the minimum cost.
                cost_matrix[n][i][j] = cost;
                // Remember from which previous position we arrived at the
                // current position with least cost.
                next_pos[n][i][j][0] = pii;
                next_pos[n][i][j][1] = pjj;
              }
            }
          }
        }
      }
    }
  }

  // Construct the solution back.

  // First find out which position for the start note has the least cost.
  position_t imin = TOP;
  position_t jmin = TOP;

  for (auto i = 0; i < POS_ANY; ++i)
    for (auto j = 0; j < POS_ANY; ++j)
      if (cost_matrix[0][i][j] < cost_matrix[0][imin][jmin]) {
        imin = static_cast<position_t>(i);
        jmin = static_cast<position_t>(j);
      }

  std::cout << "Min cost: " << cost_matrix[0][imin][jmin] << std::endl;

  // Then follow the next_post table.
  std::cout << "Steps: " << std::endl;
  for (auto n = 0u; n < N; ++n) {
    std::cout << "(" << imin << jmin << ") ";
    auto ii = next_pos[n][imin][jmin][0];
    auto jj = next_pos[n][imin][jmin][1];

    imin = ii;
    jmin = jj;
  }
  std::cout << std::endl;
}

int main() {

  srand(time(0));

  const size_t n = 10;

  // Alternating 'n' non-blank notes and blank notes.
  const size_t N = 2 * n;
  note_t notes[N];

  auto blank_note = compose_note(POS_ANY, POS_ANY);
  for (auto i = 0u; i < N; i += 2) {
    notes[i] = rand_note();
    notes[i + 1] = blank_note;
  }

  std::cout << "Notes: " << std::endl;
  for (auto i = 0u; i < N; ++i)
    std::cout << notes[i] << " ";
  std::cout << std::endl;

  std::cout << "Steps for minimum feet movement: " << std::endl;
  ddr_dp(notes, N, distance);

  std::cout << "Steps for maximum entertainment: " << std::endl;
  ddr_dp(notes, N, negative_distance);

  return 0;
}
