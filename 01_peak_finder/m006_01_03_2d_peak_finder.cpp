//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>
#include <utility>

#define M 4
#define N 4

// Greedy ascent peak finding algorithm.
// Check if the current position a peak.
// If not, pick the first higher neighboring position
// as the current position and repeat.
std::pair<int, int> greedy_ascent_peak_finder(const int a[M][N]) {
  int r = 0, c = 0;
  while (true) {
    std::cout << a[r][c] << " ";

    // Move to the neighboring position if
    // it is higher than our current position.
    if (r > 0 && a[r - 1][c] > a[r][c])
      r = r - 1;
    else if (r < M - 1 && a[r + 1][c] > a[r][c])
      r = r + 1;
    else if (c > 0 && a[r][c - 1] > a[r][c])
      c = c - 1;
    else if (c < N - 1 && a[r][c + 1] > a[r][c])
      c = c + 1;
    else
      break;
  }
  std::cout << std::endl;

  return std::make_pair(r, c);
}

// Peak finding using divide and conquer.
// If we are on the way down peak is somewhere at left.
// If we are on the way up peak is somewhere at right.
std::pair<int, int> dc_2d_peak_finder(int a[M][N]) {
  int r = -1, c = -1;

  int cl = 0, cr = N - 1;
  while (true) {
    // Start at the middle column.
    c = (cl + cr) / 2;

    // Find the row with peak at this column
    // using divide and conquer.
    int rl = 0, rr = M - 1;
    while (rl <= rr) {
      r = (rl + rr) / 2;
      std::cout << "[" << r << ", " << c << "] ";
      if (r > 0 && a[r - 1][c] > a[r][c]) {
        rr = r - 1;
      } else if (r < M - 1 && a[r + 1][c] > a[r][c]) {
        rl = r + 1;
      } else {
        break;
      }
    }
    std::cout << std::endl;

    // All columns exhausted.
    if (cl == cr)
      break;

    // Check the peak at current column is also
    // a 2-D peak. Else, search for a peak in a
    // column in the ascending direction.
    if (c > 0 && a[r][c - 1] > a[r][c]) {
      cr = c - 1;
    } else if (c < N - 1 && a[r][c + 1] > a[r][c]) {
      cl = c + 1;
    } else {
      // We are at a peak;
      break;
    }
  }

  return std::make_pair(r, c);
}

int main() {
  // This matrix makes the greedy ascent
  // visit all the elements.
  int a[M][N] = {{0, 7, 8, 15}, {1, 6, 9, 14}, {2, 5, 10, 13}, {3, 4, 11, 12}};

  std::cout << "Greedy Ascent" << std::endl;
  auto p = greedy_ascent_peak_finder(a);
  std::cout << p.first << ", " << p.second << std::endl;
  std::cout << "________________________________________" << std::endl;

  std::cout << "Divide and Conquer" << std::endl;
  auto p2 = dc_2d_peak_finder(a);
  std::cout << p2.first << ", " << p2.second << std::endl;

  return 0;
}
