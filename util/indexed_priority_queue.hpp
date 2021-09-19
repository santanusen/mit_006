//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#pragma once

#ifdef DEBUG_IPQ
#include <cassert>
#endif
#include <unordered_map>
#include <vector>

// A priority queue of elements of type T that supports efficient (O(lg n))
// update of priority of elements. The indexed priority queue is implemented
// using a heap. The template argument CMP dictates whether the heap is min
// or max. The default FANOUT of the heap is 2 (binary heap). Other fanout
// values may be used if required. The elements to index mapping is maintained
// using a hash map.
template <class T, class CMP = std::less<T>, class HASH = std::hash<T>,
          class PRED = std::equal_to<T>, size_t FANOUT = 2>
class indexed_priority_queue {

private:
  // Comparator: Should implement a function operator that takes two
  // elements of type T as argument and return a boolean result.
  CMP mCmp;

  // The heap is stored in a vector.
  typedef std::vector<T> HeapStoreT;
  HeapStoreT mHeapStore;

  // Hash-map of element to index.
  typedef std::unordered_map<T, size_t, HASH, PRED> IndexT;
  IndexT mIndex;

private:
  // Utility function to find the parent of an element.
  template <typename HS, typename HSITR>
  static HSITR get_parent_itr(HS &hs, HSITR itr) {
    if (hs.empty() || itr == hs.begin() || itr == hs.end())
      return hs.end();

    const auto idx = itr - hs.begin();
    const auto par_idx = (idx - 1) / FANOUT;

    return hs.begin() + par_idx;
  }

  // Utility function to find a child of an element.
  template <typename HS, typename HSITR>
  static HSITR get_child_itr(HS &hs, HSITR itr, size_t child_num) {
    if (hs.empty() || child_num >= FANOUT)
      return hs.end();

    const auto idx = itr - hs.begin();
    const auto ch_idx = FANOUT * idx + 1 + child_num;

    return (ch_idx < hs.size()) ? hs.begin() + ch_idx : hs.end();
  }

  // Returns iterator to the child with higher priority as per
  // the comparison function.
  typename HeapStoreT::iterator
  get_pri_child_itr(typename HeapStoreT::iterator itr) {
    auto pri_child_itr = mHeapStore.end();

    for (size_t child_num = 0; child_num < FANOUT; ++child_num) {
      auto child_itr = get_child_itr(mHeapStore, itr, child_num);
      if ((child_itr != mHeapStore.end()) &&
          (pri_child_itr == mHeapStore.end() ||
           mCmp(*child_itr, *pri_child_itr))) {
        pri_child_itr = child_itr;
      }
    }

    return pri_child_itr;
  }

  // Swap the element in the two argument iteratot positions.
  // Also update the index map accordingly.
  void element_swap(typename HeapStoreT::iterator itr1,
                    typename HeapStoreT::iterator itr2) {
    std::swap(mIndex[*itr1], mIndex[*itr2]);
    std::swap(*itr1, *itr2);
  }

  // Fix disorder at iterator itr if the priority of the element
  // at itr is lower than any of its children by swaping the
  // element with the highest priority child. Recursively
  // fix any disorder at the highest priority child index after swap.
  void heapify_down(typename HeapStoreT::iterator itr) {
    if (itr == mHeapStore.end())
      return;

    auto citr = get_pri_child_itr(itr);

    if (citr != mHeapStore.end() && !mCmp(*itr, *citr)) {
      element_swap(itr, citr);
      heapify_down(citr);
    }
  }

  // Fix disorder at iterator itr if the priority of the element
  // at itr is higher than its parent by swapping the element with the parent.
  // Recursively fix any disorder at the parent after swap.
  void heapify_up(typename HeapStoreT::iterator itr) {
    if (itr == mHeapStore.end())
      return;

    auto pitr = get_parent_itr(mHeapStore, itr);

    if (pitr != mHeapStore.end() && mCmp(*itr, *pitr)) {
      element_swap(itr, pitr);
      heapify_up(pitr);
    }
  }

#ifdef DEBUG_IPQ
  // Sanity checking utilities. Called after making modifications to
  // the internal data-structures (e.g. through push, pop and update).

  // Recursively check that the heap property is maintained at each element.
  void heap_sanity_check(typename HeapStoreT::const_iterator itr) const {
    for (size_t child_num = 0; child_num < FANOUT; ++child_num) {
      auto child_itr = get_child_itr(mHeapStore, itr, child_num);
      if (child_itr != mHeapStore.end()) {
        assert(!mCmp(*child_itr, *itr));
        heap_sanity_check(child_itr);
      }
    }
  }

  // Check that the index map is consistent with the heap.
  void index_sanity_check() const {
    assert(mIndex.size() == mHeapStore.size());
    for (const auto &i : mIndex) {
      assert(i.second < mHeapStore.size());
      assert(i.first == *(mHeapStore.begin() + i.second));
    }
  }

  // Called after modifying the internal data-structures.
  void sanity_check() {
    heap_sanity_check(mHeapStore.begin());
    index_sanity_check();
  }
#else
#define sanity_check()
#endif

public:
  // Returns the top-priority element.
  const T &top() const { return *(mHeapStore.begin()); }

  bool empty() const { return mHeapStore.empty(); }

  size_t size() const { return mHeapStore.size(); }

  // Swap the top element with the last elemet.
  // Erase the last element. Fix the disorder at top.
  void pop() {
    if (mHeapStore.empty())
      return;

    auto fitr = mHeapStore.begin();
    auto litr = mHeapStore.begin() + (mHeapStore.size() - 1);
    element_swap(fitr, litr);
    mIndex.erase(*litr);
    mHeapStore.erase(litr);
    heapify_down(mHeapStore.begin());
    sanity_check();
  }

  // Append the argument element at the end of the heap. Recursively check if it
  // violates the heap property at parent. Swap with the element with its parent
  // if it does.
  void push(const T &elem) {
    auto itr = mIndex.find(elem);
    if (itr != mIndex.end()) {
      update(itr->first, elem);
    } else {
      mHeapStore.push_back(elem);
      mIndex[elem] = mHeapStore.size() - 1;
      heapify_up(mHeapStore.begin() + (mHeapStore.size() - 1));
    }
    sanity_check();
  }

  // Update the element 'from' with 'to'. Recursively go up or down fixing
  // heap property violations.
  bool update(const T &from, const T &to) {
    auto itr = mIndex.find(from);
    if (itr == mIndex.end())
      return false;

    auto idx = itr->second;
    mHeapStore[idx] = to;
    mIndex.erase(from);
    mIndex[to] = idx;

    if (mCmp(from, to))
      heapify_down(mHeapStore.begin() + idx);
    else
      heapify_up(mHeapStore.begin() + idx);

    sanity_check();
    return true;
  }

  const T &find(const T &elem, const T &not_found) const {
    auto itr = mIndex.find(elem);
    return (itr == mIndex.end()) ? not_found : itr->first;
  }
};
