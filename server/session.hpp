#pragma once
#include <asio.hpp>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "common/protocol.hpp"
#include "common/messages.hpp"

class Hub; // forward declaration

class Session : public std::enable_shared_from_this<Session> {
  asio::ip::tcp::socket sock_;
  std::array<std::byte, 8> hdrbuf_{};
  std::vector<std::byte> payload_;
  std::string room_ = "lobby";
  std::string user_ = "anon";
  std::weak_ptr<Hub> hub_;

public:
  explicit Session(asio::ip::tcp::socket s, std::shared_ptr<Hub> hub)
    : sock_(std::move(s)), hub_(hub) {}

  void start();
  void async_read_header();
  void async_read_body(uint32_t len, uint16_t type);
  void send_raw(const std::vector<std::byte>& data);

  template <class Frame>
  void send(const Frame& f) {
    auto buf = make_frame(f);
    send_raw(buf);
  }
};
