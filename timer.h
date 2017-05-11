#pragma once

#include <chrono>
#include <iostream>

class Timer {
 private:
  std::chrono::high_resolution_clock::time_point start_;
  std::chrono::high_resolution_clock::time_point end_;

 public:
  Timer() {}
  void Start() { start_ = std::chrono::high_resolution_clock::now(); }
  void End() { end_ = std::chrono::high_resolution_clock::now(); }
  void Print() {
    std::cout << "Time: "
              << std::chrono::duration_cast<std::chrono::microseconds>(end_ -
                                                                       start_)
                     .count()
              << " ms" << std::endl;
  }
};
