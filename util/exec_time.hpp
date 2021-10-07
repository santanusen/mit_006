//
// Copyright 2021 Santanu Sen. All Rights Reserved.
//
// Licensed under the Apache License 2.0 (the "License"). You may not use
// this file except in compliance with the License. You can obtain a copy
// in the file LICENSE in the source distribution.
//

#pragma once
#include <chrono>
#include <utility>

// Utility to capture execution time of a function.
class exec_time {
private:
  double mDuration;

  // RAII capture execution time duration.
  struct TimeCapture {

    exec_time &mExect;

    typedef decltype(std::chrono::high_resolution_clock::now()) time_point_t;
    const time_point_t mTStart;

    TimeCapture(exec_time &et)
        : mExect(et), mTStart(std::chrono::high_resolution_clock::now()) {}

    ~TimeCapture() {
            const time_point_t tEnd = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> ms_double = tEnd - mTStart;
            mExect.mDuration = ms_double.count();
    }
  };

public:
  exec_time() : mDuration(0.0) {}

  // Returns execution duration in milliseconds.
  double get() const { return mDuration; }

  // Function operator to execute a function.
  template <typename R, typename... ARGS>
  constexpr auto operator()(R (*func)(ARGS...), ARGS ... args)
      -> decltype(func(args...)) {
              TimeCapture tc(*this);
    return func(std::forward<ARGS>(args)...);
  }
};

