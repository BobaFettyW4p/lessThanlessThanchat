#pragma once
#include <bit>
#include <cstdint>

// Portable big-endian helpers (avoid platform htonl/ntohl differences)
inline constexpr uint16_t to_be16(uint16_t x) {
  if constexpr (std::endian::native == std::endian::big)
    return x;
  return static_cast<uint16_t>((x << 8) | (x >> 8));
}
inline constexpr uint16_t from_be16(uint16_t x) {
  return to_be16(x);
}

inline constexpr uint32_t to_be32(uint32_t x) {
  if constexpr (std::endian::native == std::endian::big)
    return x;
  return ((x & 0x000000FFu) << 24) | ((x & 0x0000FF00u) << 8) |
         ((x & 0x00FF0000u) >> 8) | ((x & 0xFF000000u) >> 24);
}
inline constexpr uint32_t from_be32(uint32_t x) {
  return to_be32(x);
}
