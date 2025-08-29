#include "session.hpp"
#include "hub.hpp"

void Session::start() {
  // Register with hub using a callback
  if (auto h = hub_.lock()) {
    h->join(room_, shared_from_this(),
            [this](const std::vector<std::byte> &data) { send_raw(data); });
  }
  async_read_header();
}

void Session::async_read_header() {
  auto self = shared_from_this();
  asio::async_read(sock_, asio::buffer(hdrbuf_),
                   [this, self](std::error_code ec, std::size_t) {
                     if (ec)
                       return;
                     FrameHeader h = parse_header(
                         std::span<const std::byte, 8>(hdrbuf_.data(), 8));
                     auto len = header_len(h);
                     payload_.resize(len);
                     async_read_body(len, header_type(h));
                   });
}

void Session::async_read_body(uint32_t len, uint16_t type) {
  auto self = shared_from_this();
  asio::async_read(sock_, asio::buffer(payload_.data(), len),
                   [this, self, type](std::error_code ec, std::size_t) {
                     if (ec)
                       return;
                     try {
                       if (type == ChatLine::type_id) {
                         ChatLine msg = from_bytes<ChatLine>(
                             std::span<const std::byte>(payload_.data(),
                                                        payload_.size()));
                         // Basic broadcast to room (room_, user_ ignored for now;
                         // agent can enhance)
                         if (auto h = hub_.lock()) {
                           h->broadcast(msg.room, msg);
                         }
                       }
                     } catch (...) {
                       // swallow for scaffold
                     }
                     async_read_header();
                   });
}

void Session::send_raw(const std::vector<std::byte> &data) {
  auto self = shared_from_this();
  asio::async_write(sock_, asio::buffer(data),
                    [self](std::error_code, std::size_t) {});
}