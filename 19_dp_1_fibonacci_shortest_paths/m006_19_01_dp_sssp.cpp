//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iomanip>
#include <iostream>
#include <list>
#include <vector>

// Weighted graph using adjacency matrix.
class Graph {

public:
  // Vertices are indices to the adjacency matrix.
  typedef size_t vertex_t;

  // A 2-D vector to store adjacency matrix.
  typedef std::vector<std::vector<int>> adj_matrix_t;

  // A path representes as a list of vertices.
  typedef std::list<vertex_t> path_t;

  enum directionality_t { DIRECTED, UNDIRECTED };

private:
  // Infinite cost.
  static const int INFINITE = 0x0FFFFFFF;

  // Invalid index to the adjaceny matrix.
  static const size_t INVALID = 0xFFFFFFFF;

  // Whether edges are directed or undirected.
  const directionality_t mDir;

  // The adjacency matrix.
  adj_matrix_t mAdjMatrix;

  static bool is_infinite(int cost) { return (cost >= INFINITE); }

  // Cost addition with handling for INFINITE.
  static int add_cost(int a, int b) {
    return (is_infinite(a) || is_infinite(b)) ? INFINITE : (a + b);
  }

public:
  Graph(directionality_t d, size_t num_vertices)
      : mDir(d), mAdjMatrix(num_vertices) {
    // Initialize adjacency matrix with self-edge cost as 0, others as INFINITE.
    for (auto i = 0U; i < num_vertices; ++i) {
      mAdjMatrix[i].reserve(num_vertices);
      for (auto j = 0U; j < num_vertices; ++j)
        mAdjMatrix[i][j] = (i == j) ? 0 : INFINITE;
    }
  }

  // Adds an edge to the graph.
  void add_edge(const vertex_t &src, const vertex_t &dst, int cost) {
    if (src >= mAdjMatrix.size() || dst >= mAdjMatrix.size())
      return;

    mAdjMatrix[src][dst] = cost;
    if (mDir == UNDIRECTED)
      mAdjMatrix[dst][src] = cost;
  }

  // Single-source shortest path calculation using Dynamic Programming.
  // The longest acyclic shortest path in a graph can be of length |V| - 1.
  // Thus we iterate for (|V| - 1) times.
  // In k-th iteration we calculate the shortest paths at most of length k
  // from src to all other vertices.
  //
  // Invariant: After (k - 1)-th iteration the shortest paths of at most k
  // length from src to all other vertices are calculated.
  // At k-th iteration all the edges are relaxed once on top of the shortest
  // paths calculated at (k - 1)-th iteration; resulting in shortest paths
  // of at-most k lengths.
  //
  // DP Aspect: k-th iteration "reuses" the results of (k-1)-th iteration.
  // Correctness: Optimal-substructure principle - a shortest path of length k
  // from src to a vertex must include a shortest path of length (k - 1) from
  // src to the previous vertex in the path.
  //
  // In summary: We relax all edges for (|V| - 1) times.
  // Which is just Bellman-Ford.
  void sssp_dp(const vertex_t &src) const {
    const auto num_vertices = mAdjMatrix.size();
    if (src >= num_vertices)
      return;

    // The cost of shortest-paths to the vertices from src.
    // Start with a cost of 0 for src and INFINITE for all other vertices.
    int cost[num_vertices];
    for (auto i = 0U; i < num_vertices; ++i)
      cost[i] = INFINITE;
    cost[src] = 0;

    // The previous vertex in the shortest path from src to a vertex.
    size_t parent[num_vertices];
    for (auto i = 0U; i < num_vertices; ++i)
      parent[i] = INVALID;
    parent[src] = src;

    for (auto k = 0U; k < num_vertices - 1; ++k) {
      // At k-th iteration the cost and parents will hold the information
      // corresponding to the shortest path of size k from src.
      // Go through all the edges and relax them to keep this invariant.
      for (auto from = 0U; from < num_vertices; ++from) {
        for (auto to = 0U; to < num_vertices; ++to) {
          auto new_cost = add_cost(cost[from], mAdjMatrix[from][to]);
          if (new_cost < cost[to]) { // Relax
            cost[to] = new_cost;
            parent[to] = from;
          }
        }
      }
    }

    // If relaxation is still possible after |V| - 1 iterations then the graph
    // has a negative cost cycle.
    for (auto from = 0U; from < num_vertices; ++from) {
      for (auto to = 0U; to < num_vertices; ++to) {
        auto new_cost = add_cost(cost[from], mAdjMatrix[from][to]);
        if (new_cost < cost[to]) { // Relaxation still possible.
          std::cout << "Negative cost cycles present." << std::endl;
          return;
        }
      }
    }

    // Construct the shortest paths from src to a vertex by following the
    // parent links till src is reached.
    std::cout << "Shortest paths from: " << src << std::endl;
    for (auto i = 0U; i < num_vertices; ++i) {
      if (i == src)
        continue;
      if (parent[i] == INVALID)
        continue;

      path_t path;
      for (auto p = i; p != src; p = parent[p])
        path.push_front(p); // We are reverse traversing from a vertex to src.

      // Print the path.
      std::cout << src;
      for (const auto &v : path)
        std::cout << " <- " << v;
      std::cout << std::endl;
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const Graph &g);
};

std::ostream &operator<<(std::ostream &os, const Graph &g) {
  const auto num_vertices = g.mAdjMatrix.size();
  const auto col_width = 8;
  const auto row_width = ((num_vertices + 1) * col_width);

  auto hruler = [&]() {
    for (auto i = 0U; i < row_width; ++i)
      os << "=";
    os << std::endl;
  };

  // Heading.
  hruler();

  os << std::setw(col_width - 1) << " ";
  for (auto i = 0U; i < num_vertices; ++i)
    os << std::setw(col_width) << i;
  os << std::endl;

  hruler();

  // Rows.
  for (auto i = 0U; i < num_vertices; ++i) {
    // Row Heading.
    os << std::setw(col_width - 3) << i << " ||";

    // Columns.
    for (auto j = 0U; j < num_vertices; ++j) {
      os << " " << std::setw(col_width - 3);
      if (Graph::is_infinite(g.mAdjMatrix[i][j]))
        os << ".";
      else
        os << g.mAdjMatrix[i][j];
      os << " |";
    }
    os << std::endl;
  }

  hruler();

  return os;
}

int main() {
  enum { G, Y, P, R, B, O, VMAX };

  Graph g(Graph::UNDIRECTED, VMAX);

  g.add_edge(G, Y, 19);
  g.add_edge(G, P, 7);
  g.add_edge(P, Y, 11);
  g.add_edge(Y, R, 4);
  g.add_edge(P, R, 15);
  g.add_edge(P, B, 5);
  g.add_edge(R, B, 13);

  std::cout << "Graph: " << std::endl << g << std::endl;

  // Calculate SSSPs from G.
  g.sssp_dp(G);

  // Introduce a negative weight cycle.
  g.add_edge(R, O, -2);
  g.add_edge(O, Y, -3);
  std::cout << std::endl
            << "Graph with negative cost cycle: " << std::endl
            << g << std::endl;

  // This should now detect a negative cost cycle.
  g.sssp_dp(G);

  return 0;
}
