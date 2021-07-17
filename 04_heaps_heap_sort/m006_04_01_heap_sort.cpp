//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>
#include <vector>

#define INVALID -1
#define ROOT(idx) ((idx - 1) / 2)
#define LEFT(idx) (2 * idx + 1)
#define RIGHT(idx) (2 * idx + 2)

struct heap_store {
  std::vector<int> v;
  int len;
};

// Returns index of the child with higher priority as per
// the comparison function cmp.
template <typename CMP>
int get_pri_child_idx(int idx, const heap_store &h, CMP cmp) {
  int child_idx = INVALID;
  int ri = RIGHT(idx);
  if (ri < h.len) {
    child_idx = ri;
  }

  int li = LEFT(idx);
  if (li < h.len) {
    if (child_idx == INVALID || cmp(h.v[li], h.v[child_idx])) {
      child_idx = li;
    }
  }

  return child_idx;
}

// Fix disorder at index idx if the priority of the element
// at idx is lower than any of its children by swaping the
// element with the higher priority child. Recursively
// fix any disorder at the higher priority child index after swap.
template <typename CMP> void heapify(int idx, heap_store &h, CMP cmp) {
  if (idx >= h.len)
    return;
  auto ci = get_pri_child_idx(idx, h, cmp);

  if (ci != INVALID && !cmp(h.v[idx], h.v[ci])) {
    auto tmp = h.v[idx];
    h.v[idx] = h.v[ci];
    h.v[ci] = tmp;
    heapify(ci, h, cmp);
  }
}

// Convert an unordered array into a heap in a bottom up manner.
template <typename CMP> void build_heap(heap_store &h, CMP cmp) {
  // Leaves, index (len + 1) / 2 + 1 onwards, are already
  // trivial heaps.
  for (int idx = (h.len + 1) / 2; idx >= 0; --idx) {
    heapify(idx, h, cmp);
  }
}

// Swap the top element with the last elemet.
// Reduce the heap length by 1. Fix the disorder at top.
template <typename CMP> int extract_top(heap_store &h, CMP cmp) {
  int top = h.v[0];
  h.v[0] = h.v[h.len - 1];
  h.v[h.len - 1] = top;

  --h.len;

  heapify(0, h, cmp);
  return top;
}

// Keep extracting the top element (thus add it to the end of the heap array).
template <typename CMP> void heap_sort(heap_store &h, CMP cmp) {
  build_heap(h, cmp);
  while (h.len > 1) {
    extract_top(h, cmp);
  }
}

int main() {
  heap_store h = {{5, 2, 7, 1, 3, 6, 9, 4, 8, 10},
                  static_cast<int>(h.v.size())};
  heap_sort(h, std::greater<int>());
  for (const auto &x : h.v)
    std::cout << x << " ";
  std::cout << std::endl;

  return 0;
}
