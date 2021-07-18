//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <functional>
#include <iostream>
#include <string>

#define WDTH 8

// A vanilla Binary Search Tree.
// ADT operations insert, remove, find, successor and predecessor are
// implemented in iterative manner.
class BinarySearchTree {

public:
  struct Node {
    int key;
    Node *parent;
    Node *left;
    Node *right;

    Node(int v) : key(v), parent(nullptr), left(nullptr), right(nullptr) {}
  };

  BinarySearchTree() : root(nullptr) {}

  bool empty() const { return root == nullptr; }

  const Node *insert(int v) {
    if (!root) {
      root = new Node(v);
      return root;
    }

    // Reach to the position of insertion.
    // Keeping track of the parent node.
    Node *parent = nullptr, *cur = root;
    while (cur) {
      if (cur->key == v) {
        return cur;
      }
      parent = cur;
      cur = (v < cur->key) ? cur->left : cur->right;
    }

    // Stick the new node to the parent.
    Node *n = new Node(v);
    if (v < parent->key)
      parent->left = n;
    else
      parent->right = n;
    n->parent = parent;
    return n;
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

  void inorder_traverse(std::function<void(const Node *)> visit) const {
    inorder_traverse_subtree(root, visit);
  }

  const Node *successor(const Node *n) const {
    if (!n)
      return nullptr;

    if (n->right)
      return subtree_min(n->right);

      // No right subtree present. The nearest ancestor for which n is in the
      // right subtree is the successor.

#ifdef SUCCESSOR_USING_PARENT
    // Following the parent pointer.
    while (n->parent) {
      if (n->parent->left == n) {
        return n->parent;
      }
      n = n->parent;
    }

    return nullptr;
#else
    // Without using the parent pointer.
    Node *res = nullptr;
    Node *cur = root;
    while (cur && cur->key != n->key) {
      if (n->key < cur->key) {
        res = cur;
        cur = cur->left;
      } else
        cur = cur->right;
    }

    return (cur) ? res : nullptr;
#endif
  }

  const Node *predecessor(const Node *n) const {
    if (!n)
      return nullptr;

    if (n->left)
      return subtree_max(n->left);

    while (n->parent) {
      if (n->parent->right == n) {
        return n->parent;
      }
      n = n->parent;
    }

    return nullptr;
  }

  bool remove(int v) {
    // Reach the target node
    Node *par = nullptr, *cur = root;
    while (cur && cur->key != v) {
      par = cur;
      cur = (v < cur->key) ? cur->left : cur->right;
    }

    if (!cur)
      return false;

    // Case 3: Both children present
    if (cur->left && cur->right) {
      // Find successor: in the right subtree
      par = cur;
      auto suc = cur->right;
      while (suc->left) {
        par = suc;
        suc = suc->left;
      }
      // Swap key with successor
      cur->key = suc->key;
      suc->key = v;

      // Proceed to delete the successor
      cur = suc;
    }

    // Case 1,2: No children or Single child
    auto child = (cur->left) ? cur->left : cur->right;
    if (par) {
      if (cur == par->left) {
        par->left = child;
      } else {
        par->right = child;
      }
    } else { // Deletion of root node
      root = child;
    }
    if (child) {
      child->parent = par;
    }
    delete cur;

    return true;
  }

  void print() const { print_recurse(root, 0); }

private:
  void inorder_traverse_subtree(const Node *n,
                                std::function<void(const Node *)> visit) const {
    if (!n)
      return;
    inorder_traverse_subtree(n->left, visit);
    visit(n);
    inorder_traverse_subtree(n->right, visit);
  }

  const Node *subtree_max(const Node *n) const {
    if (!n)
      return nullptr;
    while (n->right)
      n = n->right;
    return n;
  }

  const Node *subtree_min(const Node *n) const {
    if (!n)
      return nullptr;
    while (n->left)
      n = n->left;
    return n;
  }

  void print_recurse(Node *n, int width) const {
    if (!n) {
      std::cout.width(width);
      std::cout << '~' << std::endl;
      return;
    }
    // Inorder print in Right-Root-Left order.
    print_recurse(n->right, width + WDTH);

    std::cout.width(width);
    std::cout << n->key << std::endl;

    print_recurse(n->left, width + WDTH);
  }

  Node *root;
};

void PrintKey(const BinarySearchTree::Node *n) { std::cout << n->key << " "; }

bool menu(BinarySearchTree &bst) {

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
  BinarySearchTree bst;

  while (menu(bst))
    ;

  return 0;
}
