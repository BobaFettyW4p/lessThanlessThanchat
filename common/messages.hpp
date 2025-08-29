#pragma once
#include <string>
#include <vector>
#include <span>
#include <cstddef>
#include <cstring>
#include "serialize.hpp"

struct ChatLine {
  static constexpr uint16_t type_id = 1;
  std::string room;
  std::string user;
  std::string text;
};

inline std::vector<std::byte> to_bytes(const ChatLine &m) {
  auto put = [](std::vector<std::byte> &out, std::string_view s) {
    uint32_t n = static_cast<uint32_t>(s.size());
    // length is little platform-endian here; framing converts separately
    out.insert(out.end(), reinterpret_cast<std::byte *>(&n),
               reinterpret_cast<std::byte *>(&n) + 4);
    out.insert(out.end(), reinterpret_cast<const std::byte *>(s.data()),
               reinterpret_cast<const std::byte *>(s.data()) + s.size());
  };
  std::vector<std::byte> out;
  out.reserve(12 + m.room.size() + m.user.size() + m.text.size());
  put(out, m.room);
  put(out, m.user);
  put(out, m.text);
  return out;
}

// Generic declaration already in serialize.hpp; provide specialization for ChatLine
template <>
inline ChatLine from_bytes<ChatLine>(std::span<const std::byte> s) {
  auto get = [](std::string &out, std::span<const std::byte> &sp) {
    if (sp.size() < 4)
      throw std::runtime_error("bad frame");
    uint32_t n;
    std::memcpy(&n, sp.data(), 4);
    sp = sp.subspan(4);
    if (sp.size() < n)
      throw std::runtime_error("bad frame");
    out.assign(reinterpret_cast<const char *>(sp.data()), n);
    sp = sp.subspan(n);
  };
  ChatLine m;
  auto sp = s;
  get(m.room, sp);
  get(m.user, sp);
  get(m.text, sp);
  return m;
}
