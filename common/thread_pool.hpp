#pragma once

#include "ts_queue.hpp"
#include <atomic>
#include <functional>
#include <thread>
#include <vector>

class ThreadPool {
  TsQueue<std::function<void()>> work_;
  std::vector<std::thread> threads_;
  std::atomic<bool> stop_{false};

public:
  explicit ThreadPool(size_t n = std::thread::hardware_concurrency()) {
    if (n == 0)
      n = 2;
    for (size_t i = 0; i < n; ++i) {
      threads_.emplace_back([this] {
        for (;;) {
          auto job = work_.pop();
          if (!job || stop_)
            break;
          (*job)();
        }
      });
    }
  }
  ~ThreadPool() {
    stop_ = true;
    work_.close();
    for (auto &t : threads_)
      if (t.joinable())
        t.join();
  }
  void submit(std::function<void()> f) {
    work_.push(std::move(f));
  }
};
