//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <cstdint>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#define INVALID INT64_MAX

// Characters denoting the significance of a maze position.
// See maze.txt for an example.
#define START 'S'
#define END 'E'
#define OBST 'O'
#define TRAIL '#'

class MazeBoard {
private:
  typedef std::vector<char> maze_row_t;
  typedef std::vector<maze_row_t> maze_t;

  maze_t maze;

  ssize_t start_i;
  ssize_t start_j;

  ssize_t end_i;
  ssize_t end_j;

  ssize_t nrows;
  ssize_t ncols;

public:
  MazeBoard()
      : start_i(INVALID), start_j(INVALID), end_i(INVALID), end_j(INVALID),
        nrows(INVALID), ncols(INVALID) {}

  // Reads a flie line-by-line, creates a maze row from each line.
  void load(const std::string &fname) {
    std::ifstream infile(fname);
    std::string line;
    ssize_t i = 0;
    while (std::getline(infile, line)) {
      maze.push_back(maze_row_t());
      auto &row = *(maze.rbegin());
      ssize_t j = 0;
      for (const auto &ch : line) {
        row.push_back(ch);

        if (ch == START) {
          start_i = i;
          start_j = j;
        } else if (ch == END) {
          end_i = i;
          end_j = j;
        }

        ++j;
      }
      ncols = std::min(j, ncols);

      ++i;
    }

    nrows = i;
  }

  // BFS solve
  void solve() {
    if (start_i == INVALID || start_j == INVALID || end_i == INVALID ||
        end_j == INVALID || nrows == INVALID || ncols == INVALID) {
      return;
    }

    // Instead of tuples, separate data-structures for each dimension is easier
    // to handle.
    ssize_t parent_i[nrows][ncols];
    ssize_t parent_j[nrows][ncols];

    for (ssize_t i = 0; i < nrows; ++i)
      for (ssize_t j = 0; j < ncols; ++j)
        parent_i[i][j] = parent_j[i][j] = INVALID;

    parent_i[start_i][start_j] = start_i;
    parent_j[start_i][start_j] = start_j;

    std::list<ssize_t> frontier_i;
    std::list<ssize_t> frontier_j;

    // Start BFS with start node.
    frontier_i.push_back(start_i);
    frontier_j.push_back(start_j);

    // Relative positions of neighbors wrt current node.
    const ssize_t move_i[] = {-1, 0, 0, 1};
    const ssize_t move_j[] = {0, -1, 1, 0};

    // BFS loop.
    while (!frontier_i.empty()) {
      auto i = frontier_i.front();
      auto j = frontier_j.front();
      frontier_i.pop_front();
      frontier_j.pop_front();

      // Found end?
      if (i == end_i && j == end_j) {
        break;
      }

      for (auto m = 0; m < 4; ++m) {
        auto ni = i + move_i[m];
        auto nj = j + move_j[m];

        if (ni >= 0 && ni < nrows && nj >= 0 && nj < ncols &&
            maze[ni][nj] != OBST && parent_i[ni][nj] == INVALID) {
          frontier_i.push_back(ni);
          frontier_j.push_back(nj);
          parent_i[ni][nj] = i;
          parent_j[ni][nj] = j;
        }
      }
    }

    auto pi = parent_i[end_i][end_j];
    auto pj = parent_j[end_i][end_j];

    // Draw the solution trail on the maze board.
    while (!(pi == start_i && pj == start_j) &&
           (pi != INVALID && pj != INVALID)) {
      maze[pi][pj] = TRAIL;
      auto pii = parent_i[pi][pj];
      auto pjj = parent_j[pi][pj];
      pi = pii;
      pj = pjj;
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const MazeBoard &maze);
};

std::ostream &operator<<(std::ostream &os, const MazeBoard &m) {
  os << "Rows: " << m.nrows << ", Cols: " << m.ncols << std::endl;
  os << "Start: (" << m.start_i << ", " << m.start_j << ")" << std::endl;
  os << "End: (" << m.end_i << ", " << m.end_j << ")" << std::endl;
  for (auto row : m.maze) {
    for (auto ch : row) {
      os << ch;
    }
    os << std::endl;
  }

  return os;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " maze_file" << std::endl;
    return 1;
  }

  MazeBoard m;
  m.load(argv[1]);
  m.solve();

  std::cout << m << std::endl;

  return 0;
}
