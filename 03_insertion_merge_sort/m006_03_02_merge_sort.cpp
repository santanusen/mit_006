//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>

// Merge routine: merges two sorted arrays A and B into a third
// sorted array C.
void merge(int *A, int m, int *B, int n, int *C) {
  int i = 0, j = 0, k = 0;
  while (i < m && j < n) {
    if (A[i] < B[j])
      C[k++] = A[i++];
    else
      C[k++] = B[j++];
  }

  while (i < m)
    C[k++] = A[i++];
  while (j < n)
    C[k++] = B[j++];
}

// Iterative merge sort using 2-way merge.
void two_way_merge_sort(int *arr, int n) {
  int aux[n];
  int *cur = arr, *nxt = aux;

  // Start with merging sub-arrays of size 1.
  // At each iteration double the size of input sub-arrays.
  // The array used as the input array at one iteration
  // becomes the output array in the next and vice versa.
  for (int step = 1; step < n; step *= 2) {
    for (int i = 0; i < n; i += 2 * step) {
      merge(cur + i, ((i + step < n) ? step : n - i),                   // A, m
            cur + i + step, ((i + 2 * step < n) ? step : n - i - step), // B, n
            nxt + i);                                                   // C
    }

    // Swap the current and next arrays.
    int *tmp = cur;
    cur = nxt;
    nxt = tmp;
  }

  if (cur != arr) {
    for (int i = 0; i < n; ++i)
      arr[i] = cur[i];
  }
}

int main() {

  int arr[] = {90, 20, 10, 30, 60, 50};
  int l = sizeof(arr) / sizeof(arr[0]);

  two_way_merge_sort(arr, l);
  for (auto &i : arr)
    std::cout << i << " ";
  std::cout << std::endl;

  return 0;
}
