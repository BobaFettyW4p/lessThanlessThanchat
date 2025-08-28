#pragma once
#include <type_traits>
#include <vector>
#include <span>
#include <cstddef>

// Declaration for from_bytes; specializations provided per message type.
template <class T>
T from_bytes(std::span<const std::byte> s);

template <class T>
concept Serializable = requires(const T& v) {
  { T::type_id } -> std::convertible_to<uint16_t>;
  { to_bytes(v) } -> std::same_as<std::vector<std::byte>>;
} && requires(std::span<const std::byte> s) {
  { from_bytes<T>(s) } -> std::same_as<T>;
};
