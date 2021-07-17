//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#include <iostream>

// Dumps an array to cout.
void print_arr(int arr[], int N) {
  for (int x = 0; x < N; ++x)
    std::cout << arr[x] << " ";
  std::cout << std::endl;
}

// Vanilla insertion sort.
void insertion_sort(int arr[], int N) {
  for (int i = 1; i < N; ++i) {
    int cur = arr[i];
    int j = i - 1;
    // Shift elements to right till exact
    // place to insert is found.
    for (; j >= 0 && arr[j]; --j) {
      arr[j + 1] = arr[j];
    }
    arr[j + 1] = cur;
  }
}

// Insertion sort employing binary search to find the
// inserion position.
void binary_insertion_sort(int arr[], int N) {
  for (int i = 1; i < N; ++i) {
    int cur = arr[i];
    // Use binary search to find insertion position
    // in O(log(n)) time.
    int l = 0, h = i - 1;
    while (l <= h) {
      int m = (l + h) / 2;
      if (cur < arr[m])
        h = m - 1;
      else
        l = m + 1;
    }
    // Still need O(n) shifts. :(
    for (int j = i - 1; j >= l; --j) {
      arr[j + 1] = arr[j];
    }
    arr[l] = cur;
  }
}

int main() {
  // int arr[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
  // int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  int arr[] = {5, 2, 7, 1, 3, 6, 4, 9, 8};
  int N = sizeof(arr) / sizeof(arr[0]);

  binary_insertion_sort(arr, N);

  print_arr(arr, N);

  return 0;
}
