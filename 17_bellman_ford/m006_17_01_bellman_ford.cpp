//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>
#include <list>
#include <unordered_map>
#include <unordered_set>

// Weighted graph using adjacency lists.
class Graph {

public:
  typedef std::string vertex_t;

  // A pair binding a vertex to a cost.
  typedef std::pair<vertex_t, int> vertex_cost_t;

  // Hash functor for vertex_cost_t. Only the vertex_t (first) component is
  // used as a key.
  struct vertex_cost_key_hash {
    size_t operator()(const vertex_cost_t &vc) const {
      return std::hash<vertex_t>{}(vc.first);
    }
  };

  // Equality functor for vertex_cost_t. Only the vertex_t (first) component is
  // used as a key.
  struct vertex_cost_key_equal {
    size_t operator()(const vertex_cost_t &lhs,
                      const vertex_cost_t &rhs) const {
      return lhs.first == rhs.first;
    }
  };

  typedef std::unordered_set<vertex_cost_t, vertex_cost_key_hash,
                             vertex_cost_key_equal>
      neighbors_t;

  typedef std::unordered_map<vertex_t, neighbors_t> adj_list_t;

  typedef std::list<vertex_t> path_t;

  enum directionality_t { DIRECTED, UNDIRECTED };

private:
  // Whether edges are directed or undirected.
  const directionality_t mDir;

  // The adjacency list.
  adj_list_t mAdjList;

public:
  Graph(directionality_t d) : mDir(d) {}

  // Adds a single vertex to the graph. Useful to specify 0-degree vertices.
  void add_vertex(const vertex_t &vertex) {
    if (mAdjList.find(vertex) == mAdjList.end())
      mAdjList[vertex] = neighbors_t();
  }

  // Adds an edge to the graph.
  void add_edge(const vertex_t &src, const vertex_t &dst, int cost) {
    mAdjList[src].insert(std::make_pair(dst, cost));
    if (mDir == UNDIRECTED)
      mAdjList[dst].insert(make_pair(src, cost));
    else
      add_vertex(dst);
  }

  // Calculate Single-Source Shortest Paths using Bellman-Ford.
  void sssp_bellman_ford(const vertex_t &src) const {

    // Map of a vertex to its previous vertex in the shortest path from src.
    std::unordered_map<vertex_t, vertex_t> parents;

    // Map of a vertex to its shortest-path cost from src.
    std::unordered_map<vertex_t, int> sp_costs;

    // Start with the src with shortest path cost 0.
    sp_costs[src] = 0;
    parents[src] = vertex_t();

    // Iterate for |V| - 1 times.
    for (ssize_t i = 0; i < static_cast<ssize_t>(mAdjList.size()) - 1; ++i) {
      // Iterate over all the edges.
      for (const auto &adj : mAdjList) {
        const auto &sitr_cost = sp_costs.find(adj.first);
        if (sitr_cost != sp_costs.end()) {
          for (const auto &nbr : adj.second) {
            const auto &ditr_cost = sp_costs.find(nbr.first);
            // Relax.
            if (ditr_cost == sp_costs.end() ||
                ditr_cost->second > sitr_cost->second + nbr.second) {
              sp_costs[nbr.first] = sitr_cost->second + nbr.second;
              parents[nbr.first] = adj.first;
            }
          }
        }
      }
    }

    // Detect negative weight cycles by iterating over all the edges once more.
    for (const auto &adj : mAdjList) {
      const auto &sitr_cost = sp_costs.find(adj.first);
      if (sitr_cost != sp_costs.end()) {
        for (const auto &nbr : adj.second) {
          const auto &ditr_cost = sp_costs.find(nbr.first);
          // Relaxation possible even after |V| - 1 iterations.
          if (ditr_cost == sp_costs.end() ||
              ditr_cost->second > sitr_cost->second + nbr.second) {
            std::cout << "Negative-weight cycles reachable from " << src
                      << std::endl;
            return;
          }
        }
      }
    }

    // Construct and print the shortest paths by following the parent links.
    std::cout << "Shortest paths from: " << src << std::endl;
    for (const auto &p : parents) {
      if (p.first == src)
        continue;

      path_t path;
      auto v = p.first;
      while (v != src) {
        path.push_front(v);
        v = parents[v];
      }
      path.push_front(src);

      for (const auto vp : path)
        std::cout << vp << " ";
      std::cout << std::endl;
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const Graph &g);
};

std::ostream &operator<<(std::ostream &os, const Graph &g) {

  for (const auto &p : g.mAdjList) {
    os << p.first << " : {";
    for (const auto &n : p.second)
      os << "(" << n.first << ", " << n.second << "), ";
    os << "}" << std::endl;
  }
  return os;
}

int main() {
  Graph g(Graph::UNDIRECTED);

  g.add_edge("G", "Y", 19);
  g.add_edge("G", "P", 7);
  g.add_edge("P", "Y", 11);
  g.add_edge("Y", "R", 4);
  g.add_edge("P", "R", 15);
  g.add_edge("P", "B", 5);
  g.add_edge("R", "B", 13);
  std::cout << "Graph: " << std::endl << g << std::endl;

  // SSSP using Bellman-Ford.
  g.sssp_bellman_ford("G");

  // Introduce a negative weight cycle.
  g.add_edge("R", "O", -2);
  g.add_edge("O", "Y", -3);
  std::cout << std::endl
            << "Graph with a negative-weight cycle: " << std::endl
            << g << std::endl;

  // SSSP using Bellman-Ford.
  g.sssp_bellman_ford("G");
  return 0;
}
