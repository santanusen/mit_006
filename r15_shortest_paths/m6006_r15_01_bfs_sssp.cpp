//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <cstdint>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>

// An encapsulation of a graph stored as an adjacency list.
class Graph {

public:
  typedef std::string vertex_t;
  typedef std::unordered_set<vertex_t> neighbors_t;
  typedef std::unordered_map<vertex_t, neighbors_t> adj_list_t;

  enum directionality_t { DIRECTED, UNDIRECTED };

  typedef std::list<vertex_t> vertex_list_t;
  typedef std::list<std::pair<vertex_t, vertex_t>> edge_list_t;

  // An interface to implement functionalities on top of graph exploration
  // algorithms such as BFS and DFS. The exploration algorithm notifies
  // the explorer about events such as start and finish of exploration of
  // a vertex, an edge being followed or ignored, etc. The explorer, in turn
  // keeps track of the vertices already visited and provides the information
  // to the exploration algorithm.
  class explorer {
  public:
    typedef std::unordered_map<vertex_t, vertex_t> parents_t;
    parents_t mParents;

    virtual void component_exploration_start(const vertex_t &v) {
      mParents[v] = vertex_t();
    }
    virtual void component_exploration_finish(const vertex_t &) {}
    virtual void exploration_start(const vertex_t &) {}
    virtual void exploration_finish(const vertex_t &) {}
    virtual void edge_followed(const vertex_t &src, const vertex_t &dst) {
      mParents[dst] = src;
    }
    virtual void edge_ignored(const vertex_t &, const vertex_t &) {}

    virtual bool is_visited(const vertex_t &v) const {
      return (mParents.find(v) != mParents.end());
    }

    virtual bool is_done() const { return false; }

    virtual ~explorer() {}
  };

private:
  // BFS
  void bfs_visit(const vertex_t &s, explorer &exp) const {

    exp.component_exploration_start(s);
    std::list<vertex_t> frontier;

    // Start BFS with start node.
    frontier.push_back(s);

    // BFS loop.
    while (!frontier.empty()) {
      auto u = frontier.front();
      frontier.pop_front();

      exp.exploration_start(u);

      // Reached end?
      if (exp.is_done()) {
        break;
      }

      const auto i = mAdjList.find(u);
      if (i != mAdjList.end()) {
        for (const auto &v : i->second) {
          if (!exp.is_visited(v)) {
            exp.edge_followed(u, v);
            frontier.push_back(v);
          } else {
            exp.edge_ignored(u, v);
          }
        }
      }

      exp.exploration_finish(u);
    }
    exp.component_exploration_finish(s);
  }

  // Functionalities implemented on top of BFS.

  // Finds the shortest path between a pair of vertices.
  class bfs_shortest_path_finder : public explorer {

  private:
    bool mReachedDst;

    const vertex_t &mSrc;

    const vertex_t &mDst;

  public:
    bfs_shortest_path_finder(const vertex_t &src, const vertex_t &dst)
        : mReachedDst(false), mSrc(src), mDst(dst) {}

    virtual void exploration_start(const vertex_t &v) override {
      // Continue BFS till destination vertex is reached.
      if (v == mDst)
        mReachedDst = true;
    }

    virtual bool is_done() const override { return mReachedDst; }

    void get_shortest_path(vertex_list_t &path) const {
      auto i = mParents.find(mDst);
      if (i == mParents.end())
        return;

      // Follow the parents from destination till source.
      while (i->first != mSrc) {
        path.push_front(i->first); // Build the path from back to front.
        i = mParents.find(i->second);
      }

      path.push_front(mSrc);
    }
  };

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
  void add_edge(const vertex_t &src, const vertex_t &dst) {
    mAdjList[src].insert(dst);
    if (mDir == UNDIRECTED)
      mAdjList[dst].insert(src);
    /*else
      add_vertex(dst);*/
  }

  void find_shortest_path(const vertex_t &src, const vertex_t &dst,
                          vertex_list_t &path) const {
    bfs_shortest_path_finder bspf(src, dst);
    bfs_visit(src, bspf);
    bspf.get_shortest_path(path);
  }

  friend std::ostream &operator<<(std::ostream &os, const Graph &g);
};

std::ostream &operator<<(std::ostream &os, const Graph &g) {

  for (const auto &p : g.mAdjList) {
    os << p.first << " : {";
    for (const auto &n : p.second)
      os << n << ", ";
    os << "}" << std::endl;
  }
  return os;
}

// Transform a weighted graph into unweighted graph by converting
// an edge of weight w into a path of length w.
bool add_weighted_edge(Graph &g, const Graph::vertex_t &src,
                       const Graph::vertex_t &dst, uint32_t weight) {
  if (weight == 0)
    return false;

  const Graph::vertex_t dummy_pref = "." + src + "-" + dst + "-";

  auto s = src;
  // Insert (weight - 1) dummy vertices between src and dst.
  for (uint32_t w = 1; w < weight; ++w) {
    auto d = dummy_pref + std::to_string(w);
    g.add_edge(s, d);
    s = d;
  }

  g.add_edge(s, dst);

  return true;
}

// Prune dummy vertices between from the path.
void prune_dummy_vertices(Graph::vertex_list_t &path) {
  for (auto i = path.begin(); i != path.end();) {
    auto ii = i++;
    if (ii->at(0) == '.')
      path.erase(ii);
  }
}

int main() {

  Graph g(Graph::DIRECTED);

  add_weighted_edge(g, "A", "B", 6);
  add_weighted_edge(g, "A", "C", 3);
  add_weighted_edge(g, "C", "B", 2);
  std::cout << g << std::endl;

  Graph::vertex_t src = "A";
  Graph::vertex_t dst = "B";
  Graph::vertex_list_t path;
  g.find_shortest_path(src, dst, path);
  prune_dummy_vertices(path);
  std::cout << "Shortest Path from " << src << " to " << dst << " : ";
  for (const auto &v : path)
    std::cout << v << " ";

  std::cout << std::endl;

  return 0;
}
