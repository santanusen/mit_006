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

  // The adjacency list of backward edges.
  adj_list_t mAdjListBack;

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
    else
      mAdjListBack[dst].insert(make_pair(src, cost));
  }

  // Calculate Shortest Path using bidirectional Dijkstra's algorithm.
  void bd_dijkstra(const vertex_t &src, const vertex_t &dst) const {
    enum { FORWARD = 0, BACKWARD = 1, NDIR = 2 };
    // Backward adjacncy list.
    const adj_list_t *adjList[NDIR] = {
        &mAdjList, (mDir == UNDIRECTED) ? &mAdjList : &mAdjListBack};

    // Map of a vertex to its previous vertex in the shortest path from src.
    std::unordered_map<vertex_t, vertex_t> parents[NDIR];

    // Indexed priority queue tracking the node with the best cost.
    indexed_priority_queue<vertex_cost_t, vertex_cost_cmp, vertex_cost_key_hash,
                           vertex_cost_key_equal>
        ipq[NDIR];

    // The vertices that have already been relaxed.
    std::unordered_set<vertex_t> relaxed[NDIR];

    // Current weights of the vertices.
    std::unordered_map<vertex_t, int> delta[NDIR];

    // Start with the src/dst in the queue/backward queue with cost 0.
    ipq[FORWARD].push(std::make_pair(src, 0));
    parents[FORWARD][src] = vertex_t();
    delta[FORWARD][src] = 0;
    ipq[BACKWARD].push(std::make_pair(dst, 0));
    parents[BACKWARD][dst] = vertex_t();
    delta[BACKWARD][dst] = 0;

    bool done = false;

    while (!done) {
      int active_ipqs = 0;
      // One iteration in each direction.
      for (auto i = 0; i < NDIR; ++i) {
        if (ipq[i].empty())
          continue;
        ++active_ipqs;

        // Get the vertex with least cost.
        const auto cur_vc = ipq[i].top();
        ipq[i].pop();
        relaxed[i].insert(cur_vc.first);
        // Have the frontiers just collided?
        if (relaxed[NDIR - 1 - i].find(cur_vc.first) !=
            relaxed[NDIR - 1 - i].end()) {
          done = true;
          break;
        }

        auto adjItr = adjList[i]->find(cur_vc.first);
        if (adjItr != adjList[i]->end()) {
          // Iterate over all the edges emanating out of the current vertex.
          for (const auto &edge : adjItr->second) {
            if (relaxed[i].find(edge.first) == relaxed[i].end()) {
              // Find the destination of the edge in the ipq.
              // Make find return INFINITE cost if not present.
              const auto invc = std::make_pair(edge.first, INFINITE);
              const auto vc = ipq[i].find(invc, invc);
              // Relax.
              if (cur_vc.second + edge.second < vc.second) {
                // If the vertex is already present in the ipq, push will update
                // its cost and reposition it in the priority queue based on the
                // updated cost.
                ipq[i].push(
                    std::make_pair(edge.first, cur_vc.second + edge.second));
                parents[i][edge.first] = cur_vc.first;
                delta[i][edge.first] = cur_vc.second + edge.second;
              }
            }
          }
        }
      }
      // All IPQs are empty.
      if (active_ipqs == 0)
        done = true;
    }

    // Find the common vertex in both forward and backward directions with
    // minimum cost.
    vertex_t min_common_vertex;
    int min_common_vertex_cost = INFINITE;
    for (const auto &vcf : delta[FORWARD]) {
      auto dbi = delta[BACKWARD].find(vcf.first);
      if (dbi != delta[BACKWARD].end()) {
        if (vcf.second + dbi->second < min_common_vertex_cost) {
          min_common_vertex = vcf.first;
          min_common_vertex_cost = vcf.second + dbi->second;
        }
      }
    }

    if (min_common_vertex_cost == INFINITE) {
      std::cout << "No paths from: " << src << " to " << dst << std::endl;
      return;
    }

    // Construct and print the shortest paths by following the parent links.
    // Inf forward direction : src to min_common_vertex.
    // Inf backward direction : from min_common_vertex to dst.
    // Combine the results.
    std::cout << "Shortest paths from: " << src << " to " << dst << std::endl;
    vertex_t s[NDIR] = {src, dst};
    vertex_t d[NDIR] = {min_common_vertex, min_common_vertex};
    path_t path;
    for (auto i = 0; i < NDIR; ++i) {
      path_t spath;
      for (auto v = d[i]; v != s[i]; v = parents[i][v]) {
        if (i == FORWARD)
          spath.push_front(v);
        else
          spath.push_front(parents[i][v]);
      }

      path.splice(path.end(), spath);
    }
    path.push_front(src);

    for (const auto vp : path)
      std::cout << vp << " ";
    std::cout << std::endl;
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
  Graph g(Graph::DIRECTED);

  g.add_edge("S", "U", 3);
  g.add_edge("U", "U1", 3);
  g.add_edge("U1", "T", 3);
  g.add_edge("S", "W", 5);
  g.add_edge("W", "T", 5);

  std::cout << "Graph: " << std::endl << g << std::endl;

  // Bidirectional Dijkstra's.
  g.bd_dijkstra("S", "T");

  return 0;
}
