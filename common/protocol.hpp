#pragma once

#include <vector>
#include <cstdint>
#include <span>
#include <asio.hpp>
#include "serialize.hpp"
#include "messages.hpp"
#include "net_endian.hpp"

struct FrameHeader {
  uint32_t len_be;   // payload length in bytes (big-endian)
  uint16_t type_be;  // message type id (big-endian)
  uint16_t flags_be; // reserved
};

template <Serializable T>
std::vector<std::byte> make_frame(const T &msg) {
  auto payload = to_bytes(msg);
  FrameHeader h{.len_be = to_be32(static_cast<uint32_t>(payload.size())),
                .type_be = to_be16(static_cast<uint16_t>(T::type_id)),
                .flags_be = to_be16(static_cast<uint16_t>(0))};
  std::vector<std::byte> buf;
  buf.reserve(sizeof(FrameHeader) + payload.size());
  buf.insert(buf.end(), reinterpret_cast<const std::byte *>(&h),
             reinterpret_cast<const std::byte *>(&h) + sizeof(FrameHeader));
  buf.insert(buf.end(), payload.begin(), payload.end());
  return buf;
}

// Small helpers to parse a header from 8 bytes
inline FrameHeader parse_header(std::span<const std::byte, 8> s) {
  FrameHeader h{};
  std::memcpy(&h, s.data(), 8);
  return h;
}

inline uint32_t header_len(const FrameHeader &h) {
  return from_be32(h.len_be);
}
inline uint16_t header_type(const FrameHeader &h) {
  return from_be16(h.type_be);
}
