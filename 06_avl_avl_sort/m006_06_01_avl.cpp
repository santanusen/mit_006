//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <functional>
#include <iostream>

#define WDTH 16

// AVL Tree, a height balanced BST.
// ADT operations insert and remove are implemented recursively.
class AVLTree {

public:
  struct Node {
    int key;
    int height;
    Node *parent;
    Node *left;
    Node *right;

    Node(int v)
        : key(v), height(0), parent(nullptr), left(nullptr), right(nullptr) {}
  };

  AVLTree() : root(nullptr) {}

  bool empty() const { return root == nullptr; }

  void print() const { print_recurse(root, 0); }

  void insert(int v) {
    root = insert_recurse(root, v);
    // Root node may change due to rebalancing.
    root->parent = nullptr;
  }

  const Node *find(int v) const {
    const Node *n = root;
    while (n) {
      if (n->key == v)
        return n;
      n = (v < n->key) ? n->left : n->right;
    }

    return nullptr;
  }

  Node *successor(const Node *n) const {
    if (!n)
      return nullptr;

    if (n->right)
      return subtree_min(n->right);

    // The nearest ancestor for which n is in the left
    // subtree is the successor.
    Node *res = nullptr;
    Node *cur = root;
    while (cur && cur->key != n->key) {
      if (n->key < cur->key) {
        res = cur;
        // Remember the last left branch.
        cur = cur->left;
      } else
        cur = cur->right;
    }

    return (cur) ? res : nullptr;
  }

  const Node *predecessor(const Node *n) const {
    if (!n)
      return nullptr;

    if (n->left)
      return subtree_max(n->left);

    // No right subtree present. The nearest ancestor for which n is in the
    // right subtree is the successor.
    Node *res = nullptr;
    Node *cur = root;
    while (cur && cur->key != n->key) {
      if (n->key > cur->key) {
        res = cur;
        // Remember the last right branch.
        cur = cur->right;
      } else
        cur = cur->left;
    }

    return (cur) ? res : nullptr;
  }

  void remove(int v) {
    root = remove_recurse(root, v);
    // Root node may change due to root node deletion or rebalancing.
    if (root)
      root->parent = nullptr;
  }

  void inorder_traverse(std::function<void(const Node *)> visit) const {
    inorder_traverse_subtree(root, visit);
  }

  // Test code to check parent link sanity after modification.
  bool check_parent_links() const {
    return check_parent_links_recurse(root, nullptr);
  }

private:
  void inorder_traverse_subtree(const Node *n,
                                std::function<void(const Node *)> visit) const {
    if (!n)
      return;
    inorder_traverse_subtree(n->left, visit);
    visit(n);
    inorder_traverse_subtree(n->right, visit);
  }

  static Node *subtree_max(Node *n) {
    if (!n)
      return nullptr;
    while (n->right)
      n = n->right;
    return n;
  }

  static Node *subtree_min(Node *n) {
    if (!n)
      return nullptr;
    while (n->left)
      n = n->left;
    return n;
  }

  // Recursively check if the parent link of a node is consistent.
  static bool check_parent_links_recurse(const Node *n, const Node *par) {
    if (!n)
      return true;
    if (n->parent != par) {
      std::cout << "Parant link mismatch: " << n->key << std::endl;
      return false;
    }
    return (check_parent_links_recurse(n->left, n) &&
            check_parent_links_recurse(n->right, n));
  }

  static inline int get_node_height(const Node *n) {
    // NIL nodes have height of -1, this simplifies height calculation.
    return (n) ? n->height : -1;
  }

  // Difference between height of the right and the left child.
  static inline int get_node_height_diff(const Node *n) {
    return (n) ? (get_node_height(n->right) - get_node_height(n->left)) : 0;
  }

  // Recalculate height as 1 + max(left child height, right child height).
  // It is fine to have a nullptr as child.
  static inline void adjust_height(Node *n) {
    if (n) {
      n->height =
          std::max(get_node_height(n->right), get_node_height(n->left)) + 1;
    }
  }

  // Left rotate subtree rooted at x and return new root of the subtree after
  // rotation.
  Node *left_rotate(Node *x) {
    Node *y = x->right;
    Node *B = y->left;

    x->right = B;
    if (B)
      B->parent = x;
    y->left = x;
    x->parent = y;

    // Adjust heights from bottom to top
    adjust_height(x);
    adjust_height(y);

    // y is the new parent
    return y;
  }

  // Right rotate subtree rooted at x and return new root of the subtree after
  // rotation.
  Node *right_rotate(Node *x) {
    Node *y = x->left;
    Node *B = y->right;

    x->left = B;
    if (B)
      B->parent = x;
    y->right = x;
    x->parent = y;

    // Adjust heights from bottom to top
    adjust_height(x);
    adjust_height(y);

    // y is the new parent
    return y;
  }

  Node *fix_height_imbalance(Node *x) {
    // Fix height imbalance
    int hdiff = get_node_height_diff(x);
    if (hdiff > 1) { // Right imbalance
      int rhdiff = get_node_height_diff(x->right);
      if (rhdiff < 0) { // Right child left heavy; fix it first.
        x->right = right_rotate(x->right);
      }
      // Fix right imbalance by left rotation.
      return left_rotate(x);

    } else if (hdiff < -1) { // Left imbalance
      int lhdiff = get_node_height_diff(x->left);
      if (lhdiff > 0) { // Left child right heavy; fix it first.
        x->left = left_rotate(x->left);
      }
      // Fix left imbalance by right rotation.
      return right_rotate(x);
    }

    return x;
  }

  Node *insert_recurse(Node *n, int v) {
    if (!n) {
      return new Node(v);
    }

    if (v < n->key) {
      n->left = insert_recurse(n->left, v);
      n->left->parent = n;
    } else if (v > n->key) {
      n->right = insert_recurse(n->right, v);
      n->right->parent = n;
    }

    // Recalculate height
    adjust_height(n);

    // Fix height imbalance before returning;
    // thus ensuring imbalance fix in bottom-up manner.
    return fix_height_imbalance(n);
  }

  Node *remove_recurse(Node *n, int v) {
    if (!n)
      return nullptr;

    if (n->key == v) {
      if (n->left && n->right) { // Both children present.
        auto successor = subtree_min(n->right);
        n->key = successor->key;
        successor->key = v;
        n->right = remove_recurse(n->right, v);
        if (n->right)
          n->right->parent = n;
      } else { // 0 or 1 children.
        auto child = (n->left) ? n->left : n->right;
        delete n;
        return child;
      }
    } else if (v < n->key) {
      n->left = remove_recurse(n->left, v);
      if (n->left)
        n->left->parent = n;
    } else {
      n->right = remove_recurse(n->right, v);
      if (n->right)
        n->right->parent = n;
    }

    // Recalculate height
    adjust_height(n);

    // Fix height imbalance before returning;
    // thus ensuring imbalance fix in bottom-up manner.
    return fix_height_imbalance(n);
  }

  void print_recurse(Node *n, int width) const {
    if (!n) {
      std::cout.width(width);
      std::cout << '~' << std::endl;
      return;
    }
    print_recurse(n->right, width + WDTH);

    std::cout.width(width);
    std::cout << n->key << " (" << n->height << ")" << std::endl;

    print_recurse(n->left, width + WDTH);
  }

  Node *root;
};

void PrintKey(const AVLTree::Node *n) { std::cout << n->key << " "; }

bool menu(AVLTree &bst) {

  std::cout << std::endl
            << "I: Insert" << std::endl
            << "R: Remove" << std::endl
            << "P: Print" << std::endl
            << "T: Traverse Inorder" << std::endl
            << "F: Find" << std::endl
            << "S: Next Smaller" << std::endl
            << "L: Next Larger" << std::endl
            << "Q: Quit" << std::endl
            << std::endl
            << "Choice: ";
  std::string choice;
  std::cin >> choice;

  switch (choice[0]) {
  case 'i':
  case 'I': {
    int key;
    std::cout << "Key: ";
    std::cin >> key;
    bst.insert(key);
    bst.print();
  } break;

  case 'r':
  case 'R': {
    int key;
    std::cout << "Key: ";
    std::cin >> key;
    bst.remove(key);
    bst.print();
  } break;

  case 'p':
  case 'P': {
    bst.print();
  } break;

  case 't':
  case 'T': {
    bst.inorder_traverse(PrintKey);
    std::cout << std::endl;
  } break;

  case 'f':
  case 'F': {
    int key;
    std::cout << "Key: ";
    std::cin >> key;
    auto n = bst.find(key);
    if (!n)
      std::cout << "Not found." << std::endl;
    else
      std::cout << "Found: " << n->key << std::endl;
  } break;

  case 'l':
  case 'L':
  case 's':
  case 'S': {
    int key;
    std::cout << "Key: ";
    std::cin >> key;
    auto n = bst.find(key);
    if (!n) {
      std::cout << "Not found." << std::endl;
      break;
    }
    bool nxt_lrg = (choice[0] == 'l' || choice[0] == 'L');
    auto nn = (nxt_lrg) ? bst.successor(n) : bst.predecessor(n);
    if (!nn)
      std::cout << "Not present." << std::endl;
    else
      std::cout << "Answer: " << nn->key << std::endl;

  } break;

  case 'q':
  case 'Q': {
    return false;
  } break;

  default:
    std::cout << "Invalid choice: " << choice << std::endl;
    break;
  }

  return true;
}

int main() {
  AVLTree bst;

  while (menu(bst))
    ;

  return 0;
}
