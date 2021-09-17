//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>
#include <list>
#include <unordered_set>

#include "indexed_priority_queue.hpp"

// Weighted graph using adjacency lists.
class Graph {

public:
  typedef std::string vertex_t;

  // A pair binding a vertex to a cost.
  typedef std::pair<vertex_t, int> vertex_cost_t;

  // Comparison functor to compare costs of two vertex_cost_t's.
  struct vertex_cost_cmp {
    bool operator()(const vertex_cost_t &lhs, const vertex_cost_t &rhs) const {
      return lhs.second < rhs.second;
    }
  };

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

  enum { INFINITE = 0x7FFFFFFF };

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
  }

  // Calculate Single-Source Shortest Paths using Dijkstra's algorith.
  void sssp_dijkstra(const vertex_t &src) const {

    // Map of a vertex to its previous vertex in the shortest path from src.
    std::unordered_map<vertex_t, vertex_t> parents;

    // Indexed priority queue tracking the node with the best cost.
    indexed_priority_queue<vertex_cost_t, vertex_cost_cmp, vertex_cost_key_hash,
                           vertex_cost_key_equal>
        ipq;

    // The vertices that have already been relaxed.
    std::unordered_set<vertex_t> relaxed;

    // Start with the src in the queue with cost 0.
    ipq.push(std::make_pair(src, 0));
    parents[src] = vertex_t();

    while (!ipq.empty()) {
      // Get the vertex with least cost.
      const auto cur_vc = ipq.top();
      ipq.pop();
      relaxed.insert(cur_vc.first);

      auto adjItr = mAdjList.find(cur_vc.first);
      if (adjItr != mAdjList.end()) {
        // Iterate over all the edges emanating out of the current vertex.
        for (const auto &edge : adjItr->second) {
          if (relaxed.find(edge.first) == relaxed.end()) {
            // Find the destination of the edge in the ipq.
            // Make find return INFINITE cost if not present.
            const auto invc = std::make_pair(edge.first, INFINITE);
            const auto vc = ipq.find(invc, invc);
            // Relax.
            if (cur_vc.second + edge.second < vc.second) {
              // If the vertex is already present in the ipq, push will update
              // its cost and reposition it in the priority queue based on the
              // updated cost.
              ipq.push(std::make_pair(edge.first, cur_vc.second + edge.second));
              parents[edge.first] = cur_vc.first;
            }
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

  // Dijkstra's.
  g.sssp_dijkstra("G");

  return 0;
}
