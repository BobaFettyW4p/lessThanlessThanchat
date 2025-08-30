#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class Session;

class Hub : public std::enable_shared_from_this<Hub> {
  using WeakSess = std::weak_ptr<Session>;
  using SendCallback = std::function<void(const std::vector<std::byte> &)>;
  std::mutex m_;
  std::unordered_map<std::string, std::vector<std::pair<WeakSess, SendCallback>>> rooms_;

public:
  void join(const std::string &room, std::shared_ptr<Session> s, SendCallback send_cb) {
    std::scoped_lock lk(m_);
    auto &vec = rooms_[room];
    vec.erase(std::remove_if(vec.begin(), vec.end(), [](auto &p) { return p.first.expired(); }),
              vec.end());
    vec.emplace_back(s, send_cb);
  }

  template <class Frame>
  void broadcast(const std::string &room, const Frame &f) {
    std::vector<SendCallback> targets;
    {
      std::scoped_lock lk(m_);
      auto it = rooms_.find(room);
      if (it == rooms_.end())
        return;
      for (auto &p : it->second) {
        if (!p.first.expired()) {
          targets.push_back(p.second);
        }
      }
    }
    auto frame_data = make_frame(f);
    for (auto &cb : targets)
      cb(frame_data);
  }
};
