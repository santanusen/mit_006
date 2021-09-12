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

    virtual ~explorer() {}
  };

private:
  // Whether edges are directed or undirected.
  const directionality_t mDir;

  // The adjacency list.
  adj_list_t mAdjList;

  // DFS recursive call.
  void dfs_visit(const vertex_t &s, explorer &exp) const {
    exp.exploration_start(s);
    const auto i = mAdjList.find(s);
    if (i != mAdjList.end()) {
      for (const auto &v : i->second) {
        if (!exp.is_visited(v)) {
          exp.edge_followed(s, v);
          dfs_visit(v, exp);
        } else {
          exp.edge_ignored(s, v);
        }
      }
    }
    exp.exploration_finish(s);
  }

  // Implementation of DFS algorithm.
  // The explorer argument implements custom functionalities based on the
  // DFS algorithm.
  void dfs(explorer &exp) const {
    for (const auto &adj : mAdjList) {
      if (!exp.is_visited(adj.first)) {
        exp.component_exploration_start(adj.first);
        dfs_visit(adj.first, exp);
        exp.component_exploration_finish(adj.first);
      }
    }
  }

  // Functionalities implemented on top of DFS.

  // Prints the vertices visited by DFS.
  class dfs_walker : public explorer {
  public:
    virtual void exploration_start(const vertex_t &v) override {
      std::cout << v << " ";
    }

    virtual ~dfs_walker() { std::cout << std::endl; }
  };

  // Builds a list of backward edges in the graph.
  class dfs_back_edge_lister : public explorer {
  public:
    // DS to Keep track of the vertices in the dfs_visit call-stack.
    typedef std::unordered_set<vertex_t> exploration_stack_t;

    virtual void exploration_start(const vertex_t &v) override {
      mStack.insert(v);
    }

    virtual void exploration_finish(const vertex_t &v) override {
      mStack.erase(v);
    }

    virtual void edge_ignored(const vertex_t &src,
                              const vertex_t &dst) override {
      // If the destination is already in the call-stack, the destination is an
      // ancestor of the source.
      if (mStack.find(dst) != mStack.end())
        mBackEdges.push_back(std::make_pair(src, dst));
    }

    const edge_list_t &get_back_edges() const { return mBackEdges; }

  private:
    exploration_stack_t mStack;
    edge_list_t mBackEdges;
  };

  // Performs a topological sorting of the vertices.
  class dfs_topo_sorter : public explorer {
  private:
    vertex_list_t &mTopoOrder;

  public:
    dfs_topo_sorter(vertex_list_t &topo) : mTopoOrder(topo) {}

    // Vertices whose explorations are finished first should appear at
    // the end of the sorted list.
    virtual void exploration_finish(const vertex_t &v) override {
      mTopoOrder.push_front(v);
    }
  };

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

  void dfs_walk() const {
    dfs_walker dw;
    dfs(dw);
  }

  bool is_acyclic() const {
    dfs_back_edge_lister dbel;
    dfs(dbel);
    const auto &be = dbel.get_back_edges();

    // A graph with no backward edges is acyclic.
    return be.empty();
  }

  bool topo_sort(vertex_list_t &topo) const {
    // Only possible on DAGs.
    if (mDir != DIRECTED || !is_acyclic())
      return false;

    dfs_topo_sorter dts(topo);
    dfs(dts);
    return true;
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

int main() {

  Graph g(Graph::DIRECTED);

  g.add_edge("A", "B");
  g.add_edge("A", "G");
  g.add_edge("B", "C");
  g.add_edge("C", "D");
  g.add_edge("C", "F");
  g.add_edge("D", "E");
  g.add_edge("D", "F");
  g.add_edge("G", "C");
  g.add_vertex("H");
  // The following edge creates a cycle.
  // g.add_edge("D", "A");

  std::cout << g << std::endl;

  // A simple DFS walk.
  std::cout << "DFS: ";
  g.dfs_walk();

  // Topological sort.
  Graph::vertex_list_t topo;
  g.topo_sort(topo);
  std::cout << "Topo-sort: ";
  for (const auto &v : topo)
    std::cout << v << " ";
  std::cout << std::endl;

  return 0;
}
