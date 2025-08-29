#pragma once
#include <mutex>
#include <queue>
#include <condition_variable>
#include <optional>

template <class T>
class TsQueue {
  std::mutex m_;
  std::condition_variable cv_;
  std::queue<T> q_;
  bool closed_ = false;

public:
  void push(T v) {
    {
      std::scoped_lock lk(m_);
      if (closed_)
        return;
      q_.push(std::move(v));
    }
    cv_.notify_one();
  }
  std::optional<T> pop() {
    std::unique_lock lk(m_);
    cv_.wait(lk, [&] { return closed_ || !q_.empty(); });
    if (q_.empty())
      return std::nullopt;
    T v = std::move(q_.front());
    q_.pop();
    return v;
  }
  void close() {
    {
      std::scoped_lock lk(m_);
      closed_ = true;
    }
    cv_.notify_all();
  }
};
